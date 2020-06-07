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
		

	File:				asn1.h

	Author:				Brian Doyle
	Date Created:		February 07, 2004
	Last Modified:		September 07, 2005

	Description:

	A quick and dirty (and incomplete) implementation of ASN.1 BER encoding (ITU-T X.690).

\*----------------------------------------------------------------------------------------*/
#ifndef __asn1_h__
#define __asn1_h__



#if _WIN32
	#include "precompiled.h"
#endif

#include "asn1_oids.h"
#include "octet_string.h"


#if __asn1_uses_apn
	#include "apn.h"
	
	typedef apn									asn1_integer;
#else
	typedef octet_string						asn1_integer;
#endif // __asn1_uses_apn


enum asn1_ber_identifier_class {
	k_asn1_ber_class_universal				=	0x00		,
	k_asn1_ber_class_application			=	0x40		,
	k_asn1_ber_class_context_specific		=	0x80		,
	k_asn1_ber_class_private				=	0xc0
};


enum asn1_ber_encoding_type {
	k_asn1_ber_primitive					=	0x00		,
	k_asn1_ber_constructed					=	0x20
};


enum asn1_universal_tags {
	k_asn1_tag_reserved_0					=	0x00		,
	k_asn1_tag_boolean						=	0x01		,
	k_asn1_tag_integer						=	0x02		,
	k_asn1_tag_bit_string					=	0x03		,
	k_asn1_tag_octet_string					=	0x04		,
	k_asn1_tag_null							=	0x05		,
	k_asn1_tag_object_identifier			=	0x06		,
	k_asn1_tag_object_descriptor			=	0x07		,
	k_asn1_tag_external_or_instance_of		=	0x08		,
	k_asn1_tag_real							=	0x09		,
	k_asn1_tag_enumerated					=	0x0a		,
	k_asn1_tag_embedded_pdv					=	0x0b		,
	k_asn1_tag_utf8_string					=	0x0c		,
	k_asn1_tag_relative_object_identifier	=	0x0d		,
	k_asn1_tag_reserved_1					=	0x0e		,
	k_asn1_tag_reserved_2					=	0x0f		,
	k_asn1_tag_sequence						=	0x10		,
	k_asn1_tag_set							=	0x11		,
	k_asn1_tag_character_string_0			=	0x12		,
	k_asn1_tag_character_string_1			=	0x13		,
	k_asn1_tag_character_string_2			=	0x14		,
	k_asn1_tag_character_string_3			=	0x15		,
	k_asn1_tag_character_string_4			=	0x16		,
	k_asn1_tag_time_0						=	0x17		,
	k_asn1_tag_time_1						=	0x18		,
	k_asn1_tag_character_string_5			=	0x19		,
	k_asn1_tag_character_string_6			=	0x1a		,
	k_asn1_tag_character_string_7			=	0x1b		,
	k_asn1_tag_character_string_8			=	0x1c		,
	k_asn1_tag_character_string_9			=	0x1d		,
	k_asn1_tag_character_string_a			=	0x1e		,
	k_asn1_tag_reserved_n					=	0x1f
};


class asn1_ber {

public:

	// on exit, out_boolean is ber encoded asn1 boolean type with value in_value
	static void encode_boolean( bool in_value, octet_string &out_boolean );

	// on entry, io_integer is the integer data to encode.
	// on exit, io_integer is a ber encoded asn1 integer type
	static void encode_integer( octet_string &io_integer, bool in_convert_data_from_host_byte_order = false );
	
	// on exit, out_null is a ber encoded asn1 null type
	static void encode_null( octet_string &out_null );
	
	// on entry, in_components is a list of object identifier components to encode
	// on exit, out_object_identifier is a ber encoded asn1 object identifier
	static void encode_object_identifier( const __u32 *in_components, __u32 in_count, octet_string &out_object_identifier );
	
