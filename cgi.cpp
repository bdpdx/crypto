#include "cgi.h"


#define ENABLE_CGI_LOGGING		( ENABLE_GLOBAL_LOG )
#define log						balance::log


static int indirect_strcasecmp( const void *in_lhs, const void *in_rhs ) {
	return strcasecmp( *reinterpret_cast<char **>(const_cast<void *>(in_lhs)), *reinterpret_cast<char **>(const_cast<void *>(in_rhs)) );
}


static int indirect_strcmp( const void *in_lhs, const void *in_rhs ) {
	return strcmp( *reinterpret_cast<char **>(const_cast<void *>(in_lhs)), *reinterpret_cast<char **>(const_cast<void *>(in_rhs)) );
}


cgi::cgi( bool in_perform_case_sensitive_comparisons, bool in_sort_keys ) {
	_case_sensitive = in_perform_case_sensitive_comparisons;
	_entries = 0;
	_keys = nil;
	_sort_keys = in_sort_keys;
	_values = nil;
}


cgi::~cgi() { done(); }


void cgi::done() {
	while ( _entries-- ) {
		delete[] _keys[ _entries ];
		delete[] _values[ _entries ];
	}
	
	delete[] _keys;
	delete[] _values;
	
	_entries = 0;
	_keys = nil;
	_values = nil;
}


err_t cgi::init( FILE *in_fp, bool in_remove_leading_and_trailing_whitespace ) {
	char					a, b, *c, *d = nil, *e;
	err_t					err = 0;
	int						i, j, k, len, val_offset, val_len;	

	done();

#if ENABLE_CGI_LOGGING
	extern char			  **environ;
	char				   *postpone = nil, **s;

	s_application_log->message( LOG_INFO, &postpone, log::k_free_io_postpone, "cgi::init() invoked with the following environment:\n\n" );
	for ( s = environ; *s; ++s ) {
		s_application_log->message( LOG_INFO, &postpone, log::k_free_io_postpone, "    %s\n", *s );
	}
#endif

	if ( ( c = getenv( k_request_method ) ) == nil ) {
		err = err_cgi_not_invoked_as_cgi;
	} else if ( ! strcmp( c, k_method_post ) ) {
		if ( ( len = strtol( getenv( k_content_length ), 0, 10 ) ) <= 0 ) err = err_cgi_unspecified_content;
		if ( ! err && ( d = new char[ len + 1 ] ) == nil ) err = err_mem_full;
		if ( ! err && fread( d, len, 1, in_fp ) < 1 ) err = err_cgi_unexpected_end_of_content;
#if ENABLE_CGI_LOGGING && 0
		if ( ! err ) s_application_log->message( LOG_INFO, &postpone, log::k_free_io_postpone, "\ncgi POST content:\n\n%s\n\n", d );
#endif
	} else if ( ! strcmp( c, k_method_get ) ) {
		if ( ( c = getenv( k_query_string ) ) == nil || ! *c ) err = err_cgi_unspecified_content;
		if ( ! err && ( d = new char[ strlen( c ) + 1 ] ) == nil ) err = err_mem_full;
		if ( ! err ) strcpy( d, c );
#if ENABLE_CGI_LOGGING && 0
		if ( ! err ) s_application_log->message( LOG_INFO, &postpone, log::k_free_io_postpone, "\ncgi GET content:\n\n%s\n\n", d );
#endif
	} else {
		err = err_cgi_invalid_method;
	}
	
	// unescape url
	if ( ! err ) {
		for ( c = e = d; ( a = *c++ ); ) {
			if ( a == '%' ) {
				if ( ! ( a = *c++ ) ) { err = err_cgi_unexpected_end_of_content; break; }
				else if ( a >= '0' && a <= '9' ) b = a - '0' << 4;
				else if ( a >= 'a' && a <= 'f' ) b = a - 'a' + 10 << 4;
				else if ( a >= 'A' && a <= 'F' ) b = a - 'A' + 10 << 4;
				else { err = err_bad_data; break; }

				if ( ! ( a = *c++ ) ) { err = err_cgi_unexpected_end_of_content; break; }
				else if ( a >= '0' && a <= '9' ) b |= a - '0';
				else if ( a >= 'a' && a <= 'f' ) b |= a - 'a' + 10;
				else if ( a >= 'A' && a <= 'F' ) b |= a - 'A' + 10;
				else { err = err_bad_data; break; }
				
				*e++ = b;
			} else {
				*e++ = a;
			}
		}
		
		*e = 0;
	}
	
	c = d;
	
	if ( ! err ) {
		for ( len = 1, d = strchr( c, '&' ); d != nil; ++len, d = strchr( ++d, '&' ) ) ;
		
		if ( ( _entries = len ) > 0 ) {
			if ( ( _keys = new char *[ len ] ) == nil ) err = err_mem_full;

			if ( ! err ) {
				for ( i = 0; i < len; ++i ) _keys[ i ] = nil;
				if ( ( _values = new char *[ len ] ) == nil ) err = err_mem_full;
			}

			if ( ! err ) {
				for ( i = 0; i < len; ++i ) _values[ i ] = nil;

				for ( i = 0, d = strtok( c, "&"); d != nil; ++i, d = strtok( nil, "&" ) ) _keys[ i ] = d;
				for ( i = 0; i < len; ++i ) {
					if ( ( d = strchr( _keys[ i ], '=' ) ) == nil ) err = err_cgi_no_equal_character_in_argument;
					else *d = 0;
				}
			}

			if ( ! err && _sort_keys ) qsort( _keys, len, sizeof(char *), _case_sensitive ? indirect_strcmp : indirect_strcasecmp );

			for ( i = 0; ! err && i < len; ++i ) {
				( d = _keys[ i ] )[ strlen( _keys[ i ] ) ] = '=';

				for ( j = 0, val_len = strlen( d ), val_offset = 0; ! err && j <= val_len; ++j ) {
					if ( d[ j ] == '+' ) d[ j ] = ' ';
					else if ( ! val_offset && d[ j ] == '=' ) val_offset = j + 1;
				}

				// copy the translated strings to local storage
				
				if ( ! err && ( _keys[ i ] = new char[ val_offset ] ) == nil ) err = err_mem_full;
				if ( ! err ) {
					memcpy( _keys[ i ], d, val_offset - 1 );
					_keys[ i ][ val_offset - 1 ] = 0;
					
					if ( in_remove_leading_and_trailing_whitespace ) {
						for ( val_len = strlen( &d[ val_offset ] ); val_len && is_whitespace( d[ val_offset + val_len - 1 ] ); --val_len ) ;
						for ( j = 0; j < val_len && is_whitespace( d[ val_offset + j ] ); ++j ) ;
	
						_values[ i ] = new char[ ( k = val_len - j ) + 1 ];
						memcpy( _values[ i ], &d[ val_offset + j ], k );
						_values[ i ][ k ] = 0;
					} else {
						_values[ i ] = new char[ val_len = strlen( &d[ val_offset ] ) + 1 ];
						memcpy( _values[ i ], &d[ val_offset ], val_len );
					}
				}
			}
		}
	}
	
#if ENABLE_CGI_LOGGING && 0
	if ( _entries ) {
		s_application_log->message( LOG_INFO, &postpone, log::k_free_io_postpone, "cgi parsed the following:\n\n" );

		for ( i = 0; i < _entries; ++i ) {
			s_application_log->message( LOG_INFO, &postpone, log::k_free_io_postpone, "    %s=%s\n", _keys[ i ], _values[ i ] );
		}
	}

	s_application_log->message( LOG_INFO, &postpone, log::k_free_io_postpone, "\ncgi::init() returning %d", err );
#endif

#if ENABLE_CGI_LOGGING
	s_application_log->message( LOG_INFO, &postpone, log::k_default_log_flags, "" );
#endif
	
	if ( err ) done();
	
	delete[] c;
	
	return err;
}


