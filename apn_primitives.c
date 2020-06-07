/**----------------------------------------------------------------------------------------*\

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
		

	File:				apn_primitives.c

	Author:				Brian Doyle
	Last Modified:		February 10, 2004

	Description:

	Definitions of the math primitives described by apn_core.h.  There are
	both C++ and PowerPC versions.  The PowerPC versions are enabled if either
	__MACH__ or __USE_PPC_APN_PRIMITIVES__ are defined.
	
\**---------------------------------------------------------------------------------------*/
#include "apn_primitives.h"


#if ! defined( __FORCE_C_APN_PRIMITIVES__ ) && __MWERKS__
	#define ENABLE_ASM_APN_PRIMITIVES	1
	#define ENABLE_PPC_APN_PRIMITIVES	1
#else
	#define ENABLE_ASM_APN_PRIMITIVES	0
#endif


#if ENABLE_ASM_APN_PRIMITIVES
#if ENABLE_PPC_APN_PRIMITIVES

#define HALF_WORD_IS_SHORT		1			// if 1, normal behavior, if 0, halfword is byte (for debugging)

#if HALF_WORD_IS_SHORT
#ifndef __balance_types
	typedef short				__s16;
	typedef unsigned short		__u16;
#endif
	#define HALF_WORD_BITS		16
	#define HALF_WORD_SIZE		2
	#define HALF_WORD_SHIFT		1
	#define LHZ					lhz
	#define LHZX				lhzx
	#define STH					sth
	#define STHX				sthx
#else
	typedef char				__s16;
	typedef unsigned char		__u16;
	#define HALF_WORD_BITS		8
	#define HALF_WORD_SIZE		1
	#define HALF_WORD_SHIFT		0
	#define LHZ					lbz
	#define LHZX				lbzx
	#define STH					stb
	#define STHX				stbx
	
	#ifdef __MWERKS__
		#pragma message( "compiling apn_asm.h with HALF_WORD_IS_SHORT set to false" )
	#else
		#warning( "compiling apn_asm.h with HALF_WORD_IS_SHORT set to false" )
	#endif
#endif

#define WORD_BITS				32
#define WORD_SIZE				4			// size in bytes of a register word ( sizeof(long) )
#define WORD_SHIFT				2			// word alignment multiplier


#endif // ENABLE_PPC_APN_PRIMITIVES
#endif // ENABLE_ASM_APN_PRIMITIVES


#ifndef WORD_SIZE
#define WORD_SIZE				4
#define HALF_WORD_SIZE			2
#endif


#if ! ENABLE_ASM_APN_PRIMITIVES

// negates the io_rhs operand in place (returns in io_rhs the two's complement of in_rhs) ( io_rhs = -io_rhs );
// io_rhs is an arbitrary precision big-endian signed two's complement integer
// in_n is length in 32-bit words of io_rhs
void s_neg( register __u32 *io_rhs, register __u32 in_n ) {
	__u64		k = 1;

	while ( in_n-- ) {
		k += (__u64)(~io_rhs[ in_n ]);
		io_rhs[ in_n ] = (__u32) k;
		k >>= 32;
	}
}

#else // ENABLE_ASM_APN_PRIMITIVES
#if ENABLE_PPC_APN_PRIMITIVES

// tested bd: 05.19.03 ok
void s_neg( register __u32 *io_rhs, register __u32 in_n ) {
	register __u32			a, b, c, i, s, t;

asm {
	li			s, WORD_SIZE;										//	s = sizeof(__u32);
	lis			t, 0x2000;											//	set_carry_bit();
	mtxer		t;													//
	slwi		i, in_n, WORD_SHIFT;								//	i = in_n * sizeof(__u32);
																	//
	complement_loop:												//	do {
																	//
		sub.		i, i, s;										//		i -= s;
		lwzx		a, io_rhs, i;									//		a = io_rhs[ i ];
		not			b, a;											//		b = ~a;
		addze		c, b;											//		update_carry_bit_after( c = b + carry_bit );
		stwx		c, io_rhs, i;									//		io_rhs[ i ] = c;
		bne+		complement_loop;								//	} while ( i );
}
}

#endif // ENABLE_PPC_APN_PRIMITIVES
#endif // ENABLE_ASM_APN_PRIMITIVES


#if ! ENABLE_ASM_APN_PRIMITIVES

// adds two arbitrary precision big-endian signed two's complement integers in_lhs and in_rhs such that out_result = in_lhs + in_rhs;
// in_n is the length in 32-bit words of in_lhs and in_rhs.
// out_result must point to a region of memory in_n+1 32-bit words in length.
void s_add( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 *out_result ) {
	__u64		k = 0;
	__u32		a, b, c, ci;
	
	while ( in_n-- ) {
		k += (__u64)(a = in_lhs[ in_n ] ) + (__u64) ( b = in_rhs[ in_n ]);
		out_result[ in_n + 1 ] = (__u32)(k & 0xffffffff);
		k >>= 32;
	}
	
	// c is the value of the most-significant bit of ( in_lhs + in_rhs ) % ( in_n * 32 ).
	// in other words, it is the value of the of the bit in out_result that is in the
	// same position as the leftmost bit of in_lhs/in_rhs.
	c = out_result[ 1 ] & 0x80000000;

	// determine if a carry occurred into bit position c during the addition
	if ( a & 0x80000000 ) {
		ci = c ^ ( b & 0x80000000 ) ? 0 : 1;
	} else if ( b & 0x80000000 ) {
		ci = c ^ ( a & 0x80000000 ) ? 0 : 1;
	} else {
		ci = c ? 1 : 0;
	}
	
	// if the carry into bit position c is not the same as the carry out of bit position c,
	// then overflow occurred, and we need to bit extend the carry into out_result[ 0 ].
	//
	// if the carry into bit c is the same as the carry out of bit c, then no overflow
	// occurred and we need to ignore the carry and extend the value of c.
	a = ci ^ (__u32) k ? (__u32) k : c;

	out_result[ 0 ] = a ? 0xffffffff : 0;
}

#else // ENABLE_ASM_APN_PRIMITIVES
#if ENABLE_PPC_APN_PRIMITIVES

