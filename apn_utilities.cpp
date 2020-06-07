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
		

	File:				apn_utilities.cpp

	Author:				Brian Doyle
	Date Created:		December 18, 2003
	Last Modified:		February 07, 2004

	Description:

	Some handy methods that use my apn class (prime number generation, random
	number generation).

\*----------------------------------------------------------------------------------------*/
#include "apn_utilities.h"
#include "crypto_random.h"

#if ENABLE_APN_OBSERVATION
	#include "observed.h"
#endif


__u32 s_small_primes[ k_number_of_small_primes_under_2000 ] = {
	   2,    3,    5,    7,   11,   13,   17,   19,
	  23,   29,   31,   37,   41,   43,   47,   53,
	  59,   61,   67,   71,   73,   79,   83,   89,
	  97,  101,  103,  107,  109,  113,  127,  131,
	 137,  139,  149,  151,  157,  163,  167,  173,
	 179,  181,  191,  193,  197,  199,  211,  223,
	 227,  229,  233,  239,  241,  251,  257,  263,
	 269,  271,  277,  281,  283,  293,  307,  311,
	 313,  317,  331,  337,  347,  349,  353,  359,
	 367,  373,  379,  383,  389,  397,  401,  409,
	 419,  421,  431,  433,  439,  443,  449,  457,
	 461,  463,  467,  479,  487,  491,  499,  503,
	 509,  521,  523,  541,  547,  557,  563,  569,
	 571,  577,  587,  593,  599,  601,  607,  613,
	 617,  619,  631,  641,  643,  647,  653,  659,
	 661,  673,  677,  683,  691,  701,  709,  719,
	 727,  733,  739,  743,  751,  757,  761,  769,
	 773,  787,  797,  809,  811,  821,  823,  827,
	 829,  839,  853,  857,  859,  863,  877,  881,
	 883,  887,  907,  911,  919,  929,  937,  941,
	 947,  953,  967,  971,  977,  983,  991,  997,
	1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049,
	1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097,
	1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163,
	1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223,
	1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283,
	1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321,
	1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423,
	1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459,
	1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511,
	1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571,
	1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619,
	1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693,
	1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747,
	1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811,
	1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877,
	1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949,
	1951, 1973, 1979, 1987, 1993, 1997, 1999
};


const apn generate_prime( __u32 in_number_of_bits, const char *in_random_number_file, __u32 in_rounds, class observed *in_observe ) {
#if ENABLE_APN_OBSERVATION
	__u32			attempt = 0;
#endif
	__u32			i, j, n, t, *p, *q;
	crypto_random	rng( in_random_number_file );

	for ( j = 1; ; ++j ) {
#if ENABLE_APN_OBSERVATION
//		if ( in_observe ) in_observe->log( "Trying possible %u bit prime %u", in_number_of_bits, ++attempt );
		if ( in_observe ) in_observe->log( ".", in_number_of_bits, ++attempt );
#endif
		apn 		prime = random_apn( rng, in_number_of_bits );

		// we need to set the top two bits to ensure that the prime is at least as
		// long as in_number_of_bits and is in the top half of the range (this makes
		// rsa keys somewhat harder to crack).  since we'll be setting the most
		// significant bit, we have to allocate an extra pad word.
		*( p = new __u32[ ( n = prime.size() ) + 1 ] ) = 0;

		q = const_cast<__u32 *>(prime.store());
		
		for ( i = 0; i < n; ++i ) p[ i + 1 ] = q[ i ];
		
		i = 1 + ( n - ( in_number_of_bits / 32 + ( ( t = in_number_of_bits % 32 ) ? 1 : 0 ) ) );

		if ( ! t ) {
			p[ i ] |= 0xc0000000;
		} else if ( t == 1 ) {
			p[ i ] |= 1;
			if ( i < n ) p[ i + 1 ] |= 0x80000000;
		} else {
			p[ i ] |= 0xc0000000 >> 32 - t;
		}

		// make sure that prime is odd
		p[ n ] |= 1;
		
		prime.set( n + 1, p );

		for ( i = 1; i < k_number_of_small_primes_under_2000; ++i ) {
			if ( ! ( prime % s_small_primes[ i ] ) ) goto next_prime;
		}

		if ( is_prime( prime, in_rounds, rng, in_observe, j ) ) {
#if ENABLE_APN_OBSERVATION
//			if ( in_observe ) in_observe->log( "found prime" );
			if ( in_observe ) in_observe->log( "*\n" );
#endif
			return apn( prime );
		}
		
	next_prime:	

		continue;				// codewarrior seems to barf if I jump to the end of a loop without a continue
	}
}