	// on exit, out_null is a ber/der encoded asn1 octet_string type
	static void encode_octet_string( octet_string &in_string, bool in_convert_data_from_host_byte_order = false );
	
	// on entry, io_sequence is concatenated set of ber encoded asn1 types to encode.
	// on exit, io_sequence is a ber encoded asn1 sequence type
	static void encode_sequence( octet_string &io_sequence );

	// on entry, io_content is a concatenation of one or more ber encoded types
	// this method consumes the first ber encoded type from io_content and outputs
	//   its content to out_content.
	// io_content is reduced to the trailing ber encoded types (if any) not consumed
	//   by the decode operation.
	//
	// returns the type (asn1_universal_tag) of element decoded
	static __u32 decode( octet_string &io_content, octet_string &out_content, bool in_convert_data_to_host_byte_order = false );

	// decodes an object identifier's components and constructs an array onto
	// out_components (this list must be delete[]'d by the caller).  the first
	// element of out_components is *not* a component.  rather, it is the number
	// of components in the list (thus it is the number of elements in the array
	// minus 1).  the first component therefore is at out_components[ 1 ].
	static void decode_object_identifier( const octet_string &in_content, __u32 *&out_components );

protected:

	// the asn1 ber encoder.  use the canonical type encoders above.
	static void encode( __u32 in_class, __u32 in_encoding_type, __u32 in_tag, octet_string &io_content );

	// support methods
	static __u32 number_of_octets_for_identifier( __u32 in_tag );
	static __u32 number_of_octets_for_length( __u32 in_length );
	
	static __u32 read_identifier( const __u8 *in_identifier_start, __u32 in_length, __u32 &out_class, __u32 &out_encoding_type, __u32 &out_tag );
	static __u32 read_length( const __u8 *in_length_start, __u32 in_length, __u32 &out_content_length );

	static void write_identifier( __u8 *out_identifier_start, __u32 in_class, __u32 in_encoding_type, __u32 in_tag );	
	static void write_length( __u8 *out_identifier_start, __u32 in_length );

};


class asn1_ber_sequence {

public:

	// a FIFO implementing an asn1 sequence type.  construct an empty type
	// to create a sequence, push all the elements, then call encode() to
	// get the fully-constructed sequence.
	asn1_ber_sequence() { }
	
	// this can be used to initialize a sequence from a fully constructed
	// representation earlier created with asn1_ber_sequence::encode().  after
	// construction you may push() or pop() as necessary.
	asn1_ber_sequence( const octet_string &in_data );

	// copy constructor
	asn1_ber_sequence( const asn1_ber_sequence &in_sequence );

   ~asn1_ber_sequence() { }

	// pushes an item of type in_tag onto the sequence.  in_data will
	// be typecast appropriately and in_size will be used if necessary.
	//
	// expected arguments for the various types of in_tag are:
	//
	// in_tag							in_data					in_size
	//
	// k_asn1_tag_boolean				bool *					1
	// k_asn1_tag_integer				asn1_integer *			1
	// k_asn1_tag_null					0						1
	// k_asn1_tag_object_identifier		__u32 *					<num components>
	// k_asn1_tag_octet_string			octet_string *			1
	// k_asn1_tag_sequence				asn1_ber_sequence *		1
	//
	// notes:
	//
	// when pushing integers, the underlying encoding routines will detect if
	// the octet_string or apn is empty and push zero.
	//
	// integers are assumed to be stored in base 2^32 digit big-endian
	// representation with each individual digit stored in the byte order
	// corresponding to the flag in_convert_data_from_host_byte_order.  if
	// in_convert_data_from_host_byte_order is false, the byte order of each
	// individual base 2^32 digit is assumed to be big-endian. if
	// in_convert_data_from_host_byte_order is true, the byte order of each
	// individual base 2^32 digit is assumed to be the byte order of the target
	// architecture. see the notes in the pop() function below for detailed
	// information on integer byte ordering.  note that on little-endian
	// architectures if the passed-in integer is not 32-bit word aligned the
	// most significant digit will be sign extended to a multiple of 32-bits
	// (i.e. padded on the left) prior to storing.
	//
	// when pushing k_asn1_tag_octet_string and specifying true for
	// in_convert_data_from_host_byte_order, the data must be 32-bit aligned or
	// an exception will be thrown.
	void push( __u32 in_tag, const void *in_data = nil, __u32 in_size = 1, bool in_convert_data_from_host_byte_order = false );

