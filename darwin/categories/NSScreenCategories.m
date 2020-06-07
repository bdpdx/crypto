#import <Foundation/NSArray.h>
#import "NSScreenCategories.h"


@implementation NSScreen( bdCategories )


+ (NSRect) get_all_screens_rect {
	unsigned long	i;
	NSRect			frame;
	NSArray		   *screens = [NSScreen screens];
	
	frame = NSZeroRect;
	
	for ( i = 0; i < [screens count]; ++i ) {
		frame = NSUnionRect( frame, [[screens objectAtIndex: i] frame] );
	}
	
	return frame;
}


@end
