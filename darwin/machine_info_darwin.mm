/**----------------------------------------------------------------------------------------*\

	! BALANCE SOFTWARE CONFIDENTIAL !
	
	Copyright (c) 2006 Balance Software Corporation
	All Rights Reserved.
	
	NOTICE:
	
		All information contained herein is, and remains the property of,
		Balance Software Corporation and its suppliers, if any.  The
		intellectual and technical concepts contained herein are proprietary
		to Balance Software Corporation and its suppliers and may be covered
		by U.S. and foreign patents or patents in process, and are protected
		by trade secret and copyright law.  Dissemination of this information,
		reproduction, or use of this material, whether in whole or in part,
		is strictly forbidden unless prior permission is obtained in writing
		from a duly authorized officer of Balance Software Corporation.

	LICENSE NOTICE:

	The code in this file and its corresponding .h file is licensed to iovation
	for use in its Mac DevicePrint SDK and is not to be released to third parties.
	
		
	File:				machine_info_darwin.mm

	Author:				Brian Doyle
	Last Modified:		May 17, 2006

	Description:

	These methods return information about the localhost culled from the
	IORegistry and other places.
	
	Each function in this file mallocs() a string large enough to store the result
	and returns this string upon success.  It is the caller's responsibility to
	free() this string.  On failure, NULL is returned.
	
	Passing true for in_display_title will cause an English language string that
	describes the returned value to be prepended to the result.

\**---------------------------------------------------------------------------------------*/


#if __OBJC__


#include <Cocoa/Cocoa.h>
#include <stdio.h>


char *operating_system_version( bool in_display_title, bool in_create_NSAutoReleasePool ) {
	NSAutoreleasePool	   *pool = nil;
	NSString			   *string;
	char				   *result = nil;
	
	if ( in_create_NSAutoReleasePool ) pool = [[NSAutoreleasePool alloc] init];

	if ( ( string = [[NSProcessInfo processInfo] operatingSystemVersionString] ) ) {
		asprintf( &result, "%s%s", in_display_title ? "Operating System Version: " : "", [string UTF8String] );
	}

	[pool release];
	
	return result;
}


#endif	// __OBJC__
