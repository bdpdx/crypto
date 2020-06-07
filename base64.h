#ifndef __base64_h__
#define __base64_h__



#if _WIN32
	#include "precompiled.h"
#endif

#ifndef __balance_types
	typedef unsigned char			__u8;
	typedef unsigned int			__u32;
#endif


#define k_base64_bits_per_char		6


// This class implements BASE-64 as specified by RFC 3548 with additional
// encodings.  Particularly, RFC 3548 specifies two encoding methods: the
// first (k_base64_default_encoding) uses the '+' and '/' characters to
// represent certain bit sequences.  This encoding is not suitable for URLs
// and UNIX filesystem paths, so RFC 3548 specifies a second encoding
// (k_base64_url_and_filename_safe_encoding) that replaces the '+' and '/'
// characters with the '-' and '_' characters.  Both of these encodings
// however use the '=' character to pad byte streams, which makes them
// incompatible with CGI arguments passed in HTTP GET requests (since the
// '=' character is used as a key/value delimiter).  For this reason we provide
// a third encoding (k_base64_cgi_safe_encoding) that is identical to the
// URL and filename safe encoding except that the '='character is replaced
// by the '.' character which allows the encoding to operate with CGIs.


enum base64_encoding {
	k_base64_default_encoding									,
	k_base64_cgi_safe_encoding									,
	k_base64_url_and_filename_safe_encoding						,
	
	// These shorter constants are provided for convenience
	k_b64_cgi	=	k_base64_cgi_safe_encoding					,
	k_b64_url	=	k_base64_url_and_filename_safe_encoding
};


inline bool is_valid_base64( char in_base64, base64_encoding in_encoding ) {
	return in_base64 >= 'A' && in_base64 <= 'Z' ||
		   in_base64 >= 'a' && in_base64 <= 'z' ||
		   in_base64 >= '0' && in_base64 <= '9' ||
		 ( in_encoding == k_base64_default_encoding ?
		   in_base64 == '+' || in_base64 == '/' || in_base64 == '=' :
		   in_base64 == '-' || in_base64 == '_' ||
		 ( in_encoding == k_base64_cgi_safe_encoding ?
		   in_base64 == '.' : in_base64 == '=' ) );
}


struct base64 {

	// Allocates and returns a C-string containing the base-64 representation
	// of the byte-stream in_data of length in_length.  Caller is responsible
	// for delete[]ing the returned string.
	static char *encode( __u8 *in_data, __u32 in_length, base64_encoding in_encoding = k_base64_default_encoding );

	// Allocates and returns a byte stream consisting of the binary data
	// encoded in the base-64 C-string in_base64_string.  The length of
	// the byte stream is returned in via out_length.  Caller is responsible
	// for delete[]ing the returned byte stream.
	static __u8 *decode( const char *in_base64_string, __u32 &out_length, base64_encoding in_encoding = k_base64_default_encoding );

	// Converts an integer 0 <= in_bits <= 63 into its base-64 encoding.
	static char base64_for_bits( __u8 in_bits, base64_encoding in_encoding = k_base64_default_encoding );

	// Converts a base-64 encoded character into an integer value 0 <= result <= 63.
	static __u8 bits_for_base64( char in_base64, base64_encoding in_encoding = k_base64_default_encoding );

};



#endif // __base64_h__
