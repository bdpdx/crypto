#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "log.h"


static const char		   *s_log_levels[] = {
	"EMERGENCY",
	"ALERT",
	"CRITICAL",
	"ERROR",
	"WARNING",
	"NOTICE",
	"INFO",
	"DEBUG"
};

#define k_log_levels			( sizeof(s_log_levels) / sizeof(char *) )


#if ENABLE_GLOBAL_LOG
#ifdef APPLICATION_LOG_INIT_PARAMETERS
	balance::log		   *balance::__s_application_log = new balance::log( APPLICATION_LOG_INIT_PARAMETERS );
#else
	balance::log		   *balance::__s_application_log;
#endif
#endif


balance::log::log( const char *in_log_filename, const char *in_syslog_name, bool in_enable_stderr, bool in_enable_timestamps ) {
	m_file = nil;

	m_show_process_id = true;
	m_show_thread_id = true;
	
	m_file_log_level = LOG_DEBUG;

	_throw_if( pthread_mutex_init( &m_mutex, nil ) );

	if ( in_enable_stderr ) enable_stderr();
	if ( in_enable_timestamps ) enable_timestamps();
	if ( in_syslog_name ) open_syslog( in_syslog_name );
	if ( in_log_filename ) open_log_file( in_log_filename );
}


balance::log::~log() {
	close_syslog();
	close_log_file();
	
	pthread_mutex_destroy( &m_mutex );
}


void balance::log::message( int in_priority, const char *in_format, ... ) {
	va_list					arguments;
	
	va_start( arguments, in_format );
	vmessage( in_priority, nil, k_default_log_flags, in_format, arguments );
	va_end( arguments );
}


void balance::log::message( int in_priority, char **io_postpone, __u32 in_flags, const char *in_format, ... ) {
	va_list					arguments;
	
	va_start( arguments, in_format );
	vmessage( in_priority, io_postpone, in_flags, in_format, arguments );
	va_end( arguments );
}


void balance::log::vmessage( int in_priority, const char *in_format, va_list in_arguments ) {
	vmessage( in_priority, nil, k_default_log_flags, in_format, in_arguments );
}


void balance::log::vmessage( int in_priority, char **io_postpone, __u32 in_flags, const char *in_format, va_list in_arguments ) {
	char				   *b0, *b1, process_id[ 12 ], thread_id[ 12 ], timestamp[ 20 ];
	bool					finalize, free_io_postpone, suppress_newline;
	struct tm			   *local_time;
	time_t					now;

	finalize = in_flags & k_finalize;
	free_io_postpone = in_flags & k_free_io_postpone;
	suppress_newline = in_flags & k_suppress_newline;

	if ( ! ( finalize || io_postpone ) ) _throw( err_bad_parameter );
	if ( (__u32) in_priority >= k_log_levels ) _throw( err_bad_parameter );

	if ( vasprintf( &b0, in_format, in_arguments ) == -1 ) _throw( err_mem_full );

	_try {
		if ( io_postpone && *io_postpone ) {
			asprintf( &b1, "%s%s%s", *io_postpone, b0, finalize && ! suppress_newline ? "\n" : "" );
		} else {
			if ( m_show_process_id ) {
				sprintf( process_id, "%05u", getpid() );
			} else {
				*process_id = 0;
			}
			
			if ( m_show_thread_id ) {
				sprintf( thread_id, "0x%08x", (__u32) pthread_self() );
			} else {
				*thread_id = 0;
			}

			if ( m_timestamps_enabled ) {
				time( &now );
				local_time = localtime( &now );
				sprintf( timestamp, "%02d/%02d/%02d %02d:%02d:%02d", local_time->tm_mon + 1, local_time->tm_mday,
					local_time->tm_year % 100, local_time->tm_hour, local_time->tm_min, local_time->tm_sec );
			} else {
				*timestamp = 0;
			}

			if ( asprintf( &b1, "%s%s%s%s%s%s%s %s%s",
				timestamp, m_timestamps_enabled ? " " : "",
				process_id, m_show_process_id ? ( m_show_thread_id ? ":" : " " ) : "",
				thread_id, m_show_thread_id ? " " : "",
				s_log_levels[ in_priority ],
				b0,
				finalize && ! suppress_newline ? "\n" : "" ) == -1 )
			{
				_throw( err_mem_full );
			}
		}
		
		free( b0 );
		b0 = b1;
	} _catch
	
	if ( finalize ) {
		if ( free_io_postpone && io_postpone ) _free( *io_postpone );
	
		if ( m_syslog_enabled && in_priority <= m_syslog_level ) {
			if ( ! m_syslog_open ) open_syslog( s_application_name );
			syslog( in_priority, "%s", b0 );
		}
		
		try {
			lock();

			if ( m_stderr_enabled ) {
				fprintf( stderr, "%s", b0 );
				fflush( stderr );
			}
			
			if ( in_priority <= m_file_log_level ) {
				if ( m_file_enabled && m_file ) { fprintf( m_file, "%s", b0 ); fflush( m_file ); }
				if ( m_message_notification_enabled ) message_notification( b0 );
			}

			unlock();
		} _catch

		free( b0 );
	} else {
		if ( *io_postpone ) free( *io_postpone );
		*io_postpone = b0;
	}
	
	_return;
}


