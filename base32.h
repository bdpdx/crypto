#ifndef __base32_h__
#define __base32_h__



#if _WIN32
	#include "precompiled.h"
#endif

#ifndef __balance_types
	typedef unsigned char			__u8;
	typedef unsigned int			__u32;
#endif


#define k_base32_bits_per_char		5


inline bool is_valid_base32( char in_base32 ) {
#if defined( ENABLE_BASE32_SWEARWORD_SAFE )
	return in_base32 >= 'B' && in_base32 <= 'Z' && in_base32 != 'I' && in_base32 != 'O' && in_base32 != 'U' || in_base32 >= '0' && in_base32 <= '9';
#else
	return in_base32 >= 'A' && in_base32 <= 'Z' || in_base32 >= '2' && in_base32 <= '7';
#endif
}


// see RFC 3548

struct base32 {

	// returns a c-string corresponding to the base32 encoding of byte-
	// stream in_data of length in_length bytes.  caller is responsible
	// for delete[]ing the returned string.
	static char *encode( __u8 *in_data, __u32 in_length );

	// takes a c-string corresponding to the base32 encoding of a byte-
	// stream and returns the binary data and the length of the data in
	// out_length.  caller must delete[] the returned buffer.
	static __u8 *decode( const char *in_base32_string, __u32 &out_length );

	// these two strings transform an integer value 0 <= in_bits < 32 into the
	// base32 encoding or vice versa.
	static char base32_for_bits( __u8 in_bits );
	static __u8 bits_for_base32( char in_base32 );

};



#endif // __base32_h__
