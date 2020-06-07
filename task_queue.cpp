#include <new>
#include <sys/syslog.h>

#include "task_queue.hpp"


enum task_control_constants {
	k_default_idle_task_handler_timeout		=		600
};


task_queue::task_queue() {
	m_shutdown = false;
	m_max_handler_threads = k_unlimited;
	m_next_task_id = 0;
	m_idle_task_handler_timeout = k_default_idle_task_handler_timeout;
}


task_queue::~task_queue() {
	shutdown();
}


#pragma mark -


task_id_t task_queue::add_task( add_task_params &in_params ) {
	proc_enter( "task_queue::add_task()" );

	_assert( in_params.in_handler );

	notify_task											   *t;
	priority_queue<notify_task>::priority_queue_entry	   *pqe;
	task_id_t												task_id;
	task_observer_data									   *info = nil;	
	task_completion_notifier							   *notifier = nil;
	bool													found = false, found_on_scheduled = false, handle_immediately = false;
	
	t = new notify_task;

	proc_state( "new task is %#08x (%c%c%c%c)", t, ostype( in_params.in_task_type ) );
	
	t->data = in_params.in_data;
	t->data_size = in_params.in_data_size;
	t->handler = in_params.in_handler;
	t->handler_context = in_params.in_handler_context;
	t->task_type = in_params.in_task_type;

	try {
		notifier = new task_completion_notifier;

		notifier->aborted = false;
		notifier->completion = in_params.in_completion;
		notifier->context = in_params.in_completion_context;

		m_mutex_shutdown.lock();

		try {
			if ( m_shutdown ) _throw( err_task_queue_shutdown );

			m_task_queue.lock();

			try {
				// if we can insert on an in-progress task, search the scheduled queue:
				if ( in_params.in_flags & k_add_piggyback_scheduled ) {
					for ( pqe = m_scheduled_tasks.m_head; pqe; pqe = pqe->m_next ) {
						if ( pqe->m_data->task_type == in_params.in_task_type ) {
							if ( in_params.in_task_comparison_proc ) {
								found_on_scheduled = ! in_params.in_task_comparison_proc( pqe->m_data, t );
								if ( ! found_on_scheduled ) continue;
							} else {
								found_on_scheduled = true;
							}

							notifier->task_id = task_id = pqe->m_data->notify_list.peek()->task_id;
							
							pqe->m_data->notify_list.insert( notifier, in_params.in_priority );
							
							// if the task already present in the scheduled queue has
							// a lower priority than the task we want to insert, then
							// pop the existing task off the scheduled queue and reinsert
							// it with the new priority.
							if ( pqe->m_priority < in_params.in_priority ) {
								delete t;
								t = reinterpret_cast<atomic_priority_queue<notify_task> *>(&m_scheduled_tasks)->remove( pqe->m_data, true );
								reinterpret_cast<atomic_priority_queue<notify_task> *>(&m_scheduled_tasks)->insert( t, in_params.in_priority, k_at_queue_tail, true );
							}

							break;
						}
					}
				}

				if ( ! found_on_scheduled ) {
					if ( in_params.in_flags & k_add_piggyback ) {
						for ( pqe = m_task_queue.m_head; pqe; pqe = pqe->m_next ) {
							if ( pqe->m_data->task_type == in_params.in_task_type ) {
								if ( in_params.in_task_comparison_proc ) {
									found = ! in_params.in_task_comparison_proc( pqe->m_data, t );
									if ( ! found ) continue;
								} else {
									found = true;
								}

								notifier->task_id = task_id = pqe->m_data->notify_list.peek()->task_id;
								pqe->m_data->notify_list.insert( notifier, in_params.in_priority );
									
								if ( pqe->m_priority < in_params.in_priority ) {
									delete t;
									t = reinterpret_cast<atomic_priority_queue<notify_task> *>(&m_task_queue)->remove( pqe->m_data, true );
									pqe = nil;
								}

								break;
							}
						}
					} else {
						pqe = nil;
					}

					if ( ! found ) {
						notifier->task_id = task_id = ++m_next_task_id ? m_next_task_id : ++m_next_task_id;
						t->notify_list.insert( notifier, in_params.in_priority );
					}

					if ( pqe ) {
						delete t;
					} else if ( ! m_max_handler_threads ) {
						handle_immediately = true;
					} else {
						m_task_queue.insert( reinterpret_cast<v_proc_pv>(create_task_thread), this, t, in_params.in_priority, k_at_queue_tail, true );
					}

					if ( ! handle_immediately ) t = nil;
				}
			} catch ( ... ) {
				try { m_task_queue.unlock(); } catch ( ... ) { }
				throw;
			}

			m_shutdown = false;

			info = new task_observer_data( this, task_id );

			info->description = in_params.in_task_description ? strdup( in_params.in_task_description ) : nil;
			info->priority = in_params.in_priority;
			info->task_type = in_params.in_task_type;
		
			_no_throw( notify_observer( k_task_added, info, sizeof(*info) ) );
			
			if ( found_on_scheduled ) {
				proc_state( "scheduled notifier due to finding task on scheduled queue" );
				_no_throw( notify_observer( k_task_scheduled, info, sizeof(*info) ) );
			}

			delete info;

			m_task_queue.unlock();
		} catch ( ... ) {
			try { m_mutex_shutdown.unlock(); } catch ( ... ) { }
			throw;
		}
		
		m_mutex_shutdown.unlock();
	} catch ( ... ) {
		delete notifier;
		delete info;
		delete t;
		throw;
	}

	if ( ! found_on_scheduled && handle_immediately ) {
		handle_task( t );
		delete t;
	}
	
	return task_id;
}


