/**----------------------------------------------------------------------------------------*\

	! BALANCE SOFTWARE CONFIDENTIAL !
	
	Copyright (c) 2003 Balance Software Corporation
	All Rights Reserved.
	
	NOTICE:
	
		All information contained herein is, and remains the property of,
		Balance Software Corporation and its suppliers, if any.  The
		intellectual and technical concepts contained herein are proprietary
		to Balance Software Corporation and its suppliers and may be covered
		by U.S. and foreign patents or patents in process, and are protected
		by trade secret and copyright law.  Dissemination of this information,
		reproduction, or use of this material, whether in whole or in part,
		is strictly forbidden unless prior permission is obtained in writing
		from a duly authorized officer of Balance Software Corporation.
		

	File:				queue.h

	Author:				Brian Doyle
	Created:			September 16, 2003
	Last Modified:		May 15, 2007

	Description:

	Linked lists on steroids.
	
\**---------------------------------------------------------------------------------------*/
#if _WIN32
	#include "precompiled.h"
#endif


#ifndef __queue_h__
#define __queue_h__


#if ! _WIN32
	#include <pthread.h>
#endif

#include <time.h>

#include "synchronizer.h"


namespace balance {


enum queue_insert_position {
	k_at_queue_head,										// element is inserted at queue head
	k_at_queue_tail,										// element is inserted at queue tail

	k_before_element,										// element is inserted immediately before first instance of passed in reference element (or at tail if reference element not found)
	k_after_element,										// element is inserted immediately after last instance of passed in reference element (or at tail if reference element not found)

	k_with_ascending_compare_before_equal_elements,			// element is inserted in list in ascending order but before elements which compare equal
	k_with_decending_compare_before_equal_elements,			// element is inserted in list in decending order but before elements which compare equal
	
	k_with_ascending_compare_after_equal_elements,			// element is inserted in list in ascending order but after elements which compare equal
	k_with_decending_compare_after_equal_elements,			// element is inserted in list in decending order but after elements which compare equal

	// unique variants of the above

	k_at_queue_head_unique,									// element is inserted at queue head iff element is not already present in list
	k_at_queue_tail_unique,									// element is inserted at queue tail iff element is not already present in list

	k_before_element_unique,								// element is inserted immediately before first instance of passed in reference element iff element is not already in list
	k_after_element_unique,									// element is inserted immediately after last instance of passed in reference element iff element is not already in list

