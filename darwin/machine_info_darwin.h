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

	File:				machine_info_darwin.h

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
#if ! defined( __machine_info_darwin_h__ ) && __MACH__
#define __machine_info_darwin_h__



enum ethernet_address_query_t {
	k_eaqt_non_built_in_only	,
	k_eaqt_built_in_only		,
	k_eaqt_all					,
};


// caller must free() returned strings
char *bus_speed( bool in_display_title = true );
char *cpu_count( bool in_display_title = true );
char *cpu_speed( bool in_display_title = true );
char *cpu_type( bool in_display_title = true );
char *machine_model( bool in_display_title = true );
char *machine_serial_number( bool in_display_title = true );
char *physical_memory( bool in_display_title = true );

// caller must free() each out_count char * pointer from result then free() result
char **ethernet_addresses( unsigned int &out_count, bool in_display_title = true, ethernet_address_query_t in_query = k_eaqt_built_in_only );
char **internal_block_devices( unsigned int &out_count, int in_display_title = true );



#endif // __machine_info_darwin_h__
