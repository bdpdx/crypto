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
		

	File:				queue.cpp

	Author:				Brian Doyle
	Created:			September 16, 2003
	Last Modified:		May 15, 2007

	Description:

	Linked lists on steroids.
	
\**---------------------------------------------------------------------------------------*/
#ifdef __queue_h__


#ifndef QUEUE_ALLOCATOR
	#define QUEUE_ALLOCATOR( _in_arg )		new _in_arg
#endif


#ifndef QUEUE_DEALLOCATOR
	#define QUEUE_DEALLOCATOR( _in_arg )	delete _in_arg
#endif


#pragma mark -


template<typename t> queue<t>::queue() {
	m_head = nil;
}


template<typename t> queue<t>::queue( const queue<t> &in_queue ) {
	m_head = nil;
	*this = in_queue;
}


template<typename t> queue<t>::~queue() {
	empty();
}


template<typename t> __u32 queue<t>::count() {
	__s32					i;
	queue_entry			   *p;

	for ( i = 0, p = m_head; p; ++i, p = p->m_next ) { }
	
	return i;
}


template<typename t> void queue<t>::insert( t *in_data, queue_insert_position in_position, void *in_reference_element_or_comparison_proc ) {
	if ( ! in_data ) return;

	typedef typename queue<t>::queue_entry queue_entry_t;

	queue_entry_t		  **p, **q, *entry;

	t					   *ref = (t *) in_reference_element_or_comparison_proc;
	comparison_proc			cmp = (comparison_proc) in_reference_element_or_comparison_proc;

	if ( in_position >= k_at_queue_head_unique ) {
		for ( p = &m_head; *p && (*p)->m_data != in_data; p = &(*p)->m_next ) { }

		if ( *p ) return;

		in_position = static_cast<queue_insert_position>( in_position - k_at_queue_head_unique );
	}

	entry = QUEUE_ALLOCATOR( queue_entry_t( in_data ) );

	try {
		switch ( in_position ) {
			case k_at_queue_head: {
				p = &m_head;
			} break;
			
			case k_at_queue_tail: {
				for ( p = &m_head; *p; p = &(*p)->m_next ) { }
			} break;

			case k_before_element: {
				for ( p = &m_head; *p && (*p)->m_data != ref; p = &(*p)->m_next ) { }
			} break;
			
			case k_after_element: {
				for ( q = nil, p = &m_head; *p; p = &(*p)->m_next ) {
					if ( (*p)->m_data == ref ) {
						q = &(*p)->m_next;
					}
				}
				
				if ( q ) p = q;
			} break;

			case k_with_ascending_compare_before_equal_elements: {
				for ( p = &m_head; *p && cmp( in_data, (*p)->m_data ) > 0; p = &(*p)->m_next ) { }
			} break;
			
			case k_with_decending_compare_before_equal_elements: {
				for ( p = &m_head; *p && cmp( in_data, (*p)->m_data ) < 0; p = &(*p)->m_next ) { }
			} break;
			
			case k_with_ascending_compare_after_equal_elements: {
				for ( p = &m_head; *p && cmp( in_data, (*p)->m_data ) >= 0; p = &(*p)->m_next ) { }
			} break;
			
			case k_with_decending_compare_after_equal_elements: {
				for ( p = &m_head; *p && cmp( in_data, (*p)->m_data ) <= 0; p = &(*p)->m_next ) { }
			} break;

			default:						_throw( err_bad_parameter );
		}

		entry->m_next = *p;
		*p = entry;
	} catch ( ... ) {
		QUEUE_DEALLOCATOR( entry );
		
		throw;
	}
}


template<typename t> queue<t> &queue<t>::operator=( const queue<t> &in_rhs ) {
	queue_entry			   *p;

	empty();
	
	for ( p = in_rhs.m_head; p; p = p->m_next ) insert( p->m_data );

	return *this;
}


