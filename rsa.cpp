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
		

	File:				rsa.cpp

	Author:				Brian Doyle
	Date Created:		February 03, 2004
	Last Modified:		February 07, 2004

	Description:

	RSA public key cryptography based on my apn library

\*----------------------------------------------------------------------------------------*/
#include <math.h>

#include "apn_utilities.h"
#include "asn1.h"
#include "base64.h"
#include "crypto_random.h"
#include "file.h"
#include "octet_string.h"
#include "rsa.h"


using balance::file;


#define k_begin_rsa_private_key		"-----BEGIN RSA PRIVATE KEY-----\n"
#define k_begin_rsa_public_key		"-----BEGIN RSA PUBLIC KEY-----\n"
#define k_end_rsa_private_key		"-----END RSA PRIVATE KEY-----\n"
#define k_end_rsa_public_key		"-----END RSA PUBLIC KEY-----\n"


#define s_len						h_len


#pragma mark -


rsa::rsa() {
	m_n = nil;
	m_e = nil;
	m_d = nil;
	m_p = nil;
	m_q = nil;
	m_dp = nil;
	m_dq = nil;
	m_qinv = nil;
}


rsa::~rsa() {
	delete m_n;
	delete m_e;
	delete m_d;
	delete m_p;
	delete m_q;
	delete m_dp;
	delete m_dq;
	delete m_qinv;
}


// RSAES-OAEP-DECRYPT:
void rsa::decrypt( octet_string &io_message ) {
	sha_algorithm_type			algorithm;
	__u8					   *cipher, hash[ k_sha512_64 ], *p;
	const __u8				   *data, *q;
	octet_string				db, dbmask, em, L, m, result, seed, seedmask, tmp;
	__s32						dbsize, h_len, i, j, k, n, max_length;

	db.set_zero_data_on_release();
	dbmask.set_zero_data_on_release();
	em.set_zero_data_on_release();
	m.set_zero_data_on_release();
	result.set_zero_data_on_release();
	seed.set_zero_data_on_release();
	seedmask.set_zero_data_on_release();

	determine_crypt_parameters( k, algorithm, h_len, max_length );
	
	if ( io_message.length() % k ) _throw( err_decryption_failure );
	
	dbsize = h_len + max_length + 1;
	
	sha::hash( L, hash, algorithm );
	
	// convert io_message to a positive apn
	*( cipher = new __u8[ k + 1 ] ) = 0;
	
	_try {
		data = io_message.data();
	
		for ( i = io_message.length(); i > 0; i -= k ) {
			for ( j = 1; j <= k; ++j ) cipher[ j ] = *data++;
			
			tmp.set( cipher, k + 1 );
			
			decrypt_primitive( tmp ).as_octet_string( em, k );

			seed.set( q = em.data() + 1, h_len );
			db.set( q + h_len, k - h_len - 1 );
			
			mgf1( db, h_len, seedmask, algorithm );
			
			p = const_cast<__u8 *>(seed.data());
			q = seedmask.data();
			
			for ( j = 0; j < h_len; ++j ) p[ j ] ^= q[ j ];
			
			mgf1( seed, n = k - h_len - 1, dbmask, algorithm );
			
			p = const_cast<__u8 *>(db.data());
			q = dbmask.data();
			
			for ( j = 0; j < n; ++j ) p[ j ] ^= q[ j ];
			
			for ( p = hash, q = db.data(), j = 0; j < h_len; ++j ) {
				if ( hash[ j ] != *q++ ) _throw( err_decryption_failure );
			}
			
			for ( ; j < dbsize && ! *q; ++j, ++q ) ;
			
			if ( ++j >= dbsize || *q++ != 0x01 ) _throw( err_decryption_failure );
			
			m.set( q, dbsize - j );
			
			result += m;
		}
	} _catch
	
	delete[] cipher;
	
	io_message = result;

	_return;
}


