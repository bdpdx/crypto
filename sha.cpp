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
		

	File:				sha.cpp

	Author:				Brian Doyle
	Date Created:		February 05, 2004
	Last Modified:		February 05, 2004

	Description:

	An implementation of the four SHA (Secure Hash Algorithm) algorithms (SHA-1, 
	SHA-256, SHA-384, and SHA-512) defined by the Secure Hash Standard FIPS-180-2.
	
	Debugging code with input vectors that match the FIPS-180-2 example data can
	be found in the comments at the end of the file.

\*----------------------------------------------------------------------------------------*/
#include "sha.h"


enum {
	k_sha1_block_bytes		=		64		,
	k_sha512_block_bytes	=		128
};


static __u32 s_sha1_init_vectors[ 5 ] = {
	0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0
};

static __u32 s_sha256_init_vectors[ 8 ] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

#if ENABLE_SHA512
static __u64 s_sha384_init_vectors[ 8 ] = {
	0xcbbb9d5dc1059ed8ull, 0x629a292a367cd507ull, 0x9159015a3070dd17ull, 0x152fecd8f70e5939ull,
	0x67332667ffc00b31ull, 0x8eb44a8768581511ull, 0xdb0c2e0d64f98fa7ull, 0x47b5481dbefa4fa4ull
};

static __u64 s_sha512_init_vectors[ 8 ] = {
	0x6a09e667f3bcc908ull, 0xbb67ae8584caa73bull, 0x3c6ef372fe94f82bull, 0xa54ff53a5f1d36f1ull,
	0x510e527fade682d1ull, 0x9b05688c2b3e6c1full, 0x1f83d9abfb41bd6bull, 0x5be0cd19137e2179ull
};
#endif

static __u32 s_sha1_k_constants[ 4 ] = {
	0x5a827999,	0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6
};

static __u32 s_sha256_k_constants[ 64 ] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#if ENABLE_SHA512
static __u64 s_sha512_k_constants[ 80 ] = {
	0x428a2f98d728ae22ull, 0x7137449123ef65cdull, 0xb5c0fbcfec4d3b2full, 0xe9b5dba58189dbbcull,
	0x3956c25bf348b538ull, 0x59f111f1b605d019ull, 0x923f82a4af194f9bull, 0xab1c5ed5da6d8118ull,
	0xd807aa98a3030242ull, 0x12835b0145706fbeull, 0x243185be4ee4b28cull, 0x550c7dc3d5ffb4e2ull,
	0x72be5d74f27b896full, 0x80deb1fe3b1696b1ull, 0x9bdc06a725c71235ull, 0xc19bf174cf692694ull,
	0xe49b69c19ef14ad2ull, 0xefbe4786384f25e3ull, 0x0fc19dc68b8cd5b5ull, 0x240ca1cc77ac9c65ull,
	0x2de92c6f592b0275ull, 0x4a7484aa6ea6e483ull, 0x5cb0a9dcbd41fbd4ull, 0x76f988da831153b5ull,
	0x983e5152ee66dfabull, 0xa831c66d2db43210ull, 0xb00327c898fb213full, 0xbf597fc7beef0ee4ull,
	0xc6e00bf33da88fc2ull, 0xd5a79147930aa725ull, 0x06ca6351e003826full, 0x142929670a0e6e70ull,
	0x27b70a8546d22ffcull, 0x2e1b21385c26c926ull, 0x4d2c6dfc5ac42aedull, 0x53380d139d95b3dfull,
	0x650a73548baf63deull, 0x766a0abb3c77b2a8ull, 0x81c2c92e47edaee6ull, 0x92722c851482353bull,
	0xa2bfe8a14cf10364ull, 0xa81a664bbc423001ull, 0xc24b8b70d0f89791ull, 0xc76c51a30654be30ull,
	0xd192e819d6ef5218ull, 0xd69906245565a910ull, 0xf40e35855771202aull, 0x106aa07032bbd1b8ull,
	0x19a4c116b8d2d0c8ull, 0x1e376c085141ab53ull, 0x2748774cdf8eeb99ull, 0x34b0bcb5e19b48a8ull,
	0x391c0cb3c5c95a63ull, 0x4ed8aa4ae3418acbull, 0x5b9cca4f7763e373ull, 0x682e6ff3d6b2b8a3ull,
	0x748f82ee5defb2fcull, 0x78a5636f43172f60ull, 0x84c87814a1f0ab72ull, 0x8cc702081a6439ecull,
	0x90befffa23631e28ull, 0xa4506cebde82bde9ull, 0xbef9a3f7b2c67915ull, 0xc67178f2e372532bull,
	0xca273eceea26619cull, 0xd186b8c721c0c207ull, 0xeada7dd6cde0eb1eull, 0xf57d4f7fee6ed178ull,
	0x06f067aa72176fbaull, 0x0a637dc5a2c898a6ull, 0x113f9804bef90daeull, 0x1b710b35131c471bull,
	0x28db77f523047d84ull, 0x32caab7b40c72493ull, 0x3c9ebe0a15c9bebcull, 0x431d67c49c100d4cull,
	0x4cc5d4becb3e42b6ull, 0x597f299cfc657e2aull, 0x5fcb6fab3ad6faecull, 0x6c44198c4a475817ull
};
#endif