template<typename t> t *queue<t>::peek( t* in_preceding_element_match ) {
	queue_entry 			   *p;
	
	if ( ! in_preceding_element_match ) {
		return m_head ? m_head->m_data : nil;
	} else {
		for ( p = m_head; p && p->m_data != in_preceding_element_match; p = p->m_next ) { }

		return p && p->m_next ? p->m_next->m_data : nil;
	}
}


template<typename t> t *queue<t>::remove( t* in_data ) {
	if ( ! in_data ) return nil;

	typedef typename queue<t>::queue_entry queue_entry_t;

	t					   *data;
	queue_entry_t		   *p = m_head, **q = &m_head;

	if ( in_data != k_queue_first_entry ) {
		for ( ; p && in_data != p->m_data; q = &p->m_next, p = p->m_next ) { }
	}

	if ( p ) {
		*q = p->m_next;
		data = p->m_data;
		QUEUE_DEALLOCATOR( p );
	} else {
		data = nil;
	}

	return data;
}


#pragma mark -


template<typename t, typename u> atomic_queue<t, u>::atomic_queue() { }


template<typename t, typename u> atomic_queue<t, u>::atomic_queue( const atomic_queue<t> &in_aq ) {
	*this = in_aq;
}


template<typename t, typename u> atomic_queue<t, u>::~atomic_queue() { }


template<typename t, typename u> atomic_queue<t, u> &atomic_queue<t, u>::operator=( const atomic_queue<t, u> &in_rhs ) {
	lock();
	in_rhs.lock();

	*reinterpret_cast<queue<t> *>(this) = in_rhs;

	in_rhs.unlock();
	unlock();
	
	return *this;
}


template<typename t, typename u> void atomic_queue<t, u>::insert( t *in_data, queue_insert_position in_position, bool in_dont_lock, void *in_reference_element_or_comparison_proc ) {
	if ( ! in_dont_lock ) lock();

	_try { queue<t>::insert( in_data, in_position, in_reference_element_or_comparison_proc ); } _catch

	_no_throw( if ( ! in_dont_lock ) unlock() );

	_return;
}


template<typename t, typename u> t *atomic_queue<t, u>::remove( t *in_data, bool in_dont_lock ) {
	t					   *result;

	if ( ! in_dont_lock ) lock();

	_try { result = queue<t>::remove( in_data ); } _catch

	_no_throw( if ( ! in_dont_lock ) unlock() );
	
	_return result;	
}


#pragma mark -


template<typename t, typename u, typename v> atomic_cond_wait_queue<t, u, v>::atomic_cond_wait_queue() : m_condition( this->m_lock ) {
	m_blocked_threads = 0;
	m_cancelled = false;
}


template<typename t, typename u, typename v> atomic_cond_wait_queue<t, u, v>::~atomic_cond_wait_queue() { }


template<typename t, typename u, typename v> void atomic_cond_wait_queue<t, u, v>::insert( v_proc_pv in_create_thread_callback, void *in_create_thread_context, t *in_data, queue_insert_position in_position, bool in_dont_lock, void *in_reference_element_or_comparison_proc ) {
	if ( ! in_dont_lock ) this->lock();

	_try {
		queue<t>::insert( in_data, in_position, in_reference_element_or_comparison_proc );

		if ( m_blocked_threads ) {
			m_condition.wake();
		} else if ( in_create_thread_callback ) {
			( *in_create_thread_callback )( in_create_thread_context );
		}
	} _catch

	_no_throw( if ( ! in_dont_lock ) this->unlock() );
	
	_return;
}


template<typename t, typename u, typename v> void atomic_cond_wait_queue<t, u, v>::reenable_queue() {
	this->lock();
	m_cancelled = false;
	this->unlock();
}


