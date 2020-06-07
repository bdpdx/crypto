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
		

	File:				asn1.cpp

	Author:				Brian Doyle
	Date Created:		February 07, 2004
	Last Modified:		September 08, 2005

	Description:

	A quick and dirty (and incomplete) implementation of ASN1 BER encoding 
	(ITU-T X.690).

\*----------------------------------------------------------------------------------------*/
#include "asn1.h"


#define LOG_ASN1_DECODE		( DEBUG && 0 )
#define LOG_ASN1_ENCODE		( DEBUG && 0 )


inline __u32 encode_word_to_octets_7_bits_per( __u32 in_word, __u8 *out_octets ) {
	__u32					i, n, t;

	for ( i = 32, t = 0x80000000; ! ( t & in_word ); --i, t >>= 1 ) ;

	n = i = ( i + 6 ) / 7;
	
	out_octets[ --i ] = in_word & 0x7f;
	
	while ( i-- ) {
		in_word >>= 7;
		out_octets[ i ] = 0x80 | in_word & 0x7f;
	}
	
	return n;
}


void asn1_ber::encode_boolean( bool in_value, octet_string &out_boolean ) {
	__u8	   *p = new __u8[ 3 ];
	
	p[ 0 ] = k_asn1_tag_boolean;
	p[ 1 ] = 1;
	p[ 2 ] = in_value ? 0xff : 0;
	
	out_boolean.set( p, 3, false );
}


void asn1_ber::encode_integer( octet_string &io_integer, bool in_convert_data_from_host_byte_order ) {
	__u32					i, n, o;
	const __u8			   *p, *q;
	__u8				   *zero;

	// special case when io_integer represents zero
	if ( ! ( ( p = q = io_integer.data() ) && ( n = io_integer.length() ) ) ) {
		*( zero = new __u8[ 1 ] ) = 0;
		io_integer.set( zero, 1, false );
	} else {
#if BYTE_ORDER == LITTLE_ENDIAN
		if ( in_convert_data_from_host_byte_order ) {
			q = new __u8[ o = round_up( n, sizeof(__u32) )];
			
			if ( n %= sizeof(__u32 ) ) {
				*(__u32 *) q = *p & 0x80 ? ~0 : 0;
			} else {
				n = sizeof(__u32);
			}

			for ( i = 0; i < n; ++i ) ((__u8 *)q)[ i ] = p[ i ];

			p = q;
			n = o;
		
			for ( i = 0; i < n; i += sizeof(__u32) ) {
				*(__u32 *)( p + i ) = swap32( *(__u32 *)( p + i ) );
			}
		
			io_integer.set( q, o, false );
		}
#endif

		if ( *p & 0x80 ) {
			for ( i = 0, o = n - 1; i < o; ++i, ++p, --n ) {
				if ( p[ 0 ] != 0xff || ! ( p[ 1 ] & 0x80 ) ) break;
			}
		} else {
			for ( i = 0, o = n - 1; i < o; ++i, ++p, --n ) {
				if ( p[ 0 ] || ( p[ 1 ] & 0x80 ) ) break;
			}
		}

		if ( p != q ) io_integer.set( p, n );
	}

	encode( k_asn1_ber_class_universal, k_asn1_ber_primitive, k_asn1_tag_integer, io_integer );
}


void asn1_ber::encode_null( octet_string &out_null ) {
	__u8	   *p = new __u8[ 2 ];
	
	p[ 0 ] = k_asn1_tag_null;
	p[ 1 ] = 0;
	
	out_null.set( p, 2, false );
}


void asn1_ber::encode_object_identifier( const __u32 *in_components, __u32 in_count, octet_string &out_object_identifier ) {
	if ( in_count < 2 ) _throw( err_bad_parameter );

	__u8				   *data, *p;
	__u32					i, t;
	
	p = data = new __u8[ in_count * sizeof(__u32) * 5 ];

	t = in_components[ 0 ] * 40 + in_components[ 1 ];

	for ( i = 1; i < in_count; ) {
		p += encode_word_to_octets_7_bits_per( t, p );
		t = in_components[ ++i ];
	}

	out_object_identifier.set( data, __u32(p - data) );

	delete[] data;

	encode( k_asn1_ber_class_universal, k_asn1_ber_primitive, k_asn1_tag_object_identifier, out_object_identifier );
}


