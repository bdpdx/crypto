#include "coalescing_range.hpp"


coalescing_range::coalescing_range() {
	_range = nil;
}


coalescing_range::coalescing_range( const coalescing_range &in_cr ) {
	_range = nil;
	*this = in_cr;
}


coalescing_range::~coalescing_range() {
	range				   *p, *q;
	
	for ( p = _range; p; p = q ) { q = p->_next; delete p; }
}


coalescing_range &coalescing_range::operator=( const coalescing_range &in_rhs ) {
	range				  **p, *q, *r;
	
	for ( ; _range; _range = q ) { q = _range->_next; delete _range; }
	
	for ( p = &_range, q = in_rhs._range; q; p = &r->_next ) {
		r = new range;
		
		r->_high = q->_high;
		r->_low = q->_low;
		r->_prev = *p;
		r->_next = nil;
		
		*p = r;
	}
	
	return *this;
}


void coalescing_range::add( __u32 in_bound1, __u32 in_bound2 ) {
	__u32					high, low;
	range				  **p, *q, *r;

	low = min( in_bound1, in_bound2 );
	high = max( in_bound1, in_bound2 );
	
	for ( p = &_range, q = nil; *p && low > (*p)->_high; q = *p, p = &(*p)->_next ) ;
	
	r = new range;
	
	r->_low = low;
	r->_high = high;

	if ( ( r->_next = *p ) ) r->_next->_prev = r;
	if ( ( r->_prev = q ) ) q->_next = r;
	*p = r;
	
	while ( ( q = r->_next ) && high >= q->_low - 1 ) {
		r->_high = max( high, q->_high );
		if ( ( r->_next = q->_next ) ) r->_next->_prev = r;
		delete q;
	}
	
	if ( ( q = r->_prev ) && low <= q->_high + 1 ) {
		q->_high = r->_high;
		q->_next = r->_next;
 		delete r;
	}
}


void coalescing_range::iterate( cr_iterator in_iterator, void *in_context ) {
	range				   *p;
	
	for ( p = _range; p; p = p->_next ) in_iterator( in_context, p->_low, p->_high );
}


void coalescing_range::unused( __u32 &out_low, __u32 &out_high, __u32 in_max_units, __u32 in_low_min ) {
	__u32					high, low;
	range				   *p;

	for ( low = in_low_min, p = _range; p; p = p->_next ) {
		if ( low < p->_low ) break;
		if ( low <= ( high = p->_high ) ) low = high + 1;
	}

	if ( in_max_units ) {
		if ( p ) high = min( low + in_max_units - 1, p->_low - 1 );
		else high = low + in_max_units - 1;
	} else {
		if ( p ) high = p->_low - 1;
		else high = UINT_MAX;
	}

	out_high = high;
	out_low = low;
}
