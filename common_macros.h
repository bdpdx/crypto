#ifndef __common_macros_h__
#define __common_macros_h__



#define _CFRelease( _ptr )					do { if ( _ptr ) { CFRelease( _ptr ); _ptr = nil; } } while ( 0 )
#define _close_( _file_descriptor )			do { if ( ( _file_descriptor ) != k_descriptor_closed ) { close( _file_descriptor ); ( _file_descriptor ) = k_descriptor_closed; } } while ( 0 )
#define _count_bits( _in_word, _out_count )	do { register __u32 _a = ( _in_word ), _b, _c; _b = _a - ( _a >> 1 & 0x55555555 ); _c = ( _b & 0x33333333 ) + ( _b >> 2 & 0x33333333 ); ( _out_count ) = ( _c + ( _c >> 4) & 0x0f0f0f0f ) * 0x01010101 >> 24; } while ( 0 )
#define _delete( _ptr )						do { delete _ptr; _ptr = nil; } while ( 0 )
#define _delete_( _ptr )					do { delete[] _ptr; _ptr = nil; } while ( 0 )
#define _fclose( _FILE )					do { if ( _FILE ) { fclose( _FILE ); _FILE = nil; } } while ( 0 )
#define _free( _ptr )						do { if ( _ptr ) { free( _ptr ); _ptr = nil; } } while ( 0 )
#define _new( _ptr, _type )					do { delete _ptr; _ptr = new _type; } while ( 0 )
#define _parity( _in_word, _out_parity )	do { register __u32 _a = ( _in_word ) >> 16 ^ ( _in_word ); _a ^= _a >> 8; _a ^= _a >> 4; _a &= 0xf; ( _out_parity ) = 0x6996 >> _a & 1; } while ( 0 )
#define _reverse_bits( _in_byte )			do { ( _in_byte ) = ( ( ( _in_byte ) * 0x0802lu & 0x22110lu ) | ( ( _in_byte ) * 0x8020lu & 0x88440lu ) ) * 0x10101lu  >> 16; } while ( 0 )
#define _zdelete( _ptr, _len )				do { if ( _ptr ) { memset( _ptr, 0, _len ); delete _ptr; _ptr = nil; } } while ( 0 )
#define _zdelete_( _ptr, _len )				do { if ( _ptr ) { memset( _ptr, 0, _len ); delete[] _ptr; _ptr = nil; } } while ( 0 )
#define _zfree( _ptr, _len )				do { if ( _ptr ) { memset( _ptr, 0, _len ); free( _ptr ); _ptr = nil; } } while ( 0 )

#define absolute( _a )						( ( _a ) < 0 ? -( _a ) : ( _a ) )
#define bit_set( _in_field, _in_bit )		( ( _in_field ) |= ( _in_bit ) )
#define bit_clear( _in_field, _in_bit )		( ( _in_field ) &= ~( _in_bit ) )
#define bit_is_set( _in_field, _in_bit )	( ( _in_field ) & ( _in_bit ) )
#if _WIN32
	#define err_exit( _err, _format, ... )	do { fprintf( stderr, _format "\n", __VA_ARGS__ ); exit( _err ); } while ( 0 )
#else
	#define err_exit( _err, _format, _args... ) do { fprintf( stderr, _format "\n", ## _args ); exit( _err ); } while ( 0 )
#endif
#define hexify( _byte, _high_char, _low_char )	do { ( _high_char ) = ( _byte ) >> 4 & 0xf; ( _high_char ) += ( _high_char ) < 10 ? '0' : 'a' - 10; ( _low_char ) = ( _byte ) & 0x0f; ( _low_char ) += ( _low_char ) < 10 ? '0' : 'a' - 10; } while ( 0 )
#define is_power_of_2( _in_value )			( ( _in_value ) == ( ( _in_value ) & ~( ( _in_value ) - 1 ) ) )
#define is_whitespace( _c )					( ( _c ) == k_ascii_space || ( _c ) == k_ascii_tab || ( _c ) == k_ascii_newline || ( _c ) == k_ascii_carriage_return || ( _c ) == k_ascii_vertical_tab || ( _c ) == k_ascii_form_feed )

#define _swap16( _n )						( ( _n ) << 8 | ( _n ) >> 8 )
#define _swap32( _n )						( ( _n ) << 24 | ( _n ) << 8 & 0xff0000 | ( _n ) >> 8 & 0xff00 | ( _n ) >> 24 )
#define _swap64( _n )						( ( _n ) << 56 | ( _n ) << 40 & 0xff000000000000ull | ( _n ) << 24 & 0xff0000000000ull | ( _n ) << 8 & 0xff00000000ull | ( _n ) >> 8 & 0xff000000ull | ( _n ) >> 24 & 0xff0000ull | ( _n ) >> 40 & 0xff00ull | ( _n ) >> 56 )