sha::sha( sha_algorithm_type in_sha_algorithm ) {
	m_hash = nil;
	m_remainder = nil;

	init( in_sha_algorithm );
}


sha::~sha() {
	delete_hash();
	delete m_remainder;
}


#if __MWERKS__
#pragma mark -
#endif


void sha::init( sha_algorithm_type in_sha_algorithm ) {
	m_done = false;
	
	delete_hash();
	_delete( m_remainder );

	switch ( in_sha_algorithm ) {
		case k_sha1:		init_sha1();					break;
		case k_sha256:		init_sha256();					break;
#if ENABLE_SHA512
		case k_sha384:		init_sha384();					break;
		case k_sha512:		init_sha512();					break;
#endif

		default:			_throw( err_unimplemented );
	}
}


void sha::init_sha1() {
	m_algorithm = k_sha1;
	
	m_size = 0;
	m_hash = new __u32[ 5 ];

	m_hash[ 0 ] = s_sha1_init_vectors[ 0 ];
	m_hash[ 1 ] = s_sha1_init_vectors[ 1 ];
	m_hash[ 2 ] = s_sha1_init_vectors[ 2 ];
	m_hash[ 3 ] = s_sha1_init_vectors[ 3 ];
	m_hash[ 4 ] = s_sha1_init_vectors[ 4 ];
}


void sha::init_sha256() {
	m_algorithm = k_sha256;
	
	m_size = 0;
	m_hash = new __u32[ 8 ];

	m_hash[ 0 ] = s_sha256_init_vectors[ 0 ];
	m_hash[ 1 ] = s_sha256_init_vectors[ 1 ];
	m_hash[ 2 ] = s_sha256_init_vectors[ 2 ];
	m_hash[ 3 ] = s_sha256_init_vectors[ 3 ];
	m_hash[ 4 ] = s_sha256_init_vectors[ 4 ];
	m_hash[ 5 ] = s_sha256_init_vectors[ 5 ];
	m_hash[ 6 ] = s_sha256_init_vectors[ 6 ];
	m_hash[ 7 ] = s_sha256_init_vectors[ 7 ];
}


#if ENABLE_SHA512
void sha::init_sha384() {
	__u64	   *m_hash_64;
	
	m_algorithm = k_sha384;
	
	m_size_128 = 0;
	m_hash = reinterpret_cast<__u32 *>(new __u64[ 8 ]);
	m_hash_64 = reinterpret_cast<__u64 *>(m_hash);

	m_hash_64[ 0 ] = s_sha384_init_vectors[ 0 ];
	m_hash_64[ 1 ] = s_sha384_init_vectors[ 1 ];
	m_hash_64[ 2 ] = s_sha384_init_vectors[ 2 ];
	m_hash_64[ 3 ] = s_sha384_init_vectors[ 3 ];
	m_hash_64[ 4 ] = s_sha384_init_vectors[ 4 ];
	m_hash_64[ 5 ] = s_sha384_init_vectors[ 5 ];
	m_hash_64[ 6 ] = s_sha384_init_vectors[ 6 ];
	m_hash_64[ 7 ] = s_sha384_init_vectors[ 7 ];
}


