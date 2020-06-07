#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

#define _try
#define _catch

typedef unsigned int			__u32;
typedef unsigned long long		__u64;


#define _throw_errno_if( _err )		do { if ( ( _err ) ) { fprintf( stderr, "error %d: %s\n", errno, strerror( errno ) ); exit( errno ); } } while ( 0 )


static __u32 s_sha1_init_vectors[ 5 ] = {
	0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0
};

static __u32 s_sha256_init_vectors[ 8 ] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

static __u64 s_sha384_init_vectors[ 8 ] = {
	0xcbbb9d5dc1059ed8ull, 0x629a292a367cd507ull, 0x9159015a3070dd17ull, 0x152fecd8f70e5939ull,
	0x67332667ffc00b31ull, 0x8eb44a8768581511ull, 0xdb0c2e0d64f98fa7ull, 0x47b5481dbefa4fa4ull
};

static __u64 s_sha512_init_vectors[ 8 ] = {
	0x6a09e667f3bcc908ull, 0xbb67ae8584caa73bull, 0x3c6ef372fe94f82bull, 0xa54ff53a5f1d36f1ull,
	0x510e527fade682d1ull, 0x9b05688c2b3e6c1full, 0x1f83d9abfb41bd6bull, 0x5be0cd19137e2179ull
};

static __u32 s_sha1_k_constants[ 4 ] = {
	0x5a827999,	0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6
};

static __u32 s_sha256_k_constants[ 64 ] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static __u64 s_sha512_k_constants[ 80 ] = {
	0x428a2f98d728ae22ull, 0x7137449123ef65cdull, 0xb5c0fbcfec4d3b2full, 0xe9b5dba58189dbbcull,
	0x3956c25bf348b538ull, 0x59f111f1b605d019ull, 0x923f82a4af194f9bull, 0xab1c5ed5da6d8118ull,
	0xd807aa98a3030242ull, 0x12835b0145706fbeull, 0x243185be4ee4b28cull, 0x550c7dc3d5ffb4e2ull,
	0x72be5d74f27b896full, 0x80deb1fe3b1696b1ull, 0x9bdc06a725c71235ull, 0xc19bf174cf692694ull,
	0xe49b69c19ef14ad2ull, 0xefbe4786384f25e3ull, 0x0fc19dc68b8cd5b5ull, 0x240ca1cc77ac9c65ull,
	0x2de92c6f592b0275ull, 0x4a7484aa6ea6e483ull, 0x5cb0a9dcbd41fbd4ull, 0x76f988da831153b5ull,
	0x983e5152ee66dfabull, 0xa831c66d2db43210ull, 0xb00327c898fb213full, 0xbf597fc7beef0ee4ull,
	0xc6e00bf33da88fc2ull, 0xd5a79147930aa725ull, 0x06ca6351e003826full, 0x142929670a0e6e70ull,
	0x27b70a8546d22ffcull, 0x2e1b21385c26c926ull, 0x4d2c6dfc5ac42aedull, 0x53380d139d95b3dfull,
	0x650a73548baf63deull, 0x766a0abb3c77b2a8ull, 0x81c2c92e47edaee6ull, 0x92722c851482353bull,
	0xa2bfe8a14cf10364ull, 0xa81a664bbc423001ull, 0xc24b8b70d0f89791ull, 0xc76c51a30654be30ull,
	0xd192e819d6ef5218ull, 0xd69906245565a910ull, 0xf40e35855771202aull, 0x106aa07032bbd1b8ull,
	0x19a4c116b8d2d0c8ull, 0x1e376c085141ab53ull, 0x2748774cdf8eeb99ull, 0x34b0bcb5e19b48a8ull,
	0x391c0cb3c5c95a63ull, 0x4ed8aa4ae3418acbull, 0x5b9cca4f7763e373ull, 0x682e6ff3d6b2b8a3ull,
	0x748f82ee5defb2fcull, 0x78a5636f43172f60ull, 0x84c87814a1f0ab72ull, 0x8cc702081a6439ecull,
	0x90befffa23631e28ull, 0xa4506cebde82bde9ull, 0xbef9a3f7b2c67915ull, 0xc67178f2e372532bull,
	0xca273eceea26619cull, 0xd186b8c721c0c207ull, 0xeada7dd6cde0eb1eull, 0xf57d4f7fee6ed178ull,
	0x06f067aa72176fbaull, 0x0a637dc5a2c898a6ull, 0x113f9804bef90daeull, 0x1b710b35131c471bull,
	0x28db77f523047d84ull, 0x32caab7b40c72493ull, 0x3c9ebe0a15c9bebcull, 0x431d67c49c100d4cull,
	0x4cc5d4becb3e42b6ull, 0x597f299cfc657e2aull, 0x5fcb6fab3ad6faecull, 0x6c44198c4a475817ull
};


