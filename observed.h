#ifndef __observed_h__
#define __observed_h__



#if WIN32
	#include "precompiled.h"
#endif


#include "queue.h"


/*	The observer_callback is called by notify_observer() under a locked mutex. Therefore,
	the observer_callback must not perform any operation that might trigger another call to
	notify_observer() since a deadlock would occur.
	
	It is recommended in almost all cases that the observer_callback simply queue the notification
	and return immediately.  Since multiple observers can record the passing of data, and since
	a notification might be handled on a separate thread, the notify_observer routine makes a
	copy of the data passed to the observer_callback.  It is the responsibility of the
	observer_callback to free() the in_data parameter if it is non-nil.  Generally, a bytewise
	copy of in_data is performed in notify_observer.  Override copy_data to change this behavior. */
typedef void (*observer_callback)( void *in_context, __u32 in_event, void *in_data, __u32 in_data_size );


enum observed_events {
	k_observed_log_message			=		0x6F6C6F67		// 'olog'
};


class observed {

public:

	observed();
	virtual ~observed();

	virtual void register_observer( observer_callback in_observer, void *in_context );
	virtual void deregister_observer( observer_callback in_observer, void *in_context );
	virtual void deregister_all();

	void log( const char *in_format, ... );

protected:

	virtual void *copy_data( __u32 in_event, const void *in_data, __u32 in_data_size );

	// see notes for observer_callback above!
	void notify_observer( __u32 in_event, const void *in_data = nil, __u32 in_data_size = 0 );

private:

	struct observer_entry {
		observer_callback					callback;
		void							   *context;
	};

	balance::atomic_queue<observer_entry>	m_observers;
};



#endif // __observed_h__