void sha::init_sha512() {
	__u64	   *m_hash_64;
	
	m_algorithm = k_sha512;
	
	m_size_128 = 0;
	m_hash = reinterpret_cast<__u32 *>(new __u64[ 8 ]);
	m_hash_64 = reinterpret_cast<__u64 *>(m_hash);

	m_hash_64[ 0 ] = s_sha512_init_vectors[ 0 ];
	m_hash_64[ 1 ] = s_sha512_init_vectors[ 1 ];
	m_hash_64[ 2 ] = s_sha512_init_vectors[ 2 ];
	m_hash_64[ 3 ] = s_sha512_init_vectors[ 3 ];
	m_hash_64[ 4 ] = s_sha512_init_vectors[ 4 ];
	m_hash_64[ 5 ] = s_sha512_init_vectors[ 5 ];
	m_hash_64[ 6 ] = s_sha512_init_vectors[ 6 ];
	m_hash_64[ 7 ] = s_sha512_init_vectors[ 7 ];
}
#endif


void sha::xor_vectors( sha_vectors in_vectors, const void *in_vectors_data ) {
	__u32					i, n, *p = nil;
	__u64				   *q = nil;

	switch ( in_vectors ) {
		case k_sha1_init_vectors:			n = 5;		p = s_sha1_init_vectors;		break;
		case k_sha256_init_vectors:			n = 8;		p = s_sha256_init_vectors;		break;
#if ENABLE_SHA512
		case k_sha384_init_vectors:			n = 8;		q = s_sha384_init_vectors;		break;
		case k_sha512_init_vectors:			n = 8;		q = s_sha512_init_vectors;		break;
#endif
		case k_sha1_constant_vectors:		n = 4;		p = s_sha1_k_constants;			break;
		case k_sha256_constant_vectors:		n = 64;		p = s_sha256_k_constants;		break;
#if ENABLE_SHA512
		case k_sha512_constant_vectors:		n = 80;		q = s_sha512_k_constants;		break;
#endif
	}
	
	if ( p ) {
		for ( i = 0; i < n; ++i ) p[ i ] ^= ((__u32 *) in_vectors_data)[ i ];
	} else {
		for ( i = 0; i < n; ++i ) q[ i ] ^= ((__u64 *) in_vectors_data)[ i ];
	}
}


#if __MWERKS__
#pragma mark -
#endif


const __u8 *sha::hash( __u8 *out_hash ) {
	__u32		i, s;

	if ( ! m_done ) {
		m_done = true;
		
		if ( ! m_remainder ) {
			m_remainder = new octet_string;
			m_remainder->set_zero_data_on_release();
		}
		
		pad( *m_remainder );
		push( *m_remainder, false );

#if BYTE_ORDER == LITTLE_ENDIAN
		switch ( m_algorithm ) {
			case	k_sha1:			s = 5;		goto do_swap_32;
			case	k_sha256:		s = 8;		goto do_swap_32;
			
		do_swap_32:
			
			for ( i = 0; i < s; ++i ) m_hash[ i ] = swap32( m_hash[ i ] );
			break;

#if ENABLE_SHA512
			case	k_sha384:		s = 6;		goto do_swap_64;
			case	k_sha512:		s = 8;		goto do_swap_64;
			
		do_swap_64:
		
			for ( i = 0; i < s; ++i ) reinterpret_cast<__u64 *>(m_hash)[ i ] = swap64( reinterpret_cast<__u64 *>(m_hash)[ i ] );
			break;
#endif

			default:							_throw( err_unimplemented );
		}
#endif
		_delete( m_remainder );
	}
	
	if ( out_hash ) for ( i = 0, s = hash_size( m_algorithm ); i < s; ++i ) {
		out_hash[ i ] = reinterpret_cast<__u8 *>(m_hash)[ i ];
	}
	
	return reinterpret_cast<__u8 *>(m_hash);
}


const __u8 *sha::hash( const char *in_c_string, __u8 *out_hash, sha_algorithm_type in_sha_algorithm ) {
	octet_string			o;
	
	o.set( (__u8 *) in_c_string, (__u32) strlen( in_c_string ), false, false );

	return sha::hash( o, out_hash, in_sha_algorithm );
}


