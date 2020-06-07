#include "aes.h"
#include "crypto_random.h"


aes::aes( const void *in_key, __u32 in_key_bytes ) {
	init( in_key, in_key_bytes );
}


aes::~aes() {
	memset( erk, 0, sizeof(erk) );
	memset( drk, 0, sizeof(drk) );
	nr = 0;
}


void aes::init( const void *in_key, __u32 in_key_bytes ) {
	if ( ! ( in_key_bytes == 16 || in_key_bytes == 24 || in_key_bytes == 32 ) ) _throw( err_bad_parameter );

	aes_set_key( this, (unsigned char *) in_key, in_key_bytes * 8 );
}


void aes::encrypt( const void *in_data, __u32 in_length, __u8 *&out_data, __u32 &out_length ) {
	crypto_random			cr;
	__u8					data[ 3 * 16 ], *input, *result;
	__u32					n;

	if ( ! ( in_data && in_length ) ) _throw( err_bad_parameter );

	// the first 16 bytes of data will be the cbc iv.
	// the next 32 bytes of data will be used to initialize the last two
	// data blocks of the input vector.
	cr.random( data, sizeof(data) );

	// our input data will be a concatenation (in order) of:
	//
	// x blocks, where x is the minimum number of blocks in_data will fit in
	// 1 block of random data in which the first byte is the number of bytes of real data
	// in the final block.
	n = ( ( in_length + 15 ) / 16 + 1 ) * 16;

	input = new __u8[ n ];
	try {
		result = new __u8[ out_length = 16 + n ];	// prepend the iv
	} catch ( ... ) {
		delete[] input;
		throw;
	}

	memcpy( &input[ n - 32 ], &data[ 16 ], 32 );
	memcpy( input, in_data, in_length );
	memcpy( result, data, 16 );
	
	// store the number of extraneous data bytes in the final data block to the
	// first byte of the extra block of random data at the end of the input vector.
	if ( ( input[ n - 16 ] = 16 - in_length % 16 ) == 16 ) input[ n - 16 ] = 0;

	// the output data will a concatenation of the iv and the ciphertext
	aes_cbc_encrypt( this, data, input, result + 16, n );
	
	memset( data, 0, sizeof(data) );
	memset( input, 0, n );
	
	delete[] input;
	
	out_data = result;
}


void aes::decrypt( const void *in_data, __u32 in_length, __u8 *&out_data, __u32 &out_length, __u32 in_out_data_pad_bytes ) {
	__u8				   *result;

	if ( ! ( in_data && in_length >= 48 ) || in_length % 16 ) _throw( err_bad_parameter );
	
	result = new __u8[ ( in_length -= 16 ) + in_out_data_pad_bytes ];

	aes_cbc_decrypt( this, (__u8 *) in_data, (__u8 *) in_data + 16, result, in_length );
	
	out_length = in_length - 16;
	out_length -= result[ out_length ];

	if ( in_out_data_pad_bytes ) memset( &result[ out_length ], 0, in_out_data_pad_bytes );

	out_data = result;
}
