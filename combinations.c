#include <stdlib.h>
#include <unistd.h>

#include "combinations.h"


static int acm_382( int *out_x, /* int *out_y, */ int *out_z, int *io_p );


// Given an array of in_N objects each of size in_size, generate all
// combinations of in_M objects, calling in_callback once for each
// new combination.  The arguments to combination_callback are an array
// array of pointers to objects in the in_objects array, and in_M, the
// number of objects in the combination array.  The in_callback function
// returns false to indicate that it wants another combination, or true
// to indicate that processing should be aborted.
void combinations(
	const void *in_objects,					// array of objects
	int in_size,							// size of each object
	int in_N,								// number of objects in array
	int in_M,								// number of objects to choose
	combination_callback in_callback,		// callback to call
	void *in_context )						// callback context
{
	void				  **c;
	int						i, *p, r, x, z;

	c = (void **) malloc( sizeof(void *) * in_M );
	p = (int *) malloc( sizeof(int) * ( in_N + 2 ) );

	r = in_N - in_M;

	for ( i = r; i; --i ) p[ i ] = 0;
	for ( i = in_M; i; --i ) p[ r + i ] = i;

	p[ 0 ] = in_N + 1;
	p[ in_N + 1 ] = -2;

	if ( in_M == 0 ) p[ 1 ] = 1;

	for ( i = in_M - 1; i >= 0; --i ) {
		c[ i ] = (void *)( (unsigned) in_objects + in_size * ( r + i ) );
	}

	for ( ;; ) {
		if ( in_callback( in_context, c, in_M ) ) break;
		if ( acm_382( &x, &z, p ) ) break;
		
		c[ z ] = (void *)( (unsigned) in_objects + in_size * x );
	}

	free( c );
	free( p );
}



// ACM Algorithm 382 (Twiddle) by Phillip J. Chase
// Communications of the ACM Volume 13, Number 6, 1970 (pp. 368, 376)
//
// Binomial Coefficient / Combination production:
//
// Produces all non-repeating combinations of M out of N objects.
//
// Adapted to C from ALGOL-60 by Brian Doyle 07.25.05
//
// NOTE:  The original algorithm also provides a means for generating
//		  a bit sequence of N bits with M 1's and N-M 0's.  Since I
//		  don't need this functionality at the moment I've commented
//		  out the out_y variable and its calculation.  To reimplement
//		  this feature see the driver() test code below.
int acm_382( int *out_x, /* int *out_y, */ int *out_z, int *io_p ) {
	int						i, j, k, t;
	
	for ( j = 1; io_p[ j ] <= 0; ++j ) ;
	
	if ( io_p[ j - 1 ] == 0 ) {
		for ( i = j - 1; i > 1; --i ) io_p[ i ] = -1;

		io_p[ j ] = 0;
		io_p[ 1 ] = 1;
		
		*out_x = 0;
//		*out_y = j - 1;
		*out_z = 0;
	} else {
		if ( j > 1 ) io_p[ j - 1 ] = 0;
		
		while ( io_p[ ++j ] > 0 ) ;
		
		i = k = j - 1;
		
		while ( io_p[ ++i ] == 0 ) io_p[ i ] = -1;

		if ( io_p[ i ] == -1 ) {
			t = io_p[ k ];
			
			io_p[ i ] = t;
			io_p[ k ] = -1;
			
			*out_x = i - 1;
//			*out_y = k - 1;
			*out_z = t - 1;
		} else {
			if ( i == io_p[ 0 ] ) return 1;
			
			t = io_p[ i ];
			
			io_p[ i ] = 0;
			io_p[ j ] = t;
			
			*out_x = j - 1;
//			*out_y = i - 1;
			*out_z = t - 1;
		}
	}
	
	return 0;
}


/*
bool driver( int n, int m ) {
	// test data for n == 3, m == 2

	int							a[ 3 ];
	int							b[ 3 ];
	int							c[ 2 ];
	int							p[ 5 ];
	int							done, i, m, n, q, r, x, y, z;
	
	for ( i = n; i; --i ) a[ i - 1 ] = i;		// a: { 1, 2, 3 }
	
	r = n - m;									// r == 1

	for ( i = r; i; --i ) p[ i ] = 0;			// p: { ?, 0 }
	for ( i = m; i; --i ) p[ r + i ] = i;		// p: { ?, 0, 1, 2 }
	
	p[ 0 ] = n + 1;								// p: { 4, 0, 1, 2 }
	
	p[ n + 1 ] = -2;							// p: { 4, 0, 1, 2, -2 }
	
	done = false;
	
	if ( m == 0 ) p[ 1 ] = 1;					// p: { 4, 0, 1, 2, -2 }
	
	// initialize c[1:m]
	
	for ( i = m; i; --i ) {
		c[ i - 1 ] = a[ r + i - 1 ];			// c: { 2, 3 }
	}
	
	// initialize b[1:n]
	
	for ( i = m; i; --i ) {
		b[ r + i - 1 ] = 1;						// b: { ?, 1, 1 }
	}
	
	for ( i = r; i; --i ) {
		b[ i - 1 ] = 0;							// b: { 0, 1, 1 }
	}
	
	// generate and output combinations and sequences
	
	q = 0;
	
	for ( ;; ) {
		++q;

		printf( "%d: ", q );					// >> 1:
												// >> 2:
		for ( i = m - 1; i >= 0; --i ) {
			printf( "%d ", c[ m - i - 1 ] );	// >> 1: 2 3
												// >> 2: 1 3
		}

		for ( i = n - 1; i >= 0; --i ) {
			printf( "%d ", b[ n - i - 1 ] );	// >> 1: 2 3 0 1 1
												// >> 2: 1 3 1 0 1
												// >> 3: 1 2 1 1 0
		}
		
		done = twiddle( &x, &y, &z, p );

		// p: { 4, 1, 0, 2, -2 }
		// x == 0, y == 1, z == 0
		
		// p: { 4, 1, 2, 0, -2 }
		// x == 1, y == 2, z == 1

		if ( done ) break;

		c[ z ] = a[ x ];						// c: { 1, 2 }
		b[ x ] = 1;								// b: { 1, 1, 1 }
		b[ y ] = 0;								// b: { 1, 1, 0 }
		
		printf( "\n" );
	}
}
*/
