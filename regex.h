#ifndef __regex_h__
#define __regex_h__
#ifdef __cplusplus



#include <pcre.h>


class regex {

public:

	regex();
	regex( const char *in_regex, int in_options = 0 );
   ~regex();

	int match( const char *in_string );
	int match( const char *in_string, int in_length );

	char *operator[]( int in_match );
	operator int() { return _matches ? _match_count : 0; }

	void free_matches();
	void set( const char *in_regex, int in_options = 0 );
	void set8( const char *in_regex, int in_options = 0 ) { set( in_regex, in_options | PCRE_UTF8 ); }

protected:

	int						_match_count;
	char				  **_matches;
	pcre				   *_pcre;
	pcre_extra			   *_pcre_extra;

};



#endif // __cplusplus
#endif // __regex_h__
