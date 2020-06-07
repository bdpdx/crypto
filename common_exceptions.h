#include "common_types.h"


#ifndef __common_exceptions_h__
#define __common_exceptions_h__
#ifdef __cplusplus



#if _WIN32
	#include "precompiled.h"
#endif


extern "C" { char *basename( const char * ); }


struct err_t_exception {
	err_t_exception();
	err_t_exception( const err_t_exception &_in_err );
	err_t_exception( err_t _in_err, __u32 in_line, const char *in_file, const char *in_message = nil );

	operator err_t() { return err; }

	err_t_exception &operator=( const err_t_exception &_in_err );
	
	void clear();
	
	static bool set_quiet( bool in_quiet = true ) { bool result = s_all_exceptions_quiet; s_all_exceptions_quiet = in_quiet; return result; }
	static void throw_message_format( err_t _in_err, __u32 in_line, const char *in_file, const char *in_format, ... );
	
	err_t			err;
	err_t			err_original;
	__u32			line;
	const char	   *file;
	char			message[ 256 ];
	
	__u32			is_errno	:	1;		// is this error code an errno result?
	__u32			is_macerr	:	1;		// is this error a macos error type from MacErrors.h?
	__u32			is_other	:	1;		// did we override an original error with a different one (if so err_original is the first error)
	__u32			is_winerr	:	1;
	
	static bool		s_all_exceptions_quiet;	// not thread safe at the moment...
};


typedef void (*exception_notification_callback)( void *in_context, err_t_exception in_err, void *in_from_object );


#if DEBUG
	#define __FILE_DEBUG__								basename( __FILE__ )
#else
	#define __FILE_DEBUG__								""
#endif

#define _always( _in_statement )						if ( _err ) { _no_throw( _in_statement ); } else { try { _in_statement; } _catch }
#define _err											_ex.err
#define _catch											catch ( err_t_exception _in_err ) { _ex = _in_err; } catch ( ... ) { _unspecified( _ex ); }
#define _clear_err()									_ex.clear()
#define _defer_throw( _in_statement )					do { try { _in_statement; } _catch } while ( 0 )
#define _define_ex										err_t_exception _ex
#define _if_err											if ( _ex.err )
#define _if_no_err										if ( ! _ex.err )
#define _no_throw( _in_statement )						do { try { _in_statement; } catch ( ... ) { } } while ( 0 )
#define _return											_throw_now(); return
#define _throw_											throw _ex
#define _throw( _in_err )								do { err_t_exception __e( ( _in_err ), __LINE__, __FILE_DEBUG__ ); _tconsole( "%s:%u throwing %d", __FILE_DEBUG__, __LINE__, ( _in_err ) ); throw __e; } while ( 0 )
#define _throw_errno()									do { err_t_exception __e( errno, __LINE__, __FILE_DEBUG__ ); _tconsole( "%s:%u throwing errno %d", __FILE_DEBUG__, __LINE__, errno ); __e.is_errno = true; throw __e; } while ( 0 )
#define _throw_errno_( _in_errno )						do { err_t_exception __e( _in_errno, __LINE__, __FILE_DEBUG__ ); _tconsole( "%s:%u throwing errno %d", __FILE_DEBUG__, __LINE__, _in_errno ); __e.is_errno = true; throw __e; } while ( 0 )
#define _throw_errno_if( _in_statement )				do { if ( ( _in_statement ) ) { err_t_exception __e( errno, __LINE__, __FILE_DEBUG__ ); __e.is_errno = true; _tconsole( "%s:%u throwing errno %d", __FILE_DEBUG__, __LINE__, errno ); throw __e; } } while ( 0 )
#define _throw_if( _in_statement )						do { err_t __err; if ( ( __err = ( _in_statement ) ) ) { err_t_exception __e( __err, __LINE__, __FILE_DEBUG__ ); _tconsole( "%s:%u throwing %d", __FILE_DEBUG__, __LINE__, __err ); throw __e; } } while ( 0 )
#define _throw_macerr( _in_err )						do { err_t_exception __e( ( _in_err ), __LINE__, __FILE_DEBUG__ ); _tconsole( "%s:%u throwing mac error %d", __FILE_DEBUG__, __LINE__, ( _in_err ) ); __e.is_macerr = true; throw __e; } while ( 0 )
#define _throw_macerr_if( _in_statement )				do { err_t __err; if ( ( __err = ( _in_statement ) ) ) { err_t_exception __e( __err, __LINE__, __FILE_DEBUG__ ); __e.is_macerr = true; _tconsole( "%s:%u throwing macos error %d", __FILE_DEBUG__, __LINE__, __err ); throw __e; } } while ( 0 )
#define _throw_now()									if ( _ex.err ) throw _ex
#define _throw_other( _in_err, _in_statement )			do { try { _in_statement; } catch ( err_t_exception in_err ) { in_err.err_original = in_err.err; in_err.err = _in_err; in_err.is_other = true; throw; } } while ( 0 )
#define _throw_quiet( _in_err )							do { err_t_exception __e( ( _in_err ), __LINE__, __FILE_DEBUG__ ); throw __e; } while ( 0 )
#define _throw_winerr( _in_winerr )						do { err_t_exception __e( _in_winerr, __LINE__, __FILE_DEBUG__ ); _tconsole( "%s:%u throwing windows error %d", __FILE_DEBUG__, __LINE__, _in_winerr ); __e.is_winerr = true; throw __e; } while ( 0 )
#define _try											_define_ex; try
#define _try_											_throw_now(); try
#define _unspecified( in_err_t_exception )				do { in_err_t_exception.err = err_unspecified_exception; in_err_t_exception.line = __LINE__; in_err_t_exception.file = __FILE_DEBUG__; } while ( 0 )

#if _WIN32
	#define _tconsole( _in_format, ... )				do { if ( ! err_t_exception::s_all_exceptions_quiet ) console( _in_format, __VA_ARGS__ ); } while ( 0 )
	#define _throw_msg( _in_err, in_format, ... )		do { err_t_exception::throw_message_format( ( _in_err ), __LINE__, __FILE_DEBUG__, in_format, __VA_ARGS__ ); } while ( 0 )
	#define _throwc( _err, _format, ... )				do { _tconsole( _format, __VA_ARGS__ ); _throw( _err ); } while ( 0 )
#else
	#define _tconsole( _in_format, _in_args... )		do { if ( ! err_t_exception::s_all_exceptions_quiet ) console( _in_format, ## _in_args ); } while ( 0 )
	#define _throw_msg( _in_err, in_format, args... )	do { err_t_exception::throw_message_format( ( _in_err ), __LINE__, __FILE_DEBUG__, in_format, ## args ); } while ( 0 )
	#define _throwc( _err, _format, _args... )			do { _tconsole( _format, ## _args ); _throw( _err ); } while ( 0 )
#endif

#if __OBJC__
	#define _raise( _in_err )							do { if ( _in_err.err ) [NSException raise: NSGenericException format: _in_err.message ? @"%s %d (%s) thrown from %s:%u" : @"%s %d%s thrown from %s:%u", _in_err.is_errno ? "Errno " : _in_err.is_macerr ? "MacOS error" : "Error", _in_err.err, _in_err.message ? _in_err.message : "", _in_err.file, _in_err.line]; } while ( 0 )
#else
	#define _raise( _in_err )							do { exit( _in_err.err ); } while ( 0 )
#endif // __MWERKS__



#endif // __cplusplus
#endif // __common_exceptions_h__
