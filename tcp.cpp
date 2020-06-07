#include "synchronizer.h"
#include "tcp.h"


#if ENABLE_TLS
	#include "tls.h"
	#define _t( x )				x
#else
	#define _t( x )
#endif


// a SIGPIPE occurs when we send data to a peer that has closed the connection.
// in this case, we get a TCP RST and the kernel sends the SIGPIPE.  The default
// behavior on SIGPIPE reception is to close the process.  This is definitely not
// what we want, so ignore it.  In case you feel tempted to write a SIGPIPE handler,
// remember that tcp_serverSpawn is intended for a multithreaded environment so there
// is no way to tell which thread caused the exception.  Better to look for EPIPE
// from the write() call or similar.
ignore_signal s_ignore_sigpipe( SIGPIPE, false );


// This is preferrable to gethostbyname_r() since the underlying BIND functions which gethostbyname_r() calls are not necessarily reentrant
struct hostent *resolver::gethostbyname_alloc( const char *in_name ) {
	enum { k_max_host_aliases = k_1k, k_max_host_addresses = k_1k, k_host_address_size = 4 };

	char				   *buf;
	struct hostent		   *host = nil;
	unsigned long			n = sizeof(struct hostent);
	int						i, host_name_length, num_host_aliases_entries, num_host_address_list_entries, host_aliases_length[ k_max_host_aliases ];

	struct hostent_data : public hostent {
		char data[ 1 ];
	}					   *result;
	
	static balance::mutex	s_mutex;
	
	s_mutex.lock();

	try {
		if ( ! ( host = gethostbyname( in_name ) ) ) _throw_msg( h_errno, "%s", hstrerror( h_errno ) );

		// determine size of hostent
		
		// add in space for hostname (h_name) plus terminating null
		n += ( host_name_length = strlen( host->h_name ) + 1 );

		// for each alias, add space for the alias data plus a terminating null
		for ( i = 0, buf = host->h_aliases[ i ]; buf; ++i, buf = host->h_aliases[ i ] ) {
			n += ( host_aliases_length[ i ] = strlen( buf ) + 1 );
		}
		num_host_aliases_entries = i++;

		// add space for pointers to the alias data plus a null pointer
		n += i * sizeof(char *);
		
		// add space for the data of each host entry
		for ( i = 0, buf = host->h_addr_list[ i ]; buf; ++i, buf = host->h_addr_list[ i ] ) {
			n += k_host_address_size;
		}
		num_host_address_list_entries = i++;

		// add space for pointers to the host address data plus a trailing null pointer
		n += i * sizeof(void *);

		// allocate new hostent and populate with values from host
		if ( ( result = (hostent_data *) new char[ n ] ) ) {
			result->h_name = &result->data[ n = 0 ];
			bcopy( host->h_name, &result->data[ n ], host_name_length );
			n += host_name_length;

			result->h_aliases = (char **) &result->data[ n ];
			buf = (char *) ( (unsigned int) &result->data[ n ] ) + ( sizeof(char*) * ( num_host_aliases_entries + 1 ) );	// buf is start of h_aliases data area in result
			for ( i = 0; i < num_host_aliases_entries; buf = (char *)( ((unsigned long) buf) + host_aliases_length[i++] ), n += sizeof(char *) ) {
				*((char **) &result->data[ n ]) = buf;							// populate result->h_aliases[ i ]
				bcopy( host->h_aliases[ i ], buf, host_aliases_length[ i ] );	// populate *result->h_aliases[ i ]
			}
			*((char **) &result->data[ n ]) = 0;								// terminate result->h_aliases[]
			n = (unsigned long) buf - (unsigned long) &result->data[0];

			result->h_addrtype = host->h_addrtype;
			result->h_length = host->h_length;

			result->h_addr_list = (char **) &result->data[n];
			buf = (char *)( (unsigned int) &result->data[n] ) + ( sizeof(char*) * ( num_host_address_list_entries + 1 ) ); // buf is start of h_addr_list data area in result
			for ( i = 0; i < num_host_address_list_entries; buf = (char *) ( ((unsigned long) buf) + k_host_address_size ), ++i, n += sizeof(void *) ) {
				*((char **) &result->data[ n ]) = buf;							// populate result->h_addr_list[ i ]
				bcopy( host->h_addr_list[ i ], buf, k_host_address_size );		// populate *result->h_addr_list[ i ]
			}
			*((char **) &result->data[ n ]) = 0;								// terminate result->h_addr_list[]
		}
	} catch ( ... ) {
		s_mutex.unlock();
		throw;
	}

