#ifndef __CircularBuffer_h__
#define __CircularBuffer_h__



#include <pthread.h>


#ifndef nil
	#define nil 0
#endif


class CircularBuffer {

public:

	// A thread-safe circular buffer producer/consumer class.
	//
	// write() is the producer (adds data to the buffer).
	// read() is the consumer (removes data from the buffer).
	//
	// The default action is to not lose unread data.  As such, if a
	// call to write() would overflow the circular buffer, the buffer is
	// resized to accomodate the new data.
	//
	// If inOverwriteUnread is true, or if a resize operation on write()
	// fails, the buffer behaves as a strict circular buffer, overwriting 
	// unread data.
	
	CircularBuffer( bool inOverwriteUnread = false );
   ~CircularBuffer() { done(); }

	// internal state management
	void done() { delete[] _buffer; _buffer = nil; flush(); }
	void flush() { _end = -1; _start = 0; }
	void init( int inBufferSize ) { done(); _buffer = new char[ _size = inBufferSize ];	}
	bool overwriteUnread() const { return _fOverwriteUnread; }
	void resize( int inBufferSize );
	void setOverwriteUnread( bool inOverwriteUnread ) { _fOverwriteUnread = inOverwriteUnread; }
	int size() const { return _size; }

	// buffer contents management
	int free() const { return _size - used(); }
	int used() const { return _end == -1 ? 0 : _end > _start ? _end - _start : _size - ( _start - _end ); }

	void consume( int inLength );
	void peek( void *outBuffer, int &ioLength ) const;
	void peek( char *&outBuffer0, int &outLength0, char *&outBuffer1, int &outLength1 ) const;
	void read( void *outBuffer, int &ioLength );
	void write( const void *inBuffer, int inLength );

	// convenience accessors
	operator int() const { return used(); }
	char operator[]( int inIndex ) const { return _buffer[ ( _start + inIndex ) % _size ]; }

private:

	char				   *_buffer;
	int						_end;
	int						_fOverwriteUnread	:	1;
	int						_size;
	int						_start;

};


class ThreadSafeCircularBuffer : public CircularBuffer {

public:

	ThreadSafeCircularBuffer( bool inOverwriteUnread = false ) : CircularBuffer( inOverwriteUnread ) { pthread_mutex_init( &_mutex, nil ); }
   ~ThreadSafeCircularBuffer() { pthread_mutex_destroy( &_mutex ); }

	#define __tscbLock			locker lock( const_cast<pthread_mutex_t *>(&_mutex) );

	void consume( int inLength ) { __tscbLock CircularBuffer::consume( inLength ); }
	void flush() { __tscbLock CircularBuffer::flush(); }
	int free() const { __tscbLock return CircularBuffer::free(); }
	bool overwriteUnread() const { __tscbLock return CircularBuffer::overwriteUnread(); }
	void peek( void *outBuffer, int &ioLength ) const { __tscbLock CircularBuffer::peek( outBuffer, ioLength ); }
	void read( void *outBuffer, int &ioLength ) { __tscbLock CircularBuffer::read( outBuffer, ioLength ); }
	void resize( int inBufferSize ) { __tscbLock CircularBuffer::resize( inBufferSize ); }
	void setOverwriteUnread( bool inOverwriteUnread ) { __tscbLock CircularBuffer::setOverwriteUnread( inOverwriteUnread ); }
	int size() const { __tscbLock return CircularBuffer::size(); }
	int used() const { __tscbLock return CircularBuffer::used(); }
	void write( const void *inBuffer, int inLength ) { __tscbLock CircularBuffer::write( inBuffer, inLength ); }

	#undef __tscbLock

	// lock() and unlock() lock and unlock the internal mutex.
	// locking and unlocking is performed automatically by all
	// methods *except* the following:
	//
	// void peek( char *&outBuffer0, int &outLength0, char *&outBuffer1, int &outBuffer1 ) const;
	// char operator[]( int inIndex ) const;
	//
	// When using any of these methods, lock() the class, read
	// as much as needed via operator[], then unlock() the class.
	// Note that it is unsafe to call any other class method while
	// the mutex is held by lock() as a double lock would occur.
	void lock() { pthread_mutex_lock( &_mutex ); }
	void unlock() { pthread_mutex_unlock( &_mutex ); }

	operator int() const { return used(); }

protected:

	struct locker {
		locker( pthread_mutex_t *inMutex ) : _mutex( inMutex ) { pthread_mutex_lock( inMutex ); }
	   ~locker() { pthread_mutex_unlock( _mutex ); }

		pthread_mutex_t	   *_mutex;
	};

	pthread_mutex_t			_mutex;

};


#endif // __CircularBuffer_h__
