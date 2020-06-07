#include "base32.h"


#if defined( ENABLE_BASE32_SWEARWORD_SAFE )
	static __u8 s_base32_d_table[] = {
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,		// 0x00
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,		// 0x10
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,		// 0x20
		22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 99, 99, 99, 99, 99, 99,		// 0x30
		99, 99,  0,  1,  2,  3,  4,  5,  6, 99,  7,  8,  9, 10, 11, 99,		// 0x40
		12, 13, 14, 15, 16, 99, 17, 18, 19, 20, 21, 99, 99, 99, 99, 99,		// 0x50
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,		// 0x60
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99		// 0x70
	};

	static char s_base32_e_table[] = "BCDEFGHJKLMNPQRSTVWXYZ0123456789";
#else
	static __u8 s_base32_d_table[] = {
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,		// 0x00
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,		// 0x10
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,		// 0x20
		99, 99, 26, 27, 28, 29, 30, 31, 99, 99, 99, 99, 99, 99, 99, 99,		// 0x30
		99,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,		// 0x40
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 99, 99, 99, 99, 99,		// 0x50
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,		// 0x60
		99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99		// 0x70
	};

	static char s_base32_e_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
#endif


char base32::base32_for_bits( __u8 in_bits ) {
	if ( in_bits >= 32 ) _throw( err_range );

	return s_base32_e_table[ in_bits ];
}


__u8 base32::bits_for_base32( char in_base32 ) {
#if defined( ENABLE_BASE32_SWEARWORD_SAFE )
	if ( ! ( in_base32 >= 'B' && in_base32 <= 'Z' || in_base32 >= '0' && in_base32 <= '9' ) ) _throw( err_range );
	if ( in_base32 == 'A' || in_base32 == 'I' || in_base32 == 'O' || in_base32 == 'U' ) _throw( err_range );
#else
	if ( ! ( in_base32 >= 'A' && in_base32 <= 'Z' || in_base32 >= '2' && in_base32 <= '7' ) ) _throw( err_range );
#endif
	
	return s_base32_d_table[ in_base32 ];
}


char *base32::encode( __u8 *in_data, __u32 in_length ) {
	if ( ! ( in_data && in_length ) ) return nil;

	__u8	   *p;
	__u32		i, n, t;
	char	   *r, *result;

	p = in_data;
	r = result = new char[ 8 * ( ( n = in_length / 5 ) + ( in_length % 5 ? 1 : 0 ) ) + 1 ];

	for ( i = 0; i < n; ++i ) {
		t = *p++ << 24;
		t |= *p++ << 16;
		t |= *p++ << 8;
		t |= *p++;

		*r++ = s_base32_e_table[ t >> 27 ];
		*r++ = s_base32_e_table[ t >> 22 & 0x1f ];
		*r++ = s_base32_e_table[ t >> 17 & 0x1f ];
		*r++ = s_base32_e_table[ t >> 12 & 0x1f ];
		*r++ = s_base32_e_table[ t >> 7 & 0x1f ];
		*r++ = s_base32_e_table[ t >> 2 & 0x1f ];

		t = t << 8 | *p++;

		*r++ = s_base32_e_table[ t >> 5 & 0x1f ];
		*r++ = s_base32_e_table[ t & 0x1f ];
	}
	
	if ( ( i = n * 5 ) < in_length ) {
		t = *p++ << 24;

		*r++ = s_base32_e_table[ t >> 27 ];

		if ( ++i < in_length ) {
			t |= *p++ << 16;

			*r++ = s_base32_e_table[ t >> 22 & 0x1f ];
			*r++ = s_base32_e_table[ t >> 17 & 0x1f ];
			
			if ( ++i < in_length ) {
				t |= *p++ << 8;

				*r++ = s_base32_e_table[ t >> 12 & 0x1f ];

				if ( ++i < in_length ) {
					t |= *p;
					*r++ = s_base32_e_table[ t >> 7 & 0x1f ];
					*r++ = s_base32_e_table[ t >> 2 & 0x1f ];
					*r++ = s_base32_e_table[ t << 3 & 0x1f ];
				} else {
					*r++ = s_base32_e_table[ t >> 7 & 0x1f ];
					*r++ = '=';
					*r++ = '=';
				}
			} else {
				*r++ = s_base32_e_table[ t >> 12 & 0x1f ];
				*r++ = '=';
				*r++ = '=';
				*r++ = '=';
			}
		} else {
			*r++ = s_base32_e_table[ t >> 22 & 0x1f ];
			*r++ = '=';
			*r++ = '=';
			*r++ = '=';
			*r++ = '=';
			*r++ = '=';
		}

		*r++ = '=';
	}
	
	*r = 0;
	
	return result;
}