void task_queue::remove_task( task_id_t in_task_id, task_type_t in_task_type, bool in_send_abort_notification ) {
	task_completion_params									completion;
	err_t_exception											err;
	bool													found = false;
	task_observer_data									   *info;
	task_completion_notifier							   *notifier, *next;
	priority_queue<task_completion_notifier>				notify_list;
	priority_queue<notify_task>::priority_queue_entry	   *pqe;
	notify_task											   *t;
	
	m_task_queue.lock();

	try {
		// Look for scheduled tasks first.  A "remove" on a scheduled task means set the
		// abort flag for the task.
		for ( t = m_scheduled_tasks.peek(); ! found && t; t = m_scheduled_tasks.peek( t ) ) {
			if ( in_task_type && t->task_type != in_task_type ) continue;
			
			for ( notifier = t->notify_list.peek(); notifier; notifier = next ) {
				next = t->notify_list.peek( notifier );
				
				if ( ! in_task_id || notifier->task_id == in_task_id ) {
					if ( in_task_id ) found = true;
					
					notifier->aborted = true;
				}
			}
		}

		// Now look on the queued tasks.  A "remove" here means remove it from the queue.
		if ( ! found ) {
			for ( pqe = m_task_queue.m_head; pqe; pqe = pqe->m_next ) {
				if ( in_task_type && pqe->m_data->task_type != in_task_type ) continue;
			
				for ( notifier = pqe->m_data->notify_list.peek(); notifier; notifier = next ) {
					next = pqe->m_data->notify_list.peek( notifier );
				
					if ( ! in_task_id || notifier->task_id == in_task_id ) {
						pqe->m_data->notify_list.remove( notifier );

						if ( in_send_abort_notification ) notify_list.insert( notifier );
						else {
							info = new task_observer_data( this, notifier->task_id );
							_no_throw( notify_observer( k_task_removed, info, sizeof(*info) ) );
							delete info;

							delete notifier;
						}
					}
				}
				
				if ( ! ( notifier = pqe->m_data->notify_list.peek() ) ) {
					delete reinterpret_cast<atomic_priority_queue<notify_task> *>(&m_task_queue)->remove( pqe->m_data, true );
				}
			}
		}
	} catch ( err_t_exception in_err ) {
		err = in_err;
	}

	try { m_task_queue.unlock(); } catch ( ... ) { }
	
	if ( ! found ) for ( notifier = notify_list.peek(); notifier; notifier = next ) {
		next = notify_list.peek( notifier );

		if ( in_send_abort_notification && notifier->completion ) {
			completion.in_context = notifier->context;
			completion.in_err = err_task_aborted;
			completion.io_result = nil;
			completion.in_result_size = 0;
			completion.in_task_id = notifier->task_id;
		
			try { notifier->completion( &completion ); } catch ( ... ) { }
		}

		info = new task_observer_data( this, notifier->task_id );
		_no_throw( notify_observer( k_task_removed, info, sizeof(*info) ) );
		delete info;

		delete notifier;
	}

	if ( err.err ) throw err;
}