// tested bd: 05.23.03 ok
void s_add( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 *out_result ) {
	register __u32			a, b, c, d, i, t, z;

asm {
	addi		out_result, out_result, WORD_SIZE;					//	out_result += sizeof(__u32);
	li			i, WORD_SIZE;										//	i = sizeof(__u32);
	li			z, 0;												//	clear_carry_bit();
	mtxer		z;													//
	slwi		in_n, in_n, WORD_SHIFT;								//	in_n *= sizeof(__u32);
																	//
	add_loop:														//	do {
																	//
		sub.		in_n,	in_n,	i;								//		in_n -= i;
		lwzx		a, in_lhs, in_n;								//		a = in_lhs[ in_n ];
		lwzx		b, in_rhs, in_n;								//		b = in_rhs[ in_n ];
		addeo		c, a, b;										//		c = a + b + carry_bit;
		stwx		c, out_result, in_n;							//		out_result[ in_n + 1 ] = c;
		bne+		add_loop										//	} while ( in_n );
																	//
	mcrxr		cr0;												//	if ( ! overflow_bit_is_set() ) {
	bc			12, 1, test_carry_bit;								//
																	//
		andis.		d, c, 0x8000;									//		if ( ! ( c & 0x80000000 ) ) goto extend_sign_zero;
		beq			extend_sign_zero;								//
																	//
		extend_sign_minus_one:										//		extend_sign_minus_one:
																	//
		li		t, -1;												//		t = 0xffffffff;
		stw		t, -WORD_SIZE(out_result);							//		out_result[ 0 ] = t;
																	//
		b		done;												//		return;
																	//
	test_carry_bit:													//	}
																	//
	bc			12, 2, extend_sign_minus_one;						//	if ( carry_bit_is_set() ) goto extend_sign_minus_one;
																	//
	extend_sign_zero:												//	extend_sign_zero:
																	//
	stw			z, -WORD_SIZE(out_result);							//	out_result[ 0 ] = 0;
																	//
	done:															//
}
}

#endif // ENABLE_PPC_APN_PRIMITIVES
#endif // ENABLE_ASM_APN_PRIMITIVES


// subtracts two arbitrary precision big-endian signed two's complement integers in_lhs and in_rhs such that out_result = in_lhs - in_rhs;
// in_n is length in 32-bit words of in_lhs and in_rhs.
// out_result must point to a region of memory in_n+1 32-bit words in length.
void s_sub( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 *out_result ) {
	s_neg( in_rhs, in_n );
	s_add( in_lhs, in_rhs, in_n, out_result );
	s_neg( in_rhs, in_n );
}


// multiplies two arbitrary precision big-endian big-endian signed two's complement integers such that out_result = in_lhs * in_rhs;
// in_n is length in 32-bit words of in_lhs, in_o is length in 32-bit words of in_rhs.
// out_result must point to a region of memory in_n+in_o+1 32-bit words in length.
//
// NOTE:  This routine does not explicitly check for multiply-by-zero (neither the case of in[LR]HS entirely
//		  equal to zero, nor the intermediate case (M2 from Knuth, p. 268 Seminumerical Algorithms) where some
//		  place Vj == 0).  It is assumed that these special cases will not occur frequently enough to warrant
//		  the overhead incurred to look for them.
void s_mul( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 in_o, register __u32 *out_result ) {
	bool					l, r;
	__u32					i, j;
	
	l = r = false;

	if ( in_lhs[ 0 ] & 0x80000000 ) { s_neg( in_lhs, in_n ); l = true; }
	if ( in_rhs[ 0 ] & 0x80000000 ) { s_neg( in_rhs, in_o ); r = true; }
	
	u_mul( in_lhs, in_rhs, in_n, in_o, out_result + 1 );

	if ( l ) s_neg( in_lhs, in_n );
	if ( r ) s_neg( in_rhs, in_o );

	*out_result = 0;

	if ( l && ! r || ! l && r ) {
		s_neg( out_result + 1, j = in_n + in_o );
		
		for ( i = 1; i < j; ++i ) if ( out_result[ i ] ) { *out_result = 0xffffffff; break; }
	}
}