const __u8 *sha::hash( const __u8 *in_path_to_file, __u8 *out_hash, sha_algorithm_type in_sha_algorithm ) {
	if ( ! out_hash ) _throw( err_bad_parameter );

	__u8			   *buf = nil;
	octet_string		data;
	__s32				fd;
	const __u8		   *h;
	__u32				i, l, n;
	off_t				l1, l2;
	sha					s( in_sha_algorithm );
	
	data.set_zero_data_on_release();
	
	_throw_errno_if( ( fd = open( (const char *) in_path_to_file, O_RDONLY ) ) == -1 );
	
	_try {
		l1 = 0;
	
		do {
			buf = new __u8[ k_256k ];
			
			_throw_errno_if( __s32( l = read( fd, buf, k_256k ) ) == -1 );
			
			data.set( buf, l, false );
			buf = nil;
		
			if ( l ) {
				l1 += l;
				s.push( data );
			}
		} while ( l );

		_throw_errno_if( ( l2 = lseek( fd, 0, SEEK_END ) ) == -1 );
		
		if ( l1 != l2 ) _throw( err_unexpected_end_of_file );

		h = s.hash();

		for ( i = 0, n = hash_size( in_sha_algorithm ); i < n; ++i ) out_hash[ i ] = h[ i ];
	} _catch
	
	delete[] buf;
	
	close( fd );

	_return out_hash;
}


const __u8 *sha::hash( const octet_string &in_message, __u8 *out_hash, sha_algorithm_type in_sha_algorithm ) {
	if ( ! out_hash ) _throw( err_bad_parameter );

	const __u8	   *h;
	__u32			i, n;
	sha				s( in_sha_algorithm );
	
	s.push( in_message );
	h = s.hash();
	
	for ( i = 0, n = hash_size( in_sha_algorithm ); i < n; ++i ) out_hash[ i ] = h[ i ];

	return out_hash;
}


const __u8 *sha::hash( const void *in_buffer, __u32 in_length, __u8 *out_hash, sha_algorithm_type in_sha_algorithm_type ) {
	octet_string			os;
	
	os.set( (const __u8 *) in_buffer, in_length, false, false );
	
	return sha::hash( os, out_hash, in_sha_algorithm_type );
}


char *sha::hash_string( const __u8 *in_hash, char *out_hash, sha_algorithm_type in_sha_algorithm ) {
	if ( ! out_hash ) _throw( err_bad_parameter );

	char		buffer[ k_sha512_64 * 2 + 1 ];		// so we can copy strings in place
	__u32		i, s;

	for ( i = 0, s = hash_size( in_sha_algorithm ); i < s; ++i ) {
		hexify( in_hash[ i ], buffer[ i * 2 ], buffer[ i * 2 + 1 ] );
	}
	buffer[ i * 2 ] = 0;

	memcpy( out_hash, buffer, i * 2 + 1 );

	return out_hash;
}


#if __MWERKS__
#pragma mark -
#endif


void sha::hash_sha1( const octet_string &in_message, __u32 in_blocks ) {
	__u32		a, b, c, d, e, i, j, *m, t, w[ 80 ];
	
	m = (__u32 *) in_message.data();

	// process each 64-byte block of in_message in order
	for ( i = 0; i < in_blocks; ++i, m += 16 ) {
		for ( j = 0; j < 80; ++j ) {
			if ( j < 16 ) w[ j ] = big32( m[ j ] );
			else rotate_left( 32, w[ j - 3 ] ^ w[ j - 8 ] ^ w[ j - 14 ] ^ w[ j - 16 ], 1, w[ j ] );
		}
		
		a = m_hash[ 0 ];
		b = m_hash[ 1 ];
		c = m_hash[ 2 ];
		d = m_hash[ 3 ];
		e = m_hash[ 4 ];

		for ( j = 0; j < 80; ++j ) {
			if ( j < 20 ) t = s_sha1_k_constants[ 0 ] + ch( b, c, d );
			else if ( j < 40  ) t = s_sha1_k_constants[ 1 ] + parity( b, c, d );
			else if ( j < 60 ) t = s_sha1_k_constants[ 2 ] + maj( b, c, d );
			else t = s_sha1_k_constants[ 3 ] + parity( b, c, d );
			
			t += rotate_left_evaluated( 32, a, 5 ) + w[ j ] + e;
			
			e = d;
			d = c;
			c = rotate_left_evaluated( 32, b, 30 );
			b = a;
			a = t;
			
//			console( "j is %u, a is %#08x", j, a );
//			console( "j is %u, b is %#08x", j, b );
//			console( "j is %u, c is %#08x", j, c );
//			console( "j is %u, d is %#08x", j, d );
//			console( "j is %u, e is %#08x", j, e );
		}
		
		// stage 4 (fips 180-2)
		m_hash[ 0 ] = a + m_hash[ 0 ];
		m_hash[ 1 ] = b + m_hash[ 1 ];
		m_hash[ 2 ] = c + m_hash[ 2 ];
		m_hash[ 3 ] = d + m_hash[ 3 ];
		m_hash[ 4 ] = e + m_hash[ 4 ];		
	}
}


