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
		

	File:				blowfish.hpp

	Author:				Brian Doyle
	Created:			October 24, 2001
	Last Modified:		December 18, 2003

	Description:

	Methods which implement blowfish block cipher and cbc mode with no padding

	For general use:
	
	Instantiate a 'blowfish' object, passing to the constructor a pointer to a
	key which is at most 448 bits long.
	
\*----------------------------------------------------------------------------------------*/


#ifndef __blowfish_hpp__
#define __blowfish_hpp__


#include <string.h>


#include "balance_types.h"


#ifdef KERNEL
	#include "malloc.hpp"	// for datavault
#endif


#if 0
#pragma mark class blowfish
#endif


class blowfish {

public:

	enum { k_max_key_bits = 448, k_max_key_bytes = 448 / 8 };

	// default constructor, if you instantiate this way be sure to call initialize_key()
	// before attempting any encipher/decipher operations
	blowfish() { }	// for C accessors, force default construction through initializers
   ~blowfish();		// for C accessors, force destruction through destroyers

	// pass in passphrase and number of bits from passphrase to use (up to 448 bits)
	// do not call initialize_key() if instantiating via this method
	blowfish( char *in_key, __u32 in_key_bits = k_max_key_bits ) { initialize_key( in_key, in_key_bits ); }

	// initializes a blowfish object, primarily broken out for C accessors
	void initialize_key( const char *in_key, __u32 in_key_bits );

	// mangler/demangler
	void encipher( register __u32 *io_left, register __u32 *io_right );
	void decipher( register __u32 *io_left, register __u32 *io_right );

	// cbc implementation
	void cbc_encipher( void *io_data, __u32 in_data_length, __u64 in_initialization_vector, __u64 *out_initialization_vector = 0 );
	void cbc_decipher( void *io_data, __u32 in_data_length, __u64 in_initialization_vector );

	// support method for short-lived keys (use only once per key)
	void generate_cbc_initialization_vector( __u64 *out_initialization_vector );	

	// returns true if key checksum is consistent with initialization state
	bool verify_key_integrity();

protected:

	enum { k_p_array_size = 18, k_s_boxes_size = 1024 };

	void generate_key_checksum( __u32 &out_checksum );

	__u32		   *m_s_boxes_0x400;
	__u32		   *m_s_boxes_0x800;
	__u32		   *m_s_boxes_0xC00;
	
	__u32			m_p_array[ k_p_array_size ];
	__u32			m_s_boxes[ k_s_boxes_size ];
	
	__u32			m_checksum;

	static __u32	s_p_array[ k_p_array_size ];
	static __u32	s_s_boxes[ k_s_boxes_size ];

};


inline blowfish::~blowfish() { memset( this, 0, sizeof(blowfish) ); }


#endif // __blowfish_hpp__
