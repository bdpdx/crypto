#ifdef __cplusplus


#import <AppKit/NSView.h>


@interface NSView( bdCategories )


// enables any subviews disabled by disable_subviews
- (void) enableSubviews:(char *&) ioSubviewState;

// records enabled state of all subviews then disables all subviews
- (void) disableSubviews:(char *&) ioSubviewState;

// enables all subviews (wrapper)
- (void) enableAllSubviews;

// disable all subviews (wrapper)
- (void) disableAllSubviews;

// the method that actually does the work for the previous two
- (void) enableAllSubviews:(BOOL) inEnable;

// removes all subviews without a redisplay
- (void) removeSubviewsWithoutNeedingDisplay;


@end


#endif // __cplusplus
