#import "NSApplicationCategories.h"


@implementation NSApplication( bdCategories )

- (void) reportException:(NSException *) inException {
	( *NSGetUncaughtExceptionHandler() )( inException );
}


@end
