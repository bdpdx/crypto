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
		

	File:				rsa.hpp

	Author:				Brian Doyle
	Date Created:		February 03, 2004
	Last Modified:		February 07, 2004

	Description:

	RSA public key cryptography based on my apn library

		sunrise, blue bright sky
		burns like fire to my eyes
		sleep?  not yet... must code.

\*----------------------------------------------------------------------------------------*/
#ifndef __rsa_h__
#define __rsa_h__



#if _WIN32
	#include "precompiled.h"
#endif

#include "apn.h"
#include "sha.h"



#define k_rsa_key		os_type( 'r', 's', 'a', '!' )		// 'rsa!'


class asn1_ber_sequence;


class rsa {

public:

	rsa();
	rsa( asn1_ber_sequence *in_sequence );
   ~rsa();

	void decrypt( octet_string &io_message );
	void encrypt( octet_string &io_message );

	void verify_signature( octet_string &in_message, octet_string &in_signature );

#if ENABLE_RSA_LOAD
	static rsa *load( const char *in_path_to_key_file );
	static rsa *load( const char *in_keyfile_data, __u32 in_length, bool in_keyfile_data_is_non_const = false );
	static rsa *load( const __u8 *in_base64_key_data_c_string_typecast_to___u8_ptr );
	static rsa *load( asn1_ber_sequence &in_sequence );
#endif

#if ENABLE_RSA_GENERATORS
	void sign( octet_string &in_message, octet_string &out_signature );

	// caller must delete result
	static rsa *generate_key( __u32 in_num_bits_in_modulus = 4096, const char *in_rand_file = "/dev/urandom", __u32 in_rounds = 50, __u32 in_e = 0, class observed *in_observe = nil );

#if ENABLE_RSA_SAVE
	void create_asn1_ber_sequence( asn1_ber_sequence &out_sequence, bool in_save_only_public_key = false ) const;
	// caller must delete out_key_file
	void save( octet_string *&out_key_file, bool in_save_only_public_key = false );
	void save( const char *in_path_to_key_file, bool in_save_only_public_key = false );
#endif
#endif

_e( void dump_key(); )

	apn		   *m_n;		// modulus
	apn		   *m_e;		// public exponent

protected:

	const apn decrypt_primitive( const apn &in_message );
	const apn encrypt_primitive( const apn &in_message );

	void determine_crypt_parameters( __s32 &out_k, sha_algorithm_type &out_algorithm, __s32 &out_h_len, __s32 &out_max_length );
	void emsa_pss_encode( octet_string &in_message, __s32 in_modulus_bits_minus_one, sha_algorithm_type in_algorithm, octet_string &out_result );
	
	static void mgf1( octet_string &in_os, __u32 in_output_length, octet_string &out_result, sha_algorithm_type in_algorithm );

	apn		   *m_d;		// private exponent
	apn		   *m_p;		// prime p
	apn		   *m_q;		// prime q
	apn		   *m_dp;		// p's crt exponent
	apn		   *m_dq;		// q's crt exponent
	apn		   *m_qinv;		// the first crt coefficient

};



#endif // __rsa_h__