void asn1_ber::encode_octet_string( octet_string &io_string, bool in_convert_data_from_host_byte_order ) {
#if BYTE_ORDER == LITTLE_ENDIAN
	__u32					i, n, *p;

	if ( in_convert_data_from_host_byte_order ) {
		if ( ( n = io_string.length() ) % sizeof(__u32) ) _throw( err_bad_data );

		p = (__u32 *) io_string.data();
		
		for ( i = 0, n /= sizeof(__u32); i < n; ++i ) p[ i ] = swap32( p[ i ] );
	}
#endif

	encode( k_asn1_ber_class_universal, k_asn1_ber_primitive, k_asn1_tag_octet_string, io_string );
}


void asn1_ber::encode_sequence( octet_string &io_sequence ) {
	encode( k_asn1_ber_class_universal, k_asn1_ber_constructed, k_asn1_tag_sequence, io_sequence );
}


#pragma mark -


// this version of encode() uses only the definite encoding form (x.690 8.1.3.4-5)
// for encoding the length octets because the content length is provided (thus
// no end-of-contents octets are used).  this is equivalent to the restricted der
// encoding method.
void asn1_ber::encode( __u32 in_class, __u32 in_encoding_type, __u32 in_tag, octet_string &io_content ) {
	__u32			i, ioc_len, i_len, l_len, n;
	const __u8	   *ioc_data;
	__u8		   *p, *q;

	ioc_data = io_content.data();
	ioc_len = io_content.length();
	
	i_len = number_of_octets_for_identifier( in_tag );
	l_len = number_of_octets_for_length( ioc_len );
	
	q = p = new __u8[ n = i_len + l_len + ioc_len ];
	
	write_identifier( p, in_class, in_encoding_type, in_tag );
	write_length( p += i_len, ioc_len );
	
#if LOG_ASN1_ENCODE
	char				   *type;

	switch ( in_tag ) {
		case k_asn1_tag_boolean:				type = "boolean";				break;		
		case k_asn1_tag_integer:				type = "integer";				break;
		case k_asn1_tag_null:					type = "null";					break;
		case k_asn1_tag_object_identifier:		type = "object identifier";		break;
		case k_asn1_tag_octet_string:			type = "octet string";			break;
		case k_asn1_tag_sequence:				type = "sequence";				break;
		
		default:								type = "unknown";				break;
	}

	console( "encoding identifier of type %s:", type );
	console( "identifier data is (%d bytes):", i_len );
	dump( q, i_len );
	console( "length data is (%d bytes):", l_len );
	dump( p, l_len );
#endif
	
	for ( i = 0, p += l_len; i < ioc_len; ++i ) p[ i ] = ioc_data[ i ];

#if LOG_ASN1_ENCODE
	console( "content data is (%d bytes):", ioc_len );
	dump( p, ioc_len );
#endif
	
	io_content.set( q, n, false );
}


