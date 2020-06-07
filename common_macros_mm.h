#ifndef __common_macros_mm_h__
#define __common_macros_mm_h__
#if __OBJC__



#define LocalizedString( _in_key )												\
	[[NSBundle mainBundle] localizedStringForKey: @_in_key value: @"" table: nil]

#define NotificationIgnore( _in_type, _in_object )								\
	[[NSNotificationCenter defaultCenter] removeObserver: self name: _in_type object: _in_object]
#define NotificationPost( _in_type )											\
	[[NSNotificationCenter defaultCenter] postNotificationName: _inType object: nil]
#define NotificationRegister( _in_type, _in_handler, _in_object )				\
	[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(_in_handler) name: _in_type object: _in_object]

#define DistributedNotificationIgnore( _in_type )								\
	[[NSDistributedNotificationCenter defaultCenter] removeObserver: self name: _in_type object: nil]
#define DistributedNotificationPost( _in_type )									\
	[[NSDistributedNotificationCenter defaultCenter] postNotificationName: _in_type object: @"" userInfo: nil deliverImmediately: NO]
#define DistributedNotificationRegister( _in_type, _in_handler, _in_object )	\
	[[NSDistributedNotificationCenter defaultCenter] addObserver: self selector: @selector(_in_handler) name: _in_type object: nil suspensionBehavior: NSNotificationSuspensionBehaviorCoalesce]



#endif // __OBJC__
#endif // __common_macros_mm_h__
