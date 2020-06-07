@interface NSBezierPath( RoundedRectangle )

//  Returns a closed bezier path describing a rectangle with curved corners
//  The corner inRadius will be trimmed to not exceed half of the lesser rectangle dimension.
+ (NSBezierPath *) bezierPathWithRoundedRectInRect:(NSRect) inRect cornerRadius: (float) inRadius;
+ (void) fillRoundedRectInRect:(NSRect) inRect cornerRadius:(float) inRadius;
+ (void) strokeRoundedRectInRect:(NSRect) inRect radius:(float) inRadius;

@end
