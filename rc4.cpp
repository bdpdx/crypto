#include "rc4.h"


rc4::rc4( const void *in_key, __u32 in_length ) {
	init( in_key, in_length );
}


rc4::rc4( __u8 *in_s_box, __u8 in_i, __u8 in_j ) {
	_i = in_i;
	_j = in_j;
	memcpy( _s_box, in_s_box, k_key_bytes );
}


rc4::rc4( const rc4 &in_rc4 ) {
	*this = in_rc4;
}


rc4::~rc4() {
	_i = _j = 0;
	memset( _s_box, 0, k_key_bytes );
}


rc4 &rc4::operator=( const rc4 &in_rc4 ) {
	_i = in_rc4._i;
	_j = in_rc4._j;
	memcpy( _s_box, in_rc4._s_box, k_key_bytes );
	
	return *this;
}


void rc4::crypt( void *io_data, __u32 in_length ) {
	__u8					c, d, i, j;
	__u32					k;

	if ( ! ( io_data && in_length ) ) return;

	for ( k = 0; k < in_length; ++k ) {
		i = ++_i;
		j = _j += c = _s_box[ i ];
		
		_s_box[ i ] = d = _s_box[ j ];
		_s_box[ j ] = c;
		
		((__u8 *) io_data)[ k ] ^= _s_box[ c += d ];
	}
	
	c = d = 0;
	i = j = 0;
}


void rc4::init( const void *in_key, __u32 in_length ) {
	__u8					c, key[ k_key_bytes ];
	__u32					i, j;

	for ( i = 0; i < k_key_bytes; ++i ) _s_box[ i ] = i;

	for ( i = j = 0; i < k_key_bytes; ++i ) {
		key[ i ] = ((const char *) in_key)[ i % in_length ];

		j = ( j + ( c = _s_box[ i ] ) + key[ i ] ) % k_key_bytes;

		_s_box[ i ] = _s_box[ j ];
		_s_box[ j ] = c;
	}

	_i = _j = 0;

	memset( key, 0, sizeof(key) );
	c = 0;
}