	k_with_ascending_compare_unique,						// element is inserted in list in ascending order iff element is not already in list
	k_with_decending_compare_unique							// element is inserted in list in decending order iff element is not already in list
};


#define k_queue_first_entry		reinterpret_cast<t *>(-1)


#pragma mark queue
template<typename t> struct queue {

	struct queue_entry {
		queue_entry( t *in_data ) : m_data( in_data ) { }
	
		t					   *m_data;
		queue_entry			   *m_next;
	};

	queue();
	queue( const queue<t> &in_queue );
	virtual ~queue();
	
	queue<t> &operator=( const queue<t> &in_rhs );

	// returns the number of entries currently in the queue
	virtual __u32	count();

	// removes all the elements from the queue
	virtual void	empty() { while ( remove() ) { } }
	
	// if in_position is k_before_element or k_after_element, then caller must pass in a t* that is already on the queue via in_reference_element_or_proc,
	// and in_data will be added either before or after the reference element, respectively.
	//
	// if in_position is k_with_compare_before or k_with_compare_after, then in_data will be inserted in the list
	virtual void	insert( t *in_data, queue_insert_position in_position = k_at_queue_tail, void *in_reference_element_or_comparison_proc = nil );

	// provides the ability to scan through the list without removing any entries
	virtual t	   *peek( t* in_preceding_element_match = nil );
	
	// removes the entry indicated by in_data from the queue
	virtual t	   *remove( t* in_data = k_queue_first_entry );

	queue_entry				   *m_head;

};


#pragma mark atomic_queue
template<typename t, typename u = balance::mutex> struct atomic_queue : public queue<t> {
 
	atomic_queue();
	atomic_queue( const atomic_queue<t> &in_aq );
	virtual ~atomic_queue();
	
	atomic_queue<t, u> &operator=( const atomic_queue<t, u> &in_rhs );

	// if either insert or remove throws, the queue is guaranteed to be unchanged
	virtual t	   *remove( t *in_data = k_queue_first_entry, bool in_dont_lock = false );
	virtual void	insert( t *in_data, queue_insert_position in_position = k_at_queue_tail, bool in_dont_lock = false, void *in_reference_element_or_comparison_proc = nil );
	
	void			lock() { m_lock.lock(); }
	void			unlock() { m_lock.unlock(); }
	
	// if you intend to use peek() or count() with this struct be sure to lock()

protected:

	u							m_lock;
	
};


#pragma mark atomic_cond_wait_queue
template<typename t, typename u = balance::mutex, typename v = balance::condition> struct atomic_cond_wait_queue : public atomic_queue<t, u> {

	atomic_cond_wait_queue();
	virtual ~atomic_cond_wait_queue();

	// insert() atomically inserts an entry into the queue at the specified in_position.
	// in the event that there are thread(s) blocked in remove() waiting for insertion,
	// this method will unblock at most one thread.  if no threads are blocked, insert()
	// will call the in_create_thread_callback method if one is provided.  presumably the
	// caller will either make a new thread to handle the insertion or assume that an
	// existing thread will pick up the next queue entry after it finishes processing the
	// current task.  if insert() throws an exception, the queue state is guaranteed to be
	// unchanged.
	//
	// note that specifying in_dont_lock is only appropriate if the lock has been
	// set prior to insert() (i.e. make sure the list operates atomically).
	virtual void	insert( v_proc_pv in_create_thread_callback, void *in_create_thread_context, t *in_data, queue_insert_position in_position = k_at_queue_tail, bool in_dont_lock = false, void *in_reference_element_or_comparison_proc = nil );

	// reverses the effect of wake_all(): remove() will now block if the queue is empty
	virtual void	reenable_queue();

	// remove() atomically removes the first entry from the queue.  in the event that the queue
	// is empty, remove() will block the calling thread until either an entry has been
	// acquired for removal, or until the in_wait_timeout_seconds wait timeout expires.
	// setting in_wait_timeout_seconds to 0 signifies the special condition that remove()
	// should never timeout.  in the event that an external abort occurs (either because an
	// error occurred, a timeout expired, or the user cancelled all waits, remove() will
	// return nil to the caller and locked state of the queue will be equal to
	// in_keep_queue_locked_on_abort.  if in_dont_lock is true, the routine assumes that
	// the queue is locked on entry and does not unlock the queue on any form of exit.
	// If you need a non-blocking atomic remove(), use the base struct's remove().
	// If you need to search for a specific element, use the base struct's remove().
	// If remove() throws an exception, the queue state is guaranteed to be unchanged.
	//
	// NOTE:  in_wait_timeout_seconds will be ignored and the process will block
	// indefinitely (or until an insertion occurs) if using semaphores instead
	// of condition variables.
	virtual t	   *remove( __u32 in_wait_timeout_seconds, bool in_keep_queue_locked_on_abort = false, bool in_dont_lock = false );
	
	// returns the number of threads blocked in remove()
	virtual __s32	wait_count();

	// Wakes all threads blocked in remove() and causes them to act as if they timed out.
	// All further remove() calls will return nil until the queue is reenabled via
	// reenable_queue().
	virtual void	wake_all();

protected:

	__s32						m_blocked_threads;
	bool						m_cancelled;
	v							m_condition;

};


#pragma mark priority_queue
template<typename t> struct priority_queue {

	struct priority_queue_entry {
		priority_queue_entry( t *in_data, __s32 in_priority ) : m_data( in_data ), m_priority( in_priority ) { }
	
		t					   *m_data;
		priority_queue_entry   *m_next;
		__s32					m_priority;
	};

	priority_queue();
	priority_queue( const priority_queue<t> &in_queue );
	virtual ~priority_queue();

	priority_queue<t> &operator=( const priority_queue<t> &in_rhs );

	virtual __u32	count();
	virtual void	empty() { while ( remove() ) { } }

	// provides the ability to scan through the list without removing any entries
	virtual t	   *peek( t* in_preceding_element_match = nil );
	virtual __s32	priority( t *in_data = nil );

	// removes in_data from queue and reinserts it after in_preceding_element_match, or
	// at the head of the list if no match is found.  if a preceding element
	// is matched, the priority of the object being inserted will be set to that of
	// the matched element.  if there is no match and the object's current priority
	// is greater than or equal to the priority of the first object on the list the
	// priority will not be changed.  otherwise the priority will be raised to match
	// that of the first element on the list.
	virtual void	reorder( t *in_data, t* in_preceding_element_match = nil );

	virtual void	insert( t *in_data, __s32 in_priority = 0, queue_insert_position in_position = k_at_queue_tail, void *in_reference_element_or_comparison_proc = nil );
	virtual t	   *remove( t* in_data = k_queue_first_entry, __s32 *out_priority = nil );
	
	priority_queue_entry	   *m_head;

};


#pragma mark atomic_priority_queue
template<typename t, typename u = balance::mutex> struct atomic_priority_queue : public priority_queue<t> {

	atomic_priority_queue();
	atomic_priority_queue( const atomic_priority_queue<t> &in_aq );
	virtual ~atomic_priority_queue();

	atomic_priority_queue<t, u> &operator=( const atomic_priority_queue<t, u> &in_rhs );

	virtual void	insert( t *in_data, __s32 in_priority = 0, queue_insert_position in_position = k_at_queue_tail, bool in_dont_lock = false, void *in_reference_element_or_comparison_proc = nil );
	virtual t	   *remove( t *in_data = k_queue_first_entry, bool in_dont_lock = false, __s32 *out_priority = nil );
	
	void			lock() { m_lock.lock(); }
	void			unlock() { m_lock.unlock(); }

	// if you intend to use peek() or count() with this struct, be sure to
	// lock appropriately.

protected:

	u							m_lock;

};


#pragma mark atomic_priority_cond_wait_queue
template<typename t, typename u = balance::mutex, typename v = balance::condition> struct atomic_priority_cond_wait_queue : public atomic_priority_queue<t, u> {

	atomic_priority_cond_wait_queue();
	virtual ~atomic_priority_cond_wait_queue();

	virtual void	insert( v_proc_pv in_create_thread_callback, void *in_create_thread_context, t *in_data, __s32 in_priority = 0, queue_insert_position in_position = k_at_queue_tail, bool in_dont_lock = false, void *in_reference_element_or_comparison_proc = nil );
	virtual void	reenable_queue();
	virtual t	   *remove( __u32 in_wait_timeout_seconds, bool in_keep_queue_locked_on_abort = false, bool in_dont_lock = false, __s32 *out_priority = nil );
	virtual __s32	wait_count();
	virtual void	wake_all();

protected:

	__s32						m_blocked_threads;
	bool						m_cancelled;
	v							m_condition;

};


// utility macros


// peek_criteria is used to find an element on in_queue that matches the given criteria
#define peek_criteria( in_queue, io_p, in_criteria ) do {														\
	for ( (io_p) = (in_queue).peek(); (io_p) && ! ( in_criteria ); (io_p) = (in_queue).peek( (io_p) ) ) ;		\
} while ( 0 )