template<typename t, typename u, typename v> t *atomic_cond_wait_queue<t, u, v>::remove( __u32 in_wait_timeout_seconds, bool in_keep_queue_locked_on_abort, bool in_dont_lock ) {
	t					   *result = nil;

	if ( ! in_dont_lock ) this->lock();

	if ( ! m_cancelled ) {
		while ( ! this->m_head ) {
			++m_blocked_threads;
			_try { m_condition.wait( in_wait_timeout_seconds ); } _catch
			--m_blocked_threads;

			if ( m_cancelled || _err == ETIMEDOUT ) {
				if ( ! in_dont_lock && ! in_keep_queue_locked_on_abort ) _no_throw( this->unlock() );
				return nil;
			} else if ( _err ) {
				if ( ! in_dont_lock && ! in_keep_queue_locked_on_abort ) _no_throw( this->unlock() );
				_throw_;
			}
		}

		result = queue<t>::remove();
	}

	_no_throw( this->unlock(); );
	
	return result;
}


template<typename t, typename u, typename v> __s32 atomic_cond_wait_queue<t, u, v>::wait_count() {
	__s32					result;
	
	this->lock();
	result = m_blocked_threads;
	this->unlock();

	return result;
}


template<typename t, typename u, typename v> void atomic_cond_wait_queue<t, u, v>::wake_all() {
	this->lock();

	m_cancelled = true;
	m_condition.wake_all();

	this->unlock();
}
 

#pragma mark -


template<typename t> priority_queue<t>::priority_queue() {
	m_head = nil;
}


template<typename t> priority_queue<t>::priority_queue( const priority_queue<t> &in_queue ) {
	m_head = nil;
	*this = in_queue;
}


template<typename t> priority_queue<t>::~priority_queue() {
	this->empty();
}


template<typename t> priority_queue<t> &priority_queue<t>::operator=( const priority_queue<t> &in_rhs ) {
	priority_queue_entry	   *p;

	empty();
	
	for ( p = in_rhs.m_head; p; p = p->m_next ) insert( p->m_data, p->m_priority );

	return *this;
}


template<typename t> __u32 priority_queue<t>::count() {
	__s32						i;
	priority_queue_entry	   *p;
	
	for ( i = 0, p = m_head; p; ++i, p = p->m_next ) { }
	
	return i;
}


template<typename t> void priority_queue<t>::insert( t *in_data, __s32 in_priority, queue_insert_position in_position, void *in_reference_element_or_comparison_proc ) {
	if ( ! in_data ) return;

	typedef typename priority_queue<t>::priority_queue_entry priority_queue_entry_t;

	priority_queue_entry_t	  **p, **q, *entry;

	t						   *ref = (t *) in_reference_element_or_comparison_proc;																					\
	comparison_proc				cmp = (comparison_proc) in_reference_element_or_comparison_proc;																		\

	if ( in_position >= k_at_queue_head_unique ) {
		for ( p = &m_head; *p && (*p)->m_priority > in_priority; p = &(*p)->m_next ) { }
		for ( ; *p && (*p)->m_priority == in_priority && (*p)->m_data != in_data; p = &(*p)->m_next ) { }

		if ( *p && (*p)->m_priority == in_priority ) return;

		in_position = static_cast<queue_insert_position>( in_position - k_at_queue_head_unique );
	}

	entry = QUEUE_ALLOCATOR( priority_queue_entry_t( in_data, in_priority ) );

	try {
		for ( p = &m_head; *p && (*p)->m_priority > in_priority; p = &(*p)->m_next ) { }

		switch ( in_position ) {
			case k_at_queue_head:			break;

			case k_at_queue_tail: {
				for ( ; *p && (*p)->m_priority == in_priority; p = &(*p)->m_next ) { }
			} break;

			case k_before_element: {
				for ( ; *p && (*p)->m_priority == in_priority && (*p)->m_data != ref; p = &(*p)->m_next ) { }
			} break;

			case k_after_element: {
				for ( q = nil; *p && (*p)->m_priority == in_priority; p = &(*p)->m_next ) {
					if ( (*p)->m_data == ref ) {
						q = &(*p)->m_next;
					}
				}

				if ( q ) p = q;
			} break;

			case k_with_ascending_compare_before_equal_elements: {
				for ( ; *p && (*p)->m_priority == in_priority && cmp( in_data, (*p)->m_data ) > 0; p = &(*p)->m_next ) { }
			} break;
			
			case k_with_decending_compare_before_equal_elements: {
				for ( ; *p && (*p)->m_priority == in_priority && cmp( in_data, (*p)->m_data ) < 0; p = &(*p)->m_next ) { }
			} break;

			case k_with_ascending_compare_after_equal_elements: {
				for ( ; *p && (*p)->m_priority == in_priority && cmp( in_data, (*p)->m_data ) >= 0; p = &(*p)->m_next ) { }
			} break;
			
			case k_with_decending_compare_after_equal_elements: {
				for ( ; *p && (*p)->m_priority == in_priority && cmp( in_data, (*p)->m_data ) <= 0; p = &(*p)->m_next ) { }
			} break;

			default:						_throw( err_bad_parameter );
		}																																								\
																																										\
		entry->m_next = *p;																																				\
		*p = entry;																																						\
	} catch ( ... ) {
		QUEUE_DEALLOCATOR( entry );
		
		throw;
	}
}


