/*----------------------------------------------------------------------------------------*\

	! BALANCE SOFTWARE CONFIDENTIAL !
	
	Copyright (c) 2004 Balance Software Corporation
	All Rights Reserved.
	
	NOTICE:
	
		All information contained herein is, and remains the property of,
		Balance Software Corporation and its suppliers, if any.  The
		intellectual and technical concepts contained herein are proprietary
		to Balance Software Corporation and its suppliers and may be covered
		by U.S. and foreign patents or patents in process, and are protected
		by trade secret and copyright law.  Dissemination of this information,
		reproduction, or use of this material, whether in whole or in part,
		is strictly forbidden unless prior permission is obtained in writing
		from a duly authorized officer of Balance Software Corporation.
		

	File:				sha.h

	Author:				Brian Doyle
	Date Created:		February 05, 2004
	Last Modified:		September 17, 2007

	Description:

	An implementation of the four SHA (Secure Hash Algorithm) algorithms (SHA-1, 
	SHA-256, SHA-384, and SHA-512) defined by the Secure Hash Standard FIPS-180-2.

	SHA-384 and SHA-512 are enabled by default and as such this library requires
	linking to my apn libraries.  To build without the apn dependency, disable
	SHA-384 and SHA-512 by defining ENABLE_SHA512 as zero at some global scope.

\*----------------------------------------------------------------------------------------*/
#ifndef __sha_h__
#define __sha_h__



#if _WIN32
	#include "precompiled.h"
#endif

#if ENABLE_SHA512
	#include "apn.h"
#endif

#include "octet_string.h"


enum sha_algorithm_type {
	k_sha1									,
	k_sha256								,
	k_sha384								,
	k_sha512								
};


enum sha_hash_bytes {
	k_sha1_20			=		20			,
	k_sha256_32			=		32			,
	k_sha384_48			=		48			,
	k_sha512_64			=		64
};


enum sha_vectors {
	k_sha1_init_vectors						,
	k_sha256_init_vectors					,
	k_sha384_init_vectors					,
	k_sha512_init_vectors					,
	k_sha1_constant_vectors					,
	k_sha256_constant_vectors				,
	k_sha512_constant_vectors
};

// usage:
//
//	when the message to be hashed is completely represented in memory, it is
//  easiest to call hash( message, out_hash ).  if the message is too large
//	to be represented in memory or it is more covenient to build the hash
//	from components, instantiate an sha object (or call init on an existing
//	one) and then call hash().

class sha {

public:

	// in_sha_algorithm is the sha algorithm to use and should be one of:
	//
	//		{ k_sha1, k_sha256, k_sha384, k_sha512 }
	sha( sha_algorithm_type in_sha_algorithm = k_sha1 );
   ~sha();

	// resets the sha block to perform a new hash calculation with the specified
	// algorithm.  this method is implicitly called during construction.
	//
	// in_sha_algorithm is the sha algorithm to use and should be one of:
	//
	//		{ k_sha1, k_sha256, k_sha384, k_sha512 }
	void init( sha_algorithm_type in_sha_algorithm = k_sha1 );

	// for all message parts other than the last, in_message.length *must* be a
	// multiple of 64 (for sha1/sha256) or 128 (for sha384/sha512).  if not, or
	// if init() is not called before the first push(), the results are undefined.
	//
	// the in_check_range parameter is used internally by the sha class, always
	// pass true.
	void push( const octet_string &in_message, bool in_check_range = true );

	// returns the hash value of all the parts processed so far.  the length of the
	// result will vary depending on the algorithm in use, and will correspond to:
	//
	// sha1:	20 bytes
	// sha256:	32 bytes
	// sha384:	48 bytes
	// sha512:	64 bytes
	//
	// note that once this method has been called, the behavior of further push()'s
	// without an intervening init() is undefined.
	const __u8 *hash( __u8 *out_hash = nil );

