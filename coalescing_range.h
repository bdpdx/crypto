#ifndef __coalescing_range_hpp__
#define __coalescing_range_hpp__



#include "useful_macros.h"


typedef void (*cr_iterator)( void *in_context, __u32 in_low, __u32 in_high );


class coalescing_range {

public:

	coalescing_range();
	coalescing_range( const coalescing_range &in_cr );
   ~coalescing_range();

	coalescing_range &operator=( const coalescing_range &in_rhs );

	void add( __u32 in_bound1, __u32 in_bound2 );
	void iterate( cr_iterator in_interator, void *in_context );
	void unused( __u32 &out_low, __u32 &out_high, __u32 in_max_units = 0, __u32 in_low_min = 0 );

protected:

	struct range {
		__u32				_high;
		__u32				_low;
		range			   *_next;
		range			   *_prev;
	};
	
	range				   *_range;
};



#endif // __coalescing_range_hpp__
