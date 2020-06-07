/*----------------------------------------------------------------------------------------*\

	! BALANCE SOFTWARE CONFIDENTIAL !
	
	Copyright (c) 2003 Balance Software Corporation
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
		

	File:				apn.h

	Author:				Brian Doyle
	Last Modified:		February 03, 2004

	Description:

	Declares a class for arbitrary precision integers.
	
\*----------------------------------------------------------------------------------------*/
#ifndef __apn_h__
#define __apn_h__



#if _WIN32
	#include "precompiled.h"
#endif

#include <stdio.h>

#include "apn_primitives.h"
#include "octet_string.h"


class apn {		// for arbitrary precision integers
	
public:

	enum apn_postpone_init_t { k_apn_postpone_init = 1 };

	apn();
	apn( const int in_int ) { m_zero_buffers = true; init_s32( in_int ); }
	apn( const __u32 in_u32 ) { m_zero_buffers = true; init_u32( in_u32 ); }
	apn( const apn &in_apn ) { m_zero_buffers = true; m_store = nil; *this = in_apn; }
	apn( apn_postpone_init_t ) { m_zero_buffers = true; m_store = nil; }
	apn( const octet_string &in_octet_string ) { m_zero_buffers = true; m_store = nil; *this = in_octet_string; }
	apn( const char *in_number, const numeric_base in_base = k_decimal ) { m_zero_buffers = true; m_store = nil; set( in_number, in_base ); }
	apn( __u32 in_size, __u32 *in_store, bool in_copy_in_store = false ) { m_zero_buffers = true; m_store = nil; set( in_size, in_store, in_copy_in_store ); }

   ~apn();

	bool is_odd() const { return m_store[ m_size - 1 ] & 1; }
	bool is_even() const { return ! ( m_store[ m_size - 1 ] & 1 ); }
	bool is_negative() const { return ! is_positive(); }
	bool is_positive() const { return ! ( m_store[ 0 ] & 0x80000000 ); }

	__u32 as__u32() const { if ( m_size > 1 ) _throw( err_range ); return m_store[ 0 ]; }
	__s32 as__s32() const { if ( m_size > 1 ) _throw( err_range ); return __s32(m_store[ 0 ]); }

	// operator! is useful to test if an apn is zero (e.g. apn a; if ( ! a ) ...).
	// note that in order to test if an apn is *not* zero, it is necessary to actually
	// write either { apn a; if ( a != 0 ) ... } or the more crude but faster { if ( !! a ) .. },
	// rather than just attempt { if ( a ) ... }.  this is an unfortunate limitation
	// but required due to restrictions imposed by the implicit conversion rules of c++.
	bool operator!() const;
	
	operator const octet_string() const { octet_string os; as_octet_string( os ); return octet_string(os);  }

	apn &operator=( const int &in_rhs );
	apn &operator=( const apn &in_rhs );
	apn &operator=( const char *in_rhs );
	apn &operator=( const __u32 &in_rhs );
	// in_rhs must be in big-endian format
	apn &operator=( const octet_string &in_rhs );
	
	apn &operator++() { *this += 1; return *this; }
	const apn operator++( int ) { apn old = *this; ++*this; return apn( old ); }

	apn &operator--() { *this -= 1; return *this; }
	const apn operator--( int ) { apn old = *this; --*this; return apn( old ); }

	apn &operator+=( const apn &in_rhs );
	apn &operator-=( const apn &in_rhs );
	apn &operator*=( const apn &in_rhs );
	apn &operator/=( const apn &in_rhs );
	apn &operator%=( const apn &in_rhs );
	
	// unlike the C <<= operator which operates on fixed_size ints, apn::<<=
	// will extend the current integer by the shift amount rather than rolling
	// the high bits out.
	apn &operator<<=( __u32 in_bits );
	apn &operator>>=( __u32 in_bits );
	
	// note that these routines constrain the maximum shift to 2**32, but
	// I figure no one is going to need to shift that much anyway.
	apn &operator<<=( const apn &in_rhs ) { return *this <<= in_rhs.m_store[ 0 ]; }
	apn &operator>>=( const apn &in_rhs ) { return *this >>= in_rhs.m_store[ 0 ]; }

	// sets the value of this apn to the contents of a user-supplied buffer.
	// the buffer must be an integer in two's complement representation
	// allocated with new __u32[ in_size ].  if in_copy_in_store is true, the
	// buffer will be copied.  if in_copy_in_store is false, the buffer will
	// be delete[]'ed by the apn and the caller should not therefore free it.
	void set( __u32 in_size, __u32 *in_store, bool in_copy_in_store = false );