__u32 asn1_ber::decode( octet_string &io_content, octet_string &out_content, bool in_convert_data_to_host_byte_order ) {
	__u32					asn1_class, encoding_type, ioc_len, tag, length;
#if BYTE_ORDER == LITTLE_ENDIAN
	__u32					i, j;
#endif
	__u32					n, *o;
	const __u8			   *ioc_data, *p;

	if ( ! ( ( ioc_len = io_content.length() ) && ( ioc_data = io_content.data() ) ) ) {
		_throw( err_bad_data );
	}

	p += read_identifier( p = ioc_data, ioc_len, asn1_class, encoding_type, tag );

#if LOG_ASN1_DECODE
	char				   *type;

	switch ( tag ) {
		case k_asn1_tag_boolean:				type = "boolean";				break;
		case k_asn1_tag_integer:				type = "integer";				break;
		case k_asn1_tag_null:					type = "null";					break;
		case k_asn1_tag_object_identifier:		type = "object identifier";		break;
		case k_asn1_tag_octet_string:			type = "octet string";			break;
		case k_asn1_tag_sequence:				type = "sequence";				break;
		
		default:								type = "unknown";				break;
	}

	console( "read %d bytes of identifier data for %s", __u32(p - ioc_data), type );
	dump( ioc_data, __u32(p - ioc_data) );
#endif

	switch ( tag ) {
		case k_asn1_tag_boolean:
		case k_asn1_tag_integer:
		case k_asn1_tag_null:
		case k_asn1_tag_object_identifier:
		case k_asn1_tag_octet_string:
		case k_asn1_tag_sequence:				break;
			
		default:								_throw( err_unimplemented );
	}

#if LOG_ASN1_DECODE
	const __u8			   *q = p;
#endif
	
	p += read_length( p, ioc_len - __u32(p - ioc_data), length );

#if LOG_ASN1_DECODE
	console( "read %d bytes of length data", __u32(p - q) );
	dump( q, __u32(p - q) );
#endif

	if ( ioc_len - __u32(p - ioc_data) < length ) _throw( err_bad_data );

	if ( tag == k_asn1_tag_integer && length ) {
		o = (__u32 *) new __u8[ n = round_up( length, sizeof(__u32) ) ];
	
		*o = *p & 0x80 ? ~0 : 0;
		
		memcpy( (__u8 *) o + n - length, p, length );
	
#if BYTE_ORDER == LITTLE_ENDIAN
		if ( in_convert_data_to_host_byte_order ) {
			for ( i = 0, j = n / sizeof(__u32); i < j; ++i ) {
				o[ i ] = swap32( o[ i ] );
			}
		}
#endif		
		out_content.set( (__u8 *) o, n, false, true );
	} else {
		out_content.set( p, length );
	}

#if BYTE_ORDER == LITTLE_ENDIAN
	if ( in_convert_data_to_host_byte_order && tag == k_asn1_tag_octet_string ) {
		if ( length % sizeof(__u32) ) _throw( err_bad_data );

		for ( i = 0, n = length / sizeof(__u32), o = (__u32 *) out_content.data(); i < n; ++i ) {
			o[ i ] = swap32( o[ i ] );
		}
	}
#endif

#if LOG_ASN1_DECODE
	console( "read %d bytes of content data", length );
	dump( p, length );
#endif
	
	if ( ( length = ioc_len - __u32(( p += length ) - ioc_data) ) ) {
		io_content.set( p, length );
	} else {
		io_content.set();
	}
	
	return tag;
}


#pragma mark -


void asn1_ber::decode_object_identifier( const octet_string &in_content, __u32 *&out_components ) {
	const __u8			   *data;
	bool					first;
	__u32					i, n, o, t1, t2;
	__u32					subidentifier_count, *subidentifier_list;

	data = in_content.data();
	n = in_content.length();
	
	if ( ! n ) _throw( err_bad_data );
	
	for ( first = true, subidentifier_count = i = 0; i < n; ++i ) {
		if ( first ) {
			if ( ! ( ( t1 = data[ i ] ) & 0x80 ) ) {
				++subidentifier_count;
			} else {
				first = false;
				for ( o = 7; ! ( t1 & 0x40 ); t1 <<= 1, --o ) ;
			}
		} else {
			// this shouldn't happen, but if a subidentifier needs more
			// than 32 bits to be represented it breaks this implementation.
			// therefore, if this exception happens come back and implement
			// subidentifiers as __u64 or asn1_integer.
			if ( ( o += 7 ) > 32 ) _throw( err_range );

			if ( ! ( ( t1 = data[ i ] ) & 0x80 ) ) {
				first = true;
				++subidentifier_count;
			}
		}
	}
	
	subidentifier_list = new __u32[ ++subidentifier_count + 1 ];
	
	for ( i = 2; i <= subidentifier_count; ++i ) {
		t1 = 0;
		do { t1 = t1 << 7 | ( t2 = *data++ ) & 0x7f; } while ( t2 & 0x80 );
		subidentifier_list[ i ] = t1;
	}
	
	subidentifier_list[ 0 ] = subidentifier_count;
	subidentifier_list[ 1 ] = subidentifier_list[ 2 ] / 40;
	subidentifier_list[ 2 ] %= 40;

	out_components = subidentifier_list;
}


#pragma mark -


__u32 asn1_ber::number_of_octets_for_identifier( __u32 in_tag ) {
	__u32		i, t;

	if ( in_tag < 31 ) return 1;
	else {
		for ( i = 32, t = 0x80000000; ! ( t & in_tag ); --i, t >>= 1 ) ;

		return 1 + i / 7 + ( i % 7 ? 1 : 0 );
	}
}


