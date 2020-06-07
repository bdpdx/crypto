#ifndef __common_headers_h__
#define __common_headers_h__



// for gcc use:  -include precompiled.h


#if _WIN32
	#include <Windows.h>

	#ifdef __cplusplus
		#define _CRT_SECURE_NO_WARNINGS			1

		#include <io.h>
		#include <process.h>
		#include <stdio.h>
		#include <tchar.h>

		#define fseeko( _a, _b, _c )			_fseeki64( ( _a ), (__int64) ( _b ), ( _c ) )
		#define ftello							( off_t ) _ftelli64
		#define snprintf						sprintf_s

		typedef int								mode_t;

		#include <iostream>
		#include <hash_map>
	#endif
#else
	#include <arpa/inet.h>
	#include <dirent.h>
	#include <getopt.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <pthread.h>
	#include <pwd.h>
	#include <regex.h>
	#include <sched.h>
	#include <semaphore.h>
	#include <termios.h>
	#include <sys/ioctl.h>
	#include <sys/mman.h>
	#include <sys/mount.h>
	#include <sys/param.h>
	#include <sys/resource.h>
	#include <sys/socket.h>
	#include <sys/syslog.h>
	#include <sys/time.h>
	#include <sys/wait.h>
	#include <unistd.h>
	#if ( __MACH__ || __linux__ )
		#include <paths.h>
		#include <sys/ptrace.h>
		#include <sys/sysctl.h>
	#endif
#endif

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#if __MACH__
	#include <machine/endian.h>
	#include <mach/mach_error.h>
	#include <mach/mach_host.h>
	#include <mach/mach_types.h>
	#include <mach/task.h>
	#include <mach/thread_act.h>
	#include <mach/vm_map.h>
	#include <mach-o/dyld.h>
	#include <sys/disk.h>
	#include <sys/ucred.h>
	#include <sys/un.h>

	#define __CF_USE_FRAMEWORK_INCLUDES__		1	// for mac headers
	#ifndef __HFSVOLUMES__
		#define __HFSVOLUMES__					1
	#endif // __HFSVOLUMES__

	#include <Carbon/Carbon.h>
	#include <CoreFoundation/CoreFoundation.h>
	#include <CoreServices/CoreServices.h>

	#undef __HFSVOLUMES__
	
	#include <hfs/hfs_encodings.h>
	#include <hfs/hfs_format.h>
	#include <hfs/hfs_mount.h>
	
	#if __OBJC__
		#import <Cocoa/Cocoa.h>
		#import <ExceptionHandling/NSExceptionHandler.h>
	#endif // __OBJC__
#endif // __MACH__

#if __linux__
	#include <endian.h>
#endif

#ifndef BIG_ENDIAN
	#define BIG_ENDIAN							4321
#endif

#ifndef LITTLE_ENDIAN
	#define LITTLE_ENDIAN						1234
#endif

#ifndef BYTE_ORDER
	#define BYTE_ORDER							LITTLE_ENDIAN
#endif


#include "common_types.h"
// #include "common_errors.h"

#include "common_debug.h"
#include "common_exceptions.h"
#include "common_macros.h"
#include "common_macros_mm.h"


#ifdef __cplusplus
extern "C" {
#endif

extern const char							   *s_application_name;

#ifdef __cplusplus
}
#endif


#ifndef __packed
	#if __GNUC__
		#define __packed						__attribute__((__packed__))
	#endif
#endif


#endif // __common_headers_c__
