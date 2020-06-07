#ifndef __including_rsa_generators_cpp__
	#ifndef ENABLE_RSA_GENERATORS_INCLUSION
		#error Don't include this file directly.  Define ENABLE_RSA_GENERATORS == 1 and include rsa.cpp.
	#endif
#endif

#include "rsa.h"


#if ENABLE_RSA_GENERATORS


#include "apn_utilities.h"
#include "asn1.h"
#include "base64.h"
#include "file.h"
#include "octet_string.h"


using balance::file;


#define k_begin_rsa_private_key		"-----BEGIN RSA PRIVATE KEY-----\n"
#define k_begin_rsa_public_key		"-----BEGIN RSA PUBLIC KEY-----\n"
#define k_end_rsa_private_key		"-----END RSA PRIVATE KEY-----\n"
#define k_end_rsa_public_key		"-----END RSA PUBLIC KEY-----\n"


#if ENABLE_RSA_OBSERVATION
	#include "observed.h"
#endif


#if ENABLE_RSA_DEFAULT_OBSERVER
void rsa_generator_observer( void *in_context, __u32 in_event, void *in_data, __u32 in_data_size ) {
	fprintf( stdout, "%s", (char *) in_data );
	fflush( stdout );

	if ( in_data ) free( in_data );
}
#endif


// RSASSA-PSS-SIGN
void rsa::sign( octet_string &in_message, octet_string &out_signature ) {
	sha_algorithm_type		algorithm;
	octet_string			em, signature;
	__s32					h_len, k, max_length, modulus_bits;

	determine_crypt_parameters( k, algorithm, h_len, max_length );

//	dump_info( in_message.data(), in_message.length(), "in_message: %u", in_message.length() );

	emsa_pss_encode( in_message, ( modulus_bits = k * 8 ) - 1, algorithm, em );
	
	decrypt_primitive( em ).as_octet_string( signature, k );

	out_signature.set( signature.data() + ( (__s32) signature.length() > k ), k );
}


rsa *rsa::generate_key( __u32 in_num_bits_in_modulus, const char *in_rand_file, __u32 in_rounds, __u32 in_e, class observed *in_observe ) {
	if ( in_num_bits_in_modulus < 3 ) _throw( err_bad_parameter );

#if ENABLE_RSA_DEFAULT_OBSERVER
	observed				rsa_observer;

	rsa_observer.register_observer( rsa_generator_observer, nil );
	if ( ! in_observe ) in_observe = &rsa_observer;
#endif

	__u32		n = in_num_bits_in_modulus / 2;
	apn			one = 1, p_minus_one( apn::k_apn_postpone_init ), q_minus_one( apn::k_apn_postpone_init ), *p_m1, *q_m1;
	rsa		   *result = new rsa;

	try {
		result->m_e = new apn( in_e );
		result->m_d = new apn;
		result->m_n = new apn;
		result->m_p = new apn;
		result->m_q = new apn;
		result->m_dp = new apn;
		result->m_dq = new apn;
		result->m_qinv = new apn;

		if ( ! in_e ) {
#if 1
			// I could generate a random number here but for large bit lengths the coprimality
			// requirement of e vs. phi can cause a *long* time to be spent waiting for valid
			// integers to be generated.  by making e prime here it saves some time a bit later.
			*result->m_e = generate_prime( n, in_rand_file, in_rounds, in_observe );
#else
			*result->m_e = random_apn( in_rand_file, one, x_exp_n( 2, in_num_bits_in_modulus - 1 ) );
			if ( result->m_e->is_even() ) ++*result->m_e;
#endif

#if ENABLE_RSA_OBSERVATION
			if ( in_observe ) in_observe->log( "Generated RSA public exponent.\n" );
#endif
		}
		
		// bd 02.04.04:
		//
		// an rsa public key consists of a positive integer modulus n, and
		// a positive integer exponent e, where n is the product of two distinct
		// odd primes p and q, and 3 <= e <= n - 1.
		//
		// additionally, e must satisfy the condition:
		//
		// gcd( e, ( p - 1 ) * ( q - 1 ) ) == 1
		//
		// in order to make the key generation go as fast as possible, it was suggested
		// at http://www.di-mgt.com.au/rsa_alg.html that if e is precomputed then after
		// the generation of each prime p and q, we can immediately test for
		// gcd( e, p - 1 ) == 1, and reject any prime which fails.
		//
		// I was unsure after reading this whether or not computing:
		//
		// gcd( e, p - 1 ) == 1 and gcd( e, q - 1 ) was the same as computing
		// gcd( e, ( p - 1 ) * ( q - 1 ) ), but after reading Knuth 4.5.2 p. 334 which
		// has this identity:
		//
		// lcm( gcd( u, v ), gcd( u, w ) ) == gcd( u, lcm( v, w ) )
		//
		// it turns out that they are equivalent:
		//
		// if u = e, v = p - 1, w = q - 1, then:
		//
		// lcm( gcd( e, p - 1 ), gcd( e, q - 1 ) ) == gcd( e, lcm( p - 1, q - 1 ) )
		//
		// since p and q are prime, lcm( p - 1, q - 1 ) == ( p - 1 ) * ( q - 1 ), so:
		//
		// lcm( 1, 1 ) == gcd( e, ( p - 1 ) * ( q - 1 ) ), so
		//
		// 1 == gcd( e, ( p - 1 ) * ( q - 1 ) ). QED.

		do {
			*result->m_p = generate_prime( n, in_rand_file, in_rounds, in_observe );
		} while ( gcd( *result->m_e, p_minus_one = *result->m_p - one ) != one );

#if ENABLE_RSA_OBSERVATION
		if ( in_observe ) in_observe->log( "Generated %u-bit prime p.\n", n );
#endif

		do {
			*result->m_q = generate_prime( n, in_rand_file, in_rounds, in_observe );
		} while ( gcd( *result->m_e, q_minus_one = *result->m_q - one ) != one );

#if ENABLE_RSA_OBSERVATION
		if ( in_observe ) in_observe->log( "Generated %u-bit prime q.\n", n );
#endif
		if ( *result->m_p == *result->m_q ) _throw( err_prng_broken );
		
		// swap p and q for CRT
		if ( *result->m_p < *result->m_q ) {
			apn *t = result->m_p;
			result->m_p = result->m_q;
			result->m_q = t;

			p_m1 = &q_minus_one;
			q_m1 = &p_minus_one;
		} else {
			p_m1 = &p_minus_one;
			q_m1 = &q_minus_one;
		}

		*result->m_n = *result->m_p * *result->m_q;
		*result->m_dp = modular_inverse( *result->m_e, *p_m1 );
		*result->m_dq = modular_inverse( *result->m_e, *q_m1 );
		*result->m_qinv = modular_inverse( *result->m_q, *result->m_p );
		*result->m_d = modular_inverse( *result->m_e, *p_m1 * *q_m1 );
	} catch ( ... ) {
		delete result;
		throw;
	}

	return result;
}