int main( int in_argc, char **in_argv ) {
	__u32					buffer[ 273 ];
	int						fd, i, j;
	char				   *prefix;
	
	if ( in_argc > 1 ) asprintf( &prefix, "%s_", in_argv[ 1 ] );
	else prefix = (char *) "";
	
	_throw_errno_if( ( fd = open( "/dev/urandom", O_RDONLY ) ) == -1 );
	
	_try {
		_throw_errno_if( read( fd, buffer, sizeof(buffer) ) != sizeof(buffer) );
		
	check:
	
		for ( i = 0; i < sizeof(buffer) / sizeof(__u32); ++i ) {
			for ( j = 0; j < sizeof(buffer) / sizeof(__u32); ++j ) {
				if ( j == i ) continue;
				if ( ( buffer[ i ] >> 24 & 0xff ) == 0xff || ( buffer[ i ] >> 24 & 0xff ) == 0 ||
					 ( buffer[ i ] >> 16 & 0xff ) == 0xff || ( buffer[ i ] >> 16 & 0xff ) == 0 ||
					 ( buffer[ i ] >>  8 & 0xff ) == 0xff || ( buffer[ i ] >>  8 & 0xff ) == 0 ||
					 ( buffer[ i ]		 & 0xff ) == 0xff || ( buffer[ i ]		 & 0xff ) == 0 ||
					   buffer[ i ] == buffer[ j ] )
				{
					_throw_errno_if( read( fd, &buffer[ i ], sizeof(__u32) ) != sizeof(__u32) );
					goto check;
				}
			}
		}
		
		for ( i = 0; i < 5; ++i ) if ( buffer[ i ] == s_sha1_init_vectors[ i ] ) goto check;
		for ( j = 0; i < 13; ++i, ++j ) if ( buffer[ i ] == s_sha256_init_vectors[ j ] ) goto check;
		for ( j = 0; i < 29; i += 2, ++j ) if ( *(__u64 *)&buffer[ i ] == s_sha384_init_vectors[ j ] ) goto check;
		for ( j = 0; i < 45; i += 2, ++j ) if ( *(__u64 *)&buffer[ i ] == s_sha512_init_vectors[ j ] ) goto check;
		for ( j = 0; i < 49; ++i, ++j ) if ( buffer[ i ] == s_sha1_k_constants[ j ] ) goto check;
		for ( j = 0; i < 113; ++i, ++j ) if ( buffer[ i ] == s_sha256_k_constants[ j ] ) goto check;
		for ( j = 0; i < 273; i += 2, ++j ) if ( *(__u64 *)&buffer[ i ] == s_sha512_k_constants[ j ] ) goto check;

		printf( "#ifndef __%ssha_init_vectors_h__\n#define __%ssha_init_vectors_h__\n\n\n\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n\n", prefix, prefix );
		printf( "static __u32 s_sha1_xor_init_vectors[ 5 ] = {\n\t" );
		for ( i = 0; i < 5; ++i ) printf( "0x%08x%s", buffer[ i ], i == 4 ? "\n" : ", " );
		printf( "};\n\nstatic __u32 s_sha256_xor_init_vectors[ 8 ] = {\n\t" );
		for ( ; i < 9; ++i ) printf( "0x%08x,%s", buffer[ i ], i == 8 ? "\n\t" : " " );
		for ( ; i < 13; ++i ) printf( "0x%08x%s", buffer[ i ], i == 12 ? "\n" : ", " );
		printf( "};\n\nstatic __u64 s_sha384_xor_init_vectors[ 8 ] = {\n\t" );
		for ( ; i < 21; i += 2 ) printf( "0x%08x%08xull,%s", buffer[ i ], buffer[ i + 1 ], i == 19 ? "\n\t" : " " );
		for ( ; i < 29; i += 2 ) printf( "0x%08x%08xull%s", buffer[ i ], buffer[ i + 1 ], i == 27 ? "\n" : ", " );
		printf( "};\n\nstatic __u64 s_sha512_xor_init_vectors[ 8 ] = {\n\t" );
		for ( ; i < 37; i += 2 ) printf( "0x%08x%08xull,%s", buffer[ i ], buffer[ i + 1 ], i == 35 ? "\n\t" : " " );
		for ( ; i < 45; i += 2 ) printf( "0x%08x%08xull%s", buffer[ i ], buffer[ i + 1 ], i == 43 ? "\n" : ", " );
		printf( "};\n\nstatic __u32 s_sha1_xor_k_constants[ 4 ] = {\n\t" );
		for ( ; i < 49; ++i ) printf( "0x%08x%s", buffer[ i ], i == 48 ? "\n" : ", " );
		printf( "};\n\nstatic __u32 s_sha256_xor_k_constants[ 64 ] = {\n\t" );
		for ( ; i < 113; ) {
			for ( j = 0; j < 4; ++i, ++j ) {
				printf( "0x%08x%s", buffer[ i ], j == 3 ? ( i == 112 ? "\n" : ",\n\t" ) : ", " );
			}
		}
		printf( "};\n\nstatic __u64 s_sha512_xor_k_constants[ 64 ] = {\n\t" );
		for ( ; i < 273; ) {
			for ( j = 0; j < 4; i += 2, ++j ) {
				printf( "0x%08x%08xull%s", buffer[ i ], buffer[ i + 1 ], j == 3 ? ( i == 271 ? "\n" : ",\n\t" ) : ", " );
			}
		}
		printf( "};\n\n\n#ifdef __cplusplus\n}\n#endif\n\n\n\n#endif\t// __%ssha_init_vectors_h__\n", prefix );
	} _catch
	
	if ( *prefix ) free( prefix );
	
	close( fd );

	return 0;
}
