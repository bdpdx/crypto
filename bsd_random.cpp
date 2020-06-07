#include "bsd_random.h"


inline long good_random( long in_x ) {
	long					high, low;

	if ( ! in_x ) in_x = 123459876;
	
	high = in_x / 127773;
	low = in_x % 127773;
	in_x = 16807 * low - 2836 * high;
	
	if ( in_x < 0 ) in_x += 0x7fffffff;

	return in_x;
}


bsd_random::bsd_random() {
	init();
}


bsd_random::bsd_random( unsigned long in_seed ) {
	srandom( in_seed, true );
}


bsd_random::~bsd_random() {
	long					i;

	for ( i = 0; i < 31; ++i ) state_table[ i ] = 0;
}


void bsd_random::init() {
	state_table[ 0 ] = 0x991539b1;
	state_table[ 1 ] = 0x16a5bce3;
	state_table[ 2 ] = 0x6774a4cd;
	state_table[ 3 ] = 0x3e01511e;
	state_table[ 4 ] = 0x4e508aaa;
	state_table[ 5 ] = 0x61048c05;
	state_table[ 6 ] = 0xf5500617;
	state_table[ 7 ] = 0x846b7115;
	state_table[ 8 ] = 0x6a19892c;
	state_table[ 9 ] = 0x896a97af;
	state_table[ 10 ] = 0xdb48f936;
	state_table[ 11 ] = 0x14898454;
	state_table[ 12 ] = 0x37ffd106;
	state_table[ 13 ] = 0xb58bff9c;
	state_table[ 14 ] = 0x59e17104;
	state_table[ 15 ] = 0xcf918a49;
	state_table[ 16 ] = 0x09378c83;
	state_table[ 17 ] = 0x52c7a471;
	state_table[ 18 ] = 0x8d293ea9;
	state_table[ 19 ] = 0x1f4fc301;
	state_table[ 20 ] = 0xc3db71be;
	state_table[ 21 ] = 0x39b44e1c;
	state_table[ 22 ] = 0xf8a44ef9;
	state_table[ 23 ] = 0x4c8b80b1;
	state_table[ 24 ] = 0x19edc328;
	state_table[ 25 ] = 0x87bf4bdd;
	state_table[ 26 ] = 0xc9b240e5;
	state_table[ 27 ] = 0xe9ee4b1b;
	state_table[ 28 ] = 0x4382aee7;
	state_table[ 29 ] = 0x535b6b41;
	state_table[ 30 ] = 0xf3bec5da;
	
	state_table_front = &state_table[ 3 ];
	state_table_rear = &state_table[ 0 ];
}


long bsd_random::random() {
	long				   *end, i, *f, *r;

	end = &state_table[ 31 ];

	f = state_table_front;
	r = state_table_rear;

	i = ( ( *f += *r ) >> 1 ) & 0x7fffffff;
	
	if ( ++f >= end ) {
		f = state_table;
		++r;
	} else if ( ++r >= end ) {
		r = state_table;
	}

	state_table_front = f;
	state_table_rear = r;

	return i;
}


void bsd_random::srandom( unsigned long in_seed, bool in_init_first ) {
	long					i;

	if ( in_init_first ) init();

	state_table[ 0 ] = (long) in_seed;

	for ( i = 1; i < 31; ++i ) {
		state_table[ i ] = good_random( state_table[ i - 1 ] );
	}

	state_table_front = &state_table[ 3 ];
	state_table_rear = &state_table[ 0 ];

	for ( i = 0; i < 310; ++i ) random();
}