// RSAES-OAEP-ENCRYPT:
void rsa::encrypt( octet_string &io_message ) {
	sha_algorithm_type		algorithm;
	octet_string			c, db, dbmask, L, m, one, result, seed, seedmask, zero;
	const __u8			   *data, *q;
	__u8					hash[ k_sha512_64 ], *message, *p, *seed_buf = nil, *tmp;
	__s32					h_len, h_len_2, i, j, k, l, max_length, n;
	crypto_random			rnd;

	db.set_zero_data_on_release();
	dbmask.set_zero_data_on_release();
	m.set_zero_data_on_release();
	seed.set_zero_data_on_release();
	seedmask.set_zero_data_on_release();

	one.set( (__u8 *) "\x01", 1 );
	zero.set( (__u8 *) "\x00", 1 );

	determine_crypt_parameters( k, algorithm, h_len, max_length );
	h_len_2 = h_len * 2;
	
	sha::hash( L, hash, algorithm );

	data = io_message.data();
	message = new __u8[ max_length ];

	_try {
		seed_buf = new __u8[ h_len ];
	
		for ( i = io_message.length(); i > 0; i -= max_length ) {
			octet_string			em, ps;
		
			em.set_zero_data_on_release();
			ps.set_zero_data_on_release();
		
			n = min( i, max_length );		// number of bytes in message;
			
			if ( ( l = k - n - h_len_2 - 2 ) > 0 ) {
				tmp = new __u8[ l ];
				for ( j = 0; j < l; ++j ) tmp[ j ] = 0;
				ps.set( tmp, l, false );
				tmp = nil;
			}

			db.set( hash, h_len );
			
			db += ps;
			db += one;

			for ( j = 0; j < n; ++ j ) message[ j ] = *data++;
			
			m.set( message, n );
			
			db += m;

			rnd.random( seed_buf, h_len );

#if DEBUG && 0
			#warning RSA broken
			memset( seed_buf, 0, h_len );
#endif
			seed.set( seed_buf, h_len );
			
			mgf1( seed, n = k - h_len - 1, dbmask, algorithm );
			
			p = const_cast<__u8 *>(db.data());
			q = dbmask.data();
			
			_assert( __u32(n) == db.length() );
			
			for ( j = 0; j < n; ++j ) p[ j ] ^= q[ j ];
			
			mgf1( db, h_len, seedmask, algorithm );
			
			p = const_cast<__u8 *>(seed.data());
			q = seedmask.data();
			
			for ( j = 0, n = seed.length(); j < n; ++j ) p[ j ] ^= q[ j ];
			
			em += zero;
			em += seed;
			em += db;

			encrypt_primitive( em ).as_octet_string( c, k );

			if ( __s32(c.length()) > k ) c.set( c.data() + 1, k );

			result += c;
		}
	} _catch

	io_message = result;
	
	_assert( ! ( io_message.length() % k ) );
	
	if ( message ) {
		for ( i = 0; i < max_length; ++i ) message[ i ] = 0;
		delete[] message;
	}
	if ( seed_buf ) {
		for ( i = 0; i < h_len; ++i ) seed_buf[ i ] = 0;
		delete[] seed_buf;
	}
	
	_return;
}