// divides two arbitrary precision big-endian signed two's complement integers such that out_div_result = in_lhs / in_rhs and out_mod_result = in_lhs % in_rhs
//
// in_n is the length in 32-bit words of in_lhs
// in_o is the length in 32-bit words of in_rhs
//
// out_div_result must be in_n 32-bit words long
// out_mod_result must be in_o 32-bit words long
//
// if this routine detects divide-by-zero, it will either throw or return (depending on whether or not exceptions are enabled) an err_divide_by_zero error
err_t s_div( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 in_o, register __u32 *out_div_result, register __u32 *out_mod_result, register malloc_t in_malloc, register v_proc_pv in_free ) {
#if BYTE_ORDER == LITTLE_ENDIAN
	__u16					a;
#endif
	bool			l, r;
	__u16		   *p, *q, *u, *v;
	__u32			i, j, k, n, o;

	// since we're going to do unsigned division, convert the two's complement in_lhs and in_rhs
	// to their positive analogues if appropriate.
	if ( in_lhs[ 0 ] & 0x80000000 ) { s_neg( in_lhs, in_n ); l = true; } else l = false;
	if ( in_rhs[ 0 ] & 0x80000000 ) { s_neg( in_rhs, in_o ); r = true; } else r = false;

#if BYTE_ORDER == LITTLE_ENDIAN
	for ( i = 0, n = in_n * 2; i < n; i += 2 ) {
		a = ((__u16 *) in_lhs)[ i ];
		((__u16 *) in_lhs)[ i ] = ((__u16 *) in_lhs)[ i + 1 ];
		((__u16 *) in_lhs)[ i + 1 ] = a;
	}

	for ( i = 0, n = in_o * 2; i < n; i += 2 ) {
		a = ((__u16 *) in_rhs)[ i ];
		((__u16 *) in_rhs)[ i ] = ((__u16 *) in_rhs)[ i + 1 ];
		((__u16 *) in_rhs)[ i + 1 ] = a;
	}
#endif

	// scan in_rhs to locate the first non-zero digit.  this is to satisfy u_div()'s precondition that Vn-1 != 0.
	// this loop also detects in_rhs == 0.  in that case (divide by zero) we throw an exception or return an error.
	for ( q = (__u16 *) in_rhs, j = 0, ( o = in_o * ( WORD_SIZE / HALF_WORD_SIZE ) ); j < o && ! *q; ++j, ++q ) ;
	
	if ( ! ( j = o - j ) ) return err_divide_by_zero;
	
	// o is now the number of 16-bit words in in_rhs
	// j is the number of digits in in_rhs minus leading zeros

	// first scan in_lhs and find out how many 16-bit words comprise the number.
	for ( p = (__u16 *) in_lhs, i = 0, ( n = in_n * ( WORD_SIZE / HALF_WORD_SIZE ) ); i < n && ! *p; ++i, ++p ) ;
	
	// p now points to the first non-zero digit of in_lhs and i is the number of digits that were skipped
	// set i to the number of digits in in_lhs minus leading zeros.
	if ( ! ( i = n - i ) ) {
		// if i is zero, then the numerator is zero, so zero out both out_div_result and out_mod_result and return.
		for ( i = 0; i < in_n; ++i ) out_div_result[ i ] = 0;
		if ( out_mod_result ) for ( i = 0; i < in_o; ++i ) out_mod_result[ i ] = 0;

		goto negate;
	}
	
	// if in_lhs is less than in_rhs, we can skip the actual division and return zero for div and in_lhs for mod
	if ( i < j || i == j && *p < *q ) {
		for ( k = 0; k < in_n; ++k ) out_div_result[ k ] = 0;
		if ( out_mod_result ) {
			for ( k = 0, q = (__u16 *) out_mod_result; k < o - i; ++k ) q[ k ] = 0;
			for ( ; k < o; ++k ) q[ k ] = *p++;
		}
		
		goto negate;
	}
	
	// if we're here, then in_lhs is greater than or equal to in_rhs, so begin setup for division
	
	// as a precondition for u_div, io_lhs must have an additional zero word prepended for normalization.
	// allocate space for u
	if ( ! ( u = (__u16 *) in_malloc( sizeof(__u16) * ( i + 1 ) ) ) ) return err_mem_full;
	if ( ! ( v = (__u16 *) in_malloc( sizeof(__u16) * j ) ) ) { in_free( u ); return err_mem_full; }

	// zero the prepended normalization word
	*u = 0;

	// copy in_lhs to u
	for ( k = 1; k <= i; ++k ) u[ k ] = *p++;

	// copy in_rhs to v
	for ( k = 0; k < j; ++k ) v[ k ] = *q++;
	
	// zero leftmost digits of out_div_result that will not be used
	for ( p = (__u16 *) out_div_result, k = 0; k < n - ( i - j + 1); ++k ) *p++ = 0;
	
	// zero leftmost digits of out_mod_result that will not be used
	if ( out_mod_result ) for ( q = (__u16 *) out_mod_result, k = 0; k < o - j; ++k ) *q++ = 0;

	// we're all set up so do the division...
	u_div( u, v, i, j, &((__u16 *) out_div_result)[ n - ( i - j + 1 ) ], out_mod_result ? &((__u16 *) out_mod_result)[ o - j ] : nil );

	// clean up
	in_free( v );
	in_free( u );

negate:

#if BYTE_ORDER == LITTLE_ENDIAN
	for ( i = 0, n = in_n * 2; i < n; i += 2 ) {
		a = ((__u16 *) in_lhs)[ i ];
		((__u16 *) in_lhs)[ i ] = ((__u16 *) in_lhs)[ i + 1 ];
		((__u16 *) in_lhs)[ i + 1 ] = a;

		a = ((__u16 *) out_div_result)[ i ];
		((__u16 *) out_div_result)[ i ] = ((__u16 *) out_div_result)[ i + 1 ];
		((__u16 *) out_div_result)[ i + 1 ] = a;
	}

	for ( i = 0, n = in_o * 2; i < n; i += 2 ) {
		a = ((__u16 *) in_rhs)[ i ];
		((__u16 *) in_rhs)[ i ] = ((__u16 *) in_rhs)[ i + 1 ];
		((__u16 *) in_rhs)[ i + 1 ] = a;
	}

	if ( out_mod_result ) {
		for ( i = 0, n = in_o * 2; i < n; i += 2 ) {
			a = ((__u16 *) out_mod_result)[ i ];
			((__u16 *) out_mod_result)[ i ] = ((__u16 *) out_mod_result)[ i + 1 ];
			((__u16 *) out_mod_result)[ i + 1 ] = a;
		}
	}
#endif

	// return in_[lr]hs to their original states
	if ( l ) s_neg( in_lhs, in_n );
	if ( r ) s_neg( in_rhs, in_o );

	// negate the quotient if necessary
	if ( l ^ r ) s_neg( out_div_result, in_n );

	// negate the remainder if necessary
	if ( out_mod_result && l ) s_neg( out_mod_result, in_o );
	
	return no_err;
}


#if __MWERKS__
#pragma mark -
#endif


#if ! ENABLE_ASM_APN_PRIMITIVES

// one's-complements the io_rhs operand in place ( io_rhs = ~io_rhs );
// io_rhs is an arbitrary precision big-endian integer
// in_n is length in 32-bit words of io_rhs
void u_not( register __u32 *io_rhs, register __u32 in_n ) {
	while ( in_n-- ) io_rhs[ in_n ] = ~io_rhs[ in_n ];
}

#else // ENABLE_ASM_APN_PRIMITIVES
#if ENABLE_PPC_APN_PRIMITIVES

// tested bd: 05.19.03 ok
void u_not( register __u32 *io_rhs, register __u32 in_n ) {
	register __u32			a, b, i, s;

asm {
	li			s, WORD_SIZE;										//	s = sizeof(__u32);
	slwi		i, in_n, WORD_SHIFT;								//	i = in_n * sizeof(__u32);
																	//
	complement_loop:												//	do {
																	//
		sub.		i, i, s;										//		i -= s;
		lwzx		a, io_rhs, i;									//		a = io_rhs[ i ];
		not			b, a;											//		b = ~a;
		stwx		b, io_rhs, i;									//		io_rhs[ i ] = c;
		bne+		complement_loop;								//	} while ( i );
}
}

#endif // ENABLE_PPC_APN_PRIMITIVES
#endif // ENABLE_ASM_APN_PRIMITIVES


#if ! ENABLE_ASM_APN_PRIMITIVES