	// copies the hash as a c-string of (lowercase) hex digits to out_string. out_hash
	// must point to a region of memory at least ( hash_size( sha_algorithm_type ) * 2 + 1 )
	// bytes long.  hash() is implicitly invoked by this method.  out_hash is returned.
	char *hash_string( char *out_string );

	// this method can be used if the message to be hashed is entirely specified
	// by in_message (i.e. not multipart).
	static const __u8 *hash( const octet_string &in_message, __u8 *out_hash, sha_algorithm_type in_sha_algorithm = k_sha1 );

	// hashes a c string
	static const __u8 *hash( const char *in_c_string, __u8 *out_hash, sha_algorithm_type in_sha_algorithm = k_sha1 );

	// hashes a file on disk
	static const __u8 *hash( const __u8 *in_path_to_file, __u8 *out_hash, sha_algorithm_type in_sha_algorithm = k_sha1 );

	// hashes an arbitrary region of memory
	static const __u8 *hash( const void *in_buffer, __u32 in_length, __u8 *out_hash, sha_algorithm_type in_sha_algorithm = k_sha1 );

	// returns the number of bytes that a hash occupies for the given sha algorithm
	static __u32 hash_size( sha_algorithm_type = k_sha1 );

	// returns a c-string describing in_sha_algorithm
	static const char *algorithm_name( sha_algorithm_type in_sha_algorithm );

	// copies a __u8 *in_hash to out_string as a c-string of (lowercase) hex digits. out_hash
	// must point to a region of memory at least ( hash_size( sha_algorithm_type ) * 2 + 1 )
	// bytes long.  out_hash is returned.  in_hash and out_string may overlap.
	static char *hash_string( const __u8 *in_hash, char *out_string, sha_algorithm_type in_sha_algorithm = k_sha1 );

	static void xor_vectors( sha_vectors in_vectors, const void *in_vectors_data );

protected:

	void pad( octet_string &io_message );
	
	void init_sha1();
	void init_sha256();

	void delete_hash();

	void hash_sha1( const octet_string &in_message, __u32 in_blocks );
	void hash_sha256( const octet_string &in_message, __u32 in_blocks );

	__u32 ch( __u32 in_x, __u32 in_y, __u32 in_z );
	__u32 maj( __u32 in_x, __u32 in_y, __u32 in_z );
	__u32 parity( __u32 in_x, __u32 in_y, __u32 in_z );

	__u32 sha256_sigma_0( __u32 in_x );
	__u32 sha256_sigma_1( __u32 in_x );
	__u32 sha256_sigma_2( __u32 in_x );
	__u32 sha256_sigma_3( __u32 in_x );

	bool					m_done;
	__u32				   *m_hash;
	__u64					m_size;
	octet_string		   *m_remainder;
	sha_algorithm_type		m_algorithm;

#if ENABLE_SHA512
	void init_sha384();
	void init_sha512();
	
	void hash_sha512( const octet_string &in_message, __u32 in_blocks );

	__u64 ch64( __u64 in_x, __u64 in_y, __u64 in_z );
	__u64 maj64( __u64 in_x, __u64 in_y, __u64 in_z );

	__u64 sha512_sigma_0( __u64 in_x );
	__u64 sha512_sigma_1( __u64 in_x );
	__u64 sha512_sigma_2( __u64 in_x );
	__u64 sha512_sigma_3( __u64 in_x );

	apn						m_size_128;
#endif

};


inline char *sha::hash_string( char *out_hash ) {
	return hash_string( hash(), out_hash, m_algorithm );
}

inline __u32 sha::hash_size( sha_algorithm_type in_sha_algorithm ) {
	switch ( in_sha_algorithm ) {
		case k_sha1:		return 20;
		case k_sha256:		return 32;
		case k_sha384:		return 48;
		case k_sha512:		return 64;
		
		default:			_throw( err_unimplemented );
	}
}

