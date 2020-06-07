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
		

	File:				blowfish_cbc_wrapper.cpp

	Author:				Brian Doyle
	Created:			December 18, 2003
	Last Modified:		December 18, 2003

	Description:

	C wrappers for blowfish CBC functions.  This file is specifically
	intended for use with DataVault's block-oriented disk driver, but
	can be used anywhere CBC functionality is needed.
	
\*----------------------------------------------------------------------------------------*/


#include <string.h>

#include "blowfish_cbc_wrapper.h"
#include "blowfish.hpp"


struct blowfish_cbc {
	blowfish	   *m_blowfish;
	__u64			m_initialization_vector;
};


opaque_blowfish_t blowfish_alloc() {
	blowfish_cbc	   *p;
	
	if ( ( p = reinterpret_cast<blowfish_cbc *>(malloc( sizeof(blowfish_cbc) )) ) ) {
		if ( ! ( p->m_blowfish = reinterpret_cast<blowfish *>(malloc( sizeof(blowfish) )) ) ) {
			free( p ); p = nil;
		}
	}

	return p;
}


void blowfish_dealloc( opaque_blowfish_t in_blowfish ) {
	if ( in_blowfish ) {
		memset( reinterpret_cast<blowfish_cbc *>(in_blowfish)->m_blowfish, 0, sizeof(blowfish) );
		reinterpret_cast<blowfish_cbc *>(in_blowfish)->m_initialization_vector = 0;

		free( reinterpret_cast<blowfish_cbc *>(in_blowfish)->m_blowfish );
		free( reinterpret_cast<blowfish_cbc *>(in_blowfish) );
	}
}


void blowfish_init( opaque_blowfish_t in_blowfish, char *in_key, __u32 in_key_bits ) {
	reinterpret_cast<blowfish_cbc *>(in_blowfish)->m_blowfish->initialize_key( in_key, in_key_bits );
	reinterpret_cast<blowfish_cbc *>(in_blowfish)->m_blowfish->generate_cbc_initialization_vector(
		&reinterpret_cast<blowfish_cbc *>(in_blowfish)->m_initialization_vector );
}


void blowfish_encrypt( opaque_blowfish_t in_blowfish, char *io_data, __u32 in_data_length,
	__u32 in_initialization_vector_offset )
{
	reinterpret_cast<blowfish_cbc *>(in_blowfish)->m_blowfish->cbc_encipher( io_data,
		in_data_length, reinterpret_cast<blowfish_cbc *>(in_blowfish)->m_initialization_vector +
		in_initialization_vector_offset );
}


void blowfish_decrypt( opaque_blowfish_t in_blowfish, char *io_data, __u32 in_data_length,
	__u32 in_initialization_vector_offset )
{
	reinterpret_cast<blowfish_cbc *>(in_blowfish)->m_blowfish->cbc_decipher( io_data,
		in_data_length, reinterpret_cast<blowfish_cbc *>(in_blowfish)->m_initialization_vector +
		in_initialization_vector_offset );
}