void task_queue::reorder_task( task_id_t in_task_id, task_id_t in_order_after_task_id ) {
	notify_task					   *p, *q;
	task_completion_notifier   *notifier;

	m_task_queue.lock();

	for ( p = m_task_queue.peek(); p; p = m_task_queue.peek( p ) ) {
		for ( notifier = p->notify_list.peek(); notifier; notifier = p->notify_list.peek( notifier ) ) {
			if ( notifier->task_id == in_task_id ) {
				goto reorder_found_task;
			}
		}
	}

	reorder_found_task:
	
	if ( p ) {
		if ( ! in_order_after_task_id ) q = nil;
		else {
			for ( q = m_task_queue.peek(); q; q = m_task_queue.peek( q ) ) {
				for ( notifier = q->notify_list.peek(); notifier; notifier = q->notify_list.peek( notifier ) ) {
					if ( notifier->task_id == in_order_after_task_id ) {
						goto reorder_found_task_two;
					}
				}
			}	
		}

		reorder_found_task_two:
		
		if ( q != p ) m_task_queue.reorder( p, q );
	}

	m_task_queue.unlock();
}


void task_queue::run() {
	m_mutex_shutting_down.lock();
	m_mutex_shutting_down.unlock();
	m_mutex_shutdown.lock();
	m_shutdown = false;
	m_mutex_shutdown.unlock();
}


void task_queue::shutdown( bool in_send_abort_notifications ) {
	__u32			count;
	timespec		delay;
	pthread_t		p, thread_id, id = pthread_self();

	m_mutex_shutting_down.lock();
	
	try {
		locked_op( m_mutex_shutdown, m_shutdown = true );

		remove_task( 0, 0, in_send_abort_notifications );

		m_task_queue.wake_all_remove_blocked_threads();

		while ( ( thread_id = reinterpret_cast<pthread_t>(m_active_handler_threads.remove()) ) ) {
			m_active_handler_no_join_threads.lock();
			for ( p = reinterpret_cast<pthread_t>(m_active_handler_no_join_threads.peek()); p && ! pthread_equal( thread_id, p ); p = reinterpret_cast<pthread_t>(m_active_handler_no_join_threads.peek( p )) ) ;
			m_active_handler_no_join_threads.unlock();
			
			if ( ! ( p || pthread_equal( id, thread_id ) ) ) pthread_join( thread_id, nil );
		}

		// we need to wait until there are no threads (except possibly this one) on the active
		// handler queue before we exit shutdown().
		delay.tv_sec = 0;
		delay.tv_nsec = 250000000;		// wait .25 sec

		m_active_handler_threads.lock();
		for ( p = reinterpret_cast<pthread_t>(m_active_handler_threads.peek()); p && ! pthread_equal( p, id ); p = reinterpret_cast<pthread_t>(m_active_handler_threads.peek( p )) ) ;

		for ( ;; ) {
			count = m_active_handler_threads.entry_count();
			m_active_handler_threads.unlock();

			if ( p && count == 1 || ! count ) break;
			
			nanosleep( &delay, nil );

			m_active_handler_threads.lock();
		}
	} catch ( ... ) {
		try { m_mutex_shutting_down.unlock(); } catch ( ... ) { }
		throw;
	}

	m_mutex_shutting_down.unlock();
}


