#include "common_support.h"


#if ENABLE_DATETIME_CONVERTERS
time_t datetime_to_system_time( char *in_datetime, bool in_ignore_timezone ) {
	struct tm	tm_time;

	if ( ! strptime( in_datetime, "%Y-%m-%d %H:%M:%S", &tm_time ) ) _throw( err_fubar );

	return in_ignore_timezone ? timegm( &tm_time ) : mktime( &tm_time );
}

char *system_time_to_datetime( time_t in_system_time, char out_datetime[ 20 ], bool in_ignore_timezone ) {
	struct tm	tm_time;
	
	if ( in_ignore_timezone ) gmtime_r( &in_system_time, &tm_time );
	else localtime_r( &in_system_time, &tm_time );

	strftime( out_datetime, 20, "%Y-%m-%d %H:%M:%S", &tm_time );
	
	return out_datetime;
}
#endif // ENABLE_DATETIME_CONVERTERS


#if ENABLE_DUP_FUNCTIONS
void *dup_mem( const void *in_buffer, __u32 in_length ) {
	if ( ! in_buffer ) return nil;

	void				   *result;
	
	result = new char[ in_length ];
	memcpy( result, in_buffer, in_length );
	return result;
}


char *dup_string( const char *in_string ) {
	if ( ! in_string ) return nil;

	size_t					n;
	char				   *result;
	
	result = new char[ n = strlen( in_string ) + 1 ];
	memcpy( result, in_string, n + 1 );
	
	return result;
}


__s32 dup_string_to( const char *in_string, char *&out_destination, bool in_delete_out_destination_first ) {
	__s32			i;

	if ( in_delete_out_destination_first && out_destination != nil ) delete[] out_destination;
	
	if ( in_string ) {
		out_destination = new char[ i = (__s32) strlen( in_string ) + 1 ];
		strncpy( out_destination, in_string, i );
	} else {
		out_destination = nil;
	}
	
	return i;
}
#endif // ENABLE_DUP_FUNCTIONS


#if ENABLE_FREAD_FWRITE_WRAPPERS
size_t _fread( void *out_buffer, size_t in_size, FILE *in_stream ) {
	size_t					result;
	
	if ( ! ( result = fread( out_buffer, 1, in_size, in_stream ) ) ) {
		if ( ferror( in_stream ) ) {
			if ( errno ) _throw_errno();
			else _throw( err_read_failure );
		}
	}

	return result;
}


void _fwrite( const void *in_buffer, size_t in_size, FILE *in_stream ) {
	if ( fwrite( in_buffer, 1, in_size, in_stream ) != in_size ) {
		if ( errno ) _throw_errno();
		else _throw( err_write_failure );
	}
}
#endif // ENABLE_FREAD_FWRITE_WRAPPERS


#if ENABLE_HEX_CONVERTERS
void *bytes_from_hex_string( const char *in_hex_string, void *out_buffer, __u32 *out_length ) {
	__u8				c, d;
	__u32				i, j, l, n;
	__u8			   *p;

	if ( ( n = (__u32) strlen( in_hex_string ) ) & 1 ) _throw( err_bad_data );

	p = new __u8[ l = n / 2 ];

	_try {
		for ( i = j = 0; i < n; ) {
			c = in_hex_string[ i++ ];
			
			if ( c >= '0' && c <= '9' ) c -= '0';
			else if ( c >= 'a' && c <= 'f' ) c = c - 'a' + 10;
			else if ( c >= 'A' && c <= 'F' ) c = c - 'A' + 10;
			else _throw( err_bad_data );

			d = in_hex_string[ i++ ];
			
			if ( d >= '0' && d <= '9' ) d -= '0';
			else if ( d >= 'a' && d <= 'f' ) d = d - 'a' + 10;
			else if ( d >= 'A' && d <= 'F' ) d = d - 'A' + 10;
			else _throw( err_bad_data );
			
			p[ j++ ] = c << 4 | d;
		}
	} _catch
	
	_if_err delete[] p;

	_throw_now();
	
	if ( out_buffer ) {
		memcpy( out_buffer, p, l );
		delete[] p;
		p = (__u8 *) out_buffer;
	}
	
	if ( out_length ) *out_length = l;

	_return p;
}


char *hex_string_from_bytes( const void *in_buffer, __u32 in_length, char *out_string ) {
	char				   *buffer, *result;
	__u32					i, n;
	
	buffer = new char[ n = in_length * 2 + 1 ];

	for ( i = 0; i < in_length; ++i ) {
		hexify( ((char *) in_buffer)[ i ], buffer[ i * 2 ], buffer[ i * 2 + 1 ] );
	}
	buffer[ i * 2 ] = 0;

	if ( out_string ) {
		memcpy( out_string, buffer, n );
		delete[] buffer;
		result = out_string;
	} else {
		result = buffer;
	}

	return result;
}
#endif // ENABLE_HEX_CONVERTERS


#if ENABLE_POSIX_PATH_TO_FSSPEC
void posix_path_to_fsspec( const char *in_path, FSSpec &out_spec ) {
	FSRef		fsr;
	
	_throw_macerr_if( FSPathMakeRef( reinterpret_cast<const __u8 *>(in_path), &fsr, nil ) );
	_throw_macerr_if( FSGetCatalogInfo( &fsr, kFSCatInfoNone, nil, nil, &out_spec, nil ) );
}
#endif // ENABLE_POSIX_PATH_TO_FSSPEC