// adds two arbitrary precision big-endian unsigned integers in_lhs and in_rhs ( out_result = in_lhs + in_rhs );
// in_n is the length in 32-bit words of in_lhs and in_rhs.
// out_result must point to a region of memory in_n+1 32-bit words in length.
void u_add( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 *out_result ) {
	__u64		k = 0;
	
	while ( in_n-- ) {
		k += (__u64) in_lhs[ in_n ] + (__u64) in_rhs[ in_n ];
		out_result[ in_n + 1 ] = (__u32) k;
		k >>= 32;
	}
	
	out_result[ 0 ] = (__u32) k;
}

#else // ENABLE_ASM_APN_PRIMITIVES
#if ENABLE_PPC_APN_PRIMITIVES

// tested bd: 05.19.03 ok
void u_add( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 *out_result ) {
	register __u32			a, b, c, i, z;

asm {
	addi		out_result, out_result, WORD_SIZE;					//	out_result += sizeof(__u32);
	li			i, WORD_SIZE;										//	i = sizeof(__u32);
	li			z, 0;												//	clear_carry_bit();
	mtxer		z;													//
	slwi		in_n, in_n, WORD_SHIFT;								//	in_n *= sizeof(__u32);
																	//
	add_loop:														//	do {
																	//
		sub.		in_n,	in_n,	i;								//		in_n -= i;
		lwzx		a, in_lhs, in_n;								//		a = in_lhs[ in_n ];
		lwzx		b, in_rhs, in_n;								//		b = in_rhs[ in_n ];
		adde		c, a, b;										//		c = a + b + carry_bit;
		stwx		c, out_result, in_n;							//		out_result[ in_n + 1 ] = c;
		bne+		add_loop										//	} while ( in_n );
																	//
	addze		a, z;												//	out_result[ 0 ] = carry_bit;
	stw			a, -WORD_SIZE(out_result);
}
}

#endif // ENABLE_PPC_APN_PRIMITIVES
#endif // ENABLE_ASM_APN_PRIMITIVES


#if ! ENABLE_ASM_APN_PRIMITIVES

// subtracts two arbitrary precision big-endian unsigned integers in_lhs and in_rhs ( out_result = in_lhs - in_rhs );
// in_n is length in 32-bit words of in_lhs, in_rhs, and out_result.
// out_result is undefined if in_rhs > in_lhs.
void u_sub( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 *out_result ) {
	__u64		k = 1;
	
	while ( in_n-- ) {
		k += (__u64) in_lhs[ in_n ] + (__u64) ~in_rhs[ in_n ];
		out_result[ in_n ] = (__u32) k;
		k >>= 32;
	}
}

#else // ENABLE_ASM_APN_PRIMITIVES
#if ENABLE_PPC_APN_PRIMITIVES

// tested bd: 05.25.03 ok
void u_sub( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 *out_result ) {
	register __s32			a, b, c, i;

asm {
	lis			i, 0x2000;											//	set_carry_bit();
	mtxer		i;													//
	li			i, WORD_SIZE;										//	i = sizeof(__u32);
	slwi		in_n, in_n, WORD_SHIFT;								//	in_n *= sizeof(__u32);
																	//
	sub_loop:														//	do {
																	//
		sub.		in_n, in_n, i;									//		in_n -= i;
		lwzx		a, in_lhs, in_n;								//		a = in_lhs[ in_n ];
		lwzx		b, in_rhs, in_n;								//		b = in_rhs[ in_n ];
		subfe		c, b, a;										//		c = a - b + carry;
		stwx		c, out_result, in_n;							//		out_result[ in_n ] = c;
		bne+		sub_loop;										//	} while ( in_n );
}
}

#endif // ENABLE_PPC_APN_PRIMITIVES
#endif // ENABLE_ASM_APN_PRIMITIVES


#if ! ENABLE_ASM_APN_PRIMITIVES

// multiplies two arbitrary precision big-endian unsigned integers ( out_result = in_lhs * in_rhs );
// in_n is length in 32-bit words of in_lhs, in_o is length in 32-bit words of in_rhs.
// out_result must point to a region of memory in_n+in_o 32-bit words in length.
//
// NOTE:  This routine does not explicitly check for multiply-by-zero (neither the case of in[LR]HS entirely
//		  equal to zero, nor the intermediate case (M2 from Knuth, p. 268 Seminumerical Algorithms) where some
//		  place Vj == 0).  It is assumed that these special cases will not occur frequently enough to warrant
//		  the overhead incurred to look for them.
void u_mul( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 in_o, register __u32 *out_result ) {
	__u64		t;
	__u32		a, i, j;

	for ( i = 0, j = in_n + in_o; i < j; ++i ) out_result[ i ] = 0;
	
	do {
		t = 0;
		j = in_n;
		i = in_n + in_o;
		a = in_rhs[ --in_o ];

		do {
			t += (__u64) a * (__u64) in_lhs[ --j ] + (__u64) out_result[ --i ];
			out_result[ i ] = (__u32) t;
			t >>= 32;
		} while ( j );
		
		out_result[ --i ] = (__u32) t;
	} while ( in_o );
}

#else // ENABLE_ASM_APN_PRIMITIVES
#if ENABLE_PPC_APN_PRIMITIVES