inline const char *sha::algorithm_name( sha_algorithm_type in_sha_algorithm ) {
	switch ( in_sha_algorithm ) {
		case k_sha1:		return "sha-1";
		case k_sha256:		return "sha-256";
		case k_sha384:		return "sha-384";
		case k_sha512:		return "sha-512";
		
		default:			_throw( err_unimplemented );
	}
}

inline void sha::delete_hash() {
	__u32			i, n;

	if ( m_hash ) {
		for ( i = 0, n = hash_size( m_algorithm ) / sizeof(__u32); i < n; ++i ) m_hash[ i ] = 0;
		delete[] m_hash;
		m_hash = nil;
	}
}

inline __u32 sha::ch( __u32 in_x, __u32 in_y, __u32 in_z ) { return in_x & in_y ^ ~in_x & in_z; }
inline __u32 sha::maj( __u32 in_x, __u32 in_y, __u32 in_z ) { return in_x & in_y ^ in_x & in_z ^ in_y & in_z; }
inline __u32 sha::parity( __u32 in_x, __u32 in_y, __u32 in_z ) { return in_x ^ in_y ^ in_z; }

inline __u32 sha::sha256_sigma_0( __u32 in_x ) { return rotate_right_evaluated( 32, in_x, 2 ) ^ rotate_right_evaluated( 32, in_x, 13 ) ^ rotate_right_evaluated( 32, in_x, 22 ); }
inline __u32 sha::sha256_sigma_1( __u32 in_x ) { return rotate_right_evaluated( 32, in_x, 6 ) ^ rotate_right_evaluated( 32, in_x, 11 ) ^ rotate_right_evaluated( 32, in_x, 25 ); }
inline __u32 sha::sha256_sigma_2( __u32 in_x ) { return rotate_right_evaluated( 32, in_x, 7 ) ^ rotate_right_evaluated( 32, in_x, 18 ) ^ in_x >> 3; }
inline __u32 sha::sha256_sigma_3( __u32 in_x ) { return rotate_right_evaluated( 32, in_x, 17 ) ^ rotate_right_evaluated( 32, in_x, 19 ) ^ in_x >> 10; }

#if ENABLE_SHA512
inline __u64 sha::ch64( __u64 in_x, __u64 in_y, __u64 in_z ) { return in_x & in_y ^ ~in_x & in_z; }
inline __u64 sha::maj64( __u64 in_x, __u64 in_y, __u64 in_z ) { return in_x & in_y ^ in_x & in_z ^ in_y & in_z; }

inline __u64 sha::sha512_sigma_0( __u64 in_x ) { return rotate_right_evaluated( 64, in_x, 28 ) ^ rotate_right_evaluated( 64, in_x, 34 ) ^ rotate_right_evaluated( 64, in_x, 39 ); }
inline __u64 sha::sha512_sigma_1( __u64 in_x ) { return rotate_right_evaluated( 64, in_x, 14 ) ^ rotate_right_evaluated( 64, in_x, 18 ) ^ rotate_right_evaluated( 64, in_x, 41 ); }
inline __u64 sha::sha512_sigma_2( __u64 in_x ) { return rotate_right_evaluated( 64, in_x, 1 ) ^ rotate_right_evaluated( 64, in_x, 8 ) ^ in_x >> 7; }
inline __u64 sha::sha512_sigma_3( __u64 in_x ) { return rotate_right_evaluated( 64, in_x, 19 ) ^ rotate_right_evaluated( 64, in_x, 61 ) ^ in_x >> 6; }
#endif


#if ENABLE_FIPS_180_2_INPUT_VECTORS
	void test_sha_with_fips_180_2_example_input_vectors();
#endif

#if ENABLE_OPENSSL_COMPATIBILITY
extern "C" {
	void SHA1( __u8 *in_message, __u32 in_length, __u8 *out_hash );
}
#endif



#endif // __sha_hpp__