template<typename t> t *priority_queue<t>::peek( t* in_preceding_element_match ) {
	priority_queue_entry	   *p;
	
	if ( ! in_preceding_element_match ) {
		return m_head ? m_head->m_data : nil;
	} else {
		for ( p = m_head; p && p->m_data != in_preceding_element_match; p = p->m_next ) { }
		
		return p && p->m_next ? p->m_next->m_data : nil;
	}
}


template<typename t> __s32 priority_queue<t>::priority( t *in_data ) {
	priority_queue_entry	   *p;
	
	if ( in_data && ( p = m_head ) ) {
		do {
			if ( p->m_data == in_data ) return p->m_priority;
		} while ( ( p = p->m_next ) );
	}

	return 0;
}


template<typename t> t *priority_queue<t>::remove( t* in_data, __s32 *out_priority ) {
	if ( ! in_data ) return nil;

	typedef typename priority_queue<t>::priority_queue_entry priority_queue_entry_t;

	t						   *data;
	priority_queue_entry_t	   *p = m_head, **q = &m_head;

	if ( in_data != k_queue_first_entry ) {
		for ( ; p && in_data != p->m_data; q = &p->m_next, p = p->m_next ) { }
	}

	if ( p ) {
		*q = p->m_next;
		data = p->m_data;
		if ( out_priority ) *out_priority = p->m_priority;
		QUEUE_DEALLOCATOR( p );
	} else {
		data = nil;
		if ( out_priority ) *out_priority = 0;
	}

	return data;
}


template<typename t> void priority_queue<t>::reorder( t *in_data, t *in_preceding_element_match ) {
	priority_queue_entry  **p, *q;
	
	for ( p = &m_head; *p && (*p)->m_data != in_data; p = &(*p)->m_next ) { }
	
	if ( ! *p ) return;

	q = *p;
	*p = q->m_next;
	
	if ( in_preceding_element_match ) {
		for ( p = &m_head; *p && (*p)->m_data != in_preceding_element_match; p = &(*p)->m_next ) { }

		if ( *p ) {
			q->m_next = (*p)->m_next;
			q->m_priority = (*p)->m_priority;
			(*p)->m_next = q;
			
			return;
		}
	}

	if ( m_head && q->m_priority < m_head->m_priority ) {
		q->m_priority = m_head->m_priority;
	}

	q->m_next = m_head;
	m_head = q;
}


#pragma mark -


template<typename t, typename u> atomic_priority_queue<t, u>::atomic_priority_queue() { }


template<typename t, typename u> atomic_priority_queue<t, u>::atomic_priority_queue( const atomic_priority_queue<t> &in_aq ) {
	*this = in_aq;
}


template<typename t, typename u> atomic_priority_queue<t, u>::~atomic_priority_queue() { }


template<typename t, typename u> atomic_priority_queue<t, u> &atomic_priority_queue<t, u>::operator=( const atomic_priority_queue<t, u> &in_rhs ) {
	lock();
	in_rhs.lock();

	*reinterpret_cast<priority_queue<t> *>(this) = in_rhs;

	in_rhs.unlock();
	unlock();
	
	return *this;
}


