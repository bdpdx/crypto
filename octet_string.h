#ifndef __octet_string_h__
#define __octet_string_h__



#if _WIN32
	#include "precompiled.h"
#endif

#include "base64.h"


class octet_string {

public:

	octet_string();
	octet_string( __u32 in_integer );										// implicit big-endian conversion
	octet_string( __u64 in_integer );										// implicit big-endian conversion
	octet_string( const octet_string &in_os );
	octet_string( const char *in_string, bool in_is_hex_string = false );	// terminating null byte is copied
// states are:

// copy: yes, own: no : meaningless (invalid)
// copy: yes, own: yes: make copy of in_data (implies ownership)
// copy: no,  own: yes: own in_data directly (no copy), delete when this is released
// copy: no,  own: no : use in_data but do not delete it when this is released
	octet_string( const __u8 *in_data, __u32 in_length, bool in_copy_data = true, bool in_own_data = true );
	
   ~octet_string();

	octet_string &operator=( __u32 in_integer );							// implicit big-endian conversion
	octet_string &operator=( __u64 in_integer );							// implicit big-endian conversion
	octet_string &operator=( const char *in_string );						// terminating null byte is copied
	octet_string &operator=( const octet_string &in_rhs );
	octet_string &operator+=( const octet_string &in_rhs );

// states are:

// copy: yes, own: <ignored>:	copy implies own
// copy: no,  own: yes:			assume pointer as own, delete when this is released
// copy: no,  own: no :			use pointer but do nothing with it on release
	octet_string &set( const __u8 *in_data = nil, __u32 in_length = 0, bool in_copy_data = true, bool in_own_data = true );
	octet_string &set_from_hex_string( const char *in_hex_string );

	octet_string &append( const __u8 *in_data, __u32 in_length );

	// returns the octet-string as a base-64 c-string.  caller is
	// responsible for delete[]ing the returned string.
	char *as_base64( base64_encoding in_encoding = k_base64_default_encoding ) const;

	// scans the octet string for the first instance of a zero byte and returns
	// a null-terminated c-string to that point (or the entire m_data string plus
	// an appended terminating null if no zero byte was found).  caller is
	// responsible for delete[]ing the returned array.
	char *as_c_string() const;

	// returns the octet-string as a hexadecimal c-string.  caller is
	// responsible for delete[]ing the returned string.
	char *as_hex() const;

	const __u8 *data() const;
	
	// returns the length of m_data
	__u32 length() const;

	// releases any stored memory (if applicable) and sets the state to default
	void reset() { set( nil, 0 ); }

	// returns m_data for the caller, sets m_data to nil and length to zero
	__u8 *take_data();

	// set this to ensure that any time m_data is released it is zeroed first
	void set_zero_data_on_release( bool in_zero = true );

	// returns whether or not zeroing released data is on
	bool zero_data_on_release() const;

	friend bool operator==( octet_string &in_lhs, octet_string &in_rhs );

protected:

	__u8	   *m_data;
	__u32		m_length;

	__u32		m_own_data					: 1;
	__u32		m_zero_data_on_release		: 1;
	
};


inline octet_string::octet_string() {
	m_data = nil;
	m_length = 0;
	m_own_data = true;
	m_zero_data_on_release = false;
}


inline octet_string::octet_string( __u32 in_integer ) {
	m_data = nil;
	m_length = 0;
	m_own_data = true;
	m_zero_data_on_release = false;
	
	*this = in_integer;
}


inline octet_string::octet_string( __u64 in_integer ) {
	m_data = nil;
	m_length = 0;
	m_own_data = true;
	m_zero_data_on_release = false;
	
	*this = in_integer;
}


inline octet_string::octet_string( const char *in_string, bool in_is_hex_string ) {
	m_data = nil;
	m_zero_data_on_release = false;

	if ( ! in_is_hex_string ) *this = in_string;
	else set_from_hex_string( in_string );
}


inline octet_string::octet_string( const __u8 *in_data, __u32 in_length, bool in_copy_data, bool in_own_data ) {
	m_data = nil;
	m_zero_data_on_release = false;

	set( in_data, in_length, in_copy_data, in_own_data );
}


#pragma mark -


inline octet_string &octet_string::append( const __u8 *in_data, __u32 in_length ) {
	return *this += octet_string( in_data, in_length, false, false );
}


inline char *octet_string::as_base64( base64_encoding in_encoding ) const {
	return base64::encode( m_data, m_length, in_encoding );
}


inline const __u8 *octet_string::data() const {
	return m_data;
}


inline __u32 octet_string::length() const {
	return m_length; 
}


inline void octet_string::set_zero_data_on_release( bool in_zero ) {
	m_zero_data_on_release = in_zero;
}


inline __u8 *octet_string::take_data() {
	__u8	   *result = m_data;
	
	m_data = nil;
	m_length = 0;
	
	return result;
}


inline bool octet_string::zero_data_on_release() const {
	return m_zero_data_on_release;
}


#pragma mark -


inline bool operator==( octet_string &in_lhs, octet_string &in_rhs ) {
	__u32					i, n;

	if ( ( i = in_lhs.m_length ) != ( n = in_rhs.m_length ) ) return false;
	
	for ( i = 0; i < n; ++i ) {
		if ( in_lhs.m_data[ i ] != in_rhs.m_data[ i ] ) return false;
	}
	
	return true;
}


inline bool operator!=( octet_string &in_lhs, octet_string &in_rhs ) {
	return ! ( in_lhs == in_rhs );
}


// for use with ASN.1 routines:

// void pop_os( asn1_ber_sequence &io_seq, octet_string *& );
#define pop_os( _seq, _onto ) do {												\
	( _onto ) = (octet_string *) _seq.pop( type, k_asn1_tag_octet_string );		\
} while ( 0 )



#endif //  __octet_string_h__
