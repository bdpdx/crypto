#include "mysql_accessor.h"


mysql_accessor::mysql_accessor() {
	if ( ! ( _sql = mysql_init( nil ) ) ) _throw( err_mem_full );
}


mysql_accessor::~mysql_accessor() {
	if ( _sql ) mysql_close( _sql );
}


#pragma mark -


void mysql_accessor::connect_host(
	const char			   *in_user,
	const char			   *in_password,
	const char			   *in_host,
	const char			   *in_database,
	__u32					in_port,
	__u32					in_flags )
{
	struct stat				sb;

	if ( ! ( strcmp( in_host, "localhost" ) && strcmp( in_host, "127.0.0.1" ) ) ) {
		if ( ! stat( "/tmp/mysql.sock", &sb ) ) {
			try {
				connect_socket( in_user, in_password, in_database, in_flags );
				return;
			} catch ( ... ) { }
		}
	}

//	This doesn't seem to work right.  Use stunnel for now...
//	mysql_ssl_set( _sql, nil, nil, nil, nil, nil );

	if ( ! mysql_real_connect( _sql, in_host, in_user, in_password, in_database, in_port, nil, in_flags ) ) {
		_throw( err_could_not_connect );
	}
}


void mysql_accessor::connect_socket(
	const char	   *in_user,
	 const char	   *in_password,	
	 const char	   *in_database,
	 __u32			in_flags,
	 const char	   *in_socket )
{
	if ( ! mysql_real_connect( _sql, nil, in_user, in_password, in_database, 0, in_socket, in_flags ) ) {
		_throw( err_could_not_connect );
	}
}


char *mysql_accessor::escape( const void *in_data, __u32 in_length ) {
	char				   *buffer, c;
	__u32					i, j, n;

	// determine the size the properly-escaped data buffer will grow by
	for ( i = n = 0; i < in_length; ++i ) {
		switch ( ((char *) in_data)[ i ] ) {
			case '\\':
			case '\'':
			case '"':
			case '\0':
			case k_ascii_control_z: {
				++n;
			} break;
		}
	}
	
	// allocate space for properly-escaped data
	buffer = new char[ in_length + n + 1 ];

	// copy properly-escaped data to the string
	for ( i = j = 0; i < in_length; ++i ) {
		switch ( ( c = ((char *) in_data)[ i ] ) ) {
			case '\\':
			case '\'':
			case '"':
			case k_ascii_control_z: {
				buffer[ j++ ] = '\\';
			} break;
			
			 case '\0': {
			 	buffer[ j++ ] = '\\';
			 	buffer[ j++ ] = '0';
			 } continue;
		}
		buffer[ j++ ] = c;
	}

	// terminate the string
	buffer[ j ] = '\0';

	return buffer;
}


__u32 mysql_accessor::query( const char *in_format, ... ) {
	va_list					args;
	const char			   *error;
	__u32					result;
	MYSQL_RES			   *rows;
	char				   *string;
	
	va_start( args, in_format );
	if ( vasprintf( &string, in_format, args ) == -1 ) _throw( err_mem_full );
	va_end( args );
	
	_try {
		if ( mysql_query( _sql, string ) ) {
			_throw_msg( mysql_errno( _sql ), mysql_error( _sql ) );
		}

		if ( ! ( rows = mysql_store_result( _sql ) ) ) {
			if ( ( error = mysql_error( _sql ) ) ) {
				_throw_msg( mysql_errno( _sql ), error );
			} else {
				result = 0;
			}
		} else {
			result = (__u32) mysql_num_rows( rows );
			mysql_free_result( rows );
		}
	} _catch	
	
	free( string );
	
	_return result;
}


__u32 mysql_accessor::query( MYSQL_RES *&out_result, const char *in_format, ... ) {
	va_list					args;
	char				   *string;
	
	va_start( args, in_format );
	if ( vasprintf( &string, in_format, args ) == -1 ) _throw( err_mem_full );
	va_end( args );
	
	_try {
		if ( mysql_query( _sql, string ) ) {
			_throw_msg( mysql_errno( _sql ), mysql_error( _sql ) );
		}

		if ( ! ( out_result = mysql_store_result( _sql ) ) ) {
			_throw_msg( mysql_errno( _sql ), mysql_error( _sql ) );
		}
	} _catch
	
	free( string );
	
	_return (__u32) mysql_num_rows( out_result );
}


