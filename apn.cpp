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
		

	File:				apn.cpp

	Author:				Brian Doyle
	Last Modified:		July 04, 2004

	Description:

	Defines a class for arbitrary precision integers.
	
	01.04.04 bd:	added auto-zeroing of buffers
	01.27.04 bd:	added <<= and >>= methods
	02.02.04 bd:	spent the last week debugging a lot of this stuff.  bleh.
	02.03.04 bd:	added gcd(), extended_euclid(), modular_inverse(), &
					chinese_remainder_theorem (both variants).
	
\*----------------------------------------------------------------------------------------*/
#include <string.h>

#include "apn.h"


#define zdelete( ptr, len ) do {								\
	if ( m_zero_buffers ) {										\
		if ( ptr ) {											\
			memset( ptr, 0, len );								\
			delete[] ptr;										\
		}														\
	} else {													\
		delete[] ptr;											\
	}															\
																\
	ptr = nil;													\
} while ( 0 )


#define zdelete_friend( obj, ptr, len ) do {					\
	if ( (obj).zero_buffers() ) {								\
		if ( ptr ) {											\
			memset( ptr, 0, len );								\
			delete[] ptr;										\
		}														\
	} else {													\
		delete[] ptr;											\
	}															\
																\
	ptr = nil;													\
} while ( 0 )


apn::apn() {
	m_zero_buffers = true;
	*( m_store = new __u32[ m_size = 1 ] ) = 0;
}


apn::~apn() {
	zdelete( m_store, m_size );
}


#pragma mark -


bool apn::operator!() const {
	__u32		i;
	
	for ( i = 0; i < m_size && ! m_store[ i ]; ++i ) ;
	
	return i == m_size;
}


apn &apn::operator=( const int &in_rhs ) {
	__u32	   *p, s;
	
	p = m_store;
	s = m_size;
	
	*( m_store = new __u32[ 1 ] ) = in_rhs;
	m_size = 1;

	zdelete( p, s );

	return *this;
}


apn &apn::operator=( const apn &in_rhs ) {
	__u32		i, *p, s;
	
	if ( this != &in_rhs ) {
		p = m_store;
		s = m_size;
		
		m_store = new __u32[ in_rhs.m_size ];
		m_size = in_rhs.m_size;
		
		zdelete( p, s );
		
		for ( i = 0; i < m_size; ++i ) m_store[ i ] = in_rhs.m_store[ i ];
	}
	
	return *this;
}

apn &apn::operator=( const char *in_rhs ) {
	apn			a( in_rhs, k_decimal );
	
	zdelete( m_store, m_size );

	m_size = a.m_size;
	m_store = a.m_store;
	a.m_store = nil;
	
	return *this;
}


apn &apn::operator=( const __u32 &in_rhs ) {
	__u32	   *p, s;

	p = m_store;
	s = m_size;
	
	init_u32( in_rhs );
	
	zdelete( p, s );

	return *this;
}


apn &apn::operator=( const octet_string &in_rhs ) {
	const __u8	   *data, *q;
	__u32			i, l, n, *p, s;
	__u8		   *r, t;

	p = m_store;
	s = m_size;
	
	m_store = new __u32[ n = ( l = in_rhs.length() ) / sizeof(__u32) + 1 ];
	m_size = n;
	
	zdelete( p, s );
	
	if ( ! ( data = in_rhs.data() ) ) {
		for ( i = 0; i < n; ++i ) m_store[ i ] = 0;
	} else {
		q = &data[ l - 1 ];
		r = reinterpret_cast<__u8 *>(&m_store[ m_size - 1 ]) + sizeof(__u32);
		
		while ( q >= data ) *--r = *q--;

		t = *r & 0x80 ? 0xff : 0;

		while ( r > reinterpret_cast<__u8 *>(m_store) ) *--r = t;
	}
	
#if BYTE_ORDER == LITTLE_ENDIAN
	for ( i = 0; i < m_size; ++i ) {
		m_store[ i ] = swap32( m_store[ i ] );
	}
#endif

	optimize();
	
	return *this;
}