// RSASSA-PSS_VERIFY (incorporating EMSA-PSS-VERIFY)
void rsa::verify_signature( octet_string &in_message, octet_string &in_signature ) {
	sha_algorithm_type		algorithm;
	octet_string			em, signature;
	__s32					em_len, h_len, i, k, max_length, modulus_bits_minus_one, n, o;
	__u8					M[ 8 + k_sha512_64 * 2 ], M_prime[ k_sha512_64 ], mask, *masked_db, *p, *q, *r, *salt;

	determine_crypt_parameters( k, algorithm, h_len, max_length );

	if ( (int) in_signature.length() != k ) _throw( err_invalid_signature );
	
	if ( *( p = (__u8 *) in_signature.data() ) & 0x80 ) {
		*( q = r = new __u8[ k + 1 ] )++ = 0;
		for ( i = 0; i < k; ++i ) *q++ = *p++;
		signature.set( r, k + 1, false, true );
	} else {
		signature.set( p, in_signature.length(), false, false );
	}

	// Ignoring EMSA-PSS-VERIFY 9.1.2.1 since ULONG_MAX < sha-1
	// message length limit and in_message.length() returns __u32

	// EMA-PSS-VERIFY 9.1.2.3
	modulus_bits_minus_one = k * 8 - 1;
	em_len = ( modulus_bits_minus_one + 7 ) / 8;
	if ( em_len < h_len + s_len + 2 ) _throw( err_range );
	
	encrypt_primitive( signature ).as_octet_string( em, em_len );

	// EMA-PSS-VERIFY 9.1.2.4/5/6
	masked_db = (__u8 *) em.data();
	n = em.length();
	while ( n && ! *masked_db ) { --n; ++masked_db; }
	if ( n <= 0 || masked_db[ n - 1 ] != 0xbc ) _throw( err_invalid_signature );

	mask = 0x0ffu << 8 - ( 8 * em_len - modulus_bits_minus_one );
	if ( masked_db[ 0 ] & mask ) _throw( err_invalid_signature );

	octet_string			db_mask;
	octet_string			H( masked_db + ( n = em_len - h_len - 1 ), h_len );
	
	// EMA-PSS-VERIFY 9.1.2.7
	mgf1( H, n, db_mask, algorithm );

	// EMA-PSS-VERIFY 9.1.2.8
	for ( i = 0, p = (__u8 *) db_mask.data(); i < n; ++i ) masked_db[ i ] ^= p[ i ];

	// EMA-PSS-VERIFY 9.1.2.9
	masked_db[ 0 ] &= ~mask;

	// EMA-PSS-VERIFY 9.1.2.10
	for ( i = 0, o = em_len - h_len - s_len - 2; i < o; ++i ) {
		if ( masked_db[ i ] ) _throw( err_invalid_signature );
	}
	if ( masked_db[ i ] != 0x01 ) _throw( err_invalid_signature );
	
	// EMA-PSS-VERIFY 9.1.2.11
	salt = masked_db + n - s_len;

	// EMA-PSS-VERIFY 9.1.2.2/12
	for ( i = 0; i < 8; ++i ) M[ i ] = 0;
	sha::hash( in_message, M + 8, algorithm );
	for ( i = 0, p = M + 8 + h_len; i < s_len; ++i ) *p++ = salt[ i ];
	
	// EMA-PSS-VERIFY 9.1.2.13
	sha::hash( M, 8 + h_len + s_len, M_prime, algorithm );
	
	for ( i = 0, p = (__u8 *) H.data(); i < h_len; ++i ) {
		if ( p[ i ] != M_prime[ i ] ) _throw( err_invalid_signature );
	}
}


#pragma mark -


const apn rsa::decrypt_primitive( const apn &in_message ) {
	if ( ! m_n ) {
		if ( ! ( m_p && m_q ) ) _throw( err_uninitialized );
		m_n = new apn( *m_p * *m_q );
	}

	if ( in_message.is_negative() || in_message >= *m_n ) {
_d(		dump_key() );
		
		_throw( err_range );
	}
	
	if ( m_p && m_q && m_dp && m_dq && m_qinv ) {
		apn		m2 = x_to_the_n_mod_m( in_message, *m_dq, *m_q );			
		apn		r = m2 + *m_q * ( ( ( x_to_the_n_mod_m( in_message, *m_dp, *m_p ) - m2 ) * *m_qinv ) % *m_p );

		return r.is_negative() ? apn( r + *m_n ) : apn( r );
	} else if ( m_d || m_p && m_q && m_e ) {
		if ( ! m_d ) m_d = new apn( modular_inverse( *m_e, ( *m_p - 1 ) * ( *m_q - 1 ) ) );
		
		return apn( x_to_the_n_mod_m( in_message, *m_d, *m_n ) );
	} else _throw( err_uninitialized );
}


const apn rsa::encrypt_primitive( const apn &in_message ) {
	if ( ! ( m_n && m_e ) ) _throw( err_uninitialized );
	if ( in_message.is_negative() || in_message >= *m_n ) _throw( err_range );
	
	return apn( x_to_the_n_mod_m( in_message, *m_e, *m_n ) );
}


#pragma mark -


void rsa::determine_crypt_parameters( __s32 &out_k, sha_algorithm_type &out_algorithm, __s32 &out_h_len, __s32 &out_max_length ) {
	sha_algorithm_type		algorithm;
	__s32					k, max_length;

	k = m_n->size( true ) * sizeof(__u32);

	if ( ( max_length = k - 2 * k_sha512_64 - 2 ) > 0 ) algorithm = k_sha512;
	else if ( ( max_length = k - 2 * k_sha384_48 - 2 ) > 0 ) algorithm = k_sha384;
	else if ( ( max_length = k - 2 * k_sha256_32 - 2 ) > 0 ) algorithm = k_sha256;
	else if ( ( max_length = k - 2 * k_sha1_20 - 2 ) > 0 ) algorithm = k_sha1;
	else _throw( err_range );	// the public modulus is too short

	out_k = k;
	out_algorithm = algorithm;
	out_h_len = sha::hash_size( algorithm );
	out_max_length = max_length;
}


