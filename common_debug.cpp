#include "common_debug.h"


#if DEBUG


#include "queue.h"
#include "synchronizer.h"


#if _WIN32
	#include <Windows.h>

	#define getpid		_getpid

	extern "C" {
		int asprintf( char **out_string, const char *in_format, ... );
		int vasprintf( char **out_string, const char *in_format, va_list in_args );
	}
#endif

// valid CONSOLE_TARGET's are:
//
// 1 == stderr
// 2 == CFShow (Mach only, non-Mach defaults to 0)
// 4 == to s_application_log

#ifndef CONSOLE_TARGET
	#define CONSOLE_TARGET			1
#else
	#if CONSOLE_TARGET == 2
		#ifndef __MACH__
			#undef CONSOLE_TARGET
			#define CONSOLE_TARGET	1
		#endif
	#elif CONSOLE_TARGET == 4
		#include "log.h"
		
		#define log				balance::log
	#endif
#endif


#undef pthread_mutex_lock
#undef pthread_mutex_unlock


//	   *profiler::s_queue;
void /* atomic_queue<profiler::thread_info> */	   *profiler::s_queue;
#define S_QUEUE										reinterpret_cast<balance::atomic_queue<profiler::thread_info> *>(s_queue)

unsigned											profiler::s_tab_width = k_profiler_default_tab_width;



profiler::profiler( int &in_error, const char *in_format, ... ) {
	va_list				args;
	
	va_start( args, in_format );
	init( &in_error, in_format, args );
	va_end( args );
}


profiler::profiler( const char *in_format, ... ) {
	va_list				args;
	
	va_start( args, in_format );
	init( nil, in_format, args );
	va_end( args );
}


void profiler::init( int *in_return_value, const char *in_format, va_list in_args ) {
	__u32				depth;
	thread_info		   *info;

	if ( ! s_queue ) s_queue = new balance::atomic_queue<thread_info>;

	m_return_value = in_return_value;
	
	if ( vasprintf( &m_name, in_format, in_args ) == -1 ) _throw( err_mem_full );

	try {
		if ( ! ( info = current() ) ) {
			info = new thread_info;
			
			info->current_depth = 0;
			info->current_name = nil;
#if _WIN32
			info->thread = GetCurrentThreadId();
#else
			info->thread = pthread_self();
#endif

			S_QUEUE->insert( info );	
		}

		m_previous_name = info->current_name;
		info->current_name = m_name;

		if ( ( depth = info->current_depth++ ) ) {
			_no_throw( console( "%*c%s entered", depth * s_tab_width, ' ', m_name ) );
		} else {
			_no_throw( console( "%s entered", m_name ) );
		}
	} catch ( ... ) {
		free( m_name );
		throw;
	}
}


profiler::~profiler() {
	char				buffer[ 32 ];
	__u32				depth;
	thread_info		   *info;

	if ( m_return_value ) {
		sprintf( buffer, "returned %d", *m_return_value );
	} else {
		sprintf( buffer, "exited" );
	}
	
	if ( ( info = current() ) ) {
		if ( ( depth = --info->current_depth ) ) {
			_no_throw( console( "%*c%s %s", depth * s_tab_width, ' ', m_name, buffer ) );
		} else {
			_no_throw( console( "%s %s", m_name, buffer ) );

			S_QUEUE->remove( info );
			delete info;
		}
	}
}


profiler::thread_info *profiler::current() {
	thread_info		   *info;

#if _WIN32
	DWORD				thread = GetCurrentThreadId();
#else
	pthread_t			thread = pthread_self();
#endif
	
	atomic_peek_criteria( *S_QUEUE, info, info->thread != thread );

	return info;
}


void profiler::set_tab_width( unsigned in_width ) {
	s_tab_width = in_width;
}


#if __MWERKS__
#pragma mark -
#pragma export on
#endif


