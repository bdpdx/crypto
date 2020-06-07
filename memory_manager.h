#ifndef __memory_manager_h__
#define __memory_manager_h__



#include <pthread.h>


namespace balance {


class memory_manager {

public:

	// construction via init() is required!
	memory_manager() { }
   ~memory_manager() { }

	void init( class synchronizer *in_synchronizer = nil );

	void set_pool( void *in_pool, __u32 in_size );		// init method, sets the backing store
	
	// behavior is similar to malloc(3), realloc(3), and free(3), except that
	// releasing a nil pointer is allowed and allocated memory is initialized to zero.

	// in_lock is used internally by reallocate().  always pass true!
	void *allocate( __u32 in_size, bool in_lock = true );
	void *reallocate( void *in_ptr, __u32 in_size );
	void release( void *in_ptr );

	// in_lock is used internally by pool_current().  always pass true!
	__u32 pool_current( bool in_lock = true );	// currently used memory minus segment headers (i.e. user requested)
	__u32 pool_unused();	// current free memory minus segment headers (i.e. user requestable)

	debug_declare( __u32 pool_allocations() );
	debug_declare( __u32 pool_first_used() );			// returns 'when' member of first used block in pool
	debug_declare( __u32 pool_highwater() );			// max amount of memory requested so far
	debug_declare( __u32 pool_releases() );

protected:

	struct segment {
		segment				   *_next;
		segment				   *_prev;
		__u32					_size;
		__u32					_used;
		debug_declare( __u32	_when );
	};

	class synchronizer		   *_lock;
	segment					   *_pool;

	debug_declare( __u32		_allocations );
	debug_declare( __u32		_highwater );
	debug_declare( __u32		_releases );

};


} // namespace balance


void operator delete( void *in_ptr, balance::memory_manager &in_manager );
void operator delete[]( void *in_ptr, balance::memory_manager &in_manager );
void *operator new( size_t in_size, balance::memory_manager &in_manager );
void *operator new( size_t in_size, balance::memory_manager &in_manager );



#endif // __memory_manager_h__