// tested bd 12.16.03
void u_mul( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 in_o, register __u32 *out_result ) {
	register __u32			a, b, c, d, e, i, j, k, t, w, z;

asm	{
	li			z, 0;												//	z = 0;
	li			k, WORD_SIZE;										//	k = sizeof(__u32);
																	//
	add			i, in_n, in_o;										//	i = sizeof(__u32) * ( in_n + in_o );
	slwi		i, i, WORD_SHIFT;									//
																	//
	zero_outResult_loop:											//	do {
																	//
		sub.		i, i, k;										//		i -= sizeof(__u32);
		stwx		z, out_result, i;								//		*reinterpret_cast<__u32 *>(__u32(out_result) + i) = z;
		bne+		zero_outResult_loop;							//	} while ( i );
																	//
	mtxer		z;													//	clear_carry_bit();
	slwi		in_n, in_n, WORD_SHIFT;								//	in_n *= sizeof(__u32);				// in_n indexes in_lhs (Knuth's U)
	slwi		in_o, in_o, WORD_SHIFT;								//	in_o *= sizeof(__u32);				// in_o indexes in_rhs (Knuth's V) (in_o is Knuth's j)
																	//
	in_rhs_digit_loop:												//	do {
																	//
		li			c, 0;											//		clear_carry();
		add			i, in_n, in_o;									//		i = in_n + in_o;				// i indexes out_result
		sub			in_o, in_o, k;									//		in_o -= k;
		lwzx		a, in_rhs, in_o;								//		a = in_rhs[ in_o ];				// load Vj
		mr			j, in_n;										//		j = in_n;						// j is Knuth's i and indexes in_lhs (j is a copy of in_n for the in_lhs_digit_loop)
																	//
		in_lhs_digit_loop:											//		do {
																	//
			sub.		j, j, k;									//			j -= k;
			lwzx		b, in_lhs, j;								//			b = in_lhs[ j ];			// load Ui
			sub			i, i, k;									//			i -= k;
			lwzx		w, out_result, i;							//			w = out_result[ i ];
			mullw		t, a, b;									//			t = LOW_32( a * b );
			adde		t, t, w;									//			update_carry_bit_after( t += w );
			mulhwu		d, a, b;									//			d = UNSIGNED_HIGH_32( a * b )
			addze		e, d;										//			e = d + carry_bit;
			adde		t, t, c;									//			t += c + carry_bit;
			addze		c, e;										//			c = e + carry_bit;			// carry_bit is guaranteed to be zero after this operation
			stwx		t, out_result, i;							//			out_result[ in_o + j ] = w;
			bne+		in_lhs_digit_loop;							//		} while ( j );
																	//
		sub			i, i, k;										//		i -= k;							// note that the code here is only sufficient to cover the case where out_result
		stwx		c, out_result, i;								//		out_result[ i ] = c;			// is zeroed on entry (as above).  the extended multiply-and-add functionality
		cmpwi		in_o, 0;										//										// obtained by *not* zeroing out_result on entry requires extra code for handling
		bne+		in_rhs_digit_loop;								//	} while ( in_o -= k );				// carries that can occur when the last operation of in_lhs_digit_loop produces
}																											// a double-digit result that causes a carry when added to the out_result position.
}

#endif // ENABLE_PPC_APN_PRIMITIVES
#endif // ENABLE_ASM_APN_PRIMITIVES


#if ! ENABLE_ASM_APN_PRIMITIVES

// divides two arbitrary precision big-endian unsigned integers such that out_div_result = io_lhs / io_rhs and out_mod_result = io_lhs % io_rhs
//
// in_n is the length in 16-bit words of io_lhs not including the prepended zero digit (see below)
// in_o is the length in 16-bit words of io_rhs and must not be greater than in_n.
//
// out_div_result must point to a region of memory in_n - in_o + 1 16-bit words in length
// out_mod_result must be nil or point to a region of memory in_o 16-bit words in length (see comments below).
//
// the caller must prepend a zeroed 16-bit half-word to io_lhs for the purposes of normalization in D1.
//   - as stated above this half-word is *not* included in the length of io_lhs.
//
// NOTE:  In order to avoid a divide-by-zero exception from the processor, it is the caller's
//		  responsibility to ensure that on entry io_rhs != 0;
//
// it is the caller's responsibility to ensure that on entry io_rhs[ 0 ] != 0 (satisfies Knuth V2 p.272 Algorithm D precondition that Vn-1 != 0).
//
// io_lhs will get clobbered by this algorithm.  operate on a copy if you want to keep io_lhs const.
// io_rhs will be shifted left (normalized) the amount of bits necessary to ensure that the highest bit is set.  it is the caller's responsibility
//   to either unnormalize io_rhs or have u_div operate on a copy of io_rhs.  to unnormalize io_[lr]hs, determine prior to calling u_div how many left
//   shifts are needed to evaluate ( io_rhs[ 0 ] & 0x80000000 ) to true.  on exit, shift io_rhs right by this amount and it will be unnormalized.
//
// on exit, io_lhs is the unnormalized result of io_lhs % io_rhs.  in the interests of speed I've chosen to allow the caller to pass nil for
// out_mod_result, which will cause u_div to return after the division but before normalizing the remainder (useful if the caller doesn't care
// to know what the remainder is).  if you pass io_lhs + in_n - in_o + 1 in out_mod_result, the remainder will be returned in io_lhs.
//
// example:
//
//	__u16	a[ 3 ] = { 0, 0, 0x10 }, b[ 2 ] = { 0x10, 0 }, c;
//
//	u_div( a, b, 2, 2, &c, a + 1 );
//
//	the code above show 'a' (io_lhs) as a two digit value (contained in a[1] and a[2] with identity 0x10) with a prepended, zeroed digit for normalization (a[0]).
//  the in_n value specifying the length of a is sent as 2 (don't include the prepended word in the count).  'b' (io_rhs) is a region  of memory no
//  larger than in_n.  in this case it is equal to in_n, so in_o is also specified as 2.  c points to a region of memory that is in_n - in_o + 1 16-bit words in length
//	and is used for out_div_result.  out_mod_result is sent as io_lhs + in_n - in_o + 1.
//
//  on exit, c will be zero ( a / b ), and a[ 1 ] and a[ 2 ] will be { 0, 0x10 } or a % b.
void u_div( register __u16 *io_lhs, register __u16 *io_rhs, register __u32 in_n, register __u32 in_o, register __u16 *out_div_result, register __u16 *out_mod_result ) {
	const __u32		b = 0x10000;
	__u32			a, d, i, j, k, m, q, r, t, _v1, _v2, w, x;

	if ( in_o > 1 ) {
		for ( d = 0, t = 0x8000; ! ( *io_rhs & t ); ++d, t >>= 1 ) ;

		if ( d ) {
			// normalize io_lhs
			for ( i = in_n, t = 0; ; t = x ) {
				a = io_lhs[ i ];
				x = a >> 16 - d;

				io_lhs[ i ] = a << d | t;

				if ( ! i-- ) break;
			}
			
			// normalize io_rhs
			for ( i = in_o - 1, t = 0; ; t = x ) {
				a = io_rhs[ i ];
				x = a >> 16 - d;

				io_rhs[ i ] = a << d | t;

				if ( ! i-- ) break;
			}
		}

		j = 0;
		
		_v1 = io_rhs[ 0 ];
		_v2 = io_rhs[ 1 ];
		
		m = in_n - in_o;
		
		do {
			a = io_lhs[ j ] << 16 | io_lhs[ j + 1 ];

			q = a / _v1;
			r = a % _v1;
		
		test_q:
		
			if ( q == b || q * _v2 > b * r + io_lhs[ j + 2 ] ) {
				--q;
				if ( ( r += _v1 ) < b ) goto test_q;
			}
			
			// i is index over U; k is index over V
			for ( i = j + in_o, k = in_o, w = 0, x = 1; ; --i ) {
				t = q * io_rhs[ --k ] + w;
				w = t >> 16;
				
				io_lhs[ i ] = ( x += io_lhs[ i ] + ( ~t & 0xffff ) ) & 0xffff;
				x >>= 16;

				if ( ! k ) {
					--i;
					io_lhs[ i ] = ( x += io_lhs[ i ] + ( ~w & 0xffff ) ) & 0xffff;
					x >>= 16;
					break;
				}
			}

			if ( ! x ) {	// d6
				--q;
				for ( i = j + in_o, k = in_o, x = 0; ; --i ) {
					t = io_lhs[ i ] + io_rhs[ --k ] + x;
					io_lhs[ i ] = t & 0xffff;
					x = t >> 16;

					if ( ! k ) {
						--i;
						io_lhs[ i ] = io_lhs[ i ] + x & 0xffff;
						break;
					}
				}
			}

			out_div_result[ j ] = q;
		} while ( ++j <= m );
		
		if ( out_mod_result ) {
			// unnormalize io_lhs into out_mod_result
			if ( d ) {
				for ( i = in_n + 1; in_o--; ) {
					a = io_lhs[ --i ] >> d;
					t = i ? io_lhs[ i - 1 ] << 16 - d : 0;
				
					out_mod_result[ in_o ] = ( a | t ) & 0xffff;
				}
			} else {
				for ( i = in_n + 1; in_o--; ) out_mod_result[ in_o ] = io_lhs[ --i ];
			}
		}
	} else {
		// simple division algorithm, in_o == 1
		j = 1;
		k = 0;
		r = 0;
		t = *io_rhs;
		
		do {
			a = r * b + io_lhs[ j ];

			out_div_result[ k++ ] = a / t;

			r = a % t;
		} while ( ++j <= in_n );
		
		if ( out_mod_result ) *out_mod_result = (__u16) r;
	}
}

