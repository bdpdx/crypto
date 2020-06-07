#include <stdlib.h>
#include <string.h>
#include <sys/sysctl.h>

#include "mach_process.h"


pid_t get_application_pid( const char * in_application_name, pid_t in_ignore_pid ) {
	int						argc, argmax, i, mib[ 3 ], n;
	char				   *args, *p, *q;
	struct kinfo_proc	   *procs;
	pid_t					result = 0;
	size_t					size;

	mib[ 0 ] = CTL_KERN;
	mib[ 1 ] = KERN_ARGMAX;
	
	size = sizeof(argmax);
	sysctl( mib, 2, &argmax, &size, NULL, 0 );	
	args = (char *) malloc( argmax );
	
	mib[ 1 ] = KERN_PROC;
	mib[ 2 ] = KERN_PROC_ALL;
	
	sysctl( mib, 3, NULL, &size, NULL, 0 );
	procs = (struct kinfo_proc *) malloc( size += size / 10 );
	sysctl( mib, 3, procs, &size, NULL, 0 );
	
	mib[ 1 ] = KERN_PROCARGS2;
	mib[ 2 ] = 0;
	
	for ( i = 0, n = size / sizeof(struct kinfo_proc); i < n; ++i ) {
		if ( ( mib[ 2 ] = procs[ i ].kp_proc.p_pid ) == in_ignore_pid ) continue;
		
		size = argmax;
		if ( sysctl( mib, 3, args, &size, NULL, 0 ) == -1 ) continue;
		
		argc = *(int *) args;
		
		// skip saved exec path
		for ( p = args + sizeof(argc); p < &args[ size ] && *p; ++p ) ;
		// skip additional trailing null characters
		for ( ++p; p < &args[ size ] && ! *p; ++p ) ;
		
		// p == argv[ 0 ]
		if ( ( q = strrchr( p, '/' ) ) ) ++q; else q = p;

		if ( ! strcmp( in_application_name, q ) ) {
			result = (pid_t) mib[ 2 ];
			break;
		}
	}
	
	free( args );
	free( procs );
	
	return result;
}
