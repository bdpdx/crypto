#if ! _WIN32
	#include <sys/errno.h>
	#include <sys/time.h>
	#include <unistd.h>
#endif

#include <stdlib.h>


#include "synchronizer.h"


#ifndef __common_exceptions_h__
	#define _throw( _stmt )				exit( ENOENT )
	#define _throw_errno_if( _stmt )	if ( _stmt ) exit( errno )
	#define _throw_if( _stmt )			if ( _stmt ) exit( EDEADLK )
#endif


balance::synchronizer::~synchronizer() { }

void balance::synchronizer::lock() { _throw( err_unimplemented ); }
void balance::synchronizer::unlock() { _throw( err_unimplemented ); }
void balance::synchronizer::wait() { _throw( err_unimplemented ); }
void balance::synchronizer::wait( __u32 in_timeout_seconds ) { _throw( err_unimplemented ); }
void balance::synchronizer::wake() { _throw( err_unimplemented ); }
void balance::synchronizer::wake_all() { _throw( err_unimplemented ); }


#pragma mark -


#if ENABLE_SYNCHRONIZER_CONDITION
#if ! _WIN32

balance::condition::condition( mutex &io_mutex ) : _mutex( io_mutex ) { _throw_if( pthread_cond_init( &_condition, nil ) ); }
balance::condition::~condition() { pthread_cond_destroy( &_condition ); }

void balance::condition::wait() {
	_throw_if( pthread_cond_wait( &_condition, _mutex ) );
}

void balance::condition::wait( __u32 in_timeout_seconds ) {
	struct timespec			ts;
	struct timeval			tv;

	_throw_errno_if( gettimeofday( &tv, nil ) == -1 );

	ts.tv_sec = time( nil ) + in_timeout_seconds;
	ts.tv_nsec = 0;

	_throw_if( pthread_cond_timedwait( &_condition, _mutex, &ts ) );
}

void balance::condition::wake() { _throw_if( pthread_cond_signal( &_condition ) ); }
void balance::condition::wake_all() { _throw_if( pthread_cond_broadcast( &_condition ) ); }


#pragma mark -


balance::condition_s::condition_s( semaphore &io_semaphore ) : _condition( false ), _mutex( io_semaphore ) { }
balance::condition_s::~condition_s() { }

void balance::condition_s::wait() {
	_mutex.unlock();
	_condition.wait();
	_mutex.lock();
}

void balance::condition_s::wait( __u32 in_timeout_seconds ) {
	_mutex.unlock();
	_condition.wait();
	_mutex.lock();
}

void balance::condition_s::wake() { _condition.wake(); }
void balance::condition_s::wake_all() { _condition.wake_all(); }

#endif // ! _WIN32
#endif // ENABLE_SYNCHRONIZER_CONDITION


#pragma mark -


#if _WIN32
	balance::mutex::mutex() {
		SECURITY_ATTRIBUTES		attributes;

		attributes.nLength = sizeof(attributes);
		attributes.lpSecurityDescriptor = NULL;
		attributes.bInheritHandle = true;

		if ( ! ( _mutex = CreateMutex( &attributes, false, NULL ) ) ) {
			_throw( err_cannot_create_mutex );
		}
	}

	balance::mutex::~mutex() { CloseHandle( _mutex ); }

	void balance::mutex::lock() { WaitForSingleObject( _mutex, INFINITE ); }
	void balance::mutex::unlock() { ReleaseMutex( _mutex ); }
#else
	balance::mutex::mutex() { _throw_if( pthread_mutex_init( &_mutex, nil ) ); }
	balance::mutex::~mutex() { pthread_mutex_destroy( &_mutex ); }

	void balance::mutex::lock() { _throw_if( pthread_mutex_lock( &_mutex ) ); }
	void balance::mutex::unlock() { _throw_if( pthread_mutex_unlock( &_mutex ) ); }
#endif


#pragma mark -
#include <stdio.h>

balance::mutex_wrapper::mutex_wrapper( mutex &io_mutex ) : _mutex( io_mutex ) { lock(); }
balance::mutex_wrapper::~mutex_wrapper() { unlock(); }


#pragma mark -

#if ENABLE_SYNCHRONIZER_SEMAPHORE
#if _WIN32
	balance::semaphore::semaphore( bool in_initialize_unlocked ) {
		SECURITY_ATTRIBUTES		attributes;

		attributes.nLength = sizeof(attributes);
		attributes.lpSecurityDescriptor = NULL;
		attributes.bInheritHandle = true;

		if ( ! ( _semaphore = CreateSemaphore( &attributes, in_initialize_unlocked, 32, NULL ) ) ) {
			_throw( err_cannot_create_semaphore );
		}
	}

	balance::semaphore::~semaphore() { CloseHandle( _semaphore ); }

	void balance::semaphore::lock() { WaitForSingleObject( _semaphore, INFINITE ); }
	void balance::semaphore::unlock() { ReleaseSemaphore( _semaphore, 1, NULL ); }
#else
	balance::semaphore::semaphore( bool in_initialize_unlocked ) {
		snprintf( _name, sizeof(_name), "semaphore.%u.%08x", getpid(), this );
		_throw_errno_if( ( _semaphore = sem_open( _name, O_CREAT, 0600, in_initialize_unlocked ) ) == (sem_t *) SEM_FAILED );
	}

	balance::semaphore::~semaphore() {
		sem_close( _semaphore );
		sem_unlink( _name );
	}

	void balance::semaphore::lock() {
		int						err;

		while ( ( err = sem_wait( _semaphore ) ) && errno == EINTR ) ;
		
		_throw_errno_if( err );
	}

	void balance::semaphore::unlock() { _throw_errno_if( sem_post( _semaphore ) == -1 ); }
#endif

void balance::semaphore::wait() { lock(); }
void balance::semaphore::wait( __u32 in_timeout_seconds ) { lock(); }
void balance::semaphore::wake() { unlock(); }
#endif // ENABLE_SYNCHRONIZER_SEMAPHORE