err_t cgi::get_key_and_value_by_index( long in_index, char *&out_key, char *&out_value ) {
	if ( in_index > _entries - 1 ) {
		out_key = out_value = nil;

		return err_cgi_invalid_index;
	} else {
		out_key = _keys[ in_index ];
		out_value = _values[ in_index ];
	
		return 0;
	}
}


err_t cgi::get_value_by_key( char *in_key, char *&out_value, long *out_index ) {
	err_t					err = err_cgi_key_not_found;
	long					index;
	char				   *res;

	if ( out_index ) *out_index = -1;

	out_value = nil;

	if ( _sort_keys ) {
		res = reinterpret_cast<char *>(bsearch( &in_key, _keys, _entries, sizeof(char *), _case_sensitive ? indirect_strcmp : indirect_strcasecmp  ));

		if ( res ) {
			err = 0;
			out_value = _values[ index = ( reinterpret_cast<__u32>(res) - reinterpret_cast<__u32>(_keys) ) / sizeof(char *) ];
			if ( out_index ) *out_index = index;
		}
	} else {
		for ( index = 0; index < _entries; ++index ) {
			if ( _case_sensitive ) {
				if ( ! strcmp( in_key, _keys[ index ] ) ) break;
			} else {
				if ( ! strcasecmp( in_key, _keys[ index ] ) ) break;
			}
		}
		
		if ( index < _entries ) {
			err = 0;
			if ( out_index ) *out_index = index;
			out_value = _values[ index ];
		}
	}

	return err;
}
