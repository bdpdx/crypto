#include "sha.hpp"


int main( int in_argc, const char *in_argv[] ) {
	sha_algorithm_type		algorithm = k_sha1;
	const char			   *app, *file;
	__u8					hash[ k_sha512_64 * 2 + 1 ];
	__u32					i;

	if ( ( app = strrchr( in_argv[ 0 ], '/' ) ) ) ++app; else app = in_argv[ 0 ];

	if ( in_argc < 2 ) {
		fprintf( stderr, "usage: %s <filename> ...\n", app );
		exit( ENOENT );
	}
	
	if ( ! strcmp( app, "sha1" ) ) algorithm = k_sha1;
	else if ( ! strcmp( app, "sha256" ) ) algorithm = k_sha256;
	else if ( ! strcmp( app, "sha384" ) ) algorithm = k_sha384;
	else if ( ! strcmp( app, "sha512" ) ) algorithm = k_sha512;

	for ( i = 1; i < in_argc; ++i ) {
		sha::hash( (__u8 *) in_argv[ i ], hash, algorithm );
		sha::hash_string( hash, (char *) hash, algorithm );

		if ( ( file = strrchr( in_argv[ i ], '/' ) ) ) ++file; else file = in_argv[ i ];

		fprintf( stdout, "%s: %s\n", file, hash );
	}
	
	return 0;
}