apn &apn::operator+=( const apn &in_rhs ) {
	__u32	   *l, *r, *d, *e, n;

	if ( m_size < in_rhs.m_size ) {
		d = l = extend_and_copy( n = in_rhs.m_size );
		r = in_rhs.m_store;
	} else if ( m_size > in_rhs.m_size ) {
		d = r = const_cast<apn &>(in_rhs).extend_and_copy( n = m_size );
		l = m_store;
	} else {
		l = m_store;
		r = in_rhs.m_store;
		n = m_size;
		d = nil;
	}

	e = new __u32[ n + 1 ];
	
	s_add( l, r, n, e );

	zdelete( d, n );
	zdelete( m_store, m_size );
	
	m_store = e;
	m_size = n + 1;

	optimize();

	return *this;
}


apn &apn::operator-=( const apn &in_rhs ) {
	__u32	   *l, *r, *d, *e, n;

	if ( m_size < in_rhs.m_size ) {
		d = l = extend_and_copy( n = in_rhs.m_size );
		r = in_rhs.m_store;
	} else if ( m_size > in_rhs.m_size ) {
		d = r = const_cast<apn &>(in_rhs).extend_and_copy( n = m_size );
		l = m_store;
	} else {
		l = m_store;
		r = in_rhs.m_store;
		n = m_size;
		d = nil;
	}

	e = new __u32[ n + 1 ];
	
	s_sub( l, r, n, e );

	zdelete( d, n );
	zdelete( m_store, m_size );
	
	m_store = e;
	m_size = n + 1;

	optimize();

	return *this;
}


apn &apn::operator*=( const apn &in_rhs ) {
	__u32		n, *p;
	
	p = new __u32[ n = m_size + in_rhs.m_size + 1 ];
	
	s_mul( m_store, in_rhs.m_store, m_size, in_rhs.m_size, p );

	zdelete( m_store, m_size );

	m_store = p;
	m_size = n;
	
	optimize();

	return *this;
}


apn &apn::operator/=( const apn &in_rhs ) {
	__u32	   *p;

	p = new __u32[ m_size ];
	
	s_div( m_store, in_rhs.m_store, m_size, in_rhs.m_size, p, nil, malloc, free );

	zdelete( m_store, m_size );
	m_store = p;
	
	optimize();

	return *this;
}


apn &apn::operator%=( const apn &in_rhs ) {
	__u32	   *div = new __u32[ m_size ];
	__u32	   *mod = new __u32[ in_rhs.m_size ];

	s_div( m_store, in_rhs.m_store, m_size, in_rhs.m_size, div, mod, malloc, free );

	zdelete( div, m_size );
	zdelete( m_store, m_size );
	
	m_store = mod;
	m_size = in_rhs.m_size;

	optimize();
	
	return *this;
}


apn &apn::operator<<=( __u32 in_bits ) {
	if ( ! in_bits ) return *this;

	__u32		i, n, t, shift_words, shift_bits, right_shift_amount, *p, s;

	shift_bits = in_bits % k_word_bits;
	shift_words = in_bits / k_word_bits;

	right_shift_amount = shift_bits ? k_word_bits - shift_bits : 0;

	p = m_store;
	s = m_size;
	
	m_store = new __u32[ n = m_size + shift_words + ( shift_bits ? 1 : 0 ) ];
	
	t = ( p[ 0 ] & 0x80000000 ) && shift_bits ? 0xffffffff << shift_bits : 0;
	
	for ( i = 0; i < n; ++i ) {
		if ( i < m_size ) t |= p[ i ] >> right_shift_amount;
		m_store[ i ] = t;
		t = i < m_size ? p[ i ] << shift_bits : 0;
	}
	
	m_size = n;

	zdelete( p, s );
	
	optimize();
	
	return *this;
}


apn &apn::operator>>=( __u32 in_bits ) {
	if ( ! in_bits ) return *this;
	
	bool		neg;
	__u32		t, shift_words, shift_bits, left_shift_amount, *p, *q;

	shift_bits = in_bits % k_word_bits;
	shift_words = in_bits / k_word_bits;

	left_shift_amount = shift_bits ? k_word_bits - shift_bits : 0;

	neg = m_store[ 0 ] & 0x80000000;
	p = ( q = &m_store[ m_size - 1 ] ) - shift_words;

	for ( ; p >= m_store; --p, --q ) {
		if ( p > m_store ) t = *( p - 1 ) << left_shift_amount;
		else t = neg && shift_bits ? 0xffffffff << left_shift_amount : 0;

		*q = ( *p >> shift_bits ) | t;
	}
	
	for ( t = neg ? 0xffffffff : 0; q >= m_store; --q ) *q = t;
	
	optimize();
	
	return *this;
}