void _console( const char *in_file, __u32 in_line, const char *in_format, ... ) {
	va_list					args;
	time_t					now_;
	const char			   *p;


	if ( ( p = strrchr( in_file, '/' ) ) ) ++p; else p = in_file;	
	
	time( &now_ );
#if _WIN32
	tm					   &now = *localtime( &now_ );
#else
	tm						now;

	localtime_r( &now_, &now );
#endif

#if CONSOLE_TARGET & 2
	CFStringRef				string, format;

	static mutex			s_console_mutex;

	va_start( args, in_format );

	#if SHOW_THREAD_ID
		if ( ( format = CFStringCreateWithFormat( nil, nil, CFSTR( "%u:%08x %02u/%02u/%02u %02u:%02u:%02u %12.12s:%4u> %s" ), getpid(), pthread_self(), now.tm_mon + 1, now.tm_mday, now.tm_year % 100, now.tm_hour, now.tm_min, now.tm_sec, p, in_line, in_format ) ) )
	#else
		if ( ( format = CFStringCreateWithFormat( nil, nil, CFSTR( "%u %02u/%02u/%02u %02u:%02u:%02u %12.12s:%4u> %s" ), getpid(), now.tm_mon + 1, now.tm_mday, now.tm_year % 100, now.tm_hour, now.tm_min, now.tm_sec, p, in_line, in_format ) ) )
	#endif
		{
			if ( ( string = CFStringCreateWithFormatAndArguments( kCFAllocatorDefault, nil, format, args ) ) ) {
				s_console_mutex.lock();
				CFShow( string );
				s_console_mutex.unlock();
				CFRelease( string );
			}
			CFRelease( format );
		}

	va_end( args );
#endif

#if CONSOLE_TARGET & ~2
	char		   *format;
	
	va_start( args, in_format );

	#if SHOW_THREAD_ID
		if ( asprintf( &format, "%u:%08x %02u/%02u/%02u %02u:%02u:%02u %12.12s:%4u> %s\n", getpid(), pthread_self(), now.tm_mon + 1, now.tm_mday, now.tm_year % 100, now.tm_hour, now.tm_min, now.tm_sec, p, in_line, in_format ) == -1 ) _throw( err_mem_full );
	#else
		if ( asprintf( &format, "%u %02u/%02u/%02u %02u:%02u:%02u %12.12s:%4u> %s\n", getpid(), now.tm_mon + 1, now.tm_mday, now.tm_year % 100, now.tm_hour, now.tm_min, now.tm_sec, p, in_line, in_format ) == -1 ) _throw( err_mem_full );
	#endif

	#if CONSOLE_TARGET & 1
		vfprintf( stderr, format, args );
	#endif
	
	#if CONSOLE_TARGET & 4
		if ( s_application_log ) s_application_log->vmessage( LOG_INFO, (char **) &( p = "" ), log::k_finalize | log::k_suppress_newline, format, args );
	#endif

	free( format );

	va_end( args );
#endif
}


void _dump( const char *in_file, __u32 in_line, const void *in_address, __u32 in_length, __u32 in_max_width ) {
	const __u8			   *address = (const __u8 *) in_address;
	char					buffer[ 256 ], *p;
	__u32					i, j, k;

	in_max_width = min( __u32(sizeof(buffer) - 2), in_max_width );

	in_max_width /= 9;
	in_max_width *= 9;
	
	if ( in_max_width < 18 ) return;

	for ( i = 0; i < in_length; ) {
		sprintf( p = buffer, "%08x:", (size_t) &address[ i ] );
		
		for ( p += j = 9; j < in_max_width && i < in_length; j += 9 ) {
			*p++ = ' ';
			
			for ( k = 0; k < 4 && i < in_length; ++k, ++i, p += 2 ) {
				hexify( address[ i ], p[ 0 ], p[ 1 ] );
			}
		}
		
//		*p++ = '\n';
		*p++ = '\0';
		
		_console( in_file, in_line, buffer );
	}
}


#if __MWERKS__
#pragma export reset
#endif


void _proc_state( const char *in_format, ... ) {
	va_list						args;
	profiler::thread_info	   *info;
	__u32						num_spaces;
	char					   *string;

	va_start( args, in_format );
	if ( vasprintf( &string, in_format, args ) == -1 ) _throw( err_mem_full );
	va_end( args );
	
	info = profiler::current();

	if ( ( num_spaces = info->current_depth * profiler::k_profiler_default_tab_width ) ) {
		_no_throw( console( "%*c%s: %s", num_spaces, ' ', info->current_name, string ) );
	} else {
		_no_throw( console( "%s: %s", info->current_name, string ) );
	}

	free( string );
}


#if __OBJC__
void NSLog( const char *in_format, ... ) {
	va_list					args;

	va_start( args, in_format );
	NSLogv( [NSString stringWithFormat: @"%s", in_format], args );
	va_end( args );
}
#endif


void _report_free( void *in_object ) {
	console( "free( 0x%08x )", in_object );
	free( in_object );
}


void *_report_malloc( __u32 in_size ) {
	void		   *result = malloc( in_size );

	console( "malloc( 0x%08x ) -> 0x%08x", in_size, result );
	return result;
}


#if ! _WIN32

int _report_pthread_mutex_lock( pthread_mutex_t *io_mutex ) {
	console( "pthread_mutex_lock( 0x%08x )", io_mutex );	
	return pthread_mutex_lock( io_mutex );
}


int _report_pthread_mutex_unlock( pthread_mutex_t *io_mutex ) {
	console( "pthread_mutex_unlock( 0x%08x )", io_mutex );	
	return pthread_mutex_unlock( io_mutex );
}

#endif // _WIN32


#endif // DEBUG
