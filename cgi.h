#ifndef __cgi_h__
#define __cgi_h__



// http methods
#define k_method_get			"GET"
#define k_method_post			"POST"

// environment strings set by apache
#define k_content_length		"CONTENT_LENGTH"
#define k_query_string			"QUERY_STRING"
#define k_request_method		"REQUEST_METHOD"

#define k_cgi_reply_prefix		"Content-type: text/plain; charset=iso-8859-1\n\n"


class cgi {

public:

	cgi( bool in_perform_case_sensitive_comparisons = false, bool in_sort_keys = true );
   ~cgi();
	
	err_t init( FILE *in_fp = stdin, bool in_remove_leading_and_trailing_whitespace = true );

	err_t get_key_and_value_by_index( long in_index, char *&out_key, char *&out_value );
	err_t get_value_by_key( char *in_key, char *&out_value, long *out_index = nil );
 
	long					_entries;		// number of key/value pairs
	char				  **_keys;
	char				  **_values;
	
protected:

	void done();

	__u32					_case_sensitive		:	1;
	__u32					_sort_keys			:	1;

};



#endif // __cgi_h__
