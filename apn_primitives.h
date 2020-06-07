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
		

	File:				apn_primitives.h

	Author:				Brian Doyle
	Last Modified:		February 10, 2004

	Description:

	Declares basic math primitives for abitrary precision integer calculations.

	WARNING:  there are *many* caveats to using these routines.
			  please read the function documentation thoroughly or use the
			  apn interface.
	
\**---------------------------------------------------------------------------------------*/
#ifndef __apn_primitives_h__
#define __apn_primitives_h__



#if _WIN32
	#include "precompiled.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif


//! negates the io_rhs operand in place (returns in io_rhs the two's complement of in_rhs) ( io_rhs = -io_rhs );
//! io_rhs is an arbitrary precision big-endian signed two's complement integer
//! in_n is length in 32-bit words of io_rhs
void s_neg( register __u32 *io_rhs, register __u32 in_n );

//! adds two arbitrary precision big-endian signed two's complement integers in_lhs and in_rhs such that out_result = in_lhs + in_rhs;
//! in_n is the length in 32-bit words of in_lhs and in_rhs.
//! out_result must point to a region of memory in_n+1 32-bit words in length.
void s_add( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 *out_result );

//! subtracts two arbitrary precision big-endian signed two's complement integers in_lhs and in_rhs such that out_result = in_lhs - in_rhs;
//! in_n is length in 32-bit words of in_lhs and in_rhs.
//! out_result must point to a region of memory in_n+1 32-bit words in length.
void s_sub( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 *out_result );

//! multiplies two arbitrary precision big-endian big-endian signed two's complement integers such that out_result = in_lhs * in_rhs;
//! in_n is length in 32-bit words of in_lhs, in_o is length in 32-bit words of in_rhs.
//! out_result must point to a region of memory in_n+in_o+1 32-bit words in length.
//!
//! NOTE: This routine does not explicitly check for multiply-by-zero (neither the case of in[LR]HS entirely
//!		  equal to zero, nor the intermediate case (M2 from Knuth, p. 268 Seminumerical Algorithms) where some
//!		  place Vj == 0).  It is assumed that these special cases will not occur frequently enough to warrant
//!		  the overhead incurred to look for them.
void s_mul( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 in_o, register __u32 *out_result );

//! divides two arbitrary precision big-endian signed two's complement integers such that out_div_result = in_lhs / in_rhs and out_mod_result = in_lhs % in_rhs
//!
//! in_n is the length in 32-bit words of in_lhs
//! in_o is the length in 32-bit words of in_rhs
//!
//! out_div_result must be in_n 32-bit words long
//! out_mod_result must be in_o 32-bit words long
//!
//! if this routine detects divide-by-zero, it will either throw or return (depending on whether or not exceptions are enabled) an err_divide_by_zero error
err_t s_div( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 in_o, register __u32 *out_div_result, register __u32 *out_mod_result, register malloc_t in_malloc, register v_proc_pv in_free );

//! one's-complements the io_rhs operand in place ( io_rhs = ~io_rhs );
//! io_rhs is an arbitrary precision big-endian integer
//! in_n is length in 32-bit words of io_rhs
void u_not( register __u32 *io_rhs, register __u32 in_n );

//! adds two arbitrary precision big-endian unsigned integers in_lhs and in_rhs ( out_result = in_lhs + in_rhs );
//! in_n is the length in 32-bit words of in_lhs and in_rhs.
//! out_result must point to a region of memory in_n+1 32-bit words in length.
void u_add( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 *out_result );

//! subtracts two arbitrary precision big-endian unsigned integers in_lhs and in_rhs ( out_result = in_lhs - in_rhs );
//! in_n is length in 32-bit words of in_lhs, in_rhs, and out_result.
//! out_result is undefined if in_rhs > in_lhs.
void u_sub( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 *out_result );

