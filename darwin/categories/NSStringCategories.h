#import <Foundation/NSString.h>


@interface NSString( bdCategories )

- (char *) new_c_string;								// calls new_c_string: true
- (char *) new_c_string:(bool) return_nil_if_empty;

@end
