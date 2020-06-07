#import "NSBezierPathCategories.h"


@implementation NSBezierPath( RoundedRectangle )


+ (void) fillRoundedRectInRect:(NSRect) inRect cornerRadius:(float) inRadius {
	NSBezierPath	   *path;
	
	path = [NSBezierPath bezierPathWithRoundedRectInRect: inRect cornerRadius: inRadius];
	[path fill];
}


+ (void) strokeRoundedRectInRect:(NSRect) inRect radius:(float) inRadius {
	NSBezierPath	   *path;
	
	path = [NSBezierPath bezierPathWithRoundedRectInRect: inRect cornerRadius: inRadius];
	[path stroke];
}


+ (NSBezierPath *) bezierPathWithRoundedRectInRect:(NSRect) inRect cornerRadius:(float) inRadius {
	NSBezierPath			   *path;
	NSRect						rect;
	
	path = [self bezierPath];
	inRadius = MIN( inRadius, 0.5f * MIN( NSWidth( inRect ), NSHeight( inRect ) ) );
	rect = NSInsetRect( inRect, inRadius, inRadius );
	
	[path appendBezierPathWithArcWithCenter: NSMakePoint( NSMinX( rect ), NSMinY( rect ) )
		radius: inRadius startAngle: 180.0 endAngle: 270.0];
	[path appendBezierPathWithArcWithCenter: NSMakePoint( NSMaxX( rect ), NSMinY( rect ) )
		radius: inRadius startAngle: 270.0 endAngle: 360.0];
	[path appendBezierPathWithArcWithCenter: NSMakePoint( NSMaxX( rect ), NSMaxY( rect ) )
		radius: inRadius startAngle: 0.0 endAngle: 90.0];
	[path appendBezierPathWithArcWithCenter: NSMakePoint( NSMinX( rect ), NSMaxY( rect ) )
		radius: inRadius startAngle: 90.0 endAngle: 180.0];
	[path closePath];

	return path;
}


@end
