#include "regex.h"


regex::regex() {
	_match_count = 0;
	_matches = nil;
	_pcre = nil;
	_pcre_extra = nil;
}


regex::regex( const char *in_regex, int in_options ) {
	_match_count = 0;
	_matches = nil;
	_pcre = nil;
	_pcre_extra = nil;
	
	set( in_regex, in_options );
}


regex::~regex() {
	free_matches();
	pcre_free( _pcre );
	if ( _pcre_extra ) pcre_free( _pcre_extra );
}


void regex::free_matches() {
	if ( _matches ) {
		while ( _match_count-- ) free( _matches[ _match_count ] );
		free( _matches );
		_matches = nil;
	}
}


int regex::match( const char *in_string ) {
	return match( in_string, strlen( in_string ) );
}


int regex::match( const char *in_string, int in_length ) {
	int					err, i, match[ ( 256 / 2 ) * 3 ], n, start;

	free_matches();

	err = pcre_exec( _pcre, _pcre_extra, in_string, in_length, 0, 0, match, sizeof(match) / sizeof(int) );

	if ( err == PCRE_ERROR_NOMATCH ) return 0;
	if ( err < 0 ) _throw_msg( err, "pcre_exec() returned %d", err );

	if ( err == 1 ) {
		_match_count = 1;
		start = 0;
	} else {
		_match_count = err - 1;
		start = 2;
	}

	if ( ! ( _matches = (char **) malloc( n = sizeof(char *) * _match_count ) ) ) _throw( err_mem_full );
	
	memset( _matches, 0, n );
	
	_try {
		for ( i = 0; i < _match_count; ++i, start += 2 ) {
			if ( ! ( _matches[ i ] = (char *) malloc( ( n = match[ start + 1 ] - match[ start ] ) + 1 ) ) ) _throw( err_mem_full );
			memcpy( _matches[ i ], in_string + match[ start ], n );
			_matches[ i ][ n ] = 0;
		}
	} _catch
	
	_if_err {
		while ( i-- ) free( _matches[ i ] );
		free( _matches );
		_matches = nil;
	}
	
	_return _match_count;
}


char *regex::operator[]( int in_match ) {
	if ( ! _matches || in_match >= _match_count ) _throw( err_range );

	return _matches[ in_match ];
}


void regex::set( const char *in_regex, int in_options ) {
	const char			   *error;
	int						error_offset;

	if ( _pcre ) pcre_free( _pcre );
	if ( _pcre_extra ) { pcre_free( _pcre_extra ); _pcre_extra = nil; }

	if ( ! ( _pcre = pcre_compile( in_regex, in_options, &error, &error_offset, nil ) ) ) {
		_throw_msg( err_fubar, "pcre error \"%s\" at offset %d", error, error_offset );
	}
	
	_pcre_extra = pcre_study( _pcre, 0, &error );
}
