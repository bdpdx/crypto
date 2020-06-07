#ifndef __tcp_h__
#define __tcp_h__



#include "queue.h"


enum {
	k_disallow_read			,			// Posix.1g SHUT_RD
	k_disallow_write		,			// Posix.1g SHUT_WR
	k_disallow_read_write				// Posix.1g SHUT_RDWR
};

enum {
	k_no_port							=	0		,
	k_ephemeral_port					=	0		,
	k_default_listen_queue_size			=	64		,
	k_no_timeout						=	-1
};


#pragma mark struct resolver


struct resolver {

	// these methods are reentrant and, according to Stevens, safer
	// than using gethostbyname_r().
	static struct hostent *gethostbyname_alloc( const char *in_name );
	static void gethostbyname_free( struct hostent *in_hostent );

};


#pragma mark class tcp_socket


class tcp_socket {

	friend class tcp_server;

public:
	
	tcp_socket();
   ~tcp_socket() { close(); }

	int get_socket_fd() { return m_socket; }

	ssize_t	read( void *out_buffer, size_t in_length, time_t in_timeout_seconds = k_no_timeout );
	ssize_t write( const void *in_buffer, size_t in_length );

	void flush_read();

#if ENABLE_TLS
	// establishes SSL connection, switches to secure state
	// if in_secure_as_client == true, a TLS connect is initiated.  otherwise a TLS accept is initiated.
	void make_secure( bool in_secure_as_client = true );
	// closes SSL connection (not socket connection), switches to insecure state
	void make_insecure( bool in_wait_for_peer_shutdown = false );	
#endif

	void close();
	void disconnect( int in_how = k_disallow_write );		// by default we allow client to keep sending data
	void connect( const char *in_hostname, __u16 in_port );
	void accept( __u16 in_port = k_ephemeral_port, time_t in_timeout = k_no_timeout, in_addr_t in_interface_ip_address = INADDR_ANY );
	void listen( __u16 in_port = k_ephemeral_port, int in_listen_queue_size = k_default_listen_queue_size, in_addr_t in_interface_ip_address = INADDR_ANY );
	void wait_for_connection( int *out_socket, struct sockaddr_in *out_client_address, socklen_t *io_address_length, time_t in_timeout_seconds = k_no_timeout );
	// effects a poll.  pass k_no_timeout to block indefinitely.
	// returns number of bytes available for reading.
	ssize_t wait_for_data( time_t in_timeout_seconds = 0 );

	bool socket_is_readable() { return m_read_ok; }
	bool socket_is_writable() { return m_write_ok; }
	bool socket_is_listening() { return m_listening; }
	bool socket_local_info_is_set() { return m_local_info_is_set; }
	bool socket_is_connected() { return ( socket_is_readable() || socket_is_writable() ); }

	ssize_t data_is_available();

	__u32 get_local_port() { return m_local_port; }
	__u32 get_remote_port() { return m_remote_port; }
	__u32 get_local_ip_address() { return m_local_ip_address; }
	__u32 get_remote_ip_address() { return m_remote_ip_address; }

	const char *get_local_hostname() { return get_local_ip_address_string(); }
	const char *get_remote_hostname() { return get_remote_ip_address_string(); }
	const char *get_local_ip_address_string() { return m_local_ip_address_string; }
	const char *get_remote_ip_address_string () { return m_remote_ip_address_string; }

	FILE			   *m_istream;
	FILE			   *m_ostream;

protected:

	void set_local_info();
	
	void set_local_port( __u16 in_port ) { m_local_port = in_port; }
	void set_remote_port( __u16 in_port ) { m_remote_port = in_port; }
	void set_local_ip_address( __u32 in_address ) { m_local_ip_address = in_address; }
	void set_remote_ip_address( __u32 in_address ) { m_remote_ip_address = in_address; }
	char *set_local_ip_address_string( char *in_address ) { return strncpy( m_local_ip_address_string, in_address, sizeof(m_local_ip_address_string) ); }
	char *set_remote_ip_address_string( char *in_address ) { return strncpy( m_remote_ip_address_string, in_address, sizeof(m_remote_ip_address_string) ); }

	void set_readable( bool in_readable = true ) { m_read_ok = in_readable; }
	void set_writable( bool in_writable = true ) { m_write_ok = in_writable; }
	void set_listening( bool in_listening = true ) { m_listening = in_listening; }
	void set_local_info_is_set( bool in_local_info_is_set = true ) { m_local_info_is_set = in_local_info_is_set; }

#if ENABLE_TLS
	class tls		   *m_tls;				// secure state information
#endif
	int					m_socket;			// socket descriptor returned by socket() or ::accept()

	__u32				m_read_ok			:	1;
	__u32				m_write_ok			:	1;
	__u32				m_listening			:	1;
	__u32				m_local_info_is_set	:	1;
	__u32				m_reserved			:	28;

private:

	__u16				m_local_port;
	__u16				m_remote_port;
	__u32				m_local_ip_address;
	__u32				m_remote_ip_address;
	char				m_local_ip_address_string[ INET_ADDRSTRLEN ];
	char				m_remote_ip_address_string[ INET_ADDRSTRLEN ];

};



#endif // __tcp_h__