#pragma mark -


void apn::set( const char *in_number, const numeric_base in_base ) {
	__u32		l, *p;
	bool		neg = false;
	char		c, *s = const_cast<char *>(in_number);
	
	p = m_store;
	l = m_size;
	
	*( m_store = new __u32[ m_size = 1 ] ) = 0;
	
	zdelete( p, l );

	if ( *s == '-' ) {
		neg = true;
		++s;
	} else if ( *s == '+' ) ++s;
	
	if ( in_base == k_decimal ) {
		for ( ; ( c = *s ) && c >= '0' && c <= '9'; ++s ) {
			*this *= 10;
			*this += c - 0x30;
		}
	} else {
		for ( ; ( c = *s ) && ( c >= '0' && c <= '9' || ( c |= 0x20 ) >= 'a' && c <= 'f' ); ++s ) {
			*this *= 16;
			if ( c <= '9' ) *this += c - 0x30;
			else if ( c <= 'f' ) *this += c - 0x57;
		}
	}
	
	if ( neg ) s_neg( m_store, m_size );

	optimize();
}


void apn::set( __u32 in_size, __u32 *in_store, bool in_copy_in_store ) {
	__u32			i, *p, s;
	
	p = m_store;
	s = m_size;
	
	if ( in_copy_in_store ) {
		m_store = new __u32[ in_size ];
		for ( i = 0; i < in_size; ++i ) m_store[ i ] = in_store[ i ];
	} else {
		m_store = in_store;
	}
	m_size = in_size;

	zdelete( p, s );

	optimize();
}


void apn::optimize() {
	__u32		i, j, n, *p, s;

	if ( *m_store & 0x80000000 ) for ( i = 0; i < m_size - 1 && m_store[ i ] == 0xffffffff && m_store[ i + 1 ] & 0x80000000; ++i ) ;
	else for ( i = 0; i < m_size - 1 && ! m_store[ i ] && ! ( m_store[ i + 1 ] & 0x80000000 ); ++i ) ;
	
	if ( i ) {
		p = m_store;
		s = m_size;
		
		m_store = new __u32[ m_size - i ];
		for ( j = 0, n = m_size -= i; n; ++i, ++j, --n ) m_store[ j ] = p[ i ];
		
		zdelete( p, s );
	}
}


__u32 apn::num_digits( __u32 in_base ) const {
	__u32	result;
	apn		a( *this ), base( in_base );
	
	for ( result = 1; !! ( a /= base ); ++result ) ;

	return result;
}


char *apn::value( numeric_base in_base, bool in_hex_letters_in_caps ) const {
	enum { k_grow_by = 256 };

	int			c, i, h;
	__u32		j, k, n = 0;
	char	   *p = nil, *q, *r, s;
	apn			a( *this ), base( in_base );

	i = j = 0;
	h = in_hex_letters_in_caps ? 0x37 : 0x57;
	
	for ( ;; ) {
		s = n;
		q = new char[ n = ( ++j * k_grow_by ) ];

		if ( p != nil ) {
			for ( k = 0; k < __u32(i); ++k ) q[ k ] = p[ k ];
			zdelete( p, s );
			p = q;
		} else {
			p = q;
			if ( *m_store & 0x80000000 ) {
				a = -a;
			}
		}

		for ( ; __u32(i) < n; ++i ) {
			c = ( a % base ).m_store[ 0 ];
			c +=  c <= 9 ? 0x30 : h;
			a /= base;

			p[ i ] = c;
			
			if ( ! a ) {
				++i;
				goto reverse_p;
			}
		}
	}

reverse_p:

	if ( *m_store & 0x80000000 ) { *( q = new char[ i + 2 ] ) = '-'; r = q + 1; }
	else r = q = new char[ i + 1 ];
	
	for ( k = 0; --i >= 0; ++k ) r[ k ] = p[ i ];

	r[ k ] = 0;
	
	zdelete( p, n );
	
	return q;
}


