#ifndef __crypto_random_h__
#define __crypto_random_h__



#if _WIN32
	#include "precompiled.h"

	#include <Windows.h>
	#include <Wincrypt.h>
#endif


struct crypto_random {

	crypto_random( const char *in_random_number_file = "/dev/urandom" );
   ~crypto_random();

	void random( void *out_buffer, __u32 in_length );

protected:

#if _WIN32
	HCRYPTPROV				_provider;
#else
	int						_fd;
#endif

};



#endif // __crypto_random_h__
