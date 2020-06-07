#import "NSStringCategories.h"


@implementation NSString( bdCategories )


- (char *) new_c_string {
	return [self new_c_string: true];
}


- (char *) new_c_string:(bool) return_nil_if_empty {
	char		   *result;
	__s32			length;

	if ( ( length = [self cStringLength] ) || ! return_nil_if_empty ) {
		result = new char [ length + 1 ];
		[self getCString: result];
	} else {
		result = nil;
	}
	
	return result;
}


@end
