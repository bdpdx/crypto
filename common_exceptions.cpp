#include <stdarg.h>
#include <stdio.h>

#include "common_exceptions.h"


bool						err_t_exception::s_all_exceptions_quiet;


err_t_exception::err_t_exception() {
	clear();
}


err_t_exception::err_t_exception( const err_t_exception &in_err ) {
	*this = in_err;
}


err_t_exception::err_t_exception( err_t in_err, __u32 in_line, const char *in_file, const char *in_message ) {
	clear();

	err = in_err;
	line = in_line;
	file = in_file;
	
	if ( in_message ) {
		strncpy( message, in_message, sizeof(message) );
		message[ sizeof(message) - 1 ] = 0;
	}
}


err_t_exception &err_t_exception::operator=( const err_t_exception &in_err ) {
	if ( this != &in_err ) {
		err = in_err.err;
		err_original = in_err.err_original;
		line = in_err.line;
		file = in_err.file;

		is_errno = in_err.is_errno;
		is_macerr = in_err.is_macerr;
		is_other = in_err.is_other;
		is_winerr = in_err.is_winerr;
		
		strncpy( message, in_err.message, sizeof(message) );

		message[ sizeof(message) - 1 ] = 0;
	}
	
	return *this;
}


void err_t_exception::throw_message_format( err_t in_err, __u32 in_line, const char *in_file, const char *in_format, ... ) {
	va_list			args;

	err_t_exception			e( in_err, in_line, in_file );

	va_start( args, in_format );
	vsnprintf( e.message, sizeof(e.message), in_format, args );
	va_end( args );
	
	throw e;
}


void err_t_exception::clear() {
	err = 0;
	err_original = 0;
	line = 0;
	file = nil;
	*message = 0;	
	is_errno = false;
	is_macerr = false;
	is_other = false;
	is_winerr = false;
}



#pragma mark -


#include <new>


void *operator new  ( std::size_t in_size ) throw( std::bad_alloc ) {
	void				   *result;
	
	if ( ! ( result = malloc( in_size ) ) ) throw std::bad_alloc();
	
	return result;
}


void *operator new[]( std::size_t in_size ) throw( std::bad_alloc ) {
	void				   *result;
	
	if ( ! ( result = malloc( in_size ) ) ) throw std::bad_alloc();
	
	return result;
}


void operator delete  ( void *io_p ) throw() { if ( io_p ) free( io_p ); }
void operator delete[]( void *io_p ) throw() { if ( io_p ) free( io_p ); }