	s_mutex.unlock();

	return result;
}


void resolver::gethostbyname_free( struct hostent *in_hostent ) {
	delete[] in_hostent;
}


#pragma mark -


tcp_socket::tcp_socket() {
_t(	m_tls = nil );

	m_local_port = 0;
	m_remote_port = 0;
	m_local_ip_address = 0;
	m_remote_ip_address = 0;

	m_socket = k_socket_closed;
	m_istream = m_ostream = nil;

	memset( m_local_ip_address_string, 0, sizeof(m_local_ip_address_string) );
	memset( m_remote_ip_address_string, 0, sizeof(m_remote_ip_address_string) );
	
	m_read_ok = m_write_ok = m_listening = m_local_info_is_set = m_reserved = false;
}


void tcp_socket::accept( __u16 in_port, time_t in_timeout_seconds, in_addr_t in_interface_ip_address ) {
	int						s;
	struct sockaddr_in		cliaddr;
	socklen_t				clilen = sizeof(cliaddr);
	
	if ( ! socket_is_listening() ) listen( in_port, 1, in_interface_ip_address );
	
	wait_for_connection( &s, &cliaddr, &clilen, in_timeout_seconds );

	close();
	m_socket = s;

	set_readable();		// we're now connected
	set_writable();
	set_local_info();
	set_remote_port( ntohs( cliaddr.sin_port ) );
	
	if ( ! inet_ntop( AF_INET, &cliaddr.sin_addr, m_remote_ip_address_string, sizeof(m_remote_ip_address_string) ) ) _throw( err_remote_address_unavailable );

	console( "Connection from %s:%d accepted", m_remote_ip_address_string, get_remote_port() );
}


void tcp_socket::listen( __u16 in_port, int in_listen_queue_size, in_addr_t in_interface_ip_address ) {
	if ( socket_is_connected() ) _throw( err_already_connected );

	int					i, sock = k_socket_closed;
	struct sockaddr_in	servaddr;
	socklen_t			addrlen = sizeof(servaddr);

	_throw_errno_if( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) == k_socket_closed );

	try {
		_throw_errno_if( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &( i = 1 ), sizeof(i) ) == -1 );

		memset( &servaddr, 0, sizeof(servaddr) );
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl( in_interface_ip_address );
		servaddr.sin_port = htons( in_port );

		_throw_errno_if( bind( sock, reinterpret_cast<struct sockaddr *>(&servaddr), sizeof(servaddr) ) == -1 );
		_throw_errno_if( ::listen( sock, in_listen_queue_size ) == -1 );
		_throw_errno_if( getsockname( sock, reinterpret_cast<struct sockaddr *>(&servaddr), &addrlen ) == -1 );

		m_local_port = ntohs( servaddr.sin_port );
		m_socket = sock;

		set_listening();
	} catch ( ... ) {
		::close( sock );
		throw;
	}
}