__u32 mysql_accessor::query( const char *in_prefix_format, const void *in_data, __u32 in_length, const char *in_suffix_format, ... ) {
	va_list					args;
	char				   *buffer, c, *format, *string;
	const char			   *error;
	__u32					i, j, k, l, n, o, result;
	MYSQL_RES			   *rows;

	buffer = nil;
	format = nil;

	// determine the length of the expanded prefix string
	va_start( args, in_suffix_format );
	if ( __s32( k = vasprintf( &string, in_prefix_format, args ) ) == -1 ) _throw( err_mem_full );
	va_end( args );
	
	free( string ); string = nil;
	
	n = strlen( in_prefix_format );
	o = strlen( in_suffix_format );
	
	_try {
		format = new char[ n + o + 1 ];
		
		memcpy( format, in_prefix_format, n );
		memcpy( format + n, in_suffix_format, o );
		
		format[ n + o ] = '\0';
		
		// expand the prefix and suffix strings into one buffer
		va_start( args, in_suffix_format );
		if ( __s32( l = vasprintf( &string, format, args ) ) == -1 ) _throw( err_mem_full );
		va_end( args );
		
		// determine the length of the expanded suffix string
		l -= k;

		// determine the size the properly-escaped data buffer will grow by
		for ( i = n = 0; i < in_length; ++i ) {
			switch ( ((char *) in_data)[ i ] ) {
				case '\\': case '\'': case '"': case '\0': {
					++n;
				} break;
			}
		}
	
		// allocate space for properly-escaped data
		buffer = new char[ k + in_length + n + l + 1 ];

		// copy expanded prefix to query string
		memcpy( buffer, string, k );
		
		// copy properly-escaped data to query string
		for ( i = 0, j = k; i < in_length; ++i ) {
			switch ( ( c = ((char *) in_data)[ i ] ) ) {
				case '\\': case '\'': case '"': {
					buffer[ j++ ] = '\\';
				} break;
				
				 case '\0': {
				 	buffer[ j++ ] = '\\';
				 	buffer[ j++ ] = '0';
				 } continue;
			}
			buffer[ j++ ] = c;
		}
		
		// copy expanded suffix to query string
		memcpy( buffer + j, string + k, l );

		// terminate query string
		buffer[ j + l ] = '\0';

		if ( mysql_query( _sql, buffer ) ) {
			_throw_msg( mysql_errno( _sql ), mysql_error( _sql ) );
		}

		if ( ! ( rows = mysql_store_result( _sql ) ) ) {
			if ( ( error = mysql_error( _sql ) ) ) {
				_throw_msg( mysql_errno( _sql ), error );
			} else {
				result = 0;
			}
		} else {
			result = (__u32) mysql_num_rows( rows );
			mysql_free_result( rows );
		}
	} _catch

	if ( string ) free( string );
	
	delete[] format;
	delete[] buffer;

	_return result;
}


__u32 mysql_accessor::query( MYSQL_RES *&out_result, const char *in_prefix_format, const void *in_data, __u32 in_length, const char *in_suffix_format, ... ) {
	va_list					args;
	char				   *buffer, c, *format, *string;
	__u32					i, j, k, l, n, o;

	buffer = nil;
	format = nil;

	// determine the length of the expanded prefix string
	va_start( args, in_suffix_format );
	if ( __s32( k = vasprintf( &string, in_prefix_format, args ) ) == -1 ) _throw( err_mem_full );
	va_end( args );
	
	free( string ); string = nil;
	
	n = strlen( in_prefix_format );
	o = strlen( in_suffix_format );
	
	_try {
		format = new char[ n + o + 1 ];
		
		memcpy( format, in_prefix_format, n );
		memcpy( format + n, in_suffix_format, o );
		
		format[ n + o ] = '\0';
		
		// expand the prefix and suffix strings into one buffer
		va_start( args, in_suffix_format );
		if ( __s32( l = vasprintf( &string, format, args ) ) == -1 ) _throw( err_mem_full );
		va_end( args );
		
		// determine the length of the expanded suffix string
		l -= k;

		// determine the size the properly-escaped data buffer will grow by
		for ( i = n = 0; i < in_length; ++i ) {
			switch ( ((char *) in_data)[ i ] ) {
				case '\\': case '\'': case '"': case '\0': {
					++n;
				} break;
			}
		}
	
		// allocate space for properly-escaped data
		buffer = new char[ k + in_length + n + l + 1 ];

		// copy expanded prefix to query string
		memcpy( buffer, string, k );
		
		// copy properly-escaped data to query string
		for ( i = 0, j = k; i < in_length; ++i ) {
			switch ( ( c = ((char *) in_data)[ i ] ) ) {
				case '\\': case '\'': case '"': {
					buffer[ j++ ] = '\\';
				} break;
				
				 case '\0': {
				 	buffer[ j++ ] = '\\';
				 	buffer[ j++ ] = '0';
				 } continue;
			}
			buffer[ j++ ] = c;
		}
		
		// copy expanded suffix to query string
		memcpy( buffer + j, string + k, l );

		// terminate query string
		buffer[ j + l ] = '\0';
		
		if ( mysql_query( _sql, buffer ) ) {
			_throw_msg( mysql_errno( _sql ), mysql_error( _sql ) );
		}

		// this step is optional...  we could be done after the query
		// but maybe the query returned a result set.  in this case
		// get it here.
		if ( ! ( out_result = mysql_store_result( _sql ) ) ) {
			_throw_msg( mysql_errno( _sql ), mysql_error( _sql ) );
		}
	} _catch

	if ( string ) free( string );
	
	delete[] format;
	delete[] buffer;
		
	_return (__u32) mysql_num_rows( out_result );
}
