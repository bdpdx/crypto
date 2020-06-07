#ifndef __common_support_h__
#define __common_support_h__



#if _WIN32
	#include "precompiled.h"
#else
	#include <pthread.h>
#endif

#include <sys/stat.h>

#include "common_debug.h"
#include "common_macros.h"


#ifdef __cplusplus
extern "C" {
#endif
	char *basename( const char *in_path );

	#if ENABLE_ASPRINTF
		#if ( __sun__ || _WIN32 )
			int asprintf( char **out_string, const char *in_format, ... );
			int vasprintf( char **out_string, const char *in_format, va_list in_args );
		#else
			#undef ENABLE_ASPRINTF
		#endif
	#endif
	#if ENABLE_STRNSTR
		#if ( __linux__ )
			char *strnstr( const char *in_string, const char *in_find, size_t in_string_length );
		#else
			#undef ENABLE_STRNSTR
		#endif
	#endif
	#if ENABLE_VSYSTEMR
		int vsystemr( bool in_direct_output_to_dev_null, const char *in_format, ... );

		#if _WIN32
			#define vsystem( _fmt, ... )		vsystemr( true, _fmt, __VA_ARGS__ )
		#else
			#define vsystem( _fmt, _args... )	vsystemr( true, _fmt, ## _args )
		#endif
	#endif
#ifdef __cplusplus
}
#endif


#ifdef __cplusplus


#if ENABLE_CHDIR
	struct __chdir {
		__chdir( const char *in_path = nil ) { _throw_errno_if( ! getcwd( _buffer, sizeof(_buffer) ) ); if ( in_path ) change( in_path ); }
	   ~__chdir() { _throw_errno_if( chdir( _buffer ) == -1 ); }
		
		void change( const char *in_path ) { _throw_errno_if( chdir( in_path ) == -1 ); }
		
		char				_buffer[ MAXPATHLEN ];
	};

	#define _chdir( _in_dir )			__chdir _dir( _in_dir )
	#define _chdir_( _in_dir )			_dir.change( _in_dir )
#endif // ENABLE_CHDIR


#if ENABLE_DATETIME_CONVERTERS
	#if ( __linux__ || __MACH__ )
		time_t datetime_to_system_time( char *in_datetime, bool in_ignore_timezone = true );
		char *system_time_to_datetime( time_t in_system_time, char out_datetime[ 20 ], bool in_ignore_timezone = true );
	#else
		#undef ENABLE_DATETIME_CONVERTERS
	#endif
#endif

#if ENABLE_DUP_FUNCTIONS
	void *dup_mem( const void *in_buffer, __u32 in_length );
	char *dup_string( const char *in_string );
	// duplicates in_string onto out_destination, optionally calling delete[] on out_destination prior
	// to allocating new memory for it if in_delete_out_destination is true.
	//
	// returns the number of characters copied, including the trailing null
	// caller is responsible for delete[]int out_destination
	__s32 dup_string_to( const char *in_string, char *&out_destination, bool in_delete_out_destination_first = true );
#endif

#if ENABLE_FREAD_FWRITE_WRAPPERS
	size_t _fread( void *out_buffer, size_t in_size, FILE *in_stream );
	void _fwrite( const void *in_buffer, size_t in_size, FILE *in_stream );
#endif

#if ENABLE_HEX_CONVERTERS
	// these functions convert an arbitrary sequence of bytes into an ASCII-readable
	// hexadecimal string and back.  if out_buffer is nil, the result buffer will
	// be allocated and it is the caller's responsibility to delete[] the returned
	// result.  otherwise, out_buffer must be large enough to hold the returned data.
	// for buffer_from_hex_string(), out_buffer (if supplied) must be at least
	// ( strlen( in_hex_string ) / 2 ) bytes long.  For hex_string_from_buffer(),
	// out_buffer must be at least ( in_length * 2 + 1 ) bytes long.  These functions
	// handle the case of the supplied input and output buffer overlapping correctly.
	void *bytes_from_hex_string( const char *in_hex_string, void *out_buffer = nil, __u32 *out_length = nil );
	char *hex_string_from_bytes( const void *in_buffer, __u32 in_length, char *out_string = nil );
#endif

#if ENABLE_POSIX_PATH_TO_FSSPEC
	#if __MACH__
		void posix_path_to_fsspec( const char *in_path, FSSpec &out_spec );
	#else
		#undef ENABLE_POSIX_PATH_TO_FSSPEC
	#endif
#endif

#if ENABLE_SIGNAL_WRAPPERS
	class set_signal {

	public:

		set_signal( int in_signal, v_proc_i in_handler ) { _throw_errno_if( ( m_handler = signal( m_signal = in_signal, in_handler ) ) == SIG_ERR ); }
	   ~set_signal() { signal( m_signal, m_handler ); }

	private:

		int				m_signal;
		v_proc_i		m_handler;

	};

	class ignore_signal : public set_signal {

	public:

		// set in_really_ignore to false if you want a signal handler that does
		// nothing.  this prevents the application from core dumping on receipt of a
		// signal, but also allows select() etc. to unblock with EINTR.
		//
		// if in_really_ignore is true, the specified signal will be completely
		// masked.
		ignore_signal( int in_signal, bool in_really_ignore = true ) : set_signal( in_signal, in_really_ignore ? SIG_IGN : ignore ) { }
	   ~ignore_signal() { }

		static void ignore( int in_signal ) { console( "received signal %d", in_signal ); return; }
	};
#endif


#endif // __cplusplus



#endif	// __common_support_h__
