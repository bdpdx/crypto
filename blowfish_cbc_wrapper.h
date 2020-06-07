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
		

	File:				blowfish_cbc_wrapper.h

	Author:				Brian Doyle
	Created:			December 18, 2003
	Last Modified:		December 18, 2003

	Description:

	C wrappers for blowfish CBC functions.  This file is specifically
	intended for use with DataVault's block-oriented disk driver, but
	can be used anywhere CBC functionality is needed.
	
\*----------------------------------------------------------------------------------------*/
#ifndef __blowfish_cbc_wrapper_h__
#define __blowfish_cbc_wrapper_h__



#include "balance_types.h"


#ifdef __MWERKS__
#pragma export on
#endif


#ifdef __cplusplus
extern "C" {
#endif


typedef void		   *opaque_blowfish_t;


// this function allocates a blowfish object for use with the other routines in this file
// call blowfish_dealloc() when done with the object to destroy it
//
opaque_blowfish_t blowfish_alloc( void );


// call this to destroy and deallocate a blowfish object.
//
// inputs:	in_blowfish								the blowfish object to destroy
//
void blowfish_dealloc( opaque_blowfish_t in_blowfish );


// call this to initialize a blowfish object.
// this routine must be called once and only once prior to using encrypt() or decrypt()
//
// inputs:	in_key									pointer to the user's key
//			in_key_bits								the number of bits (not bytes!) in the key (up to 448)
//
void blowfish_init( opaque_blowfish_t in_blowfish, char *in_key, __u32 in_key_bits );


// These routines do the encryption and decryption
//
// Inputs:		io_data								pointer to the user's data (disk block) to [en|de]crypt
//				in_data_length						length in bytes of the data (block size)
//				in_initialization_vector_offset		the offset by which to tweak the cbc initialization vector
//
// Note:
//
//		These functions are targeted primarily for use with a disk driver.
//
//		For the in_initialization_vector_offset parameter, pass the block number of
//		the physical block on disk that you are trying to [en|de]crypt, starting with
//		zero for the first block on the disk.
//
void blowfish_encrypt( opaque_blowfish_t in_blowfish, char *io_data, __u32 in_data_length, __u32 in_initialization_vector_offset );
void blowfish_decrypt( opaque_blowfish_t in_blowfish, char *io_data, __u32 in_data_length, __u32 in_initialization_vector_offset );


#ifdef __cplusplus
}
#endif



#ifdef __MWERKS__
#pragma export reset
#endif



#endif // __blowfish_cbc_wrapper_h__