#pragma mark -


void task_queue::create_task_thread( task_queue *in_queue ) {
	pthread_t		thread_id;

	in_queue->m_active_handler_threads.lock();
	
	try {
		if ( in_queue->m_max_handler_threads == k_unlimited || in_queue->m_active_handler_threads.entry_count() < static_cast<__u32>(in_queue->m_max_handler_threads) ) {
			in_queue->m_mutex_create_task.lock();
			
			try {
				_throw_if( pthread_create( &thread_id, nil, reinterpret_cast<pv_proc_pv>(task_thread_entry), in_queue ) );
			} catch ( ... ) {
				try { in_queue->m_mutex_create_task.unlock(); } catch ( ... ) { }
				throw;
			}
			
			// synchronize with the newly created thread (force the insertion prior to returning)
			// note that task_thread_entry() aborts if it cannot unlock m_mutex_create_task, and
			// if we're here, then the mutex has already been locked and unlocked once, so these
			// calls are probably never going to fail.  even if they do, the insertion is complete
			// so it doesn't really matter whether these functions fail (if there's a real problem,
			// the next insertion will throw); therefore, to prevent the caller from misinterpreting
			// a lock exception, these methods don't throw on failure.
#ifndef __MACH__
	#warning verify that the double lock() on m_mutex_create_task works on this architecture.
#endif	
			try { in_queue->m_mutex_create_task.lock(); } catch ( ... ) { }
			try { in_queue->m_mutex_create_task.unlock(); } catch ( ... ) { }
		}
	} catch ( ... ) {
		try { in_queue->m_active_handler_threads.unlock(); } catch ( ... ) { }
		throw;
	}

	in_queue->m_active_handler_threads.unlock();
}


void task_queue::become_active_handler_thread( bool in_dont_join_this_thread_on_shutdown ) {
	pthread_t		id;

	m_mutex_shutdown.lock();

	if ( m_shutdown ) {
		try { m_mutex_shutdown.unlock(); } catch ( ... ) { }
		_throw( err_task_queue_shutdown );
	}
	
	id = pthread_self();
	
	try {
		if ( in_dont_join_this_thread_on_shutdown ) {
			m_active_handler_no_join_threads.insert( id, k_at_queue_tail_unique );
		}

		try {
			m_active_handler_threads.insert( id );

			try {
				m_mutex_shutdown.unlock();
			} catch ( ... ) {
				try { m_active_handler_threads.remove( id ); } catch ( ... ) { }
			}
		} catch ( ... ) {
			try { m_active_handler_no_join_threads.remove( id ); } catch ( ... ) { }
			throw;
		}
	} catch ( ... ) {
		m_mutex_shutdown.unlock();
		throw;
	}

	thread_loop();

	// the thread is removed from the active_handler_threads list in thread_loop()

	if ( in_dont_join_this_thread_on_shutdown ) {
		m_active_handler_no_join_threads.remove( id );
	}
}


void *task_queue::task_thread_entry( task_queue *in_queue ) {
	bool					bail = false;
	err_t					err = false;
	pthread_t				thread = pthread_self();

#if __OBJC__
	NSAutoreleasePool	   *pool = [[NSAutoreleasePool alloc] init];
#endif	
	
	_no_throw( in_queue->notify_observer( k_task_thread_entered, &thread, sizeof(pthread_t *) ) );
	
	try {
		in_queue->m_active_handler_threads.insert( thread, k_at_queue_tail, true );
	} catch ( ... ) {
		bail = true;
	}
	
	try {
		in_queue->m_mutex_create_task.unlock();
	} catch ( err_t_exception in_err ) {
		// if we can't unlock the mutex, then create_task_thread() will block indefinitely.
		// while extremely unlikely, this horrific condition presents us with no choice but
		// to immediately terminate the program.
		syslog( LOG_ERR, "task_thread_entry() could not unlock m_mutex_create_task at %s:%d, aborting", __FILE__, __LINE__ );
		exit( in_err.err );
	}

	if ( ! bail ) in_queue->thread_loop();

	_no_throw( in_queue->notify_observer( k_task_thread_exited, &thread, sizeof(pthread_t *) ) );

#if __OBJC__
	[pool release];
#endif	

	return nil;
}


