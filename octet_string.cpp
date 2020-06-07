#include <string.h>

#include "octet_string.h"


#define zdelete( ptr, len ) do {								\
	if ( m_own_data && ptr ) {									\
		if ( m_zero_data_on_release ) memset( ptr, 0, len );	\
		delete[] ptr;											\
		ptr = nil;												\
	}															\
} while ( 0 )


octet_string::octet_string( const octet_string &in_os ) {
	__u32		i;

	m_zero_data_on_release = in_os.m_zero_data_on_release;

	if ( ! ( m_length = in_os.m_length ) ) {
		m_data = nil;
	} else {
		m_data = new __u8[ in_os.m_length ];
		for ( i = 0; i < in_os.m_length; ++i ) m_data[ i ] = in_os.m_data[ i ];
	}

	m_own_data = true;
}


octet_string::~octet_string() {
	zdelete( m_data, m_length );
}


octet_string &octet_string::operator=( __u32 in_integer ) {
	__u8				   *p;

	p = new __u8[ sizeof(__u32) ];
	
	*reinterpret_cast<__u32 *>(p) = big32( in_integer );
	
	zdelete( m_data, m_length );
	
	m_data = p;
	m_length = sizeof(__u32);
	m_own_data = true;
	
	return *this;
}


octet_string &octet_string::operator=( __u64 in_integer ) {
	__u8				   *p;

	p = new __u8[ sizeof(__u64) ];
	
	*reinterpret_cast<__u64 *>(p) = big64( in_integer );
	
	zdelete( m_data, m_length );
	
	m_data = p;
	m_length = sizeof(__u64);
	m_own_data = true;

	return *this;
}


octet_string &octet_string::operator=( const char *in_string ) {
	__u8	   *p;
	__u32		i, l;

	p = m_data;
	l = m_length;
	
	if ( ! in_string || ! *in_string ) {
		m_length = 0;
		m_data = nil;
	} else {
		for ( i = 0; in_string[ i ]; ++i ) ;
		m_data = new __u8[ m_length = i + 1 ];
		for ( i = 0; i < m_length; ++i ) m_data[ i ] = in_string[ i ]; 
	}
	
	zdelete( p, l );
	
	m_own_data = true;
	
	return *this;
}


octet_string &octet_string::operator=( const octet_string &in_rhs ) {
	if ( this == &in_rhs ) return *this;
	
	__u8	   *p;
	__u32		i, l;
	
	m_zero_data_on_release = in_rhs.m_zero_data_on_release;
	
	if ( ! ( in_rhs.m_length || in_rhs.m_data ) ) {
		zdelete( m_data, m_length );
		m_data = nil;
		m_length = 0;
	} else {
		p = m_data;
		l = m_length;
		m_data = new __u8[ in_rhs.m_length ];
		m_length = in_rhs.m_length;
		zdelete( p, l );
		
		for ( i = 0; i < m_length; ++i ) m_data[ i ] = in_rhs.m_data[ i ];
	}
	
	m_own_data = true;
	
	return *this;
}


octet_string &octet_string::operator+=( const octet_string &in_rhs ) {
	__u8	   *p;
	__u32		i, j, l;
	
	p = m_data;
	l = m_length;
	m_data = new __u8[ m_length + in_rhs.m_length ];
	
	for ( i = 0; i < m_length; ++i ) m_data[ i ] = p[ i ];
	
	zdelete( p, l );
	
	m_own_data = true;
	
	for ( j = 0, m_length += in_rhs.m_length; i < m_length; ++i, ++j ) m_data[ i ] = in_rhs.m_data[ j ];
	
	return *this;
}


// states are:

// copy: yes, own: no : meaningless (invalid)
// copy: yes, own: yes: copy implies own
// copy: no,  own: yes: assume pointer as own, delete when this is released
// copy: no,  own: no : use pointer but do nothing with it on release
octet_string &octet_string::set( const __u8 *in_data, __u32 in_length, bool in_copy_data, bool in_own_data ) {
	__u8	   *p;
	__u32		i, l;

	p = m_data;
	l = m_length;

	if ( ! ( in_data && in_length ) ) {
		m_data = nil;
		m_length = 0;
	} else if ( ! in_copy_data ) {
		m_data = const_cast<__u8 *>(in_data);
		m_length = in_length;
	} else {
		m_data = new __u8[ in_length ];
		m_length = in_length;

		for ( i = 0; i < in_length; ++i ) m_data[ i ] = in_data[ i ];
	}

	zdelete( p, l );
	
	m_own_data = in_own_data;
	
	return *this;
}


octet_string &octet_string::set_from_hex_string( const char *in_hex_string ) {
	__u8				c, d;
	__u32				i, j, l, n;
	__u8			   *p;

	if ( ( n = (__u32) strlen( in_hex_string ) ) & 1 ) _throw( err_bad_data );

	p = new __u8[ l = n / 2 ];

	_try {
		for ( i = j = 0; i < n; ) {
			c = in_hex_string[ i++ ];
			
			if ( c >= '0' && c <= '9' ) c -= '0';
			else if ( c >= 'a' && c <= 'f' ) c = c - 'a' + 10;
			else if ( c >= 'A' && c <= 'F' ) c = c - 'A' + 10;
			else _throw( err_bad_data );

			d = in_hex_string[ i++ ];
			
			if ( d >= '0' && d <= '9' ) d -= '0';
			else if ( d >= 'a' && d <= 'f' ) d = d - 'a' + 10;
			else if ( d >= 'A' && d <= 'F' ) d = d - 'A' + 10;
			else _throw( err_bad_data );
			
			p[ j++ ] = c << 4 | d;
		}
	} _catch
	
	_if_err {
		delete[] p;
	} else {
		zdelete( m_data, m_length );
		
		m_data = p;
		m_length = l;
		m_own_data = true;
	}

	_return *this;
}


char *octet_string::as_hex() const {
	if ( ! ( m_length && m_data ) ) return nil;
	
	__u32		i;
	char	   *result;
	
	result = new char[ m_length * 2 + 1 ];

	for ( i = 0; i < m_length; ++i ) { hexify( m_data[ i ], result[ i * 2 ], result[ i * 2 + 1 ] ); }

	result[ i * 2 ] = 0;

	return result;
}


char *octet_string::as_c_string() const {
	if ( ! ( m_length && m_data ) ) return nil;
	
	char	   *p;
	__u32		i, n;
	
	for ( n = 0; n < m_length && m_data[ n ]; ++n ) ;
	p = new char[ n + ( n == m_length ) ];
	for ( i = 0; i < n; ++i ) p[ i ] = m_data[ i ];
	if ( n == m_length ) p[ n ] = 0;

	return p;	
}
