#ifndef __bsd_random_h__
#define __bsd_random_h__



#if _WIN32
	#include "precompiled.h"
#endif

// A portable pseudo-random number generator based on and compatible with
// the FreeBSD type-3 pseudo-random number generator in the darwin source
// tree.  Usage follows random(3).

class bsd_random {

public:

	// passing in_seed == 0 means use default seeding, otherwise
	// srandom( in_seed ) is called as the final stage of initialization.
	bsd_random();
	bsd_random( unsigned long in_seed );
   ~bsd_random();

	operator long() { return random(); }
	operator unsigned long() { return (unsigned long) random(); }

	void init();
	long random();
	void srandom( unsigned long in_seed, bool in_init_first = false );

protected:

	long				   *state_table_front;
	long				   *state_table_rear;
	long					state_table[ 31 ];

};



#endif // __bsd_random_h__
