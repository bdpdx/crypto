#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "observed.h"


observed::observed() { }


observed::~observed() {
	deregister_all();
}


void observed::deregister_all() {
	queue_delete_all( m_observers, observer_entry );
}


void observed::register_observer( observer_callback in_callback, void *in_context ) {
	observer_entry		   *observer;
	
	_assert( in_callback );

	m_observers.lock();
	
	try {
		peek_criteria( m_observers, observer, ! ( observer->context == in_context && observer->callback == in_callback ) );
		
		if ( ! observer ) {
			observer = new observer_entry;
			
			observer->callback = in_callback;
			observer->context = in_context;
			
			try {
				m_observers.insert( observer, balance::k_at_queue_tail, true );
			} catch ( ... ) {
				delete observer;
				throw;
			}
		}
	} catch	( ... ) {
		try { m_observers.unlock(); } catch ( ... ) { }
		throw;
	}
	
	m_observers.unlock();
}


void observed::deregister_observer( observer_callback in_callback, void *in_context ) {
	observer_entry		   *observer;

	m_observers.lock();

	try {
		peek_criteria( m_observers, observer, ! ( observer->context == in_context && observer->callback == in_callback ) );

		if ( observer ) {
			m_observers.remove( observer, true );
			delete observer;
		}
	} catch ( ... ) {
		try { m_observers.unlock(); } catch ( ... ) { }
		throw;
	}

	m_observers.unlock();
}


void observed::log( const char *in_format, ... ) {
	va_list					args;
	char				   *buf;
	__s32					size;
	
	va_start( args, in_format );
	size = vasprintf( &buf, in_format, args );
	va_end( args );

	_throw_errno_if( size == -1 && ( errno = ENOMEM ) );
	
	notify_observer( k_observed_log_message, buf, size + 1 );
	
	free( buf );
}


void *observed::copy_data( __u32 in_event, const void *in_data, __u32 in_data_size ) {
	void				   *data;
	
	in_event;
	
	data = in_data && in_data_size ? malloc( in_data_size ) : nil;

	if ( data ) memcpy( data, in_data, in_data_size );
	
	return data;
}


void observed::notify_observer( __u32 in_event, const void *in_data, __u32 in_data_size ) {
	observer_entry		   *p;
	void				   *data;

	try {
		atomic_queue_for_each_start( m_observers, p, true );
		
		try {
			data = copy_data( in_event, in_data, in_data_size );
			p->callback( p->context, in_event, data, in_data_size );
		} catch ( ... ) { }

		atomic_queue_for_each_end( m_observers );
	} catch ( ... ) { }
}
