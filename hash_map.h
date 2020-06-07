#ifndef __hash_map_h__	// bd 07.17.07
#define __hash_map_h__



// created this as a portable hash_map container.  the gcc
// <ext/hash_map> behavior is not compatible with visual
// c++'s <hash_map> implementation even though they look a
// whole lot alike.


namespace balance {


template<typename key_t, typename value_t> class hash_map {

public:

	// on 32-bit systems with in_num_buckets == 1024
	// the _buckets array will occupy one 4k page.
	hash_map( __u32 in_num_buckets = 1024 );
   ~hash_map();

	value_t &operator[]( key_t &in_key );
	
	bool inserted( key_t &in_key, value_t *out_value = nil ) const;

protected:

	struct hash_map_node {
		hash_map_node( key_t &in_key ) : _key( in_key ), _next( nil ) { }

		key_t				_key;
		hash_map_node	   *_next;
		value_t				_value;
	};

	hash_map_node **hash( key_t &in_key ) const;
	
	hash_map_node		  **_buckets;
	__u32					_num_buckets;

};


template<typename key_t, typename value_t> hash_map<key_t, value_t>::hash_map( __u32 in_num_buckets ) {
	_buckets = new hash_map_node *[ in_num_buckets ];
	_num_buckets = in_num_buckets;

	memset( _buckets, 0, sizeof(hash_map_node *) * in_num_buckets );
}


template<typename key_t, typename value_t> hash_map<key_t, value_t>::~hash_map() {
	__u32				i;
	hash_map_node	   *p, *q;

	for ( i = 0; i < _num_buckets; ++i ) {
		for ( p = _buckets[ i ]; p; p = q ) { q = p->_next; delete p; }
	}
	
	delete[] _buckets;
}


template<typename key_t, typename value_t> value_t &hash_map<key_t, value_t>::operator[]( key_t &in_key ) {
	hash_map_node	  **p;
	
	if ( ! *( p = hash( in_key ) ) ) *p = new hash_map_node( in_key );
	
	return (*p)->_value;
}


template<typename key_t, typename value_t> typename hash_map<key_t, value_t>::hash_map_node **hash_map<key_t, value_t>::hash( key_t &in_key ) const {
	hash_map_node	  **p;
	
	p = &_buckets[ in_key.hash() % _num_buckets ];

	for ( ; *p && ! ( (*p)->_key == in_key ); p = &(*p)->_next ) ;

	return p;
}


template<typename key_t, typename value_t> bool hash_map<key_t, value_t>::inserted( key_t &in_key, value_t *out_value ) const {
	hash_map_node	  **p;

	if ( *( p = hash( in_key ) ) ) {
		if ( out_value ) *out_value = (*p)->_value;

		return true;
	}
	
	return false;
}


} // namespace balance



#endif // __hash_map_h__