__u8 *base32::decode( const char *in_base32_string, __u32 &out_length ) {
	if ( ! ( in_base32_string && *in_base32_string ) ) { out_length = 0; return nil; }
	
	const char	   *p;
	__u32			i, j, n, t;
	__u8		   *r, *result;

	for ( n = 0; in_base32_string[ n ]; ++n ) ;
	
	if ( n % 8 ) { out_length = 0; return nil; }
	
	j = 5;
	
	if ( in_base32_string[ n - 1 ] == '=' ) --j;
	if ( in_base32_string[ n - 3 ] == '=' ) --j;
	if ( in_base32_string[ n - 4 ] == '=' ) --j;
	if ( in_base32_string[ n - 6 ] == '=' ) --j;

	p = in_base32_string;
	r = result = new __u8[ out_length = 5 * ( ( n -= 8 ) / 8 ) + j ];

	for ( i = 0; i < n; i += 8 ) {
		// fetch_5_bytes:
		t = s_base32_d_table[ *p++ ] << 27;
		t |= s_base32_d_table[ *p++ ] << 22;
		t |= s_base32_d_table[ *p++ ] << 17;
		t |= s_base32_d_table[ *p++ ] << 12;
		t |= s_base32_d_table[ *p++ ] << 7;
		t |= s_base32_d_table[ *p++ ] << 2;
		t |= s_base32_d_table[ *p ] >> 3;

		*r++ = t >> 24;
		*r++ = t >> 16 & 0xff;
		*r++ = t >> 8 & 0xff;
		*r++ = t & 0xff;
		
		t = s_base32_d_table[ *p++ ] << 5;
		t |= s_base32_d_table[ *p++ ];
		
		*r++ = t & 0xff;
	}
	
	switch ( j ) {
		case 1: {
			t = s_base32_d_table[ *p++ ] << 3;
			t |= s_base32_d_table[ *p++ ] >> 2;
			
			*r++ = t;
		} break;

		case 2: {
			t = s_base32_d_table[ *p++ ] << 11;
			t |= s_base32_d_table[ *p++ ] << 6;
			t |= s_base32_d_table[ *p++ ] << 1;
			t |= s_base32_d_table[ *p ] >> 4;
			
			*r++ = t >> 8;
			*r++ = t & 0xff;
		} break;

		case 3: {
			t = s_base32_d_table[ *p++ ] << 19;
			t |= s_base32_d_table[ *p++ ] << 14;
			t |= s_base32_d_table[ *p++ ] << 9;
			t |= s_base32_d_table[ *p++ ] << 4;
			t |= s_base32_d_table[ *p ] >> 1;
			
			*r++ = t >> 16;
			*r++ = t >> 8;
			*r++ = t & 0xff;
		} break;

		case 4: {
			t = s_base32_d_table[ *p++ ] << 27;
			t |= s_base32_d_table[ *p++ ] << 22;
			t |= s_base32_d_table[ *p++ ] << 17;
			t |= s_base32_d_table[ *p++ ] << 12;
			t |= s_base32_d_table[ *p++ ] << 7;
			t |= s_base32_d_table[ *p++ ] << 2;
			t |= s_base32_d_table[ *p ] >> 3;

			*r++ = t >> 24;
			*r++ = t >> 16 & 0xff;
			*r++ = t >> 8 & 0xff;
			*r++ = t & 0xff;
		} break;

		default:			break;
	}
	
	return result;
}