#ifdef __cplusplus
	inline __u16 swap16( __u16 in_n ) { return _swap16( in_n ); }
	inline __u32 swap32( __u32 in_n ) { return _swap32( in_n ); }
	inline __u64 swap64( __u64 in_n ) { return _swap64( in_n ); }
#else
	#define swap16( _n )					_swap16( _n )
	#define swap32( _n )					_swap32( _n )
	#define swap64( _n )					_swap64( _n )
#endif

#if BYTE_ORDER == BIG_ENDIAN
	#define big16( _n )						( _n )
	#define big32( _n )						( _n )
	#define big64( _n )						( _n )

	#define little16( _n )					swap16( (__u16)(_n) )
	#define little32( _n )					swap32( (__u32)(_n) )
	#define little64( _n )					swap64( (__u64)(_n) )
	
	#define on_be( _x )						do { _x; } while ( 0 )
	#define on_le( _x )						do { } while ( 0 )
#else
	#define big16( _n )						swap16( (__u16)(_n) )
	#define big32( _n )						swap32( (__u32)(_n) )
	#define big64( _n )						swap64( (__u64)(_n) )

	#define little16( _n )					( _n )
	#define little32( _n )					( _n )
	#define little64( _n )					( _n )

	#define on_be( _x )						do { } while ( 0 )
	#define on_le( _x )						do { _x; } while ( 0 )
#endif // BYTE_ORDER

#if DEBUG
	#if _WIN32
		#define console( _in_format, ... )		_console( __FILE__, __LINE__, _in_format, __VA_ARGS__ )
	#else
		#define console( _in_format, args... )	_console( __FILE__, __LINE__, _in_format, ## args )
	#endif
	#define debug_declare( _x )				_x
	#define debug_statement( _x )			do { _x; } while ( 0 );
	#define dump( _in_address, _in_length )	_dump( __FILE__, __LINE__, _in_address, _in_length, 99 )
	#if _WIN32
		#define dump_info( _in_address, _in_length, _in_format, ... ) do { console( _in_format, __VA_ARGS__ ); dump( _in_address, _in_length ); } while ( 0 )
	#else
		#define dump_info( _in_address, _in_length, _in_format, _in_args... ) do { console( _in_format, ## _in_args ); dump( _in_address, _in_length ); } while ( 0 )
	#endif
	#define dumpw( _in_address, _in_length, _in_width )	_dump( __FILE__, __LINE__, _in_address, _in_length, _in_width )
	#define release_statement( _x )			do { } while ( 0 )

	void _console( const char *in_file, __u32 in_line, const char *_in_format, ... );
	void _dump( const char *in_file, __u32 in_line, const void *in_address, __u32 in_length, __u32 in_width );
#else
	#if _WIN32
		#define console( _in_format, ... )		do { } while ( 0 )
	#else
		#define console( _in_format, args... )	do { } while ( 0 )
	#endif
	#define debug_declare( _x )
	#define debug_statement( _x )			do { } while ( 0 );
	#define dump( _address, _length )		do { } while ( 0 )
	#if _WIN32
		#define dump_info( _in_address, _in_length, _in_format, ... ) do { } while ( 0 )
	#else
		#define dump_info( _in_address, _in_length, _in_format, _in_args... ) do { } while ( 0 )
	#endif
	#define dumpw( _in_address, _in_length, _in_width )	do { } while ( 0 )
	#define release_statement( _x )			do { _x; } while ( 0 )
#endif // DEBUG

#define _d( _x )							debug_statement( _x )
#define _e( _x )							debug_declare( _x )
#define _r( _x )							release_statement( _x )

#define deprecated( _x )					do { _x; } while ( 0 )


#ifndef __cplusplus
	#if ! _WIN32
		#define max( _a, _b )					( ( _a ) > ( _b ) ? ( _a ) : ( _b ) )
		#define min( _a, _b )					( ( _a ) < ( _b ) ? ( _a ) : ( _b ) )
	#endif
	#define inline
#else
	template<typename t> inline const t &min( const t &in_lhs, const t &in_rhs ) { return in_lhs < in_rhs ? in_lhs : in_rhs; }
	template<typename t> inline const t &max( const t &in_lhs, const t &in_rhs ) { return in_lhs > in_rhs ? in_lhs : in_rhs; }

	#if DEBUG
		#define _assert( _in_statement )	do { if ( ! ( _in_statement ) ) _throw_msg( err_assertion_failure, "assertion failed" ); } while ( 0 )
	#else
		#define _assert( _in_statement )	do { } while ( 0 )
	#endif // DEBUG && __cplusplus
