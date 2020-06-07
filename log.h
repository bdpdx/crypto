#ifndef __log_h__
#define __log_h__
#ifdef __cplusplus


#define log_message( in_priority ) do {														\
	va_list			arguments;																\
																							\
	va_start( arguments, in_format );														\
	message( in_priority, in_format, arguments );											\
	va_end( arguments );																	\
} while ( 0 )


namespace balance {


class log {

public:

	enum log_flags {
		k_finalize				=	( 1 << 0 )							,
		k_free_io_postpone		=	( 1 << 1 )							,
		k_suppress_newline		=	( 1 << 2 )							,
		
		k_default_log_flags		=	k_finalize | k_free_io_postpone
	};

	log( const char *in_log_filename = nil, const char *in_syslog_name = nil, bool in_enable_stderr = false, bool in_enable_timestamps = true );
	virtual ~log();

	void open_syslog( const char *in_name );
	void close_syslog();
	
	virtual void open_log_file( const char *in_filename );
	virtual void close_log_file();
	
	void enable_file() { m_file_enabled = true; }
	void enable_message_notification() { m_message_notification_enabled = true; }
	void enable_stderr() { m_stderr_enabled = true; }
	void enable_syslog( char *in_level = "LOG_ERR" );
	void enable_timestamps() { m_timestamps_enabled = true; }
		
	void disable_file() { m_file_enabled = false; }
	void disable_message_notification() { m_message_notification_enabled = false; }
	void disable_stderr() { m_stderr_enabled = false; }
	void disable_syslog() { m_syslog_enabled = false; }
	void disable_timestamps() { m_timestamps_enabled = false; }

	void show_process_ids( bool in_show = true ) { m_show_process_id = in_show; }
	void show_thread_ids( bool in_show = true ) { m_show_thread_id = in_show; }

	bool syslog_is_open() { return m_syslog_open; }
	bool syslog_is_enabled() { return m_syslog_enabled; }

	void message( int in_priority, const char *in_format, ... );
	void message( int in_priority, char **io_postpone, __u32 in_finalize, const char *in_format, ... );

	void vmessage( int in_priority, const char *in_format, va_list in_arguments );
	void vmessage( int in_priority, char **io_postpone, __u32 in_finalize, const char *in_format, va_list in_arguments );

	// error logging levels, in order of decreasing importance (see syslog(3))
	void emergency( const char *in_format, ... ) { log_message( LOG_EMERG ); }	// system is unusable
	void alert( const char *in_format, ... ) { log_message( LOG_ALERT ); }		// action must be taken immediately
	void critical( const char *in_format, ... ) { log_message( LOG_CRIT ); }		// critical conditions
	void error( const char *in_format, ... ) { log_message( LOG_ERR ); }			// error conditions
	void warning( const char *in_format, ... ) { log_message( LOG_WARNING ); }	// warning conditions
	void notice( const char *in_format, ... ) { log_message( LOG_NOTICE ); }		// normal, but significant, condition
	void info( const char *in_format, ... ) { log_message( LOG_INFO ); }			// informational message
	void debug( const char *in_format, ... ) { log_message( LOG_DEBUG ); }		// debug-level message

	const FILE *get_file() { return m_file; }
	void set_file( FILE *in_file ) { if ( m_file == nil || in_file == nil ) m_file = in_file; } 

	void set_file_log_level( const char *in_level );
	void set_file_log_level( long in_level ) { m_file_log_level = in_level; }
	
	log& operator()( char *in_format, ... ) { log_message( m_file_log_level ); return *this; }

	void lock() { _throw_if( pthread_mutex_lock( &m_mutex ) ); }
	void unlock() { _throw_if( pthread_mutex_unlock( &m_mutex ) ); }

protected:

	virtual void message_notification( char *in_message ) { }
	
private:

	FILE			   *m_file;
	long				m_file_log_level;
	pthread_mutex_t		m_mutex;	
	long				m_syslog_level;

	__u32				m_file_enabled					:	1;
	__u32				m_message_notification_enabled	:	1;
	__u32				m_show_process_id				:	1;
	__u32				m_show_thread_id				:	1;
	__u32				m_stderr_enabled				:	1;
	__u32				m_syslog_enabled				:	1;
	__u32				m_syslog_open					:	1;
	__u32				m_timestamps_enabled			:	1;

};


#if ENABLE_GLOBAL_LOG
	extern balance::log	   *__s_application_log;

	#define s_application_log	balance::__s_application_log

	#define log_msg_body( in_priority )															\
		if ( s_application_log ) {																\
			va_list			arguments;															\
																								\
			va_start( arguments, in_format );													\
			s_application_log->message( in_priority, in_format, arguments );					\
			va_end( arguments );																\
		}
#else
	#define log_msg_body( in_priority )
#endif	// ENABLE_GLOBAL_LOG


// writes default error message to application log when an err_t_exception is caught as in_err
#define log_default_err_t_exception_msg()	error_msg( "caught error %d (%s) in %s from line %u", in_err.err, strerror( in_err.err ), in_err.file, in_err.line )


typedef void (*status_proc)( void *in_context, char *in_format, ... );

// call with in_log_to_delete set to nil for new, otherwise delete
typedef log *(*new_delete_Log_proc)( void *in_context, char *in_title, void *in_id, log *in_log_to_delete );


#if ENABLE_BUFFERED_LOG

#include "ui_base.hpp"


enum log_event_selector {
	k_log_append
};


class buffered_log : public log, public ui_base {

public:

	buffered_log( const char *in_log_filename = nil, const char *in_syslog_name = nil, bool in_enable_stderr = false, bool in_enable_timestamps = true );
	virtual ~buffered_log();

	char *buffer();		// returns the scroll buffer as one block.  caller must free() result.
	const char *read_line();

	void clear();		// flushes the scrollback buffer
	void reset();		// resets the read pointer to the first line in the log
	void set_max_lines( __s32 in_max_lines = k_unlimited );

	// log methods:

	virtual void open_log_file( const char *in_filename );
	virtual void close_log_file();
	
protected:

	// log methods:

	virtual void message_notification( char *in_message );

	class file	   *m_fp;				// an accessor for reading a disk-based backing store

	char		  **m_buffer;			// the scrollback buffer
	__s32			m_num_lines;		// number of lines in the scrollback buffer
	__s32			m_max_lines;		// maximum number of lines in scrollback buffer (-1 == unlimited)	
	__s32			m_first_line;		// the offset of the first line
	
	__u32			m_disk_buffered	:	1;

};


#endif // ENABLE_BUFFERED_LOG


} // namespace balance


inline void emergency_msg( const char *in_format, ... ) { log_msg_body( LOG_DEBUG ) }
inline void alert_msg( const char *in_format, ... ) { log_msg_body( LOG_ERR ) }
inline void critical_msg( const char *in_format, ... ) { log_msg_body( LOG_INFO ) }
inline void error_msg( const char *in_format, ... ) { log_msg_body( LOG_ERR ) }
inline void warning_msg( const char *in_format, ... ) { log_msg_body( LOG_INFO ) }
inline void notice_msg( const char *in_format, ... ) { log_msg_body( LOG_ERR ) }
inline void info_msg( const char *in_format, ... ) { log_msg_body( LOG_INFO ) }
inline void debug_msg( const char *in_format, ... ) { debug_statement( log_msg_body( LOG_DEBUG ) ); }



#endif // __cplusplus
#endif // __log_h__