#else // ENABLE_ASM_APN_PRIMITIVES
#if ENABLE_PPC_APN_PRIMITIVES

// tested bd: 12.16.03
void u_div( register __u16 *io_lhs, register __u16 *io_rhs, register __u32 in_n, register __u32 in_o, register __u16 *out_div_result, register __u16 *out_mod_result ) {
	register __u32			a, b, c, d, e, i, j, k, m, q, r, s, t, _v1, _v2;

asm {
	li			s, HALF_WORD_SIZE;									//	s = 2;
	cmpwi		in_o, 1;											//	if ( in_o == 1 ) goto S1;			// simple division algorithm when io_rhs is only one digit
	beq			S1;													//
																	//
	D1:																//	// Knuth's 'd' (V2 p.272) is 2^e such that b/2 <= d * io_rhs[ 0 ] < b (Knuth V2 p.627 #37), b is 2^16
																	//	// Knuth's 'm' is in_n - in_o, 'u' is io_lhs, 'v' is io_rhs, 'q' is out_div_result
																	//
	sub			m, in_n, in_o;										//	m = in_n - in_o;
	slwi		m, m, HALF_WORD_SHIFT;								//	m *= sizeof(__u16);											
	li			e, 0;												//	e = 0;
	LHZX		c, io_rhs, e;										//	a = io_rhs[ 0 ];					// load V[in_n-1] for normalization loop
	slwi		a, c, WORD_BITS-HALF_WORD_BITS;						//
																	//
	determine_e_loop:												//	for ( ;; ) {						// store to e the number of left-shifts necessary to ensure the high bit of Vn-1 is set
																	//
		rlwnm.		t, a, e, 0, 0;									//		if ( ( a << e ) & 0x80000000 ) break;
		bne+		normalize;										//
		addi		e, e, 1;										//		++e;
		b			determine_e_loop;								//	}
																	//
	normalize:														//
																	//
	cmpwi		e, 0;												//	if ( e ) {
	beq			D2;													//
																	//
		li			c, 0;											//		c = 0;
		slwi		i, in_o, HALF_WORD_SHIFT;						//		i = in_o * sizeof(__u16);		// i is index over v
		subfic		t, e, HALF_WORD_BITS;							//		t = ( sizeof(__u16) * 8 ) - e;
																	//
		normalize_io_rhs_loop:										//		do {							// shift V left by e (multiply by Knuth's 'd')
																	//
			sub.		i, i, s;									//			i -= s;						// decrement i
			LHZX		a, io_rhs, i;								//			a = io_rhs[ i ];			// load Vx (moving right to left from V0 to Vn-1 (in_n is in register in_o))
			srw			b, a, t;									// 			b = a >> t;					// right shift a by t (store into b the bits we'll actually be shifting out of a)
			slw			a, a, e;									//			a <<= e;					// shift a left by e
			or			a, a, c;									//			a |= c;						// or in the bits shifted out of the previous word (b from the previous call)
			STHX		a, io_rhs, i;								//			io_rhs[ i ] = a;			// store the shifted word back to Vx
			mr			c, b;										//										// copy b to c for next iteration
			bne+		normalize_io_rhs_loop;						//		} while ( i );
																	//
		li			c, 0;											//		c = 0;
		addi		i, in_n, 1;										//		i = in_n + 1;
		slwi		i, i, HALF_WORD_SHIFT;							//		i = in_n * sizeof(__u16);
																	//
		normalize_io_lhs_loop:										//		do {							// shift U left by e (same procedure as V-shift above)
																	//
			sub.		i, i, s;									//			i -= s;
			LHZX		a, io_lhs, i;								//			a = io_lhs[ i ];
			srw			b, a, t;									// 			b = a >> t;
			slw			a, a, e;									//			a <<= e;
			or			a, a, c;									//			a |= c;
			STHX		a, io_lhs, i;								//			io_lhs[ i ] = a;
			mr			c, b;										//
			bne+		normalize_io_lhs_loop;						//		} while ( i );
																	//	}
	D2:																//
																	//
	li			j, 0;												//	j = 0;
																	//
	D3:																//
																	//
	LHZX		_v1, io_rhs, j;										//	_v1 = io_rhs[ 0 ];				 	// _v1 = Vn-1
	LHZX		_v2, io_rhs, s;										//	_v2 = io_rhs[ 1 ];					// _v2 = Vn-2
																	//
	calculate_q_loop:												//	do {
																	//
		LHZX		a, io_lhs, j;									//		a = io_lhs[ j ];				// a = Uj+in_n
		slwi		a, a, HALF_WORD_BITS;							//		a <<= sizeof(__u16);			// a = b( Uj+in_n )
		add			t, j, s;										//		t = j + s;
		LHZX		b, io_lhs, t;									//		b = io_lhs[ t ];				// b = Uj+in_n-1
		add			a, a, b;										//		a += b;
																	//
		divwu		q, a, _v1;										//		q = a / _v1;					// q <-- FLOOR( ( radix * ( Uj+in_n ) + Uj+in_n-1 ) / Vn-1 );
		mullw		r, q, _v1;										//
		sub			r, a, r;										//		r = a % _v1;					// r <-- ( radix * ( Uj+in_n ) + Uj+in_n-1 ) mod Vn-1			// (PowerPC Microprocessor Family Programming Environments for 32-bit Microprocessors p.8-55)
																	//
#if HALF_WORD_IS_SHORT												//
		lis			a, 1;											//		a = 0x00010000;					// a = radix;
#else																//
		li			a, 0x0100;										//
#endif																//
		test_q:														//
																	//
		cmplw		q, a;											//		if ( q == a ) goto adjust_q;
		beq			adjust_q;										//
																	//
		add			i, t, s;										//		i = t + s;
		mullw		d, q, _v2;										//		d = q * _v2;					// q * Vn-2
		mullw		c, r, a;										//		c = r * a;						// radix * r;
		LHZX		i, io_lhs, i;									//		i = io_lhs[ i ];				// i = Uj+in_n-2;
		add			i, c, i;										//		i += c;							// radix * r + Uj+in_n-2
		cmplw		d, i;											//		if ( d > i ) goto adjust_q;		// if ( q * Vn-2 > radix * r + Uj+in_n-2 ) goto adjust_q;
		ble			D4;												//		else goto D4;
																	//
		adjust_q:													//
																	//
		addi		q, q, -1;										//		--q;
		add			r, r, _v1;										//		c += _v1;						// r += _v1;											// increase r by Vn-1
		cmplw		r, a;											//		if ( c < a ) goto test_q;		// if ( r < radix ) goto test_q;
		blt			test_q;											//
																	//
	D4:																//										// assertion:	a, b, c, d, i, k, and t are free registers
																	//		
		li			d, 0;											//		d = 0;
		lis			t, 0x2000;										//		set_carry_bit();
		mtxer		t;												//
		slwi		i, in_o, HALF_WORD_SHIFT;						//		i = in_o * sizeof(__u16);
		add			k, io_lhs, j;									//		k = __u32(io_lhs) + j;
		add			k, k, i;										//		k = reinterpret_cast<__u16 *>(k + i);
																	//
		multiply_by_q_loop:											//		do {							// ( Uj+in_n Uj+in_n-1 ... Uj ) - q * ( 0 Vn-1 ... _v1 v0 )
																	//
			sub.		i, i, s;									//			i -= s;
			LHZX		t, io_rhs, i;								//			t = io_rhs[ i ];			// fetch Vx, starting with V0 and iterating through Vn-1
			mullw		a, q, t;									//			a *= q;
			add			a, a, d;									//			a += d;
			rlwinm		b, a, 0, WORD_BITS-HALF_WORD_BITS, 31;		//			b = a & 0xffff;
			LHZ			c, 0( k );									//			c = *k;						// fetch Ux, starting with Uj and iterating through Uj+in_n
			subfe		c, b, c;									//			c -= b + carry;
			STH			c, 0( k );									//			io_lhs[ k ] = c;
			srwi		d, a, HALF_WORD_BITS;						//			d = a >> 16;
			sub			k, k, s;									//			k -= s;
			bne			multiply_by_q_loop;							//		} while ( i );
																	//
		LHZ			c, 0( k );										//		c = io_lhs[ k ];				// store Uj-in_n - q * 0
		subfe		c, d, c;										//		c -= d;
		STH			c, 0( k );										//		io_lhs[ 0 ] = c;
																	//
	D5:																//
																	//
		mfxer		t;												//		if ( ! carry_bit_set() ) {		// end result of the D4 operation was negative, so adjust io_lhs
		andis.		t, t, 0x2000;									//
		bne+		D7;												//
																	//
	D6:																//		// Knuth says:  to test D6, use b == 2^16 and U: 0x7fff800100000000 / V: 800080020005 (set HALF_WORD_SHORT == 0 for this test)
																	//
			subi		q, q, 1;									//			--q;						// q was two too large -- probability of occurence is ~2/radix_b
			li			t, 0;										//										// t acts as the carry bit
			slwi		i, in_o, HALF_WORD_SHIFT;					//			i = in_o * sizeof(__u16);
			add			k, io_lhs, j;								//			k = __u32(io_lhs) + j;
			add			k, k, i;									//			k = reinterpret_cast<__u16 *>(k + i);
																	//
			add_back_loop:											//			do {						// add ( 0 Vn-1 ... _v1 v0 ) to ( Uj+in_n Uj+in_n-1 ... Uj )
																	//
				sub.		i, i, s;								//				i -= s;
				LHZX		a, io_rhs, i;							//				a = io_rhs[ i ];		// fetch Vx, starting with V0 and iterating through Vn-1
				LHZ			b, 0( k );								//				b = *k;					// fetch Ux, starting with Uj and iterating through Uj+in_n-1
				add			c, a, b;								//				c = a + b;				// a carry occurs out of the final c = a + b operation which is ignored
				add			d, c, t;								//				d = c + t;				// add in the carry bit if necessary
				STH			d, 0( k );								//				*k = d;					// store the low order digit of the addition to the result
				rlwinm		t, d, WORD_BITS-HALF_WORD_BITS, 31, 31;	//				t = d >> 16 & 0x01;		// cache the carry for the next round
				sub			k, k, s;								//				k -= s;
				bne+		add_back_loop;							//			} while ( i );
																	//
			LHZ			a, 0( k );									//			a = *k;						// fetch Uj+in_n
			add			b, a, t;									//			b = a + t;					// add carry from the last round, this will also result in a carry, which will be ignored
			STH			b, 0( k );									//			*k = b;						// store the low order digit of the addition to the result
																	//		}
	D7:																//
																	//
		STHX		q, out_div_result, j;							//		out_div_result[ j ] = q;
		add			j, j, s;										//		j += sizeof(__u16);
		cmpw		j, m;											//
		ble+		calculate_q_loop;								// } while ( j <= m );
																	//
	D8:																//
																	//
	cmpwi		out_mod_result, 0;									//	if ( out_mod_result != nil ) {
	beq			done;												//
																	//
	add			io_lhs, io_lhs, s;									//		++io_lhs;						// io_lhs points to Um+in_n for Dx series above.  Increment to Um+in_n-1 for remainder calculation.
																	//
	cmpwi		e, 0;												//		if ( e ) {						// unnormalize ( Un-1 ... U1 U0 ) and copy to out_mod_result
	beq			copy_remainder;										//
																	//
		slwi		i, in_n, HALF_WORD_SHIFT;						//			i = in_n * sizeof(__u16);	// i is fetch loop counter over io_lhs ( Un-1 ... U1 U0 ) from U0 to Un-1
		subfic		t, e, HALF_WORD_BITS;							//			t = ( sizeof(__u16) * 8 ) - e;
		sub			i, i, s;										//			i -= s;
		LHZX		a, io_lhs, i;									//			a = io_lhs[ i ];			// load U0
		slwi		j, in_o, HALF_WORD_SHIFT;						//			j = in_o * sizeof(__u16);	// j is store loop counter over out_mod_result ( outModResultn-1 ... outModResult0 ) from out_mod_result[ 0 ] to out_mod_result[ in_n-1 ]
																	//
		unnormalize_io_lhs_loop:									//			do {
																	//
			sub			i, i, s;									//				i -= s;
			LHZX		b, io_lhs, i;								//				b = io_lhs[ i ];		// load Ux+1
			slw			c, b, t;									//				c = b << t;				// capture low t bits of Ux+1 (will be high t bits of unnormalized Ux)
			srw			d, a, e;									//				d = a >> e;				// shift Ux right e bits
			or			r, d, c;									//				r = q | c;				// or in high t bits of Ux+1
			sub.		j, j, s;									//				j -= s;					// align j
			STHX		r, out_mod_result, j;						//				out_mod_result[ j ] = r;// store r to out_mod_result[ j ]
			mr			a, b;										//				a = b;
			bne+		unnormalize_io_lhs_loop;					//			} while ( i );
																	//
		b			done;											//		
																	//
	copy_remainder:													//		} else {						// just copy ( Un-1 ... U1 U0 ) to out_mod_result
																	//
		slwi		i, in_n, HALF_WORD_SHIFT;						//			i = in_n * sizeof(__u16);
		slwi		j, in_o, HALF_WORD_SHIFT;						//			j = in_o * sizeof(__u16);
																	//
		copy_remainder_loop:										//			do {
																	//
			sub			i, i, s;									//				i -= s;
			LHZX		a, io_lhs, i;								//				a = io_lhs[ i ];
			sub.		j, j, s;									//				j -= s;
			STHX		a, out_mod_result, j;						//				out_mod_result[ j ] = a;
			bne+		copy_remainder_loop;						//			} while( i );
																	//		}
		b			done;											//	}
																	//
	// S series is logically a separate function:					//	return;
																	//
	S1:																//	S1:									// The simple division algorithm from Knuth V2 ch 4.3.1 exercise 16 (solution p.625)
																	//
	add			io_lhs, io_lhs, s;									//	++io_lhs;							// io_lhs points to Um+in_n for Dx series above.  Increment to Um+in_n-1 for simplified Sx series since normalization is unnecessary.
	slwi		i, in_n, HALF_WORD_SHIFT;							//	i = in_n * sizeof(__u16);			// i is address of byte past U0
	li			j, 0;												//	j = 0;								// j is index over ( Un-1 ... U1 U0 ) from Un-1 to U0
	li			r, 0;												//	r = 0;
#if HALF_WORD_IS_SHORT												//
	lis			t, 0x0001											//	t = 0x00010000;						// t = radix (2^16);
#else																//
	li			t, 0x0100											//	// this is here solely to aid debugging where the operative half-word is a byte
#endif																//
	LHZX		b, io_rhs, j;										//	b = io_rhs[ j ];					// b = V
																	//
	S2_loop:														//	do {
																	//
		LHZX		a, io_lhs, j;									//		a = io_lhs[ j ];				// a = Ux from Un-1 to U0
		mullw		c, r, t;										//		c = r * t;						// c = r * radix
		add			d, a, c;										//		d = a + c;						// d = ( r * b + Uj )
		divwu		e, d, b;										//		e = d / b;						// e = d / V 				// (Knuth's Wj)
		mullw		a, e, b;										//		a = e * b;
		subf		r, a, d;										//		r = d - a;						// r = d % V 				// (PowerPC Microprocessor Family Programming Environments for 32-bit Microprocessors p.8-55)
		STHX		e, out_div_result, j;							//		out_div_result[ j ] = e;
																	//
	S3:																//
																	//
		add			j, j, s;										//		j += sizeof(__u16);
		cmpw		j, i;											//
		blt+		S2_loop;										//	} while ( j < i );
																	//
	cmpwi		out_mod_result, 0;									//	if ( out_mod_result != nil ) {
	beq			done;												//
																	//
		li			i, 0;											//		i = 0;
		STHX		r, out_mod_result, i;							//		out_mod_result[ i ] = r;
																	//	}
	done:															//
}
}

#endif // ENABLE_PPC_APN_PRIMITIVES
#endif // ENABLE_ASM_APN_PRIMITIVES