__u32 asn1_ber::number_of_octets_for_length( __u32 in_length ) {
	__u32		i, t;

	if ( in_length < 128 ) return 1;
	else {
		for ( i = 32, t = 0x80000000; ! ( t & in_length ); --i, t >>= 1 ) ;

		return 1 + i / 8 + ( i % 8 ? 1 : 0 );
	}
}


#pragma mark -


// returns the number of bytes in the identifier
__u32 asn1_ber::read_identifier( const __u8 *in_identifier_start, __u32 in_length, __u32 &out_class, __u32 &out_encoding_type, __u32 &out_tag ) {
	__u32		i = 0;

	out_class = *in_identifier_start & 0xc0;
	out_encoding_type = *in_identifier_start & 0x20;

	if ( ( *in_identifier_start & 0x1f ) != 0x1f ) {
		out_tag = *in_identifier_start & 0x1f;
	} else {
		for ( i = 1, out_tag = 0; i < in_length; ++i ) {
			out_tag = out_tag << 7 | in_identifier_start[ i ] & 0x7f;

			if ( ! ( in_identifier_start[ i ] & 0x80 ) ) break;
		}
		
		if ( i == in_length ) _throw( err_bad_data );
	}
	
	return ++i;
}


__u32 asn1_ber::read_length( const __u8 *in_length_start, __u32 in_length, __u32 &out_content_length ) {
	if ( ! in_length ) _throw( err_bad_data );
	
	__u32		i = 1, n;

	if ( ! ( *in_length_start & 0x80 ) ) {
		out_content_length = *in_length_start;
	} else {
		if ( ( n = *in_length_start & 0x7f ) > 4 ) _throw( err_unimplemented );
		
		for ( out_content_length = 0; i < in_length && i <= n; ++i ) {
			out_content_length = out_content_length << 8 | in_length_start[ i ];
		}
		
		if ( i != n + 1 ) _throw( err_bad_data );
	}
	
	return i;
}


#pragma mark -


void asn1_ber::write_identifier( __u8 *out_identifier_start, __u32 in_class, __u32 in_encoding_type, __u32 in_tag ) {
	if ( in_tag < 31 ) {
		out_identifier_start[ 0 ] = in_class | in_encoding_type | in_tag;
	} else {
		out_identifier_start[ 0 ] = in_class | in_encoding_type | 0x1f;
		
		encode_word_to_octets_7_bits_per( in_tag, ++out_identifier_start );
	}
}


void asn1_ber::write_length( __u8 *out_length_start, __u32 in_length ) {
	__u8		i;

	if ( in_length < 128 ) {
		out_length_start[ 0 ] = in_length;
	} else {
		out_length_start[ 0 ] = 0x80 | ( i = number_of_octets_for_length( in_length ) - 1 );
		
		for ( ; i; --i, in_length >>= 8 ) {
			out_length_start[ i ] = in_length & 0xff;
		}
	}
}


#pragma mark -


asn1_ber_sequence::asn1_ber_sequence( const octet_string &in_data ) {
	octet_string		o( in_data );

	if ( in_data.zero_data_on_release() ) set_zero_data_on_release();
	if ( asn1_ber::decode( o, m_seq ) != k_asn1_tag_sequence ) _throw( err_bad_data );
}


asn1_ber_sequence::asn1_ber_sequence( const asn1_ber_sequence &in_sequence ) {
	m_seq = in_sequence.m_seq;
}


void asn1_ber_sequence::init( const octet_string &in_data ) {
	octet_string		o( in_data ), p;

	if ( in_data.zero_data_on_release() ) {
		set_zero_data_on_release();
		p.set_zero_data_on_release();
	}
	if ( asn1_ber::decode( o, p ) != k_asn1_tag_sequence ) _throw( err_bad_data );

	m_seq = p;
}


void asn1_ber_sequence::encode( octet_string &out_sequence ) const {
	asn1_ber::encode_sequence( out_sequence = m_seq );
}


