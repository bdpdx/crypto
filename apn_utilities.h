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
		

	File:				apn_utilities.h

	Author:				Brian Doyle
	Date Created:		December 18, 2003
	Last Modified:		July 19, 2004

	Description:

	Some handy methods that use my apn class (prime number generation, random
	number generation).

\*----------------------------------------------------------------------------------------*/
#ifndef __apn_utilities_h__
#define __apn_utilities_h__



#if _WIN32
	#include "precompiled.h"
#endif


#include "apn.h"
#include "crypto_random.h"


// these are useful for secret_share.hpp
#define k_largest_8_bit_prime		/* 2^8-5		*/		"251"
#define k_largest_16_bit_prime		/* 2^16-15		*/		"65521"
#define k_largest_32_bit_prime		/* 2^32-5		*/		"4294967291"
#define k_largest_64_bit_prime		/* 2^64-59		*/		"18446744073709551557"
#define k_largest_128_bit_prime		/* 2^128-159	*/		"340282366920938463463374607431768211297"
#define k_largest_129_bit_prime		/* 2^129-25		*/		"680564733841876926926749214863536422887"
#define k_largest_160_bit_prime		/* 2^160-91		*/		"1461501637671185285124623296179657627087700754341"
#define k_largest_190_bit_prime		/* 2^190-11		*/		"1569275433846670190958947355801916604025588861116008628213"
#define k_largest_193_bit_prime		/* 2^193-31		*/		"12554203470773361527671578846415332832204710888928069025761"
#define k_largest_222_bit_prime		/* 2^222-117	*/		"6739986666787659948666753771754907668409286105635143120275902562187"
#define k_largest_256_bit_prime		/* 2^256-189	*/		"115792089237316195423570985008687907853269984665640564039457584007913129639747"
#define k_largest_257_bit_prime		/* 2^257-93		*/		"231584178474632390847141970017375815706539969331281128078915168015826259279779"
#define k_largest_512_bit_prime		/* 2^512-21		*/		"13407807933064345649890017229587443357272532126340991520388532594580920989698918715621639918287209530753523829346312981975160505707394545625721934312374251"
#define k_largest_1024_bit_prime	/* 2^1024-1213	*/		"179769313528087395741144086303450326840704018619285532730677487698504175351217801067276294607298097693608233337980560268500709566673289485215203447967538880528934201252734890646709371888047166795477970225045764292873488475247079839549689546318817643202714037694630034480910301280903270334728420340707058776899"
#define k_largest_2048_bit_prime	/* 2^2048-603	*/		"32317006078835396625264231138682247627682159320204145324538818293374934578929232663858239500413917073212573451459425493352187795182553959917000271806481266856404655683081419060377383332514791937055117219078033727497592853180627707187367802158127539089169648825873534897323306859884061054441681751387932443972962055994239796039890447409722111411227631799457983570809920781373111623137550639104495523601043568727020055390224157015970850517102013211391835830806270961167333180816309923030489848485326454617827644452113309506101065422018864704999424290685708849316133858930572420136044302433278332171314400015962385415589"


enum small_primes_constants {
	k_number_of_small_primes_under_2000		=	303
};

enum apn_utilities_constants {
	k_default_rabin_miller_rounds			=	50
};

extern __u32 s_small_primes[ k_number_of_small_primes_under_2000 ];


// generates a probable prime that is in_bits bits in length.  the routine uses
// random numbers from in_rand_file and tests all potential prime candidates for
// divisibilty by all primes < 2,000 followed by 50 rounds of rabin-miller testing.
const apn generate_prime( __u32 in_bits, const char *in_rand_file = "/dev/urandom", __u32 in_rounds = k_default_rabin_miller_rounds, class observed *in_observe = nil );

// implements in_rounds iterations of the rabin-miller primality test.  returns true if
// in_possible_prime passes the test.  note that it is a good idea to test any prospective prime
// for divisibility by all small primes under 2,000 (per applied cryptography p. 260).
// the generate_prime() routine above performs this test prior to making the rabin-miller
// test.
//
// in_random_number_file_open_descriptor is an open file descriptor from which to gather
// random numbers for the witnesses.
//
// it is the caller's responsibility to ensure that on entry in_possible_prime >= 2
bool is_prime( const apn &in_possible_prime, __u32 in_rounds, crypto_random &in_rng, class observed *in_observe = nil, __u32 in_candidate_number = 1 );

const apn random_apn( const char *in_rand_file, __u32 in_num_bits, bool in_negative_return_values_ok = false, bool in_zero_return_value_ok = false, __u32 in_timeout_seconds = 60 );
const apn random_apn( const char *in_rand_file, const apn &in_lower_bound, const apn &in_upper_bound, __u32 in_timeout_seconds = 60 );

const apn random_apn( crypto_random &in_rng, __u32 in_num_bits, bool in_negative_return_values_ok = false, bool in_zero_return_value_ok = false, __u32 in_timeout_seconds = 60 );
const apn random_apn( crypto_random &in_rng, const apn &in_lower_bound, const apn &in_upper_bound, __u32 in_timeout_seconds = 60 );



#endif // __apn_utilities_h__