void apn::print( numeric_base in_base, FILE *in_stream, bool in_hex_letters_in_caps ) const {
	char	   *val = value( in_base, in_hex_letters_in_caps );
	
	fprintf( in_stream, "%s", val );
	zdelete( val, strlen( val ) );
}


void apn::as_octet_string( octet_string &out_os, __u32 in_min_width ) const {
	__u32			i, j, k, n;
	__u8		   *p, *q, *r, v;

#if BYTE_ORDER == LITTLE_ENDIAN
	for ( i = 0; i < m_size; ++i ) m_store[ i ] = swap32( m_store[ i ] );
#endif

	p = reinterpret_cast<__u8 *>(m_store);
	q = reinterpret_cast<__u8 *>(&m_store[ m_size - 1 ]) + sizeof(__u32);

	if ( *p & 0x80 ) {
		while ( p < q && *p == 0xff ) ++p;
		if ( p == q || ! ( *p & 0x80 ) ) --p;
	} else {
		while ( p < q && ! *p ) ++p;
		if ( p == q || *p & 0x80 ) --p;
	}
	
	n = __u32(q - p);

	r = new __u8[ j = max( in_min_width, n ) ];

	for ( i = 0, v = p[ i ] & 0x80 ? 0xff : 0, k = j - n; i < k; ++i ) r[ i ] = v;
	for ( k = 0; i < j; ++i, ++k ) r[ i ] = p[ k ];

#if BYTE_ORDER == LITTLE_ENDIAN
	for ( i = 0; i < m_size; ++i ) m_store[ i ] = swap32( m_store[ i ] );
#endif
	
	out_os.set( r, j, false );
}


__u32 *apn::extend_and_copy( __u32 in_to_size ) {
	__u32		i, j, n, *r, set;
	
	r = new __u32[ in_to_size = max( in_to_size, m_size ) ];
	
	if ( m_store[ 0 ] & 0x80000000 ) set = 0xffffffff;
	else set = 0;

	for ( i = 0, n = in_to_size - m_size; i < n; ++i ) r[ i ] = set;
	for ( j = 0; i < in_to_size; ++i, ++j ) r[ i ] = m_store[ j ];	

	return r;
}


void apn::clear_and_zero() {
	__u32		i;
	
	for ( i = 0; i < m_size; ++i ) m_store[ i ] = 0;
	optimize();
}


void apn::init_s32( const __s32 in_rhs ) {
	*( m_store = new __u32[ m_size = 1 ] ) = in_rhs;
}


void apn::init_u32( const __u32 in_rhs ) {
	if ( in_rhs & 0x80000000 ) {
		*( m_store = new __u32[ m_size = 2 ] ) = 0;
		m_store[ 1 ] = in_rhs;
	} else {
		*( m_store = new __u32[ m_size = 1 ] ) = in_rhs;
	}
}


__u32 apn::size( bool in_ignore_leading_zero_words ) const {
	__u32					i, result;
	
	result = m_size;

	if ( in_ignore_leading_zero_words && result > 1 ) {
		for ( i = 0; i < m_size - 1; ++i, --result ) {
			if ( m_store[ i ] ) break;
		}
	}
	
	return result;
}


#pragma mark -


const apn operator-( const apn &in_rhs ) {
	__u32	   *p = new __u32[ in_rhs.m_size ];

	for ( __u32 i = 0; i < in_rhs.m_size; ++i ) p[ i ] = in_rhs.m_store[ i ];

	s_neg( p, in_rhs.m_size );

	return apn( in_rhs.m_size, p );
}


const bool operator==( const apn &in_lhs, const apn &in_rhs ) {
	__u32		l, r, n;
	
	l = r = 0;
	
	if ( in_lhs.m_size > in_rhs.m_size ) { for ( n = in_lhs.m_size - in_rhs.m_size; l < n; ++l ) if ( in_lhs.m_store[ l ] ) return false; }
	else if ( in_lhs.m_size < in_rhs.m_size ) { for ( n = in_rhs.m_size - in_lhs.m_size; r < n; ++r ) if ( in_lhs.m_store[ r ] ) return false; }

	for ( ; l < in_lhs.m_size; ++l, ++r ) if ( in_lhs.m_store[ l ] != in_rhs.m_store[ r ] ) return false;
	
	return true;
}