void task_queue::thread_loop() {
	proc_enter( "task_queue::thread_loop()" );

	notify_task			   *t;
	bool					removed = false;
	void				   *thread = pthread_self();

	try {
		while ( ! ( m_shutdown || removed ) ) {
			if ( ! ( t = m_task_queue.remove( m_idle_task_handler_timeout ) ) ) break;

			#if __OBJC__
				NSAutoreleasePool	   *pool = [[NSAutoreleasePool alloc] init];
			#endif	

			proc_state( "handling task %#08x (%c%c%c%c)", t, ostype( t->task_type ) );
			handle_task( t );
			proc_state( "handled task %#08x (%c%c%c%c)", t, ostype( t->task_type ) );

			#if __OBJC__
				[pool release];
			#endif	

			// for runtime reduction of maximum number of threads
			m_active_handler_threads.lock();
			if ( m_active_handler_threads.entry_count() > m_max_handler_threads ) {
				m_active_handler_threads.remove( thread, true );
				removed = true;
			}
			m_active_handler_threads.unlock();
		}
	} catch ( ... ) { }

	/*	If this remove() fails we don't need to worry about it because ultimately we're just going
		to do a pthread_join() on the values in the m_active_handler_threads queue and a lingering
		thread id would get noticed and cleaned up then. */
	if ( ! removed ) _no_throw( m_active_handler_threads.remove( thread ) );
}


void task_queue::handle_task( notify_task *in_task ) {
	proc_enter( "task_queue::handle_task()" );

	task_completion_params			completion;
	task_handler_params				handler( this, in_task, reinterpret_cast<v_proc_pv_d_cpc>(progress_callback), reinterpret_cast<b_proc_pv>(should_abort_callback) );
	task_observer_data			   *info;
	task_completion_notifier	   *notifier;	

	_try {
		m_task_queue.lock();

		try {
			m_scheduled_tasks.insert( in_task, in_task->notify_list.priority() );

			proc_state( "task %#08x (%c%c%c%c) scheduled", in_task, ostype( in_task->task_type ) );

			for ( notifier = in_task->notify_list.peek(); notifier; notifier = in_task->notify_list.peek( notifier ) ) {
				info = new task_observer_data( this, notifier->task_id );
				proc_state( "notifying observer that task was scheduled" );
				_no_throw( notify_observer( k_task_scheduled, info, sizeof(*info) ) );
				delete info;
			}
		} _catch

		_always( m_task_queue.unlock() );
	} _catch

	try {
		// an error at this point indicates failure in the task system so requeue the task
		_if_err _throw( err_requeue_task );

		if ( in_task->handler ) {
			handler.in_context = in_task->handler_context;
			handler.in_data = in_task->data;
			handler.in_data_size = in_task->data_size;
			handler.in_task_type = in_task->task_type;

			in_task->handler( &handler );
		}
	} _catch
	
	if ( _err == err_requeue_task ) {
		try {
			m_task_queue.lock();
			m_scheduled_tasks.remove( in_task );
			m_task_queue.insert( reinterpret_cast<v_proc_pv>(create_task_thread), this, in_task, in_task->notify_list.priority( in_task->notify_list.peek() ), k_at_queue_head, true );

			for ( notifier = in_task->notify_list.peek(); notifier; notifier = in_task->notify_list.peek( notifier ) ) {
				info = new task_observer_data( this, notifier->task_id );
				_no_throw( notify_observer( k_task_requeued, info, sizeof(*info) ) );
				delete info;
			}
		} _catch

		_always( m_task_queue.unlock() );
		
		return;
	}

	try {
		handler.out_err = _err;

		while ( ( notifier = in_task->notify_list.remove() ) ) {
			if ( handler.out_err != err_task_aborted_no_completion && notifier->completion ) {
				completion.in_context = notifier->context;
				completion.in_err = handler.out_err;
				completion.io_result = handler.out_data;
				completion.in_result_size = handler.out_data_size;
				completion.in_task_id = notifier->task_id;
			
				_no_throw( notifier->completion( &completion ) );
			}
			
			info = new task_observer_data( this, notifier->task_id );
			_no_throw( notify_observer( k_task_completed, info, sizeof(*info) ) );
			delete info;
			
			delete notifier;
		}
	} _catch

	locked_op( m_task_queue, m_scheduled_tasks.remove( in_task ) );

	delete in_task;
}


