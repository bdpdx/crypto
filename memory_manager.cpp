#include "memory_manager.h"
#include "synchronizer.h"


void balance::memory_manager::init( class synchronizer *in_synchronizer ) {
	_lock = in_synchronizer;
	_pool = nil;
	
	debug_statement( _allocations = 0 );
	debug_statement( _highwater = 0 );
	debug_statement( _releases = 0 );
}


void balance::memory_manager::set_pool( void *in_pool, __u32 in_size ) {
	__u32					i;

	in_size = in_size + sizeof(__u32) - 1 & ~3;

	if ( ( _pool = reinterpret_cast<segment *>(in_pool) ) ) {
		for ( i = 0; i < in_size; ++i ) ((__u8 *) in_pool)[ i ] = 0;

		_pool->_size = in_size;
	}
}


void *balance::memory_manager::allocate( __u32 in_size, bool in_lock ) {
	__u32			i, n;
	segment		   *p = nil, *q, *r;
	
	in_size = in_size + sizeof(segment) + sizeof(__u32) - 1 & ~3;

	if ( in_lock && _lock ) _lock->lock();

	for ( q = _pool, r = nil; q; r = q, q = q->_next ) {
		if ( q->_used || q->_size < in_size ) continue;
		p = q;
		break;
	}
	
	// if the segment can be broken into another segment, do so
	if ( p ) {
		p->_prev = r;
		p->_used = true;

		if ( p->_size >= in_size + sizeof(segment) ) {
			q = p->_next;
			
			p->_next = reinterpret_cast<segment *>(reinterpret_cast<__u8 *>(p) + in_size);
			p->_next->_next = q;
			p->_next->_prev = p;
			p->_next->_size = p->_size - in_size;
			p->_next->_used = false;
			p->_size = in_size;
			
			if ( q ) q->_prev = p->_next;
		}

		debug_statement( p->_when = _allocations++ );
		debug_statement( _highwater = max( _highwater, pool_current( false ) ) );

		p = reinterpret_cast<segment *>(reinterpret_cast<__u8 *>(p) + sizeof(segment));
	}

	if ( in_lock && _lock ) _lock->unlock();

	if ( p ) {
		n = in_size - sizeof(segment) >> 2;

		for ( i = 0; i < n; ++i ) reinterpret_cast<__u32 *>(p)[ i ] = 0;
	}
	
	return p;
}


void *balance::memory_manager::reallocate( void *in_ptr, __u32 in_size ) {
	__u8		   *dst, *src;
	__u32			i, n, size;
	segment		   *p, *q;

	if ( ! in_ptr ) return allocate( in_size );

	in_size = in_size + sizeof(segment) + sizeof(__u32) - 1 & ~3;

	p = reinterpret_cast<segment *>(reinterpret_cast<__u8 *>(in_ptr) - sizeof(segment));
	
	if ( ( size = p->_size ) == in_size ) return p;

	if ( _lock ) _lock->lock();

	if ( ( q = p->_next ) && ! q->_used ) {
		p->_size += q->_size;
		p->_next = q->_next;
		
		if ( p->_next ) p->_next->_prev = p;
	}

	if ( p->_size < in_size ) {
		src = reinterpret_cast<__u8 *>(p) + sizeof(segment);
		n = p->_size - sizeof(segment);

		if ( ( q = p->_prev ) && ! q->_used && q->_size + p->_size >= in_size ) {
			// we can combine this one and the previous one, but we need to move data
			dst = reinterpret_cast<__u8 *>(q) + sizeof(segment);
			
			q->_next = p->_next;
			q->_size += p->_size;
			q->_used = true;
			
			if ( q->_next ) q->_next->_prev = q;
			
			p = q;
		} else {
			// can't combine, need to allocate a new block
			dst = (__u8 *) allocate( in_size - sizeof(segment), false );
			p = nil;
		}
		
		if ( dst ) memmove( dst, src, n );
	} else {
		dst = reinterpret_cast<__u8 *>(p) + sizeof(segment);
	}

	if ( p && p->_size >= in_size + sizeof(segment) ) {
		q = p->_next;
		
		p->_next = reinterpret_cast<segment *>(reinterpret_cast<__u8 *>(p) + in_size);
		p->_next->_next = q;
		p->_next->_prev = p;
		p->_next->_size = p->_size - in_size;
		p->_next->_used = false;
		p->_size = in_size;
		
		if ( q ) q->_prev = p->_next;
	}

	if ( dst ) debug_statement( _highwater = max( _highwater, pool_current( false ) ) );

	if ( _lock ) _lock->unlock();
	
	if ( ( src = dst ) ) {
		size -= sizeof(segment);
	
		src += size;
		n = in_size - sizeof(segment) - size;
		
		for ( ; n & 3; --n ) *src++ = 0;
		for ( i = 0, n >>= 2; i < n; ++i ) reinterpret_cast<__u32 *>(src)[ i ] = 0;
	}
	
	return dst;
}


void balance::memory_manager::release( void *in_ptr ) {
	segment		   *p, *q;

	if ( ! in_ptr ) return;

	if ( _lock ) _lock->lock();

	debug_statement( ++_releases );
	
	p = reinterpret_cast<segment *>(__u32(in_ptr) - sizeof(segment));

	p->_used = false;

	if ( ( q = p->_next ) && ! q->_used ) {
		p->_size += q->_size;
		p->_next = q->_next;

		if ( p->_next ) p->_next->_prev = p;
	}
	
	if ( ( q = p->_prev ) && ! q->_used ) {
		q->_size += p->_size;
		q->_next = p->_next;
		
		if ( q->_next ) q->_next->_prev = q;
	}

	if ( _lock ) _lock->unlock();
}


__u32 balance::memory_manager::pool_current( bool in_lock ) {
	segment		   *p;
	__u32			result = 0;

	if ( in_lock && _lock ) _lock->lock();
	
	for ( p = _pool; p; p = p->_next ) if ( p->_used ) result += p->_size - sizeof(segment);

	if ( in_lock && _lock ) _lock->unlock();
	
	return result;
}


#if DEBUG
__u32 balance::memory_manager::pool_allocations() {
	return _allocations;
}


__u32 balance::memory_manager::pool_first_used() {
	segment		   *p;
	__u32			result;
	
	if ( _lock ) _lock->lock();

	for ( p = _pool; p && ! p->_used; p = p->_next ) ;

	result = p ? p->_when : 0xffffffff;

	if ( _lock ) _lock->unlock();

	return result;
}


__u32 balance::memory_manager::pool_highwater() {
	return _highwater;
}


__u32 balance::memory_manager::pool_releases() {
	return _releases;
}
#endif


__u32 balance::memory_manager::pool_unused() {
	segment		   *p;
	__u32			result = 0;

	if ( _lock ) _lock->lock();

	for ( p = _pool; p; p = p->_next ) if ( ! p->_used ) result += p->_size - sizeof(segment);

	if ( _lock ) _lock->unlock();

	return result;
}


#pragma mark -


void operator delete( void *in_ptr, balance::memory_manager &in_manager ) {
	in_manager.release( in_ptr );
}


void operator delete[]( void *in_ptr, balance::memory_manager &in_manager ) {
	in_manager.release( in_ptr );
}


void *operator new( size_t in_size, balance::memory_manager &in_manager ) {
	return in_manager.allocate( in_size );
}


void *operator new[]( size_t in_size, balance::memory_manager &in_manager ) {
	return in_manager.allocate( in_size );
}