// rabin-miller from FIPS-186-2 appendix 2
bool is_prime( const apn &in_possible_prime, __u32 in_rounds, crypto_random &in_rng, class observed *in_observe, __u32 in_candidate_number ) {
	const __u32	   *p;
	__u32			i, j, n;
	apn				a, one = 1, possible_prime_minus_one = in_possible_prime - 1, m = possible_prime_minus_one;

	p = m.store();
	n = m.size() - 1;

	for ( ; n && ! p[ n ]; --n ) a += 32;
	for ( j = p[ n ]; ! ( j & 1 ); j >>= 1 ) ++a;
	m >>= a;

	for ( i = 1; i <= in_rounds; ++i ) {
#if ENABLE_APN_OBSERVATION
//		if ( in_observe ) in_observe->log( "Testing prime candidate #%u: Round %u of %u", in_candidate_number, i, in_rounds );
		if ( in_observe ) in_observe->log( "+", in_candidate_number, i, in_rounds );
#endif
		apn			b = random_apn( in_rng, one, in_possible_prime );
		apn			z = x_to_the_n_mod_m( b, m, in_possible_prime );

#if ! _WIN32
		pthread_testcancel();
#endif
		if ( z != one ) {	
			for ( j = 0; z != possible_prime_minus_one; ( z *= z ) %= in_possible_prime ) {
				if ( j && z == one || ++j == a ) return false;
			}
		}
	}

	return true;
}


const apn random_apn( const char *in_rand_file, __u32 in_num_bits, bool in_negative_return_values_ok, bool in_zero_return_value_ok, __u32 in_timeout_seconds ) {
	crypto_random			rng( in_rand_file );

	return random_apn( rng, in_num_bits, in_negative_return_values_ok, in_zero_return_value_ok, in_timeout_seconds );
}


const apn random_apn( crypto_random &in_rng, __u32 in_num_bits, bool in_negative_return_values_ok, bool in_zero_return_value_ok, __u32 in_timeout_seconds ) {
	if ( ! in_num_bits ) _throw( err_bad_parameter );

	__u32					i, n, size, *store;
	bool					negative;
	time_t					then;

	( store = new __u32[ ( size = ( in_num_bits + 31 ) / 32 ) + 1 ] )++;

	try {
		n = size * sizeof(__u32);
		then = time( nil ) + in_timeout_seconds;
	
		for ( ;; ) {
			in_rng.random( store, n );
			
			if ( in_zero_return_value_ok ) break;
			
			for ( i = 0; i < size && ! store[ i ]; ++i ) ;
			if ( i != size ) break;
			if ( time( nil ) >= then ) _throw( err_prng_broken );
#if ! _WIN32
			sleep( 1 );
#endif
		}
	} catch ( ... ) {
		delete[] store;
		throw;
	}

	n = in_num_bits % 32;

	if ( in_negative_return_values_ok ) {
		negative = *store & 1 << ( n ? n - 1 : 31 );
	} else {
		negative = false;
	}
	
	if ( negative ) {
		if ( n ) *store |= ~( -1 << n );
		*--store = __u32(-1);
	} else {
		if ( n ) *store &= ~( -1 << n );
		*--store = 0;
	}
	
	return apn( ++size, store );
}


// generates a random apn such that in_lower_bound < result < in_upper_bound,
// random data is pulled from the open file descriptor in_open_file_descriptor
const apn random_apn( const char *in_rand_file, const apn &in_lower_bound, const apn &in_upper_bound, __u32 in_timeout_seconds ) {
	crypto_random			rng( in_rand_file );

	return random_apn( rng, in_lower_bound, in_upper_bound, in_timeout_seconds );
}


const apn random_apn( crypto_random &in_rng, const apn &in_lower_bound, const apn &in_upper_bound, __u32 in_timeout_seconds ) {
	apn				r;
	apn				zero;
	time_t			then = time( nil ) + in_timeout_seconds;
	
	for ( ;; ) {
		r = random_apn( in_rng, in_upper_bound.size() * 32, *in_lower_bound.store() & 0x80000000, in_lower_bound < zero && in_upper_bound > zero, in_timeout_seconds );
		if ( ( r %= in_upper_bound ) > in_lower_bound ) break;
		if ( time( nil ) >= then ) _throw( err_timeout );
	}

	return apn( r );
}