void balance::log::open_syslog( const char *in_name ) {
	if ( m_syslog_open ) close_syslog();

	openlog( in_name, LOG_CONS | LOG_PID, LOG_USER );

	m_syslog_open = true;
	enable_syslog();
}


void balance::log::close_syslog() {
	if ( m_syslog_open ) {
		closelog();
		m_syslog_open = false;
	}
}


void balance::log::open_log_file( const char *in_filename ) {
	close_log_file();

	_throw_errno_if( ! ( m_file = fopen( in_filename, "a" ) ) );

	setbuf( m_file, nil );
	enable_file();
}


void balance::log::close_log_file() {
	lock();
	disable_file();
	
	if ( m_file ) {
		fclose( m_file );
		m_file = nil;
	}
	unlock();
}


void balance::log::enable_syslog( char *in_level ) {
	if ( ! strcasecmp( in_level, "LOG_EMERG" ) ) m_syslog_level = LOG_EMERG;
	else if ( ! strcasecmp( in_level, "LOG_ALERT" ) ) m_syslog_level = LOG_ALERT;
	else if ( ! strcasecmp( in_level, "LOG_CRIT" ) ) m_syslog_level = LOG_CRIT;
	else if ( ! strcasecmp( in_level, "LOG_ERR" ) ) m_syslog_level = LOG_ERR;
	else if ( ! strcasecmp( in_level, "LOG_WARNING" ) ) m_syslog_level = LOG_WARNING;
	else if ( ! strcasecmp( in_level, "LOG_NOTICE" ) ) m_syslog_level = LOG_NOTICE;
	else if ( ! strcasecmp( in_level, "LOG_INFO" ) ) m_syslog_level = LOG_INFO;
	else if ( ! strcasecmp( in_level, "LOG_DEBUG" ) ) m_syslog_level = LOG_DEBUG;
	else _throw( err_bad_parameter );
	
	m_syslog_enabled = true;
}


void balance::log::set_file_log_level( const char *in_level ) {
	if ( ! strcasecmp( in_level, "LOG_EMERG" ) ) m_file_log_level = LOG_EMERG;
	else if ( ! strcasecmp( in_level, "LOG_ALERT" ) ) m_file_log_level = LOG_ALERT;
	else if ( ! strcasecmp( in_level, "LOG_CRIT" ) ) m_file_log_level = LOG_CRIT;
	else if ( ! strcasecmp( in_level, "LOG_ERR" ) ) m_file_log_level = LOG_ERR;
	else if ( ! strcasecmp( in_level, "LOG_WARNING" ) ) m_file_log_level = LOG_WARNING;
	else if ( ! strcasecmp( in_level, "LOG_NOTICE" ) ) m_file_log_level = LOG_NOTICE;
	else if ( ! strcasecmp( in_level, "LOG_INFO" ) ) m_file_log_level = LOG_INFO;
	else if ( ! strcasecmp( in_level, "LOG_DEBUG" ) ) m_file_log_level = LOG_DEBUG;
	else _throw( err_bad_parameter );
}


#pragma mark -


#if ENABLE_BUFFERED_LOG


balance::buffered_log::buffered_log( const char *in_log_filename, const char *in_syslog_name, bool in_enable_stderr, bool in_enable_timestamps ) : log_( in_log_filename, in_syslog_name, in_enable_stderr, in_enable_timestamps ) {
	m_buffer = nil;
	m_num_lines = 0;
	m_max_lines = -1;
	m_first_line = 0;
	
	if ( in_log_filename ) open_log_file( in_log_filename );

	enable_message_notification();
}


