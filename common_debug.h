#ifndef __common_debug_h__
#define __common_debug_h__
#ifdef __cplusplus


#if DEBUG

#if _WIN32
	#include "precompiled.h"
#endif


void _console( const char *in_file, __u32 in_line, const char *in_format, ... );
void _dump( const char *in_file, __u32 in_line, const void *in_address, __u32 in_length, __u32 in_width );
void _proc_state( const char *in_format, ... );


#if __OBJC__
	// Because I always forget to type the '@'.  Lazy I know, but hey...
	void NSLog( const char *in_format, ... );
#endif

class profiler {

	friend void _proc_state( const char *in_format, ... );

public:

	profiler( int &in_return_value, const char *in_format, ... );
	profiler( const char *in_format, ... );

   ~profiler();

	enum { k_profiler_default_tab_width = 2 };

	static void set_tab_width( unsigned in_chars = k_profiler_default_tab_width );

protected:

	void init( int *in_return_value, const char *in_format, va_list in_args );

	struct thread_info {
		unsigned		current_depth;
		const char	   *current_name;
#if _WIN32
		__u32			thread;
#else
		pthread_t		thread;
#endif
	};

	static thread_info *current();

	char							   *m_name;
	const char						   *m_previous_name;
	int								   *m_return_value;

//	static atomic_queue<thread_info>   *s_queue;
	static void						   *s_queue;
	static unsigned						s_tab_width;

};


#if _WIN32
	#define proc_enter( _in_format, ... )						profiler	__profiler( _in_format, __VA_ARGS__ )
	#define proc_enter_r( _in_error, _in_format, ... )			profiler	__profiler( _in_error, _in_format, __VA_ARGS__ )
	#define proc_state( _in_format, ... )						_proc_state( _in_format, __VA_ARGS__ )
#else
	#define proc_enter( _in_format, _in_args... )				profiler	__profiler( _in_format, ## _in_args )
	#define proc_enter_r( _in_error, _in_format, _in_args... )	profiler	__profiler( _in_error, _in_format, ## _in_args )
	#define proc_state( _in_format, _in_args... )				_proc_state( _in_format, ## _in_args )
#endif


#if DEBUG_MUTEX
	#define pthread_mutex_lock					_report_pthread_mutex_lock
	#define pthread_mutex_unlock				_report_pthread_mutex_unlock
#endif	// DEBUG_MUTEX


extern "C" {
	void _report_free( void *in_object );
	void *_report_malloc( __u32 in_size );
#if ! _WIN32
	int _report_pthread_mutex_lock( pthread_mutex_t *io_mutex );
	int _report_pthread_mutex_unlock( pthread_mutex_t *io_mutex );
#endif
}


#else // DEBUG

#if _WIN32
	#define proc_enter( in_format, ... )		do { } while ( 0 )
	#define proc_state( in_format, ... )		do { } while ( 0 )
#else
	#define proc_enter( in_format, args... )	do { } while ( 0 )
	#define proc_state( in_format, args... )	do { } while ( 0 )
#endif // _WIN32

#endif // DEBUG



#endif // __cplusplus
#endif // __common_debug_h__
