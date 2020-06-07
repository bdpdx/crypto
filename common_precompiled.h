#ifndef __precompiled_h__
#define __precompiled_h__



#if ! DEBUG
	#define ENABLE_OBFUSCATION				1
	#undef  NDEBUG
	#define NDEBUG							1		// removes assert() macros
#endif


#include "common_headers.h"


#ifdef __cplusplus
extern "C" {
#endif

extern const char						   *s_application_name;
extern __s32								s_os_version;

#ifdef __cplusplus
}
#endif


void application_main( int in_argc, const char *in_argv[] );


#include "precompiled_project.h"


#if __MACH__
	#if INCLUDE_CARBON
		#include <Carbon/Carbon.h>
	#endif

	#if INCLUDE_QUICKTIME
		#include <QuickTime/QuickTime.h>
	#endif

	#if __OBJC__ && INCLUDE_COCOA_CATEGORIES
		#import "AppKitCategories.h"
		#import "FoundationCategories.h"
	#endif
#endif


#include "common_errors.h"


#if ENABLE_GLOBAL_LOG
	#include "log.h"
#endif


#include "common_support.h"



#endif // __precompiled_h__
