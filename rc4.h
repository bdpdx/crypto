#ifndef __rc4_h__
#define __rc4_h__



#if _WIN32
	#include "precompiled.h"
#endif


class rc4 {

public:

	enum { k_key_bytes = 256 };

	rc4() { }
	rc4( const void *in_key, __u32 in_length = k_key_bytes );
	rc4( __u8 *in_s_box, __u8 in_i, __u8 in_j );
	rc4( const rc4 &in_rc4 );
	
	rc4 &operator=( const rc4 &in_rc4 );
	
   ~rc4();
	
	void init( const void *in_key, __u32 in_length );

	void crypt( void *io_data, __u32 in_length );
	
	__u8					_i;
	__u8					_j;
	__u8					_s_box[ k_key_bytes ];

};



#endif // __rc4_h__