void rsa::emsa_pss_encode( octet_string &in_message, __s32 in_modulus_bits_minus_one, sha_algorithm_type in_algorithm, octet_string &out_result ) {
	__u8				   *db, H[ k_sha512_64 + 1 ], M[ 8 + k_sha512_64 * 2 ], *p, *q;
	octet_string			db_mask;
	__s32					em_len, h_len, i, j, n;
	crypto_random			rnd;

	// Ignoring EMSA-PSS-ENCODE 9.1.1.1 since ULONG_MAX < sha-1
	// message length limit and in_message.length() returns __u32
	
	h_len = sha::hash_size( in_algorithm );

	// EMA-PSS-ENCODE 9.1.1.3
	em_len = ( in_modulus_bits_minus_one + 7 ) / 8;
	if ( em_len < h_len + s_len + 2 ) _throw( err_range );

	// EMA-PSS-ENCODE 9.1.1.2
	sha::hash( in_message, M + 8, in_algorithm );

	_try {
		// EMA-PSS-ENCODE 9.1.1.4
#if DEBUG && 0	// fill the seed with zeros
		#warning RSA-SSA broken
		for ( i = 0; i < s_len; ++i ) M[ 8 + h_len + i ] = 0;
#else
		rnd.random( M + 8 + h_len, s_len );
#endif

		// EMA-PSS-ENCODE 9.1.1.5
		for ( i = 0; i < 8; ++i ) M[ i ] = 0;

//		dump_info( M, 8 + h_len + s_len, "M:" );

		// EMA-PSS-ENCODE 9.1.1.6
		sha::hash( M, 8 + h_len + s_len, H, in_algorithm );

//		dump_info( H, h_len, "H:" );

		// EMA-PSS-ENCODE 9.1.1.7/8
		db = new __u8[ ( n = em_len - s_len - h_len - 2 ) + 1 + s_len ];
		for ( i = 0; i < n; ++i ) db[ i ] = 0;							// PS
		db[ i++ ] = 0x01;												// 0x01
		for ( j = 0; j < h_len; ++j ) db[ i++ ] = M[ 8 + h_len + j ];	// salt
		
		out_result.set( db, n += 1 + h_len, false, true );

//		dump_info( db, n, "db (%u)", n );

		// EMA-PSS-ENCODE 9.1.1.9
		octet_string		os( H, h_len, false, false );
		
		mgf1( os, n, db_mask, in_algorithm );

//		dump_info( db_mask.data(), db_mask.length(), "db_mask (%u)", db_mask.length() );

		// EMA-PSS-ENCODE 9.1.1.10
		p = (__u8 *) out_result.data();
		q = (__u8 *) db_mask.data();
		for ( i = 0; i < n; ++i ) p[ i ] ^= q[ i ];

//		dump_info( p, db_mask.length(), "db ^= db_mask (%u)", db_mask.length() );
		
		// EMA-PSS-ENCODE 9.1.1.11
		p[ 0 ] &= 0x0ffu >> 8 * em_len - in_modulus_bits_minus_one;
		
		// EMA-PSS-ENCODE 9.1.1.12
		H[ h_len ] = 0xbc;
		
		os.set( H, h_len + 1, false, false );
		
		out_result += os;

//		dump( out_result.data(), out_result.length(), "out_result (%u)", out_result.length() );
	} _catch

	for ( i = 0, n = 8 + h_len + s_len; i < n; ++i ) M[ i ] = 0;
	for ( i = 0, n = h_len + 1; i < n; ++i ) H[ i ] = 0;

	_return;
}


// MGF1 from PKCS #1 v2.1
void rsa::mgf1( octet_string &in_seed, __u32 in_output_length, octet_string &out_result, sha_algorithm_type in_algorithm ) {
	if ( ! in_output_length ) _throw( err_bad_parameter );

	octet_string			c, ch, t;
	__u8					hash[ k_sha512_64 ], *result;
	__u32					i, n, hash_size;
	const __u8			   *p;
	
	hash_size = sha::hash_size( in_algorithm );

	result = new __u8[ in_output_length ];
	
	_try {
		c.set_zero_data_on_release();
		ch.set_zero_data_on_release();
		t.set_zero_data_on_release();

		n = ( in_output_length + hash_size - 1 ) / hash_size;

		for ( i = 0; i < n; ++i ) {
			apn( i ).as_octet_string( c, 4 );
			
			ch = in_seed;
			ch += c;
			
			sha::hash( ch, hash, in_algorithm );
			
			ch.set( hash, hash_size );
			
			t += ch;
		}
		
		for ( i = 0, p = t.data(); i < in_output_length; ++i ) result[ i ] = *p++;
		
		out_result.set( result, i, false ); 
	} _catch
	
	for ( i = 0; i < hash_size; ++i ) hash[ i ] = 0;
	
	_return;
}


