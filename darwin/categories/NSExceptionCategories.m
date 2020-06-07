#import "NSExceptionCategories.h"


// requires ExceptionHandling.framework
@implementation NSException( bdCategories )


- (void) printStackTrace {
	char				   *buffer, *p;
	FILE				   *file;
	size_t					length;
	int						n, o;
	NSString			   *stackTrace, *string;

	stackTrace = [[self userInfo] objectForKey: NSStackTraceKey];
	string = [NSString stringWithFormat:
		@"/usr/bin/atos -p %d %@ | tail -n +3 | head -n +%d | c++filt | cat -n",
		[[NSProcessInfo processInfo] processIdentifier], stackTrace,
		( [[stackTrace componentsSeparatedByString: @"  "] count] - 4 )];

	buffer = (char *) malloc( o = 1024 );
	n = snprintf( buffer, o - 2, "An exception of type %s occured because %s.\n",
		[[self name] UTF8String], [[self reason] UTF8String] );
		
	if ( ( file = popen( [string UTF8String], "r" ) ) ) {
		while ( ( length = fread( buffer + n, 1, o - n - 1, file ) ) ) {
			if ( ( n += length ) == o - 1 ) {
				if ( ( p = (char *) realloc( buffer, o += 1024 ) ) ) buffer = p;
				else break;
			}
		}

		buffer[ n ] = 0;
		NSLog( @"%s", buffer );
		pclose( file );
	}
	
	free( buffer );
}


@end