#define atomic_peek( in_queue )		atomic_peek_criteria( in_queue, 0 )

// same functionality as peek_criteria but for atomic queues
#define atomic_peek_criteria( in_queue, io_p, in_criteria ) do {												\
	(in_queue).lock();																							\
																												\
	try {																										\
		for ( (io_p) = (in_queue).peek(); (io_p) && ! ( in_criteria ); (io_p) = (in_queue).peek( (io_p) ) ) ;	\
	} catch ( ... ) {																							\
		try { (in_queue).unlock(); } catch ( ... ) { }															\
		throw;																									\
	}																											\
																												\
	(in_queue).unlock();																						\
} while ( 0 )


#define atomic_queue_for_each_start( in_queue, io_p, in_criteria ) do {											\
	(in_queue).lock();																							\
																												\
	try {																										\
		for ( (io_p) = (in_queue).peek(); (io_p); (io_p) = (in_queue).peek( (io_p) ) ) {						\
			if ( in_criteria ) {																				\
				do { } while ( 0 )

#define atomic_queue_for_each_end( in_queue )																	\
			}																									\
		}																										\
	} catch ( ... ) {																							\
		try { (in_queue).unlock(); } catch ( ... ) { }															\
		throw;																									\
	}																											\
																												\
	(in_queue).unlock();																						\
} while ( 0 )


// removes all elements from in_queue, a queue that contains elements of in_type
#define queue_delete_all( in_queue, in_type ) do {																\
	in_type		   *p;																							\
																												\
	while ( ( p = (in_queue).remove() ) ) QUEUE_DEALLOCATOR( p );																\
} while ( 0 )


#include "queue.cpp"


} // namespace balance



#endif // __queue_h__