void sha::hash_sha256( const octet_string &in_message, __u32 in_blocks ) {
	__u32		a, b, c, d, e, f, g, h, i, j, *m, t1, t2, w[ 64 ];

	m = (__u32 *) in_message.data();

	// process each 64-byte block of in_message in order
	for ( i = 0; i < in_blocks; ++i, m += 16 ) {
		for ( j = 0; j < 64; ++j ) {
			if ( j < 16 ) w[ j ] = big32( m[ j ] );
			else w[ j ] = w[ j - 7 ] + w[ j - 16 ] + sha256_sigma_2( w[ j - 15 ] ) + sha256_sigma_3( w[ j - 2 ] );
		}

		a = m_hash[ 0 ];
		b = m_hash[ 1 ];
		c = m_hash[ 2 ];
		d = m_hash[ 3 ];
		e = m_hash[ 4 ];
		f = m_hash[ 5 ];
		g = m_hash[ 6 ];
		h = m_hash[ 7 ];
		
		for ( j = 0; j < 64; ++j ) {
			t1 = h + sha256_sigma_1( e ) + ch( e, f, g ) + s_sha256_k_constants[ j ] + w[ j ];
			t2 = sha256_sigma_0( a ) + maj( a, b, c );

			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}
		
		m_hash[ 0 ] = a + m_hash[ 0 ];
		m_hash[ 1 ] = b + m_hash[ 1 ];
		m_hash[ 2 ] = c + m_hash[ 2 ];
		m_hash[ 3 ] = d + m_hash[ 3 ];
		m_hash[ 4 ] = e + m_hash[ 4 ];		
		m_hash[ 5 ] = f + m_hash[ 5 ];
		m_hash[ 6 ] = g + m_hash[ 6 ];
		m_hash[ 7 ] = h + m_hash[ 7 ];		
	}
}


#if ENABLE_SHA512
void sha::hash_sha512( const octet_string &in_message, __u32 in_blocks ) {
	__u32	i, j;
	__u64	a, b, c, d, e, f, g, h, *m, *m_hash_64, t1, t2, w[ 80 ];

	m = (__u64 *) in_message.data();
	m_hash_64 = reinterpret_cast<__u64 *>(m_hash);

	// process each 128-byte block of in_message in order
	for ( i = 0; i < in_blocks; ++i, m += 16 ) {
		for ( j = 0; j < 80; ++j ) {
			if ( j < 16 ) w[ j ] = big64( m[ j ] );
			else w[ j ] = w[ j - 7 ] + w[ j - 16 ] + sha512_sigma_2( w[ j - 15 ] ) + sha512_sigma_3( w[ j - 2 ] );
		}

		a = m_hash_64[ 0 ];
		b = m_hash_64[ 1 ];
		c = m_hash_64[ 2 ];
		d = m_hash_64[ 3 ];
		e = m_hash_64[ 4 ];
		f = m_hash_64[ 5 ];
		g = m_hash_64[ 6 ];
		h = m_hash_64[ 7 ];
		
		for ( j = 0; j < 80; ++j ) {
			t1 = h + sha512_sigma_1( e ) + ch64( e, f, g ) + s_sha512_k_constants[ j ] + w[ j ];
			t2 = sha512_sigma_0( a ) + maj64( a, b, c );
			
			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}

		m_hash_64[ 0 ] = a + m_hash_64[ 0 ];
		m_hash_64[ 1 ] = b + m_hash_64[ 1 ];
		m_hash_64[ 2 ] = c + m_hash_64[ 2 ];
		m_hash_64[ 3 ] = d + m_hash_64[ 3 ];
		m_hash_64[ 4 ] = e + m_hash_64[ 4 ];		
		m_hash_64[ 5 ] = f + m_hash_64[ 5 ];
		m_hash_64[ 6 ] = g + m_hash_64[ 6 ];
		m_hash_64[ 7 ] = h + m_hash_64[ 7 ];
	}
}
#endif


