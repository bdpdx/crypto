#include <string.h>

#include "CircularBuffer.h"


#ifndef MIN
	#define MIN( a, b )		( ( a ) < ( b ) ? ( a ) : ( b ) )
#endif

CircularBuffer::CircularBuffer( bool inOverwriteUnread ) {
	_buffer = nil;
	_fOverwriteUnread = inOverwriteUnread;
	flush();
	_size = 0;
}


#pragma mark -


void CircularBuffer::consume( int inLength ) {
	if ( inLength < 1 || _end == -1 ) return;

	if ( used() <= inLength ) flush();
	else _start = ( _start + inLength ) % _size;
}


void CircularBuffer::peek( void *outBuffer, int &ioLength ) const {
	int						n, o;

	if ( _end == -1 ) {
		ioLength = 0;
		return;
	}

	n = used();
	o = ioLength;

	if ( o > n ) ioLength = o = n;
	
	if ( _end > _start ) {
		memcpy( outBuffer, &_buffer[ _start ], o );
	} else {
		n = _size - _start;
		n = MIN( n, o );
	
		memcpy( outBuffer, &_buffer[ _start ], n );
		
		if ( ( o -= n ) ) memcpy( (char *) outBuffer + n, _buffer, o );
	}
}


void CircularBuffer::peek( char *&outBuffer0, int &outLength0, char *&outBuffer1, int &outLength1 ) const {
	if ( _end == -1 ) {
		outBuffer0 = nil;
		outLength0 = 0;
		outBuffer1 = nil;
		outLength1 = 0;
	} else if ( _end > _start ) {
		outBuffer0 = &_buffer[ _start ];
		outLength0 = _end - _start;
		outBuffer1 = nil;
		outLength1 = 0;
	} else {
		outBuffer0 = &_buffer[ _start ];
		outLength0 = _size - _start;
		outBuffer1 = _buffer;
		outLength1 = _end;
	}
}
	
	
void CircularBuffer::read( void *outBuffer, int &ioLength ) {
	peek( outBuffer, ioLength );
	consume( ioLength );
}


void CircularBuffer::resize( int inBufferSize ) {
	int						n, o;
	char				   *p;
	
	if ( ! _buffer ) {
		init( inBufferSize );
	} else {
		n = used();
		o = n - inBufferSize;
		
		if ( o > 0 ) {
			if ( ! _fOverwriteUnread ) return;
			
			_start = ( _start + o ) % _size;
		}
		
		p = new char[ n = inBufferSize ];

		if ( _end != -1 ) {	
			read( p, n );
			
			_start = 0;
			_end = n;
		}
		
		delete[] _buffer;

		_buffer = p;
		_size = inBufferSize;
	}
}	


void CircularBuffer::write( const void *inBuffer, int inLength ) {
	if ( inLength < 1 ) return;

	int						n;

	if ( ! _buffer ) {
		init( inLength );
	} else {
		n = inLength - free();
	
		if ( n > 0 ) {
			if ( _fOverwriteUnread ) {
				if ( inLength >= _size ) {
					_end = 0;
					_start = 0;

					memcpy( _buffer, (char *) inBuffer + inLength - _size, _size );
				} else {
					memcpy( &_buffer[ _end ], inBuffer, n = _size - _end );
					memcpy( _buffer, (char *) inBuffer + n, n = inLength - n );
					
					_end = n;
					_start = n;
				}
				
				return;
			} else {
				resize( _size + n );
			}
		}
	}

	if ( _end == -1 ) _end = 0;
	
	memcpy( _buffer, inBuffer, inLength );

	_end += inLength;
}