const bool operator<( const apn &in_lhs, const apn &in_rhs ) {
	bool		neg;
	__u32		l, r, n;

	if ( ( neg = in_lhs.m_store[ 0 ] & 0x80000000 ) != bool(in_rhs.m_store[ 0 ] & 0x80000000) ) {
		return in_lhs.m_store[ 0 ] & 0x80000000;
	}

	l = r = 0;
	
	if ( neg ) {	
		if ( in_lhs.m_size > in_rhs.m_size ) { for ( n = in_lhs.m_size - in_rhs.m_size; l < n; ++l ) if ( in_lhs.m_store[ l ] != 0xffffffff ) return true; }
		else if ( in_rhs.m_size > in_lhs.m_size ) { for ( n = in_rhs.m_size - in_lhs.m_size; r < n; ++r ) if ( in_rhs.m_store[ r ] != 0xffffffff ) return false; }
		
		for ( ; l < in_lhs.m_size; ++l, ++r ) if ( in_lhs.m_store[ l ] != in_rhs.m_store[ r ] ) return in_lhs.m_store[ l ] > in_rhs.m_store[ r ];
	} else {
		if ( in_lhs.m_size > in_rhs.m_size ) { for ( n = in_lhs.m_size - in_rhs.m_size; l < n; ++l ) if ( in_lhs.m_store[ l ] ) return false; }
		else if ( in_rhs.m_size > in_lhs.m_size ) { for ( n = in_rhs.m_size - in_lhs.m_size; r < n; ++r ) if ( in_rhs.m_store[ r ] ) return true; }
		
		for ( ; l < in_lhs.m_size; ++l, ++r ) if ( in_lhs.m_store[ l ] != in_rhs.m_store[ r ] ) return in_lhs.m_store[ l ] < in_rhs.m_store[ r ];
	}
	
	return false;	
}


const bool operator>( const apn &in_lhs, const apn &in_rhs ) {
	bool		neg;
	__u32		l, r, n;

	if ( ( neg = in_lhs.m_store[ 0 ] & 0x80000000 ) != bool(in_rhs.m_store[ 0 ] & 0x80000000) ) {
		return in_rhs.m_store[ 0 ] & 0x80000000;
	}

	l = r = 0;
	
	if ( neg ) {	
		if ( in_lhs.m_size > in_rhs.m_size ) { for ( n = in_lhs.m_size - in_rhs.m_size; l < n; ++l ) if ( in_lhs.m_store[ l ] != 0xffffffff ) return false; }
		else if ( in_rhs.m_size > in_lhs.m_size ) { for ( n = in_rhs.m_size - in_lhs.m_size; r < n; ++r ) if ( in_rhs.m_store[ r ] != 0xffffffff ) return true; }
		
		for ( ; l < in_lhs.m_size; ++l, ++r ) if ( in_lhs.m_store[ l ] != in_rhs.m_store[ r ] ) return in_lhs.m_store[ l ] < in_rhs.m_store[ r ];
	} else {
		if ( in_lhs.m_size > in_rhs.m_size ) { for ( n = in_lhs.m_size - in_rhs.m_size; l < n; ++l ) if ( in_lhs.m_store[ l ] ) return true; }
		else if ( in_rhs.m_size > in_lhs.m_size ) { for ( n = in_rhs.m_size - in_lhs.m_size; r < n; ++r ) if ( in_rhs.m_store[ r ] ) return false; }
		
		for ( ; l < in_lhs.m_size; ++l, ++r ) if ( in_lhs.m_store[ l ] != in_rhs.m_store[ r ] ) return in_lhs.m_store[ l ] > in_rhs.m_store[ r ];
	}
	
	return false;	
}


#pragma mark -


// binomial coefficient ( n! / ( k! * ( n - k )! )
const apn binomial( const apn &in_n, const apn &in_k ) {
	if ( ! in_n ) return ! in_k ? apn( 1 ) : apn( 0 );

	apn			dividend( 1 ), divisor( 1 ), i( 1 ), n_minus_k( in_n - in_k );

	for ( ; i <= in_k; ++i ) divisor *= i;
	for ( i = in_n; i > n_minus_k; --i ) dividend *= i;
	
	return apn( dividend / divisor );
}