#if ENABLE_RSA_LOAD
rsa *rsa::load( const char *in_path_to_file ) {
	char				   *buffer;
	rsa					   *key = nil;
	__u32					n;

	buffer = (char *) file::acquire( in_path_to_file, n = 0 );
	
	_try { key = load( buffer, n, true ); } _catch
	
	delete[] buffer;
	
	_return key;
}


rsa *rsa::load( const char *in_keyfile_data, __u32 in_length, bool in_keyfile_data_is_non_const ) {
	char				   *buffer, c, *data, *p, *q, *r;
	__u32					i, j;
	rsa					   *key = nil;

	if ( ! in_keyfile_data_is_non_const ) {
		data = buffer = (char *) dup_mem( in_keyfile_data, in_length );
	} else {
		data = (char *) in_keyfile_data;
	}
	
	// mysql translates text newlines to \r\n and a key might get resaved
	// with CR's instead of LF so do a quick rewrite to use only newlines.
	for ( i = j = 0; i < in_length; ++i ) {
		if ( data[ i ] == '\r' || data[ i ] == '\n' ) {
			if ( j && data[ j - 1 ] != '\n' ) data[ j++ ] = '\n';
		} else {			
			data[ j++ ] = data[ i ];
		}
	}
	
	_try {
		if ( strncmp( data, k_begin_rsa_private_key, i = strlen( k_begin_rsa_private_key ) - 1 ) ) _throw( err_bad_data );

		p = data + i + int(data[ i ] == '\n');
		q = strnstr( p, k_end_rsa_private_key, in_length - __u32( p - data ) );

		if ( ! q ) _throw( err_bad_data );

		*q = 0;
		
		for ( r = q = p; ( c = *q ); ++q ) if ( c != '\n' ) *r++ = c;

		*r = 0;

		key = load( (__u8 *) p );
	} _catch
	
	if ( ! in_keyfile_data_is_non_const ) delete[] buffer;
	
	_return key;
}


rsa *rsa::load( const __u8 *in_base64_key_data_c_string_typecast_to___u8_ptr ) {
	__u8				   *buf;
	__u32					l;
	octet_string			o;

	buf = base64::decode( (const char *) in_base64_key_data_c_string_typecast_to___u8_ptr, l );
	o.set( buf, l, false );

	asn1_ber_sequence		seq( o );
		
	return rsa::load( seq );
}


rsa *rsa::load( asn1_ber_sequence &io_sequence ) {
	rsa					   *result;
	__u32					type;
	apn					   *version = nil;
	
	result = new rsa;
	
	_try {	
		pop_apn( io_sequence, version );

		pop_apn( io_sequence, result->m_n );
		pop_apn( io_sequence, result->m_e );

		if ( ! *version ) {
			pop_apn( io_sequence, result->m_d );
			pop_apn( io_sequence, result->m_p );
			pop_apn( io_sequence, result->m_q );
			pop_apn( io_sequence, result->m_dp );
			pop_apn( io_sequence, result->m_dq );
			pop_apn( io_sequence, result->m_qinv );
		}
	} _catch
	
	_if_err delete result;
	
	delete version;
	
	_return result;
}
#endif // ENABLE_RSA_LOAD


#if DEBUG
void rsa::dump_key() {
	char				   *value;

	#define __rsadumpkey( _apn ) do {		\
		if ( ! _apn ) break;				\
		value = _apn->value( k_decimal );	\
		printf( #_apn ": %s\n", value );	\
		delete[] value;						\
	} while ( 0 )
	
	__rsadumpkey( m_n );
	__rsadumpkey( m_e );
	__rsadumpkey( m_d );
	__rsadumpkey( m_p );
	__rsadumpkey( m_q );
	__rsadumpkey( m_dp );
	__rsadumpkey( m_dq );
	__rsadumpkey( m_qinv );
}
#endif


#define __including_rsa_generators_cpp__	1
#include "rsa_generators.cpp"
#undef __including_rsa_generators_cpp__