#if __MWERKS__
#pragma mark -
#endif


void sha::pad( octet_string &io_message ) {
	const __u8	   *data;
	__u8		   *m;
	__u32			i, l, n, bb, lb;

	// the padded message will be a multiple of 64 (for sha1 and sha256) or
	// 128 (for sha384 and sha512) bytes.  the padded message will consist of:
	//
	// io_message.data + 0x80 + k + __u64		// (sha1/sha256)	-or-
	// io_message.data + 0x80 + k + __128		// (sha384/sha512)
	//
	// where k is zero or more intermediate zeroed pad bytes necessary to align
	// the length of the message to either 64 or 128 bytes as described above.
	//
	// in the final 4 bytes (__u64) for sha1/sha256 or 8 bytes (__u128) for
	// sha384/sha512 are stored the binary representation of the length of
	// io_message.data.
	
	// bb = block_bytes, determine from the algorithm that we're using
	bb = m_algorithm <= k_sha256 ? k_sha1_block_bytes : k_sha512_block_bytes;

	// lb = last_bytes, either sizeof(__u64) or sizeof(__u128)
	lb = sizeof(__u32) * ( m_algorithm <= k_sha256 ? 2 : 4 );

	// allocate space for the pad block(s) based on the amount of data
	// currently in io_message.  at most we can have two blocks after padding.
	m = new __u8[ n = bb * ( ( l = io_message.length() ) <= bb - lb - 1 ? 1 : 2 ) ];

	// copy the io_message data to the new pad string
	data = io_message.data();
	for ( i = 0; i < l; ++i ) m[ i ] = data[ i ];
	
	// add the 0x80 byte
	m[ i++ ] = 0x80;
	
	// fill in the zeroed k bytes
	for ( ; i < n - lb; ++i ) m[ i ] = 0;
	
	// finally, copy the total length of the message to the pad block
	if ( m_algorithm <= k_sha256 ) {
		*reinterpret_cast<__u64 *>(&m[ i ]) = big64( m_size );
	} else {
#if ENABLE_SHA512
		__u32		j, k;

		k = m_size_128.size();

		// the apn representation is in the same binary format that needs to
		// be stored to the length bytes, but it may not be as long as lb, so
		// effectively extend k by copying zeros to the unused high bits of
		// the length.
		for ( j = 0; j < 4 - k; i += sizeof(__u32), ++j ) {		// 4 is number of __u32's in __u128
			*reinterpret_cast<__u32 *>(&m[ i ]) = 0;
		}
		
		// copy the length from the apn
		for ( j = 0; j < k; i += sizeof(__u32), ++j ) {
			*reinterpret_cast<__u32 *>(&m[ i ]) = big32( m_size_128.store()[ j ] );
		}
#else
		_throw( err_unimplemented );
#endif
	}
	
	io_message.set( m, n, false );
}


void sha::push( const octet_string &in_message, bool in_check_range ) {
	__u32			bb, i, l, mb, n, r;
	__u8		   *buf;
	const __u8	   *data;

	l = in_message.length();
	data = in_message.data();

	if ( in_check_range ) {
		mb = 8 * l;

		if ( m_algorithm <= k_sha256 ) {
			if ( mb > ~__u64(0) - m_size ) _throw( err_range );
			else m_size += mb;
		} else {
#if ENABLE_SHA512
			if ( apn( mb ) > apn( "ffffffffffffffffffffffffffffffff", k_hex ) - m_size_128 ) _throw( err_range );
			else m_size_128 += mb;
#else
			_throw( err_unimplemented );
#endif
		}
	}

	bb = ( m_algorithm <= k_sha256 ? k_sha1_block_bytes : k_sha512_block_bytes );

	if ( ( n = l / bb ) ) {
		switch ( m_algorithm ) {
			case k_sha1:		hash_sha1( in_message, n );			break;
			case k_sha256:		hash_sha256( in_message, n );		break;
#if ENABLE_SHA512
			case k_sha384:		hash_sha512( in_message, n );		break;
			case k_sha512:		hash_sha512( in_message, n );		break;
#endif
			default:			_throw( err_unimplemented );
		}
	}

	if ( ( r = l % bb ) ) {
		buf = new __u8[ r ];

		try {
			m_remainder = new octet_string();
			m_remainder->set_zero_data_on_release();
		} catch ( ... ) {
			delete[] buf;
			throw;
		}
		
		for ( i = 0; i < r; ++i ) buf[ i ] = data[ n * bb + i ];

		m_remainder->set( buf, r, false );
	}
}