void tcp_socket::connect( const char *in_hostname, __u16 in_port ) {
	if ( ! ( in_hostname && in_port ) ) _throw( err_bad_parameter );

	struct sockaddr_in			con;
	struct hostent			   *host;
	int							i;

	host = resolver::gethostbyname_alloc( in_hostname );

	try {
		try {
			for ( i = 0;; ) {
				close();
				_throw_errno_if( ( m_socket = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 );
				
				try {
					con.sin_family = host->h_addrtype;
					con.sin_port = htons( in_port );
					m_remote_port = in_port;

					bcopy( host->h_addr_list[ i ], &con.sin_addr.s_addr, sizeof(con.sin_addr.s_addr) );
					m_remote_ip_address = con.sin_addr.s_addr;

					if ( ::connect( m_socket, reinterpret_cast<struct sockaddr *>(&con), sizeof(con) ) == -1 ) _throw_quiet( errno );

					break;
				} catch ( ... ) {
					if ( ! host->h_addr_list[ ++i ] ) _throw_errno();
				}
			}

			set_local_info();

			if ( ! inet_ntop( AF_INET, &con.sin_addr, m_remote_ip_address_string, sizeof(m_remote_ip_address_string) ) ) _throw( err_remote_address_unavailable );

			set_readable();
			set_writable();
		} catch ( ... ) {
			close();
			throw;
		}
	} catch ( ... ) {
		resolver::gethostbyname_free( host );
		throw;
	}

	resolver::gethostbyname_free( host );
}


void tcp_socket::flush_read() {
	char			   *buf;
	ssize_t				n;
	
	buf = new char[ k_128k ];
	
	_no_throw( do { n = read( buf, k_128k, 0 ); } while ( n ) );
	
	delete[] buf;
}


void tcp_socket::wait_for_connection( int *out_socket, struct sockaddr_in *out_client_address, socklen_t *io_address_length, time_t in_timeout_seconds ) {
	if ( ! ( out_socket && out_client_address && io_address_length ) ) _throw( err_bad_parameter );
	if ( ! socket_is_listening() ) _throw( err_not_listening );

	struct timeval		to;
	err_t				err;
	fd_set				rset;

	FD_ZERO( &rset );
	FD_SET( m_socket, &rset );

	*out_socket = k_socket_closed;

	to.tv_usec = 0;
	to.tv_sec = in_timeout_seconds;
	
	_throw_errno_if( ( err = select( m_socket + 1, &rset, nil, nil, in_timeout_seconds < 0 ? nil : &to ) ) == -1 );

	if ( ! err ) _throw( err_timeout );
	if ( ! FD_ISSET( m_socket, &rset ) ) _throw( err_select_failed );

	_throw_errno_if( ( *out_socket = ::accept( m_socket, reinterpret_cast<struct sockaddr *>(out_client_address), io_address_length ) ) == -1 );
}


ssize_t tcp_socket::wait_for_data( time_t in_timeout_seconds ) {
	ssize_t					result;
	fd_set					rset;
	struct timeval			to;

	FD_ZERO( &rset );
	FD_SET( m_socket, &rset );
	
	to.tv_usec = 0;
	to.tv_sec = in_timeout_seconds;

	_throw_errno_if( ( result = select( m_socket + 1, &rset, nil, nil, in_timeout_seconds < 0 ? nil : &to ) ) == -1 );

	if ( ! result ) _throw( err_timeout );
	if ( ! FD_ISSET( m_socket, &rset ) ) _throw( err_select_failed );

	return data_is_available();
}


void tcp_socket::set_local_info() {
	if ( m_local_info_is_set ) return;
	if ( m_socket == k_socket_closed ) _throw( err_socket_closed );

	struct sockaddr_in	info;
	socklen_t			info_length = sizeof(info);

	_throw_errno_if( getsockname( m_socket, reinterpret_cast<struct sockaddr *>(&info), &info_length ) == -1 );

	m_local_port = ntohs( info.sin_port );
	m_local_ip_address = info.sin_addr.s_addr;

	if ( ! inet_ntop( AF_INET, &info.sin_addr, m_local_ip_address_string, sizeof(m_local_ip_address_string) ) ) _throw( err_local_address_unavailable );

	_throw_errno_if( ! ( m_istream = fdopen( m_socket, "r" ) ) );

	try {
		_throw_errno_if( ! ( m_ostream = fdopen( m_socket, "w" ) ) );
	} catch ( ... ) {
		fclose( m_istream ); m_istream = nil;
		throw;
	}

#if defined( linux )
	setlinebuf( m_istream );
	setlinebuf( m_ostream );
#else
	try {
		if ( setlinebuf( m_istream ) == EOF ) _throw( err_could_not_line_buffer_stream );
		if ( setlinebuf( m_ostream ) == EOF ) _throw( err_could_not_line_buffer_stream );
	} catch ( ... ) {
		fclose( m_istream ); m_istream = nil;
		fclose( m_ostream ); m_ostream = nil;
		throw;
	}
#endif

	m_local_info_is_set = true;
}


void tcp_socket::disconnect( int in_how ) {
	switch ( in_how ) {
		case k_disallow_read:		if ( ! socket_is_readable() ) return; else m_read_ok = false;					break;
		case k_disallow_write:		if ( ! socket_is_writable() ) return; else m_write_ok = false;					break;
		case k_disallow_read_write:	if ( ! socket_is_connected() ) return; else m_read_ok = m_write_ok = false;		break;

		default:					debug_statement( _throw( err_bad_parameter ) );									break;
	}
	
_t(	if ( m_tls ) make_insecure() );

	_throw_errno_if( shutdown( m_socket, in_how ) == -1 && errno != ENOTCONN );
}


ssize_t tcp_socket::read( void *out_buffer, size_t in_length, time_t in_timeout_seconds ) {
	if ( ! socket_is_readable() ) _throw( err_not_connected );
	if ( ! out_buffer ) _throw( err_bad_parameter );

	ssize_t				result;

#if ENABLE_TLS
	if ( m_tls ) {
		try {
			result = m_tls->read( out_buffer, in_length );
		} catch ( ... ) {
			make_insecure();
			throw;
		}
	} else {
#endif
		wait_for_data( in_timeout_seconds );

		result = ::read( m_socket, out_buffer, in_length );
		
		if ( result == -1 ) {
			if ( ! ( errno == ECONNRESET || errno == ENOTCONN ) ) {
				m_read_ok = false;
				_throw_errno();
			}
			_throw( err_not_connected );
		} else if ( ! result ) {
			_throw( err_not_connected );
		}
#if ENABLE_TLS
	}
#endif
	
	return result;
}


ssize_t tcp_socket::write( const void *in_buffer, size_t in_length ) {	
	if ( ! in_buffer ) _throw( err_bad_parameter );

	ssize_t result = err_not_connected;

	if ( socket_is_writable() ) {
#if ENABLE_TLS
		if ( m_tls ) {
			try {
				result = m_tls->write( in_buffer, in_length );
			} catch ( ... ) {
				make_insecure();
				throw;
			}
		} else {
#endif
			if ( ( result = ::write( m_socket, in_buffer, in_length ) ) == -1 ) {
				if ( errno == EPIPE ) m_write_ok = false;
				_throw( err_not_connected );
			}
#if ENABLE_TLS
		}
#endif
	}

	return result;
}


void tcp_socket::close() {
	_no_throw( disconnect( k_disallow_read_write ) );

	set_listening( false );
	set_local_info_is_set( false );

	m_local_port = 0;
	m_remote_port = 0;
	m_local_ip_address = 0;
	m_remote_ip_address = 0;

	memset( m_local_ip_address_string, 0, sizeof(m_local_ip_address_string) );
	memset( m_remote_ip_address_string, 0, sizeof(m_remote_ip_address_string) );

	if ( m_istream ) { fclose( m_istream ); m_istream = nil; }
	if ( m_ostream ) { fclose( m_ostream ); m_ostream = nil; }

	if ( m_socket != k_socket_closed ) {
		::close( m_socket );
		m_socket = k_socket_closed;
	}
}


ssize_t tcp_socket::data_is_available() {
	ssize_t			result;

#if ENABLE_TLS
	if ( m_tls ) result = m_tls->data_is_available();
	else {
#endif
		_throw_errno_if( ioctl( m_socket, FIONREAD, &result ) == -1 );
#if ENABLE_TLS
	}
#endif

	return result;
}


#if ENABLE_TLS
void tcp_socket::make_secure( bool in_secure_as_client ) {
	if ( m_tls ) return;
	
	m_tls = new tls( m_socket );

	try {
		in_secure_as_client ? m_tls->connect() : m_tls->accept();
	} catch ( ... ) {
		delete m_tls;
		m_tls = nil;
		throw;
	}

	debug_msg( "Connection to %s:%d secured", get_remote_hostname(), get_remote_port() );
}


void tcp_socket::make_insecure( bool in_wait_for_peer_shutdown ) {
	if ( m_tls ) {
		m_tls->shutdown( in_wait_for_peer_shutdown );
		debug_msg( "Connection to %s:%d now insecure", get_remote_hostname(), get_remote_port() );
		delete m_tls;
		m_tls = nil;
	}
}
#endif