#if ENABLE_GARNERS_ALGORITHM_FOR_CHINESE_REMAINDER_THEOREM

// garner's algorithm
const apn chinese_remainder_theorem( __u32 in_num_moduli, const apn *in_moduli, const apn *in_modular_residues ) {
	__u32		i, j;
	apn		   *c = new apn[ j = in_num_moduli - 1 ];
	
	for ( i = 0; i < j; ++i ) c[ i ].set_zero_buffers();
	
	for ( i = 1; i < in_num_moduli; ++i ) {
		for ( c[ i - 1 ] = 1, j = 0; j < i; ++j ) {
			( c[ i - 1 ] *= modular_inverse( in_moduli[ j ], in_moduli[ i ] ) ) %= in_moduli[ i ];
		}
	}
	
	apn			x = in_modular_residues[ 0 ], product( apn::k_apn_postpone_init ), u( apn::k_apn_postpone_init );
	
	for ( i = 1; i < in_num_moduli; ++i ) {
		( ( u = ( in_modular_residues[ i ] - x ) ) *= c[ i - 1 ] ) %= in_moduli[ i ];
		for ( product = 1, j = 0; j < i; ++j ) product *= in_moduli[ j ];
		x += product *= u;
	}
	
	if ( x.is_negative() ) {
		for ( product = 1, i = 0; i < in_num_moduli; ++i ) product *= in_moduli[ i ];
		x += product;
	}
	
	delete[] c;
	
	return apn( x );
}	

#else

// my implementation of the crt algorithm (using extended_euclid()) at:
//
// http://en2.wikipedia.org/wiki/Chinese_remainder_theorem
const apn chinese_remainder_theorem( __u32 in_num_moduli, const apn *in_moduli, const apn *in_modular_residues ) {
	__u32		i;
	apn			product_of_moduli = 1;
	apn			sum, e( apn::k_apn_postpone_init ), u1( apn::k_apn_postpone_init ), u2( apn::k_apn_postpone_init ), u3( apn::k_apn_postpone_init );

	for ( i = 0; i < in_num_moduli; ++i ) product_of_moduli *= in_moduli[ i ];

	for ( i = 0; i < in_num_moduli; ++i ) {
		extended_euclid( in_moduli[ i ], e = product_of_moduli / in_moduli[ i ], u1, u2, u3 );
		sum += in_modular_residues[ i ] * u2 * e;
	}
	
	sum %= product_of_moduli;
	
	return apn( sum.is_negative() ? sum + product_of_moduli : sum );
}

#endif


// performs in_lhs / in_rhs and in_lhs % in_rhs concurrently.
void div_and_mod( const apn &in_lhs, const apn &in_rhs, apn &out_div_result, apn &out_mod_result ) {
	__u32	   *p, *q, s, t;

	p = out_div_result.m_store;
	q = out_mod_result.m_store;

	s = out_div_result.m_size;
	t = out_mod_result.m_size;
	
	out_div_result.m_store = new __u32[ in_lhs.m_size ];
	out_mod_result.m_store = new __u32[ in_rhs.m_size ];

	out_div_result.m_size = in_lhs.m_size;
	out_mod_result.m_size = in_rhs.m_size;

	zdelete_friend( out_div_result, p, s );
	zdelete_friend( out_mod_result, q, t );

	s_div( in_lhs.m_store, in_rhs.m_store, in_lhs.m_size, in_rhs.m_size, out_div_result.m_store, out_mod_result.m_store, malloc, free );

	out_div_result.optimize();	
	out_mod_result.optimize();
}