	// pops the first element in the sequence and removes it from the sequence.
	// out_type is the asn1_universal_tag corresponding to the popped type.
	// returns nil if there are no more elements in the sequence.
	//
	// note that the caller *may* have to delete the returned value, depending on
	// the type returned.  return types marked with a * must be deleted, return
	// types marked with a # must be delete[]ed, and return types not otherwise
	// marked do not need to be released.
	//
	// k_asn1_tag_boolean:				return value corresponds to the value of the boolean
	// k_asn1_tag_integer*:				return value is an asn1_integer
	// k_asn1_tag_null:					return value is nil
	// k_asn1_tag_object_identifier#:	return value is an array of __u32
	// k_asn1_tag_octet_string*:		return value is an octet string
	// k_asn1_tag_sequence*:			return value is an asn1_ber_sequence
	//
	// pop() may optionally be passed an expected type.  if the decoded type
	// does not match the expected type err_wrong_type is thrown
	//
	// a popped integer will be represented as a string of base 2^32 digits in
	// two's complement representation.  the most significant digit will be
	// sign extended to a multiple of sizeof(__u32) as necessary.  the 2^32
	// digit sequence will be arranged in big-endian representation although
	// each digit will be stored based on the state of the parameter
	// in_convert_data_to_host_byte_order.  when in_convert_data_to_host_byte_order
	// is false, each base 2^32 digit will be stored in big-endian format
	// regardless of the host architecture.  if in_convert_data_to_host_byte_order
	// is true, each base 2^32 digit will be stored in the byte order of the host
	// architecture.  further discussion of this state follows:
	//
	// example:  say the encoded integer is the unsigned quantity:
	//
	//	0x0123456789abcdef
	//
	// this can be thought of as two digits in base 2^32 in big endian
	// representation (i.e. the most significant digit is 0x01234567 and the
	// least significant digit is 0x89abcdef).
	//
	// when popped on a big-endian architecture, the memory layout (where
	// lower addresses are on the left) will be exactly as shown above.
	// however, when popped on a little-endian architecture the memory
	// layout will be as follows:
	//
	// 0x67453210efcdab89
	//
	// as shown here, the most significant base 2^32 digit is still left-most
	// (i.e. big-endian), but each base 2^32 digit is stored in little-endian
	// byte order.
	//
	// when popping k_asn1_tag_octet_string, data can be converted to host
	// byte order by specifying in_convert_data_to_host_byte_order == true.
	// in this case the data must be 32-bit aligned or an exception will be thrown.
	void *pop( __u32 &out_type, __u32 in_expected_type = 0, bool in_convert_data_to_host_byte_order = false );

	// just like the constructor, initializes the sequence to the contents
	// of the octet_string (presumed to be a valid asn1_ber_sequence).
	void init( const octet_string &in_data );
	
	// reinitializes the sequence to an empty state
	void reset() { m_seq.reset(); }

	// returns a fully constructed asn1_ber sequence type that is a
	// concatenation of all the inputs (in the order they were pushed).
	void encode( octet_string &out_sequence ) const;

	void set_zero_data_on_release( bool in_zero = true );
	
protected:

	octet_string			m_seq;

};


inline void asn1_ber_sequence::set_zero_data_on_release( bool in_zero ) {
	m_seq.set_zero_data_on_release( in_zero );
}


_e(void parse_asn1_ber_sequence( const asn1_ber_sequence &in_sequence ));



#endif // __asn1_h__
