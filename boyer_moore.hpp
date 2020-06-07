#ifndef __boyer_moore_hpp__
#define __boyer_moore_hpp__



#include "balance_types.h"


#define k_call_strlen_on_in_pattern		-1
#define k_default_alphabet_size			256


class boyer_moore {

public:

	boyer_moore();
	boyer_moore( char *in_pattern, __s32 in_pat_len = k_call_strlen_on_in_pattern, __s32 in_alphabet_size = k_default_alphabet_size ) { init( in_pattern, in_pat_len, in_alphabet_size ); }
	
   ~boyer_moore();

	void	init( char *in_pattern, __s32 in_pat_len = k_call_strlen_on_in_pattern, __s32 in_alphabet_size = k_default_alphabet_size );

	char   *search( char *in_text, __s32 in_text_len );

protected:

	__s32			m_pat_len;			// the pattern length
	char		   *m_pattern;			// the pattern to search for
	__s32		   *m_gs_table;			// the "good suffix" table for partial matches
	__s32		   *m_bc_table;			// the "bad character" table for character mismatch offsets

};



#endif // __boyer_moore_hpp__