void extended_euclid( const apn &in_u, const apn &in_v, apn &out_u1, apn &out_u2, apn &out_u3 ) {
	if ( ! in_u || ! in_v ) {
		out_u1 = out_u2 = out_u3 = 0;
		return;
	}

	apn		v1;
	apn		v2 = 1;
	apn		v3 = in_v.is_negative() ? -in_v : in_v;
	apn		q( apn::k_apn_postpone_init ), t1( apn::k_apn_postpone_init ), t2( apn::k_apn_postpone_init ), t3( apn::k_apn_postpone_init );
	
	out_u1 = 1;
	out_u2 = 0;
	out_u3 = in_u.is_negative() ? -in_u : in_u;
	
	while ( !! v3 ) {
		q = out_u3 / v3;

		t1 = out_u1 - v1 * q;
		t2 = out_u2 - v2 * q;
		t3 = out_u3 - v3 * q;

		out_u1 = v1;
		out_u2 = v2;
		out_u3 = v3;
		
		v1 = t1;
		v2 = t2;
		v3 = t3;
	}
}


// binary gcd algorithm, Knuth p. 338 4.5.2
const apn gcd( const apn &in_lhs, const apn &in_rhs ) {
	if ( ! in_lhs || ! in_rhs ) return apn( 0 );

	__u32		i, j, k, n, *p, *q;
	apn			u( in_lhs ), v( in_rhs ), zero;

	if ( u.is_negative() ) s_neg( u.m_store, u.m_size );
	if ( v.is_negative() ) s_neg( v.m_store, v.m_size );

	for ( j = 0, p = &u.m_store[ u.m_size - 1 ], q = u.m_store; p >= q; --p ) {
		n = *p;
		
		for ( i = 1; i; i <<= 1, ++j ) if ( n & i ) goto b1_a;
	}
	
b1_a:

	for ( k = 0, p = &v.m_store[ v.m_size - 1 ], q = v.m_store; p >= q; --p ) {
		n = *p;

		for ( i = 1; i; i <<= 1, ++k ) if ( n & i ) goto b1_b;
	}
	
b1_b:

	if ( ( k = min( j, k ) ) ) {
		u >>= k;
		v >>= k;
	}
	
	apn			t( apn::k_apn_postpone_init );
	
	t = u.is_odd() ? -v : u;

	do {
		if ( t.is_even() ) for( j = 0, p = &t.m_store[ t.m_size - 1 ], q = t.m_store; p >= q; --p ) {
			n = *p;

			for ( i = 1; i; i <<= 1, ++j ) if ( n & i ) goto b3_b;
			
			continue;
			
		b3_b:
		
			if ( j ) t >>= j;

			break;
		}

		if ( t > zero ) u = t;
		else v = -t;
		
		t = u - v;
	} while ( !! t );
	
	return apn( u <<= k );
}


const apn modular_inverse( const apn &in_x, const apn &in_m ) {
	// a variant of the extended euclid above, ignores u2, v2, t2 and avoids negative numbers

	bool	subtract = false;
	apn		u1 = 1, u3 = in_x;
	apn		v1 = 0, v3 = in_m;
	apn		q( apn::k_apn_postpone_init ), t1( apn::k_apn_postpone_init ), t3( apn::k_apn_postpone_init );
	
	while ( !! v3 ) {
		div_and_mod( u3, v3, q, t3 );

		t1 = u1 + q * v1;
		
		u1 = v1;
		v1 = t1;
		u3 = v3;
		v3 = t3;
		
		subtract = ! subtract;
	}
	
	if ( u3 != 1 ) _throw( err_unsolvable );
	
	return apn( subtract ? in_m - u1 : u1 );
}


#if ! USE_LR_BINARY_EXPONENTIATION

// right-to-left binary exponentiation from Knuth v2, 4.6.3 p.462 (Algorithm A)
// for calculation of x**n.  this method assumes that n is a positive integer.
const apn x_to_the_n( const apn &in_x, const apn &in_n ) {
	__u32		i, j, k, t;
	__u32		n_len = in_n.m_size;
	__u32	   *n_buf = in_n.m_store;

	// determine the position of the most significant bit in m_store.
	for ( i = 0; i < n_len && ! n_buf[ i ]; ++i ) ;

	// if no bits were set in m_store, in_n is zero, so return 1.
	if ( i == n_len ) return apn( 1 );

	apn			y = 1, z = in_x;

	// iterate over the digits of m_store from right to left, performing Knuth's
	// algorithm on all the digits up to and including the i + 1'th digit.
	for ( j = n_len - 1; j > i; --j ) {
		for ( k = 0, t = n_buf[ j ]; ; t >>= 1 ) {
			if ( t & 1 ) y *= z;
			if ( ++k == 32 ) break;
			z *= z;
		}
	}
	
	// now we are at in_n.m_store[ i ] and we need to repeat the calculation loop
	// above for the final (leftmost) digit.
	for ( t = n_buf[ i ]; ; ) {
		if ( t & 1 ) y *= z;
		if ( ! ( t >>= 1 ) ) break;
		z *= z;
	}
	
	return apn( y );
}


