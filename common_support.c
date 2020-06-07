#include "common_support.h"


char *basename( const char *in_path ) {
	char				   *result;
	
	return ( result = strrchr( in_path, k_path_separator ) ) ? result + 1 : (char *) in_path;
}


#if ENABLE_ASPRINTF
int asprintf( char **out_string, const char *in_format, ... ) {
	va_list				args;
	int					n;

	va_start( args, in_format );
	n = vasprintf( out_string, in_format, args );
	va_end( args );

	return n;
}


int vasprintf( char **out_string, const char *in_format, va_list in_args ) {
	int					n;

	if ( ( n = vsnprintf( NULL, 0, in_format, in_args ) ) < 0 ) return -1;
	if ( ! ( *out_string = (char *) malloc( ++n ) ) ) return -1;
	
	return vsnprintf( *out_string, n, in_format, in_args );
}
#endif // ENABLE_ASPRINTF


#if ENABLE_STRNSTR
char *strnstr( const char *in_string, const char *in_find, size_t in_string_length ) {
	char					c, sc;
	size_t					len;

	if ( ( c = *in_find++ ) != '\0' ) {
		len = strlen( in_find );
		
		do {
			do {
				if ( in_string_length-- < 1 || ( sc = *in_string++ ) == '\0') {
					return NULL;
				}
			} while ( sc != c );

			if ( len > in_string_length ) {
				return NULL;
			}
		} while ( strncmp( in_string, in_find, len ) != 0 );

		in_string--;
	}
	
	return (char *) in_string;
}
#endif // ENABLE_STRNSTR


#if ENABLE_VSYSTEMR
int vsystemr( bool in_direct_output_to_dev_null, const char *in_format, ... ) {
	va_list					args;
	char				   *command = nil, *tmp;
	int						err;

	va_start( args, in_format );
	err = vasprintf( &tmp, in_format, args ) >= 0 ? 0 : -1;
	if ( ! err ) {
		if ( in_direct_output_to_dev_null ) {
			err = asprintf( &command, "%s%s", tmp, in_direct_output_to_dev_null ? " &>/dev/null" : "" ) >= 0 ? 0 : -1;
			free( tmp );
		} else {
			command = tmp;
		}
	}
	va_end( args );
	
//	console( "vsystemr command is: %s", command );
	
	if ( ! err ) err = system( command );
	
	if ( command ) free( command );
	
	return err;
}
#endif