#if ENABLE_RSA_SAVE
void rsa::create_asn1_ber_sequence( asn1_ber_sequence &out_sequence, bool in_save_only_public_key ) const {
	apn					version;

	out_sequence.reset();
	
	version = in_save_only_public_key ? 1 : 0;

	push_apn( out_sequence, &version );			// version 0 (from pkcs #1 v2.1)
	push_apn( out_sequence, m_n );				// modulus
	push_apn( out_sequence, m_e );				// public exponent

	if ( ! in_save_only_public_key ) {
		push_apn( out_sequence, m_d );			// private exponent
		push_apn( out_sequence, m_p );			// prime 1
		push_apn( out_sequence, m_q );			// prime 2
		push_apn( out_sequence, m_dp );			// exponent 1
		push_apn( out_sequence, m_dq );			// exponent 2
		push_apn( out_sequence, m_qinv );		// coefficient
	}	
}


void rsa::save( octet_string *&out_key_file, bool in_save_only_public_key ) {
	__u8				c;
	char			   *end, *key, *start;
	file				f;
	__u32				i, j, n;
	octet_string		rep;
	asn1_ber_sequence	seq;
 
	out_key_file = new octet_string;
	
	create_asn1_ber_sequence( seq, in_save_only_public_key );
	seq.encode( rep );
	
	key = rep.as_base64();

	try {
		n = strlen( key );
		
		if ( in_save_only_public_key ) {
			start = k_begin_rsa_public_key;
			end = k_end_rsa_public_key;
		} else {
			start = k_begin_rsa_private_key;
			end = k_end_rsa_private_key;
		}
		
		out_key_file->append( (__u8 *) start, strlen( start ) );

		// write an openssl compatible key, 64 bytes per line (+ newline)
		for ( i = 0; i < n; ) {
			if ( n - i >= 64 ) {
				c = key[ i + 64 ];
				key[ i + 64 ] = '\n';
				out_key_file->append( (__u8 *) &key[ i ], 65 );
				key[ i + 64 ] = c;
				i += 64;
			} else {
				key[ n ] = '\n';
				out_key_file->append( (__u8 *) &key[ i ], j = n - i + 1 );
				i += j;
			}
		}

		out_key_file->append( (__u8 *) end, strlen( end ) );
	} catch ( ... ) {
		delete[] key;
		throw;
	}
	
	delete[] key;
}


void rsa::save( const char *in_path_to_file, bool in_save_only_public_key ) {
	file				f;
	octet_string	   *os;
	
	save( os, in_save_only_public_key );
	
	_try {
		f.open( in_path_to_file, k_default_truncate_flags );
		f.write( os->data(), os->length() );
	} _catch
	
	delete os;
	
	_throw_now();
}
#endif // ENABLE_RSA_SAVE


#endif // ENABLE_RSA_GENERATORS