// implements x**n % m
const apn x_to_the_n_mod_m( const apn &in_x, const apn &in_n, const apn &in_m ) {
	__u32		i, j, k, t;
	__u32		n_len = in_n.m_size;
	__u32	   *n_buf = in_n.m_store;

	// determine the position of the most significant bit in m_store.
	for ( i = 0; i < n_len && ! n_buf[ i ]; ++i ) ;

	// if no bits were set in m_store, in_n is zero, so return 1.
	if ( i == n_len ) return apn( 1 );

	apn			y = 1, z = in_x;

	// iterate over the digits of m_store from right to left, performing Knuth's
	// algorithm on all the digits up to and including the i + 1'th digit.
	for ( j = n_len - 1; j > i; --j ) {
		for ( k = 0, t = n_buf[ j ]; ; t >>= 1 ) {
			if ( t & 1 ) ( y *= z ) %= in_m;
			if ( ++k == 32 ) break;
			( z *= z ) %= in_m;
		}
	}
	
	// now we are at in_n.m_store[ i ] and we need to repeat the calculation loop
	// above for the final (leftmost) digit.
	for ( t = n_buf[ i ]; ; ) {
		if ( t & 1 ) ( y *= z ) %= in_m;
		if ( ! ( t >>= 1 ) ) break;
		( z *= z ) %= in_m;
	}
	
	return apn( y );
}

#else

// left-to-right binary exponentiation from Knuth v2, 4.6.3 p.461
// for calculation of x**n.  this method assumes that n is a positive integer.
const apn x_to_the_n( const apn &in_x, const apn &in_n ) {
	__u32		i, j, t;
	__u32		n_len = in_n.m_size;
	__u32	   *n_buf = in_n.m_store;

	// determine the position of the most significant bit in m_store.
	for ( i = 0; i < n_len && ! n_buf[ i ]; ++i ) ;

	// if no bits were set in m_store, in_n is zero, so return 1.
	if ( i == n_len ) return apn( 1 );

	apn			y = in_x;

	// proceed from most significant bit of most significant digit of in_n
	for ( t = 0x80000000, j = n_buf[ i ]; ! ( t & j ); t >>= 1 ) ;
	
	while ( t >>= 1 ) {
		y *= y;
		if ( t & j ) y *= in_x;
	}

	while ( ++i < n_len ) {
		t = 0x80000000;
		j = n_buf[ i ];

		do {
			y *= y;
			if ( t & j ) y *= in_x;
		} while ( t >>= 1 );
	}

	return apn( y );
}


// implements x**n % n
const apn x_to_the_n_mod_m( const apn &in_x, const apn &in_n, const apn &in_m ) {
	__u32		i, j, t;
	__u32		n_len = in_n.m_size;
	__u32	   *n_buf = in_n.m_store;

	// determine the position of the most significant bit in m_store.
	for ( i = 0; i < n_len && ! n_buf[ i ]; ++i ) ;

	// if no bits were set in m_store, in_n is zero, so return 1.
	if ( i == n_len ) return apn( 1 );

	apn			y = in_x;

	// proceed from most significant bit of most significant digit of in_n
	for ( t = 0x80000000, j = n_buf[ i ]; ! ( t & j ); t >>= 1 ) ;

	while ( t >>= 1 ) {
		( y *= y ) %= in_m;
		if ( t & j ) ( y *= in_x ) %= in_m;
	}

	while ( ++i < n_len ) {
		t = 0x80000000;
		j = n_buf[ i ];

		do {
			( y *= y ) %= in_m;
			if ( t & j ) ( y *= in_x ) %= in_m;
		} while ( t >>= 1 );
	}

	return apn( y );
}

#endif
