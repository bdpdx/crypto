#ifndef __mysql_accessor_h__
#define __mysql_accessor_h__



#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>

#include "octet_string.h"
#include "queue.h"


class mysql_accessor {

public:

	mysql_accessor();
	virtual ~mysql_accessor();

	void connect_host(
		const char		   *in_user,
		const char		   *in_password,	
		const char		   *in_host = "localhost",
		const char		   *in_database = nil,
		__u32				in_port = 3306,
		__u32				in_flags = 0 );
				  
	void connect_socket(
		const char		   *in_user,
		const char		   *in_password,	
		const char		   *in_database = nil,
		__u32				in_flags = 0,
		const char		   *in_socket = "/tmp/mysql.sock" );

	// allocates a new string containing in_data with certain characters
	// escaped for mysql text compatibility.  caller is responsible for
	// delete[]'ing the returned result.
	char *escape( const void *in_data, __u32 in_length );

	// returns 0 for statements that don't select data, otherwise returns
	// the number of rows in the result set.
	__u32 query( const char *in_format, ... );

	// returns the number of rows in the result set
	__u32 query( MYSQL_RES *&out_result, const char *in_format, ... );

	// these forms of the query() command are similar to the above, but allow
	// binary (blob) data to be utilized.  the data in in_data will be converted
	// to a properly escaped text sequence and then a query will be sent having
	// the form:
	//
	// in_prefix escaped-in_data in_suffix
	//
	// example:  query( "update table set field='", my_data, my_length, "' where whatever" );
	//			 note the enclosing single-quotes around the BLOB data.  these routines do NOT
	//			 automatically insert them in order to preserve flexibility by they should
	//			 *always* be utilized.
	__u32 query( const char *in_prefix_format, const void *in_data, __u32 in_length, const char *in_suffix_format, ... );
	__u32 query( MYSQL_RES *&out_result, const char *in_prefix_format, const void *in_data, __u32 in_length, const char *in_suffix_format, ... );

	__u32 last_insert_id() { return (__u32) mysql_insert_id( _sql ); }

	void free_rows( MYSQL_RES *&io_rows );
	MYSQL_ROW next_row( MYSQL_RES *&in_rows, _ul32 **out_field_lengths = nil );

	const char *error_message() { return mysql_error( _sql ); }

protected:

	MYSQL				   *_sql;

};


inline void mysql_accessor::free_rows( MYSQL_RES *&io_rows ) {
	if ( io_rows ) {
		mysql_free_result( io_rows );
		io_rows = nil;
	}
}


inline MYSQL_ROW mysql_accessor::next_row( MYSQL_RES *&in_rows, _ul32 **out_field_lengths ) {
	MYSQL_ROW				result;

	if ( ! ( result = mysql_fetch_row( in_rows ) ) ) {
		_throw_msg( err_query_failed, mysql_error( _sql ) );
	}
	
	if ( out_field_lengths ) *out_field_lengths = mysql_fetch_lengths( in_rows );
	
	return result;
}



#endif // __mysql_accessor_hpp__
