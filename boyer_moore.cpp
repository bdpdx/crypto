#include <string.h>

#include "boyer_moore.hpp"
#include "useful_macros.h"


boyer_moore::boyer_moore() {
	m_bc_table = m_gs_table = nil;
}


boyer_moore::~boyer_moore() {
	delete[] m_bc_table;
	delete[] m_gs_table;
}


char *boyer_moore::search( char *in_text, __s32 in_text_len ) {
	__s32			i, j, k, l;

	for ( i = j = m_pat_len - 1; j < in_text_len && i >= 0; ) {
		if ( in_text[ j ] == m_pattern[ i ] ) { --i; --j; }
		else {
			k = m_gs_table[ i + 1 ];
			l = m_bc_table[ static_cast<unsigned char>(in_text[ j ]) ];

			j += max( k, l );
			
			i = m_pat_len - 1;
		}
	}
	
	return i < 0 ? in_text + j + 1 : nil;
}


void boyer_moore::init( char *in_pattern, __s32 in_pat_len, __s32 in_alphabet_size ) {
	__s32			i, j, k, *backup;

	m_pattern = in_pattern;
	in_pat_len = m_pat_len = in_pat_len == k_call_strlen_on_in_pattern ? strlen( in_pattern ) : in_pat_len;

	// generate the "bad character" table
	
	m_bc_table = new __s32[ in_alphabet_size ];
	
	for ( i = 0; i < in_alphabet_size; ++i ) m_bc_table[ i ] = in_pat_len;
	for ( i = 0; i < in_pat_len - 1; ++i ) m_bc_table[ static_cast<unsigned char>(in_pattern[ i ]) ] = in_pat_len - i - 1;
	
	// generate the "good suffix" table
	
	m_gs_table = new __s32[ 2 * sizeof(__s32) * ( in_pat_len + 1 ) ];
	
	backup = m_gs_table + in_pat_len + 1;
	
	for ( i = 1; i <= in_pat_len; ++i ) m_gs_table[ i ] = 2 * in_pat_len - i;
	for ( i = in_pat_len, j = in_pat_len + 1; i; --i, --j ) {
		backup[ i ] = j;

		while ( j <= in_pat_len && in_pattern[ i - 1 ] != in_pattern[ j - 1 ] ) {
			if ( m_gs_table[ j ] > in_pat_len - i ) m_gs_table[ j ] = in_pat_len - i;
			j = backup[ j ];	
		}
	}
	for ( i = 1; i <= j; ++i ) if ( m_gs_table[ i ] > in_pat_len + j - i ) m_gs_table[ i ] = in_pat_len + j - i;
	
	k = backup[ j ];

	for ( ; j <= in_pat_len; k = backup[ k ] ) {
		for ( ; j <= k; ++j ) if ( m_gs_table[ j ] >= k - j + in_pat_len ) m_gs_table[ j ] = k - j + in_pat_len;
	}
}
