#include <sys/types.h>

#include "balance_types.h"


#define k_crc32_polynomial (__u32)	0xedb88320

static __u32 s_crc_table[ 256 ];

/*
 * This routine writes each s_crc_table entry exactly once,
 * with the correct final value.  Thus, it is safe to call
 * even on a table that someone else is using concurrently.
 */
static void make_crc_table();
static void make_crc_table() {
	__u32		i, j, h = 1;

	s_crc_table[ 0 ] = 0;
	for ( i = 128; i; i >>= 1 ) {
		h = ( h >> 1 ) ^ ( ( h & 1 ) ? k_crc32_polynomial : 0 );

		// h is now s_crc_table[ i ]
		for ( j = 0; j < 256; j += 2 * i ) s_crc_table[ i + j ] = s_crc_table[ j ] ^ h;
	}
}


/*
 * This computes the standard preset and inverted CRC, as used
 * by most networking standards.  Start by passing in an initial
 * chaining value of 0, and then pass in the return value from the
 * previous crc_32() call.  The final return value is the CRC.
 * Note that this is a little-endian CRC, which is best used with
 * data transmitted lsbit-first, and it should, itself, be appended
 * to data in little-endian byte and bit order to preserve the
 * property of detecting all burst errors of length 32 bits or less.
 */
__u32 crc_32( __u32 in_crc, char const *in_buf, off_t in_len );
__u32 crc_32( __u32 in_crc, char const *in_buf, off_t in_len ) {
	if ( ! s_crc_table[ 255 ] ) make_crc_table();

	in_crc ^= 0xffffffff;

	while ( in_len-- ) {
		in_crc = ( in_crc >> 8 ) ^ s_crc_table[ ( in_crc ^ *in_buf++ ) & 0xff ];
	}

	return in_crc ^ 0xffffffff;
}


#ifdef GENERATE_APPLICATION

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "useful_macros.h"


// gcc -I/Users/in/code/utility/shared -DGENERATE_APPLICATION -o crc32 -g crc32.c

int main( int argc, char **argv ) {
	off_t		n;
	char	   *buf;
	__u32		i, crc;
	__s32		fd, size;
	off_t		read_so_far, read_to;

	if ( argc == 1 ) {
		fprintf( stderr, "usage: %s <file_to_checksum> [-b to_hex_offset] [<file_to_checksum>] [-b to_hex_offset] ...\n", *argv );
		exit( 1 );
	}
	
	if ( ! ( buf = (char *) malloc( k_1m ) ) ) {
		fprintf( stderr, "unable to allocate buffer to perform calculation\n" );
		exit( 1 );
	}
	
	for ( i = 1; i < argc; ++i ) {
		if ( ( fd = open( argv[ i ], O_RDONLY ) ) == -1 ) {
			fprintf( stderr, "%s: %s\n", argv[ i ], strerror( errno ) );
			continue;
		}
		
		if ( i + 2 < argc && ! strncmp( argv[ i + 1 ], "-b", 2 ) ) {
			if ( ! ( read_to = strtoul( argv[ i + 2 ], nil, 0 ) ) ) {
				fprintf( stderr, "error reading offset parameter (%d): %s\n", errno, strerror( errno ) );
				exit( 1 );
			}
		} else {
			read_to = 0;
		}
		
		crc = 0;
		read_so_far = 0;
		
		while ( ( size = read( fd, buf, k_1m ) ) > 0 ) {
			if ( read_to ) read_so_far += size = min( size, read_to - read_so_far );

			crc = crc_32( crc, buf, size );
			
			if ( read_to && read_so_far == read_to ) break;
		}
		
		close( fd );
		
		if ( size == -1 ) {
			fprintf( stderr, "%s: %s\n", argv[ i ], strerror( errno ) );
			if ( read_to ) i += 2;
			continue;
		}

		fprintf( stdout, "%s: 0x%08x", argv[ i ], crc );

		if ( read_to ) {
			fprintf( stdout, " (checksum of first %#01llx bytes)", read_so_far );
			i += 2;
		}
		
		fprintf( stdout, "\n" );
	}
	
	free( buf );
	
	return 0;
}

#endif
