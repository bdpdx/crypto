// When compiling under gcc/Xcode add -x objective-c++ to
// enable objective-c compilation (under xcode select the file,
// get-info, then add to additional compiler flags under the
// Build tab.

#if _WIN32
	#include "precompiled.h"
#endif


const char				   *s_application_name;
__s32						s_os_version;


#if __OBJC__ && ENABLE_COCOA_EXCEPTION_HANDLER
static void uncaught_exception_handler( NSException *inException ) {
	if ( [inException respondsToSelector: @selector(printStackTrace)] ) {
		[inException printStackTrace];
	}

	exit( err_unspecified_exception );
}
#endif // __OBJC__


static void application_init() {
#if ! WIN32
	rlimit			rl_core = { 0, 0 };

	srandomdev();
	// prevent the application from core dumping
	_throw_errno_if( setrlimit( RLIMIT_CORE, &rl_core ) == -1 );
#endif

	errno = 0;
	
#if ! DEBUG && __MACH__
	// in release builds prevent other processes from debugging our app
	_throw_errno_if( ptrace( PT_DENY_ATTACH, getpid(), nil, 0 ) == -1 && errno );
#endif

#if __MACH__
	Gestalt( gestaltSystemVersion, (long *) &s_os_version );
#endif

#if __OBJC__ && ENABLE_COCOA_EXCEPTION_HANDLER
	NSSetUncaughtExceptionHandler( uncaught_exception_handler );

	[[NSExceptionHandler defaultExceptionHandler] setExceptionHandlingMask: (
		NSHandleUncaughtExceptionMask | NSHandleUncaughtSystemExceptionMask |
		NSHandleUncaughtRuntimeErrorMask | NSHandleTopLevelExceptionMask |
		NSHandleOtherExceptionMask )];
#endif
}


#if ! USE_ALTERNATE_APPLICATION_MAIN

void application_main( int in_argc, const char *in_argv[] ) {
#if __OBJC__
	NSApplicationMain( in_argc, in_argv );
#endif
}

#else

void application_main( int in_argc, const char *in_argv[] );

#endif // USE_ALTERNATE_APPLICATION_MAIN


#pragma mark -


int main( int in_argc, const char *in_argv[] ) {
	char					c;

	static char				s_copyright_notice[] = "Copyright (c) 2008 Balance Software Corporation - All Rights Reserved";

	// ensure the copyright notice is included in the binary
	// and suppress gcc's unused variable warning for c.
	++( c = *s_copyright_notice );

	_try {
		application_init();
		
		if ( ( s_application_name = strrchr( in_argv[ 0 ], '/' ) ) ) ++s_application_name;
		else s_application_name = in_argv[ 0 ];

#if __OBJC__ && USE_ALTERNATE_APPLICATION_MAIN
		NSAutoreleasePool  *pool = nil;
		
		try {
			pool = [[NSAutoreleasePool alloc] init];
#endif

#ifdef __BALANCE_PRIVATE_APP_NOT_FOR_PUBLIC_DISTRIBUTION__
			printf( "*********************************************************\n" );
			printf( "*                                                       *\n" );
			printf( "*         !!! BALANCE SOFTWARE CONFIDENTIAL !!!         *\n" );
			printf( "*                                                       *\n" );
			printf( "*  This application is intended for use within Balance  *\n" );
			printf( "*  Software ONLY.  Any distribution outside Balance     *\n" );
			printf( "*  Software is prohibited.                              *\n" );
			printf( "*                                                       *\n" );
			printf( "*********************************************************\n\n" );
#endif // __BALANCE_PRIVATE_APP_NOT_FOR_PUBLIC_DISTRIBUTION__

			application_main( in_argc, in_argv );
#if __OBJC__ && USE_ALTERNATE_APPLICATION_MAIN
		} _catch

		[pool release];
		
		_throw_now();
#endif
	} _catch

	_if_err console( "application caught %d from %s:%u in main(), aborting", _err, _ex.file, _ex.line );
	
	return _err;
}
