#include "crypto_random.h"


crypto_random::crypto_random( const char *in_random_number_file ) {
#if _WIN32
	in_random_number_file;

	if ( ! CryptAcquireContext( &_provider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) ) _throw_winerr( GetLastError() );
#else
	if ( ( _fd = open( in_random_number_file, O_RDONLY ) ) == -1 ) _throw_errno();
#endif
}

crypto_random::~crypto_random() {
#if _WIN32
	CryptReleaseContext( _provider, 0 );
#else
	close( _fd );
#endif
}


void crypto_random::random( void *out_buffer, __u32 in_length ) {
#if _WIN32
	if ( ! CryptGenRandom( _provider, (DWORD) in_length, (BYTE *) out_buffer ) ) _throw_winerr( GetLastError() );
#else
	if ( read( _fd, out_buffer, in_length ) != int(in_length) ) _throw( err_read_failure );
#endif
}
