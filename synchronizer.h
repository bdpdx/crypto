#ifndef __synchronizer_h__
#define __synchronizer_h__



#if _WIN32
	#include "precompiled.h"

	#include <Windows.h>
#else
	#include <pthread.h>
	#include <semaphore.h>
#endif


#ifndef __balance_types
	typedef unsigned int	__u32;
	
	#ifndef nil
		#define nil			0
	#endif
#endif


namespace balance {


struct synchronizer {

	synchronizer() { }
	virtual ~synchronizer() = 0;

	// this abstract base class provides synchronization functions.
	// most synchronizers do not implement all functions, use only
	// those functions explicitly declared in the derived class.

	// mutex methods
	virtual void lock();
	virtual void unlock();

	// condition variable methods
	virtual void wait();							// wait indefinitely
	virtual void wait( __u32 in_timeout_seconds );	// in_timeout_seconds == 0 effects a poll
	virtual void wake();
	virtual void wake_all();
};


#define locked_op( _in_synchronizer, _in_statement ) do {		\
	( _in_synchronizer ).lock();								\
	try { _in_statement; } _catch								\
	( _in_synchronizer ).unlock();								\
	_throw_now();												\
} while ( 0 )


#pragma mark -


struct mutex : public synchronizer {

	mutex();
	virtual ~mutex();

	virtual void lock();
	virtual void unlock();

#if _WIN32
	operator HANDLE() { return _mutex; }
#else
	operator pthread_mutex_t *() { return &_mutex; }
#endif

protected:

#if _WIN32
	HANDLE					_mutex;
#else
	pthread_mutex_t			_mutex;
#endif
};


struct mutex_wrapper {

	mutex_wrapper( mutex &io_mutex );
   ~mutex_wrapper();

	void lock() { _mutex.lock(); _locked = 1; }
	void unlock() { if ( _locked ) { _mutex.unlock(); _locked = 0; } }
   
protected:

	unsigned				_locked		:	1;
	mutex				   &_mutex;
};


struct semaphore : public synchronizer {

	semaphore( bool in_initialize_unlocked = true );
	virtual ~semaphore();

	// lock/unlock and wait/wake do the same things internally.
	// separate calling semantics are provided for convenience
	// depending on whether the semaphore is intended to be used
	// as a mutex or condition variable.

	virtual void lock();		// equivalent to wait() (UP)
	virtual void unlock();		// equivalent to wake() (DOWN)

	virtual void wait();		// wait indefinitely
	virtual void wait( __u32 in_timeout_seconds );	// wait indefinitely regardless of in_timeout_seconds!
	virtual void wake();

#if _WIN32
	operator HANDLE() { return _semaphore; }
#else
	operator sem_t *() { return _semaphore; }
#endif

protected:

#if _WIN32
	HANDLE					_semaphore;
#else
	char					_name[ 32 ];
	sem_t				   *_semaphore;
#endif
};


#if ! _WIN32

struct condition : public synchronizer {

	condition( mutex &in_mutex );
	virtual ~condition();

	// wait() calls require mutex to be locked on entry
	virtual void wait();
	virtual void wait( __u32 in_timeout_seconds );
	virtual void wake();
	virtual void wake_all();

	operator pthread_cond_t *() { return &_condition; }
	operator pthread_mutex_t *() { return _mutex; }

protected:

	pthread_cond_t			_condition;
	mutex				   &_mutex;
};


struct condition_s : public synchronizer {
	condition_s( semaphore &in_mutex );
	virtual ~condition_s();

	// wait() calls require mutex to be locked on entry
	virtual void wait();
	virtual void wait( __u32 in_timeout_seconds );
	virtual void wake();
	virtual void wake_all();

protected:

	semaphore				_condition;
	semaphore			   &_mutex;
};


#endif // ! _WIN32


} // namespace balance



#endif // __synchronizer_h__
