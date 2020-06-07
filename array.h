#ifndef __array_h__		// bd 07.17.07
#define __array_h__


namespace balance {


template<typename element_t> struct array {

	array( __u32 in_element_count ) : _array( new element_t[ in_element_count ] ) { }
	array( __u32 in_element_count, bool in_zero_array ) : _array( new element_t[ in_element_count ] ) { if ( in_zero_array ) memset( _array, 0, sizeof(element_t) * in_element_count ); }
   ~array() { delete[] _array; }

	element_t &operator[]( int in_index ) { return _array[ in_index ]; }
	element_t &operator[]( unsigned in_index ) { return _array[ in_index]; }

	operator element_t *() { return _array; }

protected:

	element_t			   *_array;

};


} // namespace balance



#endif // __array_h__