#if ENABLE_FIPS_180_2_INPUT_VECTORS
void test_sha_with_fips_180_2_example_input_vectors() {
/* correct values are:

6193 06/02/06 12:29:04      sha.cpp: 654> sha-1 a: 0xa9993e364706816aba3e25717850c26c9cd0d89d
6193 06/02/06 12:29:04      sha.cpp: 655> sha-1 b: 0x84983e441c3bd26ebaae4aa1f95129e5e54670f1
6193 06/02/06 12:29:04      sha.cpp: 656> sha-1 c: 0x34aa973cd4c4daa4f61eeb2bdbad27316534016f
6193 06/02/06 12:29:04      sha.cpp: 654> sha-256 a: 0xba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
6193 06/02/06 12:29:04      sha.cpp: 655> sha-256 b: 0x248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1
6193 06/02/06 12:29:04      sha.cpp: 656> sha-256 c: 0xcdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0
6193 06/02/06 12:29:04      sha.cpp: 662> sha-384 a: 0xcb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7
6193 06/02/06 12:29:04      sha.cpp: 663> sha-384 b: 0x09330c33f71147e83d192fc782cd1b4753111b173b3b05d22fa08086e3b0f712fcc7c71a557e2db966c3e9fa91746039
6193 06/02/06 12:29:04      sha.cpp: 664> sha-384 c: 0x9d0e1809716474cb086e834e310a4a1ced149e9c00f248527972cec5704c2a5b07b8b3dc38ecc4ebae97ddd87f3d8985
6193 06/02/06 12:29:04      sha.cpp: 662> sha-512 a: 0xddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f
6193 06/02/06 12:29:04      sha.cpp: 663> sha-512 b: 0x8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909
6193 06/02/06 12:29:04      sha.cpp: 664> sha-512 c: 0xe718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973ebde0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b

*/
	sha_algorithm_type	algorithm;
	__u8				h[ k_sha512_64 ];
	char				v[ k_sha512_64 * 2 + 1 ];

	char			   *a = "abc";
	char			   *b = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
	char			   *c = new char[ 1000000 + 1 ];

	memset( c, 'a', 1000000 ); c[ 1000000 ] = 0;

	for ( algorithm = k_sha1; algorithm <= k_sha256; ++*reinterpret_cast<__u32 *>(&algorithm) ) {
		console( "%s a: 0x%s", sha::algorithm_name( algorithm ), sha::hash_string( sha::hash( a, h, algorithm ), v, algorithm ) );
		console( "%s b: 0x%s", sha::algorithm_name( algorithm ), sha::hash_string( sha::hash( b, h, algorithm ), v, algorithm ) );
		console( "%s c: 0x%s", sha::algorithm_name( algorithm ), sha::hash_string( sha::hash( c, h, algorithm ), v, algorithm ) );
	}

	b = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";

	for ( ; algorithm <= k_sha512; ++*reinterpret_cast<__u32 *>(&algorithm) ) {
		console( "%s a: 0x%s", sha::algorithm_name( algorithm ), sha::hash_string( sha::hash( a, h, algorithm ), v, algorithm ) );
		console( "%s b: 0x%s", sha::algorithm_name( algorithm ), sha::hash_string( sha::hash( b, h, algorithm ), v, algorithm ) );
		console( "%s c: 0x%s", sha::algorithm_name( algorithm ), sha::hash_string( sha::hash( c, h, algorithm ), v, algorithm ) );
	}
	
	delete[] c;
}
#endif


#if ENABLE_OPENSSL_COMPATIBILITY
__u8 *SHA1( __u8 *in_message, __u32 in_length, __u8 *out_hash ) {
	static __u8				s_hash[ k_sha1_20 ];

	try {
		octet_string			o;
	
		o.set( in_message, in_length, false, false );

		sha::hash( o, out_hash ? out_hash : out_hash = s_hash );
	} catch ( ... ) { }
	
	return out_hash;
}
#endif
