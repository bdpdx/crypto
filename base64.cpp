#include "base64.h"


#ifndef nil
	#define nil 0
#endif


static const __u8			s_base64_d_table[] = {
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,		// 0x00
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,		// 0x10
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 62, 99, 62, 99, 63,		// 0x20
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 99, 99, 99, 99, 99, 99,		// 0x30
	99,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,		// 0x40
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 99, 99, 99, 99, 63,		// 0x50
	99, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,		// 0x60
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 99, 99, 99, 99, 99		// 0x70
};

static const char			s_base64_e_table[]		= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char			s_base64_e_url_table[]	= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";


char base64::base64_for_bits( __u8 in_bits, base64_encoding in_encoding ) {
	const char			   *e_table;

	switch ( in_encoding ) {
		case k_base64_cgi_safe_encoding:
		case k_base64_url_and_filename_safe_encoding:	e_table = s_base64_e_url_table;				break;

		default:
		case k_base64_default_encoding:					e_table = s_base64_e_table;					break;
	}

	return e_table[ in_bits & 0x3f ];
}


__u8 base64::bits_for_base64( char in_base64, base64_encoding in_encoding ) {
	in_encoding;

	return s_base64_d_table[ in_base64 ];
}


char *base64::encode( __u8 *in_data, __u32 in_length, base64_encoding in_encoding ) {
	if ( ! ( in_data && in_length ) ) return nil;

	const char			   *e_table;
	__u32					i, n, t;
	__u8				   *p, pad;
	char				   *r, *result;

	switch ( in_encoding ) {
		case k_base64_cgi_safe_encoding:				e_table = s_base64_e_url_table;	pad = '.';	break;
		case k_base64_url_and_filename_safe_encoding:	e_table = s_base64_e_url_table;	pad = '=';	break;

		default:
		case k_base64_default_encoding:					e_table = s_base64_e_table;		pad = '=';	break;
	}

	p = in_data;
	r = result = new char[ 4 * ( ( n = in_length / 3 ) + ( in_length % 3 ? 1 : 0 ) ) + 1 ];

	for ( i = 0; i < n; ++i ) {
		t = *p++ << 16;
		t |= *p++ << 8;
		t |= *p++;

		*r++ = e_table[ t >> 18 ];
		*r++ = e_table[ t >> 12 & 0x3f ];
		*r++ = e_table[ t >> 6 & 0x3f ];
		*r++ = e_table[ t & 0x3f ];
	}
	
	if ( ( i = n * 3 ) < in_length ) {
		t = *p++ << 16;

		*r++ = e_table[ t >> 18 ];

		if ( ++i < in_length ) {
			t |= *p++ << 8;

			*r++ = e_table[ t >> 12 & 0x3f ];
			*r++ = e_table[ t >> 6 & 0x3f ];
		} else {
			*r++ = e_table[ t >> 12 & 0x3f ];
			*r++ = pad;
		}
		
		*r++ = pad;
	}
	
	*r = 0;
	
	return result;
}


__u8 *base64::decode( const char *in_base64_string, __u32 &out_length, base64_encoding in_encoding ) {
	if ( ! ( in_base64_string && *in_base64_string ) ) { out_length = 0; return nil; }
	
	__u32					i, j, n, t;
	const char			   *p;
	char					pad;
	__u8				   *r, *result;

	switch ( in_encoding ) {
		case k_base64_cgi_safe_encoding:				pad = '.';									break;

		default:
		case k_base64_default_encoding:
		case k_base64_url_and_filename_safe_encoding:	pad = '=';									break;
	}

	for ( n = 0; in_base64_string[ n ]; ++n ) ;
	
	if ( n < 4 ) { out_length = 0; return nil; }
	
	j = 3;
	
	if ( in_base64_string[ n - 1 ] == pad ) --j;
	if ( in_base64_string[ n - 2 ] == pad ) --j;

	p = in_base64_string;
	r = result = new __u8[ out_length = 3 * ( ( n -= 4 ) / 4 ) + j ];

	for ( i = 0; i < n; i += 4 ) {

	fetch_3_bytes:

		t = s_base64_d_table[ *p++ ] << 18;
		t |= s_base64_d_table[ *p++ ] << 12;
		t |= s_base64_d_table[ *p++ ] << 6;
		t |= s_base64_d_table[ *p++ ];

		*r++ = t >> 16;
		*r++ = t >> 8 & 0xff;
		*r++ = t & 0xff;
	}
	
	switch ( j ) {
		case 1: {
			t = s_base64_d_table[ *p++ ] << 2;
			t |= s_base64_d_table[ *p++ ] >> 4;
			
			*r++ = t;
		} break;

		case 2: {
			t = s_base64_d_table[ *p++ ] << 10;
			t |= s_base64_d_table[ *p++ ] << 4;
			t |= s_base64_d_table[ *p++ ] >> 2;
			
			*r++ = t >> 8;
			*r++ = t & 0xff;
		} break;

		case 3:				j = 0;	goto fetch_3_bytes;

		default:			break;
	}
	
	return result;
}
