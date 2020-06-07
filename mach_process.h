#ifndef __mach_process_h__
#define __mach_process_h__



#include <sys/types.h>


#ifdef __cplusplus
extern "C" {
	pid_t get_application_pid( const char *in_application_name, pid_t in_ignore_pid = 0 );
}
#else
	pid_t get_application_pid( const char *in_application_name, pid_t in_ignore_pid );
#endif



#endif // __mach_process_h__