#endif	// __cplusplus


#define max2								max
#define max3( _a, _b, _c )					max( ( _a ), max( ( _b ), ( _c ) ) )
#define max4( _a, _b, _c, __d )				max( ( _a ), max3( ( _b ), ( _c ), ( __d ) ) )
#define max5( _a, _b, _c, __d, __e )		max( ( _a ), max4( ( _b ), ( _c ), ( __d ), ( __e ) ) )
#define max6( _a, _b, _c, __d, __e, _f )	max( ( _a ), max5( ( _b ), ( _c ), ( __d ), ( __e ), ( _f ) ) )

#define min2								min
#define min3( _a, _b, _c )					min( ( _a ), min( ( _b ), ( _c ) ) )
#define min4( _a, _b, _c, __d )				min( ( _a ), min3( ( _b ), ( _c ), ( __d ) ) )
#define min5( _a, _b, _c, __d, __e )		min( ( _a ), min4( ( _b ), ( _c ), ( __d ), ( __e ) ) )
#define min6( _a, _b, _c, __d, __e, _f )	min( ( _a ), min5( ( _b ), ( _c ), ( __d ), ( __e ), ( _f ) ) )


#define pigeonholes( _in_amount, _in_container_size )								\
	( ( ( _in_amount ) + ( _in_container_size ) - 1 ) / ( _in_container_size ) )

#define round_up( _in_amount, _in_container_size )									\
	( pigeonholes( ( _in_amount ), ( _in_container_size ) ) * ( _in_container_size ) )


#ifdef __cplusplus
extern "C" {
#endif

#define wait_ms( _in_millisecond_delay ) do {										\
	struct timespec			delay;													\
	__u32					ms = ( _in_millisecond_delay );							\
																					\
	delay.tv_sec = ms / 1000000;													\
	delay.tv_nsec = ms % 1000000 * 1000000;											\
																					\
	nanosleep( &delay, nil );														\
} while ( 0 )

#define wait_ns( _in_nanosecond_delay ) do {										\
	struct timespec			delay;													\
	__u32					ns = ( _in_nanosecond_delay );							\
																					\
	delay.tv_sec = ns / 1000000000;													\
	delay.tv_nsec = ns % 1000000000;												\
																					\
	nanosleep( &delay, nil );														\
} while ( 0 )

#define wait_us( _in_microsecond_delay ) do {										\
	struct timespec			delay;													\
	__u32					us = ( _in_microsecond_delay );							\
																					\
	delay.tv_sec = us / 1000000;													\
	delay.tv_nsec = us % 1000000 * 1000;											\
																					\
	nanosleep( &delay, nil );														\
} while ( 0 )


// rotate left that makes no assumptions about its arguments; each argument is
// evaluated only once.
#define rotate_left( _in_word_bits, _in_word, _in_bits, _out_result ) do {		\
	__u##_in_word_bits	_b = (__u##_in_word_bits) ( _in_bits );					\
	__u##_in_word_bits	_w = (__u##_in_word_bits) ( _in_word );					\
																				\
	( _out_result ) = _w << _b | _w >> _in_word_bits - _b;						\
} while ( 0 )

#define rotate_right( _in_word_bits, _in_word, _in_bits, _out_result )			\
	rotate_left( _in_word_bits, ( _in_word ), _in_word_bits - ( _in_bits ), ( _out_result ) )

// convenience macro: rotate left that assumes each of its arguments have already
// been evaluated.  arguments may be evaluated more than once.
#define rotate_left_evaluated( _in_word_bits, _in_word, _in_bits )				\
	( ( _in_word ) << ( _in_bits ) | (__u##_in_word_bits) ( _in_word ) >> _in_word_bits - ( _in_bits ) )
	
#define rotate_right_evaluated( _in_word_bits, _in_word, _in_bits )				\
	rotate_left_evaluated( _in_word_bits, ( _in_word ), _in_word_bits - ( _in_bits ) )


#ifdef __cplusplus
}
#endif


#ifndef MAX
	#define MAX( _a, _b )						( ( _a ) > ( _b ) ? ( _a ) : ( _b ) )
#endif

#ifndef MIN
	#define MIN( _a, _b )						( ( _a ) < ( _b ) ? ( _a ) : ( _b ) )
#endif



#endif	// __common_macros_h__