	// sets the value of this apn to the value of the base in_base number
	// represented by the string in_number (in_number may be optionally preceded
	// by a '+' or '-' character to denote a positive or negative number
	// respectively).
	void set( const char *in_number, const numeric_base in_base = k_decimal );

	// allocates a string with new char[] and stores the current ASCII value of
	// *this in the given base in_base to it.  if the base is in_hexadecimal,
	// in_hex_letters_in_caps dicates whether hex numbers are represented in upper
	// or lower case.  the caller is responsible for delete[]ing the return array.
	char *value( numeric_base in_base = k_decimal, bool in_hex_letters_in_caps = false ) const;
	
	// stores the value of *this as a two's-complement base-256 string in out_os.
	// it is assumed that this method precludes a disk write or some other storage
	// operation therefore conversion to big-endian representation is automatic.
	void as_octet_string( octet_string &out_os, __u32 in_min_width = 0 ) const;
	
	// returns the number of digits in base in_base necessary to represent
	// the current apn (e.g. if *this == 100, num_digits( k_decimal ) == 3).
	__u32 num_digits( __u32 in_base = k_decimal ) const;

	// prints the ascii value of the current number in base in_base to
	// the supplied in_stream.
	void print( numeric_base in_base = k_decimal, FILE *in_stream = stdout, bool in_hex_letters_in_caps = false ) const;
	
	// returns the number of __u32's allocated for m_size
	__u32 size( bool in_ignore_leading_zero_words = false ) const;
	
	// returns a pointer to the m_store buffer
	const __u32 *store() const { return m_store; }
	
	// a "secure zero", zeros all the memory associated with the apn then sets
	// the value of the apn itself to zero.
	void clear_and_zero();

	// Sets up the apn such that any time m_store is deleted or optimized, any memory that is
	// to be released is zeroed before the release (for secure applications).  This is enabled
	// by default.
	void set_zero_buffers( bool in_zero = true ) { m_zero_buffers = in_zero; }
	
	bool zero_buffers() { return m_zero_buffers; }
	
protected:

	// constrains m_store to be the least number of __u32's needed to represent
	// the current value (m_store can grow beyond what is necessary due to 
	// allocation of buffers for mathematical operations).
	void optimize();

	// allocates a new __u32[] buffer that is max( m_size, in_to_size ) __u32's
	// long and copies m_store into it, extending the sign as necessary.
	__u32 *extend_and_copy( __u32 in_to_size = 0 );

	void init_s32( const __s32 in_val );
	void init_u32( const __u32 in_val );
	
	__u32		m_size;
	__u32	   *m_store;
	bool		m_zero_buffers;

private:

	friend const apn operator-( const apn &in_rhs );
	friend const bool operator<( const apn &in_lhs, const apn &in_rhs );
	friend const bool operator>( const apn &in_lhs, const apn &in_rhs );
	friend const bool operator==( const apn &in_lhs, const apn &in_rhs );
	
	friend const apn gcd( const apn &in_lhs, const apn &in_rhs );
	friend const apn x_to_the_n( const apn &in_x, const apn &in_n );
	friend const apn x_to_the_n_mod_m( const apn &in_x, const apn &in_n, const apn &in_m );

	friend void div_and_mod( const apn &in_lhs, const apn &in_rhs, apn &out_div_result, apn &out_mod_result );

};


#if 0
#pragma mark -
#endif


// returns the binomial coefficient (n choose k)
const apn binomial( const apn &in_n, const apn &in_k );

// if ENABLE_GARNERS_ALGORITHM_FOR_CHINESE_REMAINDER_THEOREM is true, the
// chinese_remainder_theorem() function will use garner's algorithm (faster)
// instead of the extended euclidian method.
#define	ENABLE_GARNERS_ALGORITHM_FOR_CHINESE_REMAINDER_THEOREM		1
// in_num_moduli is the number of elements in the in_moduli and in_coefficients array
// in_moduli is an array of in_num_moduli pairwise relatively prime integer moduli
// in_coefficients is an array of in_num_moduli coefficients ai
const apn chinese_remainder_theorem( __u32 in_num_moduli, const apn *in_moduli, const apn *in_coefficients );

// for operations where it is necessary to know both the result of a division
// and the remainder, this is more efficient than calling / and % separately.
void div_and_mod( const apn &in_lhs, const apn &in_rhs, apn &out_div_result, apn &out_mod_result );