template<typename t, typename u> void atomic_priority_queue<t, u>::insert( t *in_data, __s32 in_priority, queue_insert_position in_position, bool in_dont_lock, void *in_reference_element_or_comparison_proc ) {
	if ( ! in_dont_lock ) lock();

	_try { priority_queue<t>::insert( in_data, in_priority, in_position, in_reference_element_or_comparison_proc ); } _catch

	_no_throw( if ( ! in_dont_lock ) unlock() );

	_return;
}


template<typename t, typename u> t *atomic_priority_queue<t, u>::remove( t *in_data, bool in_dont_lock, __s32 *out_priority ) {
	t					   *result;

	if ( ! in_dont_lock ) lock();

	_try { result = priority_queue<t>::remove( in_data, out_priority ); } _catch

	_no_throw( if ( ! in_dont_lock ) lock() );
	
	_return result;
}


#pragma mark -


template<typename t, typename u, typename v> atomic_priority_cond_wait_queue<t, u, v>::atomic_priority_cond_wait_queue() : m_condition( this->m_lock ) {
	m_blocked_threads = 0;
	m_cancelled = false;
}


template<typename t, typename u, typename v> atomic_priority_cond_wait_queue<t, u, v>::~atomic_priority_cond_wait_queue() { }


template<typename t, typename u, typename v> void atomic_priority_cond_wait_queue<t, u, v>::insert( v_proc_pv in_create_thread_callback, void *in_create_thread_context, t *in_data, __s32 in_priority, queue_insert_position in_position, bool in_dont_lock, void *in_reference_element_or_comparison_proc ) {
	if ( ! in_dont_lock ) this->lock();

	_try {
		priority_queue<t>::insert( in_data, in_priority, in_position, in_reference_element_or_comparison_proc );

		if ( m_blocked_threads ) {
			m_condition.wake();
		} else if ( in_create_thread_callback ) {
			( *in_create_thread_callback )( in_create_thread_context );
		}
	} _catch

	_no_throw( if ( ! in_dont_lock ) this->unlock() );
	
	_return;
}


template<typename t, typename u, typename v> void atomic_priority_cond_wait_queue<t, u, v>::reenable_queue() {
	this->lock();
	m_cancelled = false;
	this->unlock();
}


template<typename t, typename u, typename v> t *atomic_priority_cond_wait_queue<t, u, v>::remove( __u32 in_wait_timeout_seconds, bool in_keep_queue_locked_on_abort, bool in_dont_lock, __s32 *out_priority ) {
	t					   *result = nil;

	if ( ! in_dont_lock ) this->lock();

	if ( ! m_cancelled ) {
		while ( ! this->m_head ) {
			++m_blocked_threads;
			_try { m_condition.wait( in_wait_timeout_seconds ); } _catch
			--m_blocked_threads;

			if ( m_cancelled || _err == ETIMEDOUT ) {
				if ( ! in_dont_lock && ! in_keep_queue_locked_on_abort ) _no_throw( this->unlock() );
				return nil;
			} else if ( _err ) {
				if ( ! in_dont_lock && ! in_keep_queue_locked_on_abort ) _no_throw( this->unlock() );
				_throw_;
			}
		}

		result = priority_queue<t>::remove( k_queue_first_entry, out_priority );
	}

	_no_throw( this->unlock(); );
	
	return result;
}


template<typename t, typename u, typename v> __s32 atomic_priority_cond_wait_queue<t, u, v>::wait_count() {
	__s32						result;
	
	this->lock();
	result = m_blocked_threads;
	this->unlock();
	
	return result;
}


template<typename t, typename u, typename v> void atomic_priority_cond_wait_queue<t, u, v>::wake_all() {
	this->lock();

	m_cancelled = true;
	m_condition.wake_all();

	this->unlock();
}


#endif // __queue_h__