//! multiplies two arbitrary precision big-endian unsigned integers ( out_result = in_lhs * in_rhs );
//! in_n is length in 32-bit words of in_lhs, in_o is length in 32-bit words of in_rhs.
//! out_result must point to a region of memory in_n+in_o 32-bit words in length.
//!
//! NOTE: This routine does not explicitly check for multiply-by-zero (neither the case of in[LR]HS entirely
//!		  equal to zero, nor the intermediate case (M2 from Knuth, p. 268 Seminumerical Algorithms) where some
//!		  place Vj == 0).  It is assumed that these special cases will not occur frequently enough to warrant
//!		  the overhead incurred to look for them.
void u_mul( register __u32 *in_lhs, register __u32 *in_rhs, register __u32 in_n, register __u32 in_o, register __u32 *out_result );

//! divides two arbitrary precision big-endian unsigned integers such that out_div_result = io_lhs / io_rhs and out_mod_result = io_lhs % io_rhs
//!
//! in_n is the length in 16-bit words of io_lhs not including the prepended zero digit (see below)
//! in_o is the length in 16-bit words of io_rhs and must not be greater than in_n.
//!
//! out_div_result must point to a region of memory in_n - in_o + 1 16-bit words in length
//! out_mod_result must be nil or point to a region of memory in_o 16-bit words in length (see comments below).
//!
//! the caller must prepend a zeroed 16-bit half-word to io_lhs for the purposes of normalization in D1.
//!   - as stated above this half-word is *not* included in the length of io_lhs.
//!
//! NOTE:  In order to avoid a divide-by-zero exception from the processor, it is the caller's
//!		  responsibility to ensure that on entry io_rhs != 0;
//!
//! it is the caller's responsibility to ensure that on entry io_rhs[ 0 ] != 0 (satisfies Knuth V2 p.272 Algorithm D precondition that Vn-1 != 0).
//!
//! io_lhs will get clobbered by this algorithm.  operate on a copy if you want to keep io_lhs const.
//! io_rhs will be shifted left (normalized) the amount of bits necessary to ensure that the highest bit is set.  it is the caller's responsibility
//!   to either unnormalize io_rhs or have u_div operate on a copy of io_rhs.  to unnormalize io_[lr]hs, determine prior to calling u_div how many left
//!   shifts are needed to evaluate ( io_rhs[ 0 ] & 0x80000000 ) to true.  on exit, shift io_rhs right by this amount and it will be unnormalized.
//!
//! on exit, io_lhs is the unnormalized result of io_lhs % io_rhs.  in the interests of speed I've chosen to allow the caller to pass nil for
//! out_mod_result, which will cause u_div to return after the division but before normalizing the remainder (useful if the caller doesn't care
//! to know what the remainder is).  if you pass io_lhs + in_n - in_o + 1 in out_mod_result, the remainder will be returned in io_lhs.
//!
//! example:
//!
//!	__u16	a[ 3 ] = { 0, 0, 0x10 }, b[ 2 ] = { 0x10, 0 }, c;
//!
//!	u_div( a, b, 2, 2, &c, a + 1 );
//!
//!	the code above shows 'a' (io_lhs) as a two digit value (contained in a[1] and a[2] with identity 0x10) with a prepended, zeroed digit for normalization (a[0]).
//! the in_n value specifying the length of a is sent as 2 (don't include the prepended word in the count).  'b' (io_rhs) is a region  of memory no
//!   larger than in_n.  in this case it is equal to in_n, so in_o is also specified as 2.  c points to a region of memory that is in_n - in_o + 1 16-bit words in length
//!	  and is used for out_div_result.  out_mod_result is sent as io_lhs + in_n - in_o + 1.
//!
//!  on exit, c will be zero ( a / b ), and a[ 1 ] and a[ 2 ] will be { 0, 0x10 } or a % b.
void u_div( register __u16 *io_lhs, register __u16 *in_ohs, register __u32 in_n, register __u32 in_o, register __u16 *out_div_result, register __u16 *out_mod_result );


#ifdef __cplusplus
}
#endif



#endif // __apn_primitives_h__
