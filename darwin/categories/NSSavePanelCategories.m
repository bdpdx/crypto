#import "NSSavePanelCategories.h"


@interface NSSavePanel( bdCategoriesPrivate )

- (NSTextField *) filename_field_private:(NSView *) in_view;

@end


@implementation NSSavePanel( bdCategories )


- (NSTextField *) filename_field {
	return [self filename_field_private: [self contentView]];
}


- (NSTextField *) filename_field_private:(NSView *) in_view {
	id				z;
	NSArray		   *sv;
	__u32			i, j;
	NSTextField	   *result = nil;	
	
	sv = [in_view subviews];
	
	for ( i = 0, j = [sv count]; ! result && i < j; ++i ) {
		z = [sv objectAtIndex: i];
		
		if ( [z isKindOfClass: [NSTextField class]] && [z isEditable] ) return z;
		if ( [z isKindOfClass: [NSView class]] ) result = [self filename_field_private: z];
	}
	
	return result;
}


@end
