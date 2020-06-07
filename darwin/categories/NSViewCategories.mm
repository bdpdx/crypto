#import "NSViewCategories.h"


@implementation NSView( bdCategories )


- (void) enableSubviews:(char *&) ioSubviewState {
	int				i, n;
	NSArray		   *subviews;

	if ( ioSubviewState ) {
		n = [( subviews = [self subviews] ) count];	

		for ( i = 0; i < n; ++i ) {
			if ( ioSubviewState[ i / 8 ] & 1 << 7 - i % 8 ) {
				[[subviews objectAtIndex: i] setEnabled: true ];
			}
		}

		delete[] ioSubviewState;
		ioSubviewState = nil;
	}
}


- (void) disableSubviews:(char *&) ioSubviewState {
	id				obj;
	int				i, n, o;
	NSArray		   *subviews;

	if ( ioSubviewState ) return;

	if ( ( n = [( subviews = [self subviews] ) count] ) ) {
		ioSubviewState = new char[ o = n / 8 + ( n % 8 ? 1 : 0 ) ];
		
		for ( i = 0; i < o; ++i ) ioSubviewState[ i ] = 0;
		for ( i = 0; i < n; ++i ) {
			if ( [( obj = [subviews objectAtIndex: i] ) respondsToSelector: @selector(setEnabled:)] ) {
				ioSubviewState[ i / 8 ] |= ( [obj isEnabled] ? 1 : 0 ) << 7 - i % 8;
				[obj setEnabled: false];
			}
		}
	}
}


- (void) enableAllSubviews:(BOOL) inEnable {
	id				obj;
	int				i, n;
	NSArray		   *subviews;

	n = [( subviews = [self subviews] ) count];

	for ( i = 0; i < n; ++i ) {
		if ( [( obj = [subviews objectAtIndex: i] ) respondsToSelector: @selector(setEnabled:)] ) {
			[obj setEnabled: inEnable];
		}
	}
}


- (void) enableAllSubviews { [self enableAllSubviews: true]; }
- (void) disableAllSubviews { [self enableAllSubviews: false]; }


- (void) removeSubviewsWithoutNeedingDisplay {
	NSView			   *view;
	NSEnumerator	   *enumerator;
	
	enumerator = [[self subviews] objectEnumerator];
	
	while ( ( view = [enumerator nextObject] ) ) {
		[view removeFromSuperviewWithoutNeedingDisplay];
	}
}


@end