// given positive integers in_u and in_v, extended_euclid determines a vector (u1, u2, u3)
// such that in_u * u1 + in_v * u2 = u3 = gcd( in_u, in_v )
void extended_euclid( const apn &in_u, const apn &in_v, apn &out_u1, apn &out_u2, apn &out_u3 );

// returns the greatest common divisor of in_lhs and in_rhs
const apn gcd( const apn &in_lhs, const apn &in_rhs );

// returns a such that 1 = (a * in_x) mod in_m (i.e. a**-1 is equivalent to in_x mod in_m).
// note that in_x and in_m must be relatively prime, otherwise modular_inverse will throw
// an err_unsolvable exception.
const apn modular_inverse( const apn &in_x, const apn &in_m );

// if USE_LR_BINARY_EXPONENTIATION is true, x_to_the_n.* will use left-to-right
// binary exponentiation, otherwise right-to-left binary exponentiation will
// be used.  left-to-right is probably faster for general use and is definitely
// faster if using fermat primes for exponentiation (e.g. in RSA).
#define USE_LR_BINARY_EXPONENTIATION							1
// returns x**n
const apn x_to_the_n( const apn &in_x, const apn &in_n );
// return x**n % m
const apn x_to_the_n_mod_m( const apn &in_x, const apn &in_n, const apn &in_m );


const apn operator-( const apn &in_rhs );
const bool operator<( const apn &in_lhs, const apn &in_rhs );
const bool operator>( const apn &in_lhs, const apn &in_rhs );
const bool operator==( const apn &in_lhs, const apn &in_rhs );

inline const bool operator<=( const apn &in_lhs, const apn &in_rhs ) { return ! ( in_lhs > in_rhs ); }
inline const bool operator>=( const apn &in_lhs, const apn &in_rhs ) { return ! ( in_lhs < in_rhs ); }
inline const bool operator!=( const apn &in_lhs, const apn &in_rhs ) { return ! ( in_lhs == in_rhs ); }

inline const apn operator+( const apn &in_rhs ) { return apn( in_rhs ); }
inline const apn operator+( const apn &in_lhs, const apn &in_rhs ) { return apn( in_lhs ) += in_rhs; }
inline const apn operator-( const apn &in_lhs, const apn &in_rhs ) { return apn( in_lhs ) -= in_rhs; }
inline const apn operator*( const apn &in_lhs, const apn &in_rhs ) { return apn( in_lhs ) *= in_rhs; }
inline const apn operator/( const apn &in_lhs, const apn &in_rhs ) { return apn( in_lhs ) /= in_rhs; }
inline const apn operator%( const apn &in_lhs, const apn &in_rhs ) { return apn( in_lhs ) %= in_rhs; }

inline const apn operator<<( const apn &in_lhs, const apn &in_rhs ) { return apn( in_lhs ) <<= in_rhs; }
inline const apn operator>>( const apn &in_lhs, const apn &in_rhs ) { return apn( in_lhs ) >>= in_rhs; }


// for use with ASN.1 routines:

// void pop_apn( asn1_ber_sequence &io_seq, apn *&out_apn );
#define pop_apn( _seq, _onto ) do {											\
	octet_string		   *os;												\
																			\
	os = (octet_string *) ( _seq ).pop( type, k_asn1_tag_integer );			\
	try { ( _onto ) = new apn( *os ); } catch ( ... ) { delete os; throw; }	\
	delete os;																\
} while ( 0 )

// void pop_apn_via( asn1_ber_sequence &io_seq, apn &out_apn );
#define pop_apn_via( _seq, _to_apn ) do {									\
	octet_string		   *os;												\
																			\
	os = (octet_string *) ( _seq ).pop( type, k_asn1_tag_integer );			\
	try { ( _to_apn ) = *os; } catch ( ... ) { delete os; throw; }			\
	delete os;																\
} while ( 0 )

// void push_apn( asn1_ber_sequence &io_seq, apn *in_apn );
#define push_apn( _seq, _from ) do {										\
	octet_string			os;												\
																			\
	( _from )->as_octet_string( os );										\
	( _seq ).push( k_asn1_tag_integer, &os );								\
} while ( 0 )

// void push_apn_via( asn1_ber_sequence &io_seq, apn &in_apn, octet_string &out_os );
#define push_apn_via( _seq, _from, _to_os ) do {							\
	( _from ).as_octet_string( _to_os );									\
	( _seq ).push( k_asn1_tag_integer, &( _to_os ) );						\
} while ( 0 )



#endif // __apn_h__