void *task_queue::copy_data( __u32 in_event, const void *in_data, __u32 in_data_size ) {
	void					   *data;
	task_observer_data		   *p;

	switch ( in_event ) {
		case k_task_added:
		case k_task_completed:
		case k_task_progress:
		case k_task_removed:
		case k_task_scheduled: {
			data = observed::copy_data( in_event, in_data, in_data_size );
			
			p = reinterpret_cast<task_observer_data *>(data);

			if ( p->description ) p->description = strdup( p->description );
		} break;
	
		default: {
			data = observed::copy_data( in_event, in_data, in_data_size );
		} break;
	}
	
	return data;
}


void task_queue::progress_callback( task_handler_params *in_params, double in_percent_done, const char *in_description ) {
	notify_task					   *t;
	task_queue					   *tq;
	task_observer_data			   *info;
	task_completion_notifier	   *notifier;
	
	t = const_cast<notify_task *>(in_params->in_opaque_data);
	tq = reinterpret_cast<task_queue *>(in_params->in_callbacks_context);
	
	for ( notifier = t->notify_list.peek(); notifier; notifier = t->notify_list.peek( notifier ) ) {
		info = new task_observer_data( tq, notifier->task_id );

		info->percent_done = in_percent_done;
		info->description = in_description ? strdup( in_description ) : nil;
		
		_no_throw( tq->notify_observer( k_task_progress, info, sizeof(*info) ) );
		
		delete info;
	}
}


bool task_queue::should_abort_callback( task_handler_params *in_params ) {
	notify_task					   *t;
	task_queue					   *tq;
	task_completion_notifier	   *notifier;
	bool							result = false;
	
	t = const_cast<notify_task *>(in_params->in_opaque_data);
	tq = reinterpret_cast<task_queue *>(in_params->in_callbacks_context);

	if ( tq->m_shutdown ) result = true;
	else {
		peek_criteria( t->notify_list, notifier, notifier->aborted );

		if ( ! notifier ) result = true;
	}
	
	return result;	
}


#pragma mark -


__u32 task_queue::current_task_handler_threads() {
	__u32 result;
	
	locked_op( m_active_handler_threads, result = m_active_handler_threads.entry_count() );
	
	return result;
}


bool task_queue::is_shutdown() {
	bool result;
	
	locked_op( m_mutex_shutdown, result = m_shutdown );
	
	return result;
}


__s32 task_queue::max_concurrent_task_handler_threads() {
	__s32		result;
	
	locked_op( m_task_queue, result = m_max_handler_threads );
	
	return result;
}


__u32 task_queue::seconds_before_idle_task_handler_exits() {
	__u32		result;

	locked_op( m_task_queue, result = m_idle_task_handler_timeout );
	
	return result;
}


#pragma mark -


void task_queue::set_max_concurrent_task_handler_threads( __s32 in_max_handlers ) {
	locked_op( m_task_queue, m_max_handler_threads = in_max_handlers );
}


void task_queue::set_seconds_before_idle_task_handler_exits( __u32 in_seconds ) {
	locked_op( m_task_queue, m_idle_task_handler_timeout = in_seconds );
}