balance::buffered_log::~buffered_log() {
	clear();
}


void balance::buffered_log::clear() {
	__s32		i;
	
	lock();

	if ( m_buffer ) {
		for ( i = 0; i < m_num_lines; ++i ) free( m_buffer[ i ] );

		free( m_buffer );
		m_buffer = nil;
	}

	m_num_lines = 0;
	m_first_line = 0;
	
	unlock();
}


const char *balance::buffered_log::read_line() {
	const char			   *result;
	
	lock();
	result = m_num_lines ? m_buffer[ m_num_lines - 1 ] : nil;
	unlock();
	
	return result;
}


char *balance::buffered_log::buffer() {
	int						i, j, k, max_lines, n, o;
	char				   *result;

	lock();
	
	max_lines = m_max_lines == k_unlimited ? INT_MAX : m_max_lines;
	
	for ( i = 0, n = m_num_lines, o = 1; i < n; ++i ) {
		o += strlen( m_buffer[ ( m_first_line + i ) % max_lines ] );
	}

	result = (char *) malloc( o );

	for ( i = j = 0, n = m_num_lines; i < n; ++i, j += o ) {
		o = strlen( m_buffer[ k = ( m_first_line + i ) % max_lines ] );
		memcpy( result + j, m_buffer[ k ], o );
	}
	
	result[ j ] = '\0';
	
	unlock();

	return result;
}


void balance::buffered_log::set_max_lines( __s32 in_max_lines ) {
	if ( m_disk_buffered || in_max_lines == m_max_lines ) return;

	char		  **buffer;
	__s32			i, j, k;

	lock();
	
	if ( ! m_buffer || ( in_max_lines == k_unlimited || in_max_lines >= m_num_lines ) && ! m_first_line ) { m_max_lines = in_max_lines; return; }

	_assert( m_num_lines == m_max_lines || in_max_lines >= m_num_lines && m_first_line || in_max_lines < m_max_lines );

	if ( ! ( buffer = (char **) malloc( ( k = ( in_max_lines == k_unlimited ? m_num_lines : min( m_num_lines, in_max_lines ) ) ) * sizeof(char *) ) ) ) _throw( err_mem_full );

	j = m_num_lines > k ? ( m_first_line + ( m_num_lines - k ) ) % m_max_lines : m_first_line;

	for ( i = 0; i < k; ++i, j = ( j + 1 ) % m_max_lines ) buffer[ i ] = m_buffer[ j ];

	for ( i = 0; i < m_num_lines; ++i ) free( m_buffer[ i ] );

	free( m_buffer );

	m_max_lines = in_max_lines;
	m_buffer = buffer;
	m_first_line = 0;
	m_num_lines = k;
	
	unlock();
}


void balance::buffered_log::message_notification( char *in_message ) {
	if ( m_disk_buffered ) {
		notify_ui( k_log_append );
	} else if ( m_max_lines ) {
		char		   *msg, **buffer;
		
		if ( asprintf( &msg, "%s", in_message ) == -1 ) _throw( err_mem_full );
		
		if ( m_max_lines == k_unlimited || m_num_lines < m_max_lines ) {
			try {
				if ( ! ( buffer = (char **) realloc( m_buffer, ( m_num_lines + 1 ) * sizeof(char *) ) ) ) _throw( err_mem_full );
			} catch ( ... ) {
				free( msg );
				throw;
			}
			
			buffer[ m_num_lines++ ] = msg;
			m_buffer = buffer;
		} else {
			free( m_buffer[ m_first_line ] );
			m_buffer[ m_first_line ] = msg;
			
			m_first_line = ( m_first_line + 1 ) % m_max_lines;
		}

		notify_ui( k_log_append );
	}
}


void balance::buffered_log::open_log_file( const char *in_filename ) {
	log_::open_log_file( in_filename );

	m_fp = new file( in_filename );
	m_disk_buffered = true;
	clear();
}


void balance::buffered_log::close_log_file() {
	log_::close_log_file();

	lock();
	if ( m_fp ) {
		delete m_fp;
		m_fp = nil;
	}
	unlock();
}


#endif // ENABLE_BUFFERED_LOG