void asn1_ber_sequence::push( __u32 in_tag, const void *in_data, __u32 in_size, bool in_convert_data_from_host_byte_order ) {
	octet_string			o;

	switch ( in_tag ) {
		case k_asn1_tag_boolean: {
			asn1_ber::encode_boolean( *reinterpret_cast<const bool *>(in_data), o );
		} break;

		case k_asn1_tag_integer: {
			#if __asn1_uses_apn
				reinterpret_cast<const apn *>(in_data)->as_octet_string( o );
				asn1_ber::encode_integer( o );
			#else
				o = *reinterpret_cast<const octet_string *>(in_data);
				asn1_ber::encode_integer( o, in_convert_data_from_host_byte_order );
			#endif
		} break;

		case k_asn1_tag_null: {
			asn1_ber::encode_null( o );
		} break;
		
		case k_asn1_tag_object_identifier: {
			asn1_ber::encode_object_identifier( reinterpret_cast<const __u32 *>(in_data), in_size, o );
		} break;
		
		case k_asn1_tag_octet_string: {
			o = *reinterpret_cast<const octet_string *>(in_data);
		
			asn1_ber::encode_octet_string( o, in_convert_data_from_host_byte_order );
		} break;
		
		case k_asn1_tag_sequence: {
			o = reinterpret_cast<const asn1_ber_sequence *>(in_data)->m_seq;
			
			asn1_ber::encode_sequence( o );
		} break;
	}
	
	m_seq += o;
}


void *asn1_ber_sequence::pop( __u32 &out_type, __u32 in_expected_type, bool in_convert_data_to_host_byte_order ) {
	if ( ! m_seq.length() ) _throw_quiet( err_item_not_found );

	octet_string		o;
	void			   *result;
	
	out_type = asn1_ber::decode( m_seq, o, in_convert_data_to_host_byte_order );
	
	if ( in_expected_type && out_type != in_expected_type ) _throw( err_wrong_type );
	
	switch ( out_type ) {
		case k_asn1_tag_boolean: 		result = reinterpret_cast<void *>(*o.data());	break;
		case k_asn1_tag_integer:		result = new asn1_integer( o );					break;
		case k_asn1_tag_null:			result = nil;									break;
		
		case k_asn1_tag_object_identifier: {
			asn1_ber::decode_object_identifier( o, reinterpret_cast<__u32 *&>(result) );
		} break;
		
		case k_asn1_tag_octet_string:	result = new octet_string( o );					break;

		case k_asn1_tag_sequence: {
			result = new asn1_ber_sequence;

			reinterpret_cast<asn1_ber_sequence *>(result)->m_seq = o;
			reinterpret_cast<asn1_ber_sequence *>(result)->m_seq.set_zero_data_on_release( m_seq.zero_data_on_release() );
		} break;

		default:						_throw( err_unimplemented );
	}
	
	return result;
}


#pragma mark -


#if DEBUG


void parse_asn1_ber_sequence( const asn1_ber_sequence &in_sequence ) {
	void				   *p;
	asn1_ber_sequence		seq( in_sequence );
	const char			   *tag;
	__u32					type;
	
	static __u32			s_depth;
	
	printf( "%*csequence:\n", s_depth * 4, ' ' );
	fflush( stdout );
	
	++s_depth;
	
	_try {
		for ( ;; ) {
			p = seq.pop( type );
			
			switch ( type ) {
				case k_asn1_tag_boolean:	tag = "boolean";			break;

				case k_asn1_tag_integer: {
					tag = "integer";
					delete (asn1_integer *) p;
				} break;

				case k_asn1_tag_null:		tag = "null";				break;

				case k_asn1_tag_object_identifier: {
					tag = "object identifier";
					delete[] (__u32 *) p;
				} break;

				case k_asn1_tag_octet_string: {
					tag = "octet string";
					delete (octet_string *) p;
				} break;

				case k_asn1_tag_sequence: {
					parse_asn1_ber_sequence( *(asn1_ber_sequence *) p );
					delete (asn1_ber_sequence *) p;
				} continue;
				
				default:					_throw( err_unimplemented );
			}

			printf( "%*c%s\n", s_depth * 4, ' ', tag );
			fflush( stdout );
		}
	} _catch
	
	--s_depth;
	
	if ( _ex.err != err_item_not_found ) throw _err;	
}


#endif // DEBUG
