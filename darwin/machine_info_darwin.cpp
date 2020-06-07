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


/*
	information according to system_profiler
	
	hardware:
+		machine model
		cpu type, count, speed, cache
+		total physical memory
+		bus speed
+		boot rom version
+		serial number
	
	network:
+		primary ethernet mac address
		primary airport mac address
	
	ata:
+		internal optical drive model
	
	audio:
		codec, sample rate
	
	bluetooth:
		hardware manufacturer
		address
	
	graphics:
		chipset model, vendor, id
	
	pci cards:
+		vendor, id
	
	modem:
		model, modulation, sku name, hardware version
	
	
	
	
	information in device tree
	
	IODeviceTree Root device-tree
		cpus
			*
				name|cpu-device-type,clock-frequency,bus-frequency,d-cache-size
				*
					name,d-cache-size,i-cache-size,clock-frequency
		sep
			*|fans,power-supplies,temperatures
				*
					name|location,??sensor-id,??version
		pci*
			model
		vsp*
			model
		
		rom*
			boot-rom*
				model
	
	IOPower Root IORootParent IOPowerConnection IOPMrootDomain * IOClass=IOHWSensor
		name|location,type,??version
	
*/


#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetController.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/storage/IOMedia.h>
#include <sys/stat.h>
#include <sys/sysctl.h>

#include "machine_info_darwin.h"


typedef int					err_t;

#ifndef min
	#define min( _a, _b )	( ( _a ) < ( _b ) ? ( _a ) : ( _b ) )
#endif



static kern_return_t IOServiceGetMatchingServicesFixed( mach_port_t inMasterPort, CFDictionaryRef ioMatching, io_iterator_t *outIterator ) {
	// IOServiceGetMatchingServices() is supposed to release the ioMatching parameter.
	// However, on OS versions < 10.4 there is a documented bug that may cause this
	// behavior to fail, thus some extra logic here to make sure ioMatching is released.
	kern_return_t			err;
	CFIndex					n;
	
	CFRetain( ioMatching );
	n = CFGetRetainCount( ioMatching );
	
	err = IOServiceGetMatchingServices( inMasterPort, ioMatching, outIterator );
	
	if ( CFGetRetainCount( ioMatching ) == n ) CFRelease( ioMatching );
	CFRelease( ioMatching );
	
	return err;
}
#define IOServiceGetMatchingServices	IOServiceGetMatchingServicesFixed


io_registry_entry_t IORegistryEntryFromAlias( mach_port_t inPort , const char *inAlias ) {
	io_registry_entry_t			aliases = 0;
	
	CFStringRef					string = NULL;
	CFDataRef					data = NULL;
	char						buffer[128];
	
	aliases = IORegistryEntryFromPath( inPort , kIODeviceTreePlane ":/aliases" );
	
	if ( aliases ) {
		string = CFStringCreateWithCString( NULL , inAlias , kCFStringEncodingUTF8 );
		data = (CFDataRef)IORegistryEntryCreateCFProperty( aliases , string , 0 , 0 );
		
		IOObjectRelease( aliases );
		CFRelease( string );
	}
	
	if ( data ) {
		snprintf( buffer , sizeof(buffer) , "%s:%s" , kIODeviceTreePlane , CFDataGetBytePtr( data ) );
		
		CFRelease( data );
	} else {
		snprintf( buffer , sizeof(buffer) , "%s:/%s" , kIODeviceTreePlane , inAlias );
	}
	
	return IORegistryEntryFromPath( inPort , buffer );
}


#pragma mark -


char *bus_speed( bool in_display_title ) {
	err_t					err = 0;
	UInt32					n;
	UInt64					o;
	char				   *result = nil;

	if ( sysctlbyname( "hw.busfrequency_max", &o, &( n = sizeof(o) ), nil, 0 ) == -1 ) err = errno;
	if ( ! err && n != sizeof(o) ) err = EINVAL;
	if ( ! err ) asprintf( &result, "%s%llu%s", in_display_title ? "Bus Speed: " : "", o, in_display_title ? " Hz" : "" );

	return result;
}


char *cpu_count( bool in_display_title ) {
	err_t					err = 0;
	UInt32					n, o;
	char				   *result = nil;

	if ( sysctlbyname( "hw.physicalcpu", &o, &( n = sizeof(o) ), nil, 0 ) == -1 ) err = errno;
	if ( ! err && n != sizeof(o) ) err = EINVAL;
	if ( ! err ) asprintf( &result, "%s%u", in_display_title ? "CPU Count: " : "", o );

	return result;
}


char *cpu_speed( bool in_display_title ) {
	err_t					err = 0;
	UInt32					n;
	UInt64					o;
	char				   *result = nil;

	if ( sysctlbyname( "hw.cpufrequency_max", &o, &( n = sizeof(o) ), nil, 0 ) == -1 ) err = errno;
	if ( ! err && n != sizeof(o) ) err = EINVAL;
	if ( ! err ) asprintf( &result, "%s%llu%s", in_display_title ? "CPU Speed: " : "", o, in_display_title ? " Hz" : "" );

	return result;
}


char *cpu_type( bool in_display_title ) {
	char					c, *result = nil, *p;
	CFDataRef				data;
	CFMutableDictionaryRef	dictionary;
	err_t					err = 0;
	int						found = 0;
	bool					has_version = false;
	io_iterator_t			iterator;
	io_object_t				object;
	CFStringRef				string;
	UInt8					version_major, version_minor;

	if ( ! ( dictionary = IOServiceMatching( "IOPlatformDevice" ) ) ) err = ENOENT;
	if ( ! err ) err = IOServiceGetMatchingServices( kIOMasterPortDefault, dictionary, &iterator );
	if ( ! err ) {
		while ( ! found && ( object = IOIteratorNext( iterator ) ) ) {
			err = IORegistryEntryCreateCFProperties( object, &dictionary, kCFAllocatorDefault, 0 );
			if ( ! err ) {
				if ( ( data = (CFDataRef) CFDictionaryGetValue( dictionary, CFSTR( "device_type" ) ) ) ) {
					if ( ! ( string = CFStringCreateWithCString( kCFAllocatorDefault, (const char *) CFDataGetBytePtr( data ), kCFStringEncodingASCII ) ) ) err = ENOMEM;
					if ( ! err ) {
						if ( ! CFStringCompare( string, CFSTR( "cpu" ), 0 ) ) found = 1;			// powerpc
						if ( ! CFStringCompare( string, CFSTR( "processor" ), 0 ) ) found = 2;		// intel
						CFRelease( string );
					}
				}

				if ( found == 1 ) {
					if ( ( data = (CFDataRef) CFDictionaryGetValue( dictionary, CFSTR( "cpu-version" ) ) ) ) {
						has_version = true;
						version_major = CFDataGetBytePtr( data )[ 2 ];
						version_minor = CFDataGetBytePtr( data )[ 3 ];
					}
					if ( ( data = (CFDataRef) CFDictionaryGetValue( dictionary, CFSTR( "name" ) ) ) ) {
						if ( has_version ) {
							if ( asprintf( &result, "%s%s (%x.%x)", in_display_title ? "CPU Type: " : "", (const char *) CFDataGetBytePtr( data ), version_major, version_minor ) == -1 ) err = ENOMEM;
						} else {
							if ( asprintf( &result, "%s%s", in_display_title ? "CPU Type: " : "", (const char *) CFDataGetBytePtr( data ) ) == -1 ) err = ENOMEM;
						}
						if ( ! err ) for ( p = result; ( c = *p ); ++p ) if ( c == ',' ) *p = ' ';
					}
				} else if ( found == 2 ) {
					if ( ( data = (CFDataRef) CFDictionaryGetValue( dictionary, CFSTR( "cpu-type" ) ) ) ) {
						version_major = CFDataGetBytePtr( data )[ 1 ];
						version_minor = CFDataGetBytePtr( data )[ 0 ];
						
						if ( asprintf( &result, "%s%u", in_display_title ? "CPU Type: Intel " : "", version_major << 8 | version_minor ) == -1 ) err = ENOMEM;
					}
				}
			
				CFRelease( dictionary );
			}

			IOObjectRelease( object );
		}
		
		IOObjectRelease( iterator );
	}
	
	return result;
}


char **ethernet_addresses( unsigned int &out_count, bool in_display_title, ethernet_address_query_t in_query ) {
	CFDataRef					address;
	UInt8						buffer[ kIOEthernetAddressSize ];
	CFMutableDictionaryRef		d1 = nil, d2;
	int							err = 0;
	io_object_t					interface;
	io_iterator_t				iterator;
	mach_port_t					master;
	char					   **p, *q, **result = nil;

	out_count = 0;

	err = IOMasterPort( MACH_PORT_NULL, &master );
	if ( ! err && ! ( d1 = IOServiceMatching( kIOEthernetInterfaceClass ) ) ) err = ENOENT;
	if ( ! err ) {
		if ( in_query != k_eaqt_all ) {	
			if ( ! ( d2 = CFDictionaryCreateMutable( 0, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks ) ) ) err = ENOMEM;

			if ( ! err ) {
				CFDictionarySetValue( d2, CFSTR( kIOBuiltin ), in_query == k_eaqt_built_in_only ? kCFBooleanTrue : kCFBooleanFalse );
				CFDictionarySetValue( d1, CFSTR( kIOPropertyMatchKey ), d2 );
				CFRelease( d2 );
			
			}
		}

		if ( ! err ) {
			err = IOServiceGetMatchingServices( master, d1, &iterator );
		} else {
			CFRelease( d1 );
		}
	}
	
	if ( ! err ) {
		while ( ( interface = IOIteratorNext( iterator ) ) ) {
			if ( ( address = (CFDataRef) IORegistryEntrySearchCFProperty( interface, kIOServicePlane, CFSTR( kIOMACAddress ), NULL, kIORegistryIterateRecursively | kIORegistryIterateParents ) ) ) {
				CFDataGetBytes( address, CFRangeMake( 0, kIOEthernetAddressSize ), buffer );

				if ( asprintf( &q, "%s%02x:%02x:%02x:%02x:%02x:%02x", in_display_title ? "Ethernet MAC Address: " : "", buffer[ 0 ], buffer[ 1 ], buffer[ 2 ], buffer[ 3 ], buffer[ 4 ], buffer[ 5 ] ) != -1 ) {
					if ( ( p = (char **)(result ? realloc( result, sizeof(char *) * ( out_count + 1 ) ) : malloc( sizeof(char *) )) ) ) {
						( result = p )[ out_count++ ] = q;
					} else {
						free( q );
					}
				}

				CFRelease( address );
			}
			
			IOObjectRelease( interface );
		}

		IOObjectRelease( iterator );
	}

	return result;
}


char **internal_block_devices( unsigned int &out_count, int in_display_title ) {
	CFMutableDictionaryRef		dictionary = nil;
	int							err, internal, n;
	io_iterator_t				iterator;
	mach_port_t					master;
	char					   *p, *q, **r, **result = nil;
	io_service_t				service;
	CFStringRef					model, serial, string;
	
	out_count = 0;
	
	err = IOMasterPort( MACH_PORT_NULL, &master );
	if ( ! err && ! ( dictionary = IOServiceMatching( "IOBlockStorageDevice" ) ) ) err = ENOENT;
	if ( ! err ) err = IOServiceGetMatchingServices( master, dictionary, &iterator );
	if ( ! err ) {
		while ( ( service = IOIteratorNext( iterator ) ) ) {
			if ( ( string = (CFStringRef) IORegistryEntrySearchCFProperty( service, kIOServicePlane, CFSTR( "Physical Interconnect Location" ), NULL, kIORegistryIterateRecursively | kIORegistryIterateParents ) ) ) {
				internal = ! CFStringCompare( string, CFSTR( "internal" ), kCFCompareCaseInsensitive );			
				CFRelease( string );
			} else {
				internal = false;
			}
			if ( internal ) {
				model = (CFStringRef) IORegistryEntrySearchCFProperty( service, kIOServicePlane, CFSTR( "device model" ), NULL, kIORegistryIterateRecursively | kIORegistryIterateParents );

				if ( model ) {
					serial = (CFStringRef) IORegistryEntrySearchCFProperty( service, kIOServicePlane, CFSTR( "device serial" ), NULL, kIORegistryIterateRecursively | kIORegistryIterateParents );
				} else {
					model = (CFStringRef) IORegistryEntrySearchCFProperty( service, kIOServicePlane, CFSTR( "Model" ), NULL, kIORegistryIterateRecursively | kIORegistryIterateParents );
					serial = (CFStringRef) IORegistryEntrySearchCFProperty( service, kIOServicePlane, CFSTR( "Serial Number" ), NULL, kIORegistryIterateRecursively | kIORegistryIterateParents );
				}
				
				if ( ( string = CFStringCreateWithFormat( NULL, 0, CFSTR( "%@ %@" ), model ? model : CFSTR( "[no model]" ), serial ? serial : CFSTR( "[no serial]" ) ) ) ) {
					if ( ( p = (char *) malloc( n = CFStringGetLength( string ) + 1 ) ) ) {
						CFStringGetCString( string, p, n, kCFStringEncodingASCII );
						
						if ( asprintf( &q, "%s%s", in_display_title ? "Block Device: " : "", p ) != -1 ) {
							if ( ( r = (char **)(result ? realloc( result, sizeof(char *) * ( out_count + 1 ) ) : malloc( sizeof(char *) )) ) ) {
								( result = r )[ out_count++ ] = q;
							} else {
								free( q );
							}
						}
						
						free( p );
					}
				
					CFRelease( string );
				}

				if ( model ) CFRelease( model );
				if ( serial ) CFRelease( serial );
			}

			IOObjectRelease( service );
		}
	
		IOObjectRelease( iterator );
	}

    return result;
}


char *machine_model( bool in_display_title ) {
	char					*result = NULL;
	char					buffer[256];
	int						mib[4];
	size_t					size = sizeof(buffer);
	
	mib[0] = CTL_HW;
	mib[1] = HW_MODEL;
	
	if ( KERN_SUCCESS == sysctl( mib , 2 , buffer , &size , NULL , 0 ) ) {
		asprintf( &result, in_display_title ? "Machine Model: %s" : "%s" , buffer );
	}
	
	return result;
}


char *machine_serial_number( bool in_display_title ) {
	const char			   *data;
	io_registry_entry_t		entry;
	err_t					err = 0;
	CFIndex					n;
	CFTypeRef				property;
	char				   *result = nil, *tmp = nil;
	const UInt32			serial_number_offset = 13;
	
	if ( ! ( entry = IORegistryGetRootEntry( kIOMasterPortDefault ) ) ) err = ENOENT;
	if ( ! err ) {
		if ( ( property = IORegistryEntrySearchCFProperty( entry, kIODeviceTreePlane, CFSTR( "IOPlatformSerialNumber" ), 0, kIORegistryIterateRecursively ) ) ) {
			if ( CFGetTypeID( property ) != CFStringGetTypeID() ) err = EINVAL;
			if ( ! err && ! ( n = CFStringGetLength( (CFStringRef) property ) ) ) err = ENOENT;
			if ( ! err && ! ( tmp = (char *) malloc( ++n ) ) ) err = ENOMEM;
			if ( ! err && ! CFStringGetCString( (CFStringRef) property, tmp, n, kCFStringEncodingASCII ) ) err = EINVAL;
			
			CFRelease( property );
		} else if ( ( property = IORegistryEntrySearchCFProperty( entry, kIODeviceTreePlane, CFSTR( "serial-number" ), 0, kIORegistryIterateRecursively ) ) ) {
			if ( CFGetTypeID( property ) != CFDataGetTypeID() ) err = EINVAL;
			if ( ! err && ! ( data = (const char *) CFDataGetBytePtr( (CFDataRef) property ) ) ) err = ENOENT;
			if ( ! err && ( ! ( n = CFDataGetLength( (CFDataRef) property ) ) || ( n -= serial_number_offset ) <= 0 ) ) err = ENOENT;
			if ( ! err && ! ( tmp = (char *) malloc( n + 1 ) ) ) err = ENOMEM;
			if ( ! err ) { memcpy( tmp, data + serial_number_offset, n ); tmp[ n - 1 ] = '\0'; }

			CFRelease( property );
		} else {
			err = EINVAL;
		}

		IOObjectRelease( entry );
	}

	if ( err && tmp ) { free( tmp ); tmp = nil; }

	if ( ( result = tmp ) && in_display_title ) {
		asprintf( &result, "Machine Serial Number: %s", tmp );
		free( tmp );
	}
	
	return result;
}


char *physical_memory( bool in_display_title ) {
	err_t					err = 0;
	UInt32					n;
	UInt64					o;
	char				   *result = nil;

	if ( sysctlbyname( "hw.memsize", &o, &( n = sizeof(o) ), nil, 0 ) == -1 ) err = errno;
	if ( ! err && n != sizeof(o) ) err = EINVAL;
	if ( ! err ) asprintf( &result, "%s%llu%s", in_display_title ? "Physical Memory: " : "", o, in_display_title ? " Bytes" : "" );

	return result;
}


#pragma mark -
#if 0

char *boot_rom_version( bool in_display_title ) {
	char					buffer[ 128 ], minor, *p, *q, *result = nil;
	CFDataRef				data;
	io_registry_entry_t		entry, rom;
	err_t					err = 0;
	int						length, major, offset;
	
	if ( ! ( entry = IORegistryGetRootEntry( kIOMasterPortDefault ) ) ) err = ENOENT;
	if ( ! err ) {
		if ( ( data = (CFDataRef) IORegistryEntrySearchCFProperty( entry, kIODeviceTreePlane, CFSTR( "BootROM-version" ), 0, kIORegistryIterateRecursively ) ) ) {
			if ( CFGetTypeID( data ) != CFDataGetTypeID() ) err = EINVAL;
			if ( ! err && sscanf( p = (char *) CFDataGetBytePtr( data ), "$%x.%c%n", &major, &minor, &offset ) != 2 ) err = EINVAL;
			if ( ! err ) asprintf( &result, "%s%x.%c.%s", in_display_title ? "Boot ROM Version: " : "", major, minor, p + offset );

			CFRelease( data );
		} if ( ( rom = IORegistryEntryFromPath( kIOMasterPortDefault, kIODeviceTreePlane ":/rom" ) ) ) {
			if ( ( data = (CFDataRef) IORegistryEntryCreateCFProperty( rom, CFSTR( "version" ), kCFAllocatorDefault, 0 ) ) ) {
				if ( CFGetTypeID( data ) != CFDataGetTypeID() ) err = EINVAL;
				if ( ! err ) {
					length = CFDataGetLength( data );
					CFDataGetBytes( data, CFRangeMake( 0, offset = min( length, (int) sizeof(buffer) - 1 ) ), (UInt8 *) buffer );
					buffer[ offset ] = 0;
					for ( p = buffer; *p; ++p ) if ( *p == '.' ) { *p++ = 0; break; }
					for ( ; *p; ++p ) if ( *p == '.' ) break;
					for ( q = ++p; *p && *p != '.'; ++p ) ;
					while ( *++p && *p != '.' ) ;
					*p = 0;
					if ( p > q ) asprintf( &result, "%s%s.%s", in_display_title ? "Boot ROM Version: " : "", buffer, q );
				}

				CFRelease( data );
			}
			
			IOObjectRelease( rom );
		}
	}

	return result;
}


char *boot_volume_model( bool in_display_title ) {
	char						buffer[ 128 ], *device, *p, *result = nil;
	CFMutableDictionaryRef		dictionary;
	int							err = 0;
	io_iterator_t				iterator;
	mach_port_t					master;
	io_service_t				parent, service = nil;
	struct stat					sb;
	CFStringRef					string;

	err = IOMasterPort( MACH_PORT_NULL, &master );
	if ( ! err && stat( "/", &sb ) == -1 ) err = errno;
	if ( ! err && ! ( device = devname( sb.st_dev, S_IFBLK ) ) ) err = ENOENT;
	if ( ! err && ! ( dictionary = IOBSDNameMatching( master, 0, device ) ) ) err = ENOENT;
	if ( ! err ) err = IOServiceGetMatchingServices( master, dictionary, &iterator );
	if ( ! err ) {
		if ( ! ( service = IOIteratorNext( iterator ) ) ) err = ENOENT;

		IOObjectRelease( iterator );

		for ( ; service; service = parent ) {
			if ( ! ( string = (CFStringRef) IORegistryEntryCreateCFProperty( service, CFSTR( "device model" ), 0, 0 ) ) ) {
				if ( ! ( string = (CFStringRef) IORegistryEntryCreateCFProperty( service, CFSTR( "Model" ), 0, 0 ) ) ) {
					if ( IORegistryEntryGetParentEntry( service, kIOServicePlane, &parent ) ) parent = nil;
					IOObjectRelease( service );
					continue;
				}
			}

			CFStringGetCString( string, buffer, sizeof(buffer), kCFStringEncodingASCII );
			for ( p = &buffer[ strlen( buffer ) ]; --p >= buffer && *p == ' '; ) ;
			*++p = 0;
			for ( p = buffer; *p == ' '; ++p ) ;
			asprintf( &result, "%s%s", in_display_title ? "Boot Hard Drive Model: " : "", p );
			CFRelease( string );
			IOObjectRelease( service );

			break;
		}
		
		if ( ! service ) err = ENOENT;
	}

	return result;
}


char *boot_volume_serial_number( bool in_display_title ) {
	char						buffer[ 128 ], *device, *p, *result = nil;
	CFMutableDictionaryRef		dictionary;
	int							err = 0;
	io_iterator_t				iterator;
	mach_port_t					master;
	io_service_t				parent, service = nil;
	struct stat					sb;
	CFStringRef					string;

	err = IOMasterPort( MACH_PORT_NULL, &master );
	if ( ! err && stat( "/", &sb ) == -1 ) err = errno;
	if ( ! err && ! ( device = devname( sb.st_dev, S_IFBLK ) ) ) err = ENOENT;
	if ( ! err && ! ( dictionary = IOBSDNameMatching( master, 0, device ) ) ) err = ENOENT;
	if ( ! err ) err = IOServiceGetMatchingServices( master, dictionary, &iterator );
	if ( ! err ) {
		if ( ! ( service = IOIteratorNext( iterator ) ) ) err = ENOENT;

		IOObjectRelease( iterator );

		for ( ; service; service = parent ) {
			if ( ! ( string = (CFStringRef) IORegistryEntryCreateCFProperty( service, CFSTR( "device serial" ), 0, 0 ) ) ) {
				if ( ! ( string = (CFStringRef) IORegistryEntryCreateCFProperty( service, CFSTR( "Serial Number" ), 0, 0 ) ) ) {
					if ( IORegistryEntryGetParentEntry( service, kIOServicePlane, &parent ) ) parent = nil;
					IOObjectRelease( service );
					continue;
				}
			}

			CFStringGetCString( string, buffer, sizeof(buffer), kCFStringEncodingASCII );
			for ( p = &buffer[ strlen( buffer ) ]; --p >= buffer && *p == ' '; ) ;
			*++p = 0;
			for ( p = buffer; *p == ' '; ++p ) ;
			asprintf( &result, "%s%s", in_display_title ? "Boot Hard Drive Serial Number: " : "", p );
			CFRelease( string );
			IOObjectRelease( service );

			break;
		}
		
		if ( ! service ) err = ENOENT;
	}

	return result;
}


char *kernel_version( bool in_display_title ) {
	char					*result = NULL;
	char					buffer[256];
	int						mib[4];
	size_t					size = sizeof(buffer);
	
	mib[0] = CTL_KERN;
	mib[1] = KERN_OSRELEASE;
	
	if ( KERN_SUCCESS == sysctl( mib , 2 , buffer , &size , NULL , 0 ) ) {
		asprintf( &result, in_display_title ? "Kernel Version: %s" : "%s" , buffer );
	}
	
	return result;
}


char *sudden_motion_sensor_version( bool in_display_title ) {
	CFMutableDictionaryRef	dictionary;
	err_t					err = 0;
	io_iterator_t			iterator;
	CFNumberRef				number;
	io_object_t				object;
	char				   *result = nil;
	UInt32					version;

	if ( ! ( dictionary = IOServiceMatching( "IOI2CMotionSensor" ) ) ) err = ENOENT;
	if ( ! err ) err = IOServiceGetMatchingServices( kIOMasterPortDefault, dictionary, &iterator );
	if ( ! err ) {
		if ( ! ( object = IOIteratorNext( iterator ) ) ) err = ENOENT;
		IOObjectRelease( iterator );
	}

	if ( err ) {
		err = ( ( dictionary = IOServiceMatching( "PMUMotionSensor" ) ) ? 0 : ENOENT );
		if ( ! err ) err = IOServiceGetMatchingServices( kIOMasterPortDefault, dictionary, &iterator );
		if ( ! err ) {
			if ( ! ( object = IOIteratorNext( iterator ) ) ) err = ENOENT;
			IOObjectRelease( iterator );
		}
	}
	
	if ( ! err ) {
		err = IORegistryEntryCreateCFProperties( object, &dictionary, kCFAllocatorDefault, 0 );
		if ( ! err ) {
			if ( ! ( number = (CFNumberRef) CFDictionaryGetValue( dictionary, CFSTR( "ams-application-version" ) ) ) ) err = ENOENT;
			if ( ! err && ! ( CFNumberGetValue( number, CFNumberGetType( number ), &version ) ) ) err = ENOENT;
			if ( ! err ) asprintf( &result, "%s%u.%u", in_display_title ? "Sudden Motion Sensor Version: " : "", version >> 16, version & 0xff );
			
			CFRelease( dictionary );
		}

		IOObjectRelease( object );
	}
	
	return result;
}


#pragma mark -


#ifdef TESTING
#define IOObjectDisplay( _o ) do {\
	CFMutableDictionaryRef			display = NULL;\
	IORegistryEntryCreateCFProperties( _o , &display , NULL , 0 );\
	if ( display ) { CFShow( display ); CFRelease( display ); }\
} while (0)
#else
#define IOObjectDisplay( _o )
#endif
			

#define kHWSensorNubName			"temp-sensor"
#define kHWSensorParamsVersionKey	"version"
#define kHWSensorIDKey				"sensor-id"
#define kHWSensorZoneKey			"zone"
#define kHWSensorTypeKey			"type"
#define kHWSensorLocationKey		"location"
#define kHWSensorPollingPeriodKey	"polling-period"


long string_append_string( CFMutableStringRef ioString , CFTypeRef inSource , UniChar inPrefix ) {
	CFTypeID					id = inSource ? CFGetTypeID( inSource ) : NULL;
	
	if ( id ) {
		if ( inPrefix ) CFStringAppendCharacters( ioString , &inPrefix , 1 );
	}
	
	if ( id == CFDataGetTypeID() ) {
		const void				*data = CFDataGetBytePtr( (CFDataRef)inSource );
		
		CFStringAppendFormat( ioString , NULL , CFSTR( "%s" ) , (char *)data );
	} else if ( inSource ) {
		CFStringAppendFormat( ioString , NULL , CFSTR( "%@" ) , inSource );
	}
	
	return 1;
}


long string_append_number( CFMutableStringRef ioString , CFTypeRef inSource , UniChar inPrefix ) {
	long						result = 0;
	SInt64						number;
	CFTypeID					id = inSource ? CFGetTypeID( inSource ) : NULL;
	
	if ( id == CFDataGetTypeID() ) {
		const void				*data = CFDataGetBytePtr( (CFDataRef)inSource );
		
		switch ( CFDataGetLength( (CFDataRef)inSource ) ) {
		case sizeof(SInt64): number = *(SInt64 *)data; result = 1; break;
		case sizeof(SInt32): result = 1;
		default: number = *(SInt32 *)data; break;
		case 3:
		case sizeof(SInt16): number = *(SInt16 *)data; break;
		case sizeof(SInt8): number = *(SInt8 *)data; break;
		}
	} else if ( id == CFNumberGetTypeID() ) {
		result = CFNumberGetValue( (CFNumberRef)inSource , kCFNumberSInt64Type , &number );
	} else if ( id == CFStringGetTypeID() ) {
		number = CFStringGetIntValue( (CFStringRef)inSource );
		result = 1;
	} else {
		number = 0;
	}
	
	if ( inPrefix ) CFStringAppendCharacters( ioString , &inPrefix , 1 );
	CFStringAppendFormat( ioString , NULL , CFSTR( "%qd" ) , number );
	
	return result;
}


long string_append_string_property( CFMutableStringRef ioString , io_object_t inObject , CFStringRef inProperty , UniChar inPrefix ) {
	long						result = 0;
	CFTypeRef					value = IORegistryEntryCreateCFProperty( inObject , inProperty , 0 , 0 );
	
	if ( value ) {
		result = string_append_string( ioString , value , inPrefix );
		CFRelease( value );
	}
	
	return result;
}


long string_append_number_property( CFMutableStringRef ioString , io_object_t inObject , CFStringRef inProperty , UniChar inPrefix ) {
	long						result = 0;
	CFTypeRef					value = IORegistryEntryCreateCFProperty( inObject , inProperty , 0 , 0 );
	
	if ( value ) {
		result = string_append_number( ioString , value , inPrefix );
		CFRelease( value );
	}
	
	return result;
}


long array_append_string_property( CFMutableArrayRef ioArray , io_object_t inObject , CFStringRef inProperty ) {
	long						result = 0;
	CFTypeRef					value = IORegistryEntryCreateCFProperty( inObject , inProperty , 0 , 0 );
	CFTypeID					id = value ? CFGetTypeID( value ) : 0;
	
	if ( id == CFDataGetTypeID() ) {
		CFDataRef				data = (CFDataRef)value;
		CFStringRef				string;
		const UInt8				*raw = CFDataGetBytePtr( data );
		SInt32					index;// , count = *(UInt32 *)raw;
		CFIndex					offset = 0 , length = CFDataGetLength( data );
		unsigned				amount;
		
//		if ( length < offset || count < 0 || count > length / 2 ) offset = 0;
//		else raw += offset;
		
		for ( index = 0 ; offset < length ; ++result ) {
			while ( *raw <= ' ' ) { ++raw; ++offset; }
			
			amount = strlen( (const char *)raw );
			string = NULL;
			
			if ( amount > 0 ) {
				string = CFStringCreateWithBytes( NULL , raw , amount , kCFStringEncodingUTF8 , 0 );
			}
			
			if ( string ) {
				CFArrayAppendValue( ioArray , string );
				CFRelease( string );
			}
			
			raw += amount + 1;
			offset += amount + 1;
		}
	} else if ( value ) {
		CFArrayAppendValue( ioArray , value );
		
		result = 1;
	}
	
	if ( value ) CFRelease( value );
	
	return result;
}


void array_append_dictionary( CFTypeRef inKey , CFTypeRef inValue , void *inContext ) {
	CFMutableStringRef			string = CFStringCreateMutable( NULL , 0 );
	
	if ( string ) {
		string_append_string( string , inKey , 0 );
		string_append_string( string , inValue , '=' );
		
		CFArrayAppendValue( (CFMutableArrayRef)inContext , string );
		CFRelease( string );
	}
}


char *string_allocate_string( CFStringRef inString ) {
	char						*result = NULL;
	
	if ( inString ) {
		CFIndex					used = 0 , length = CFStringGetLength( inString );
		
		result = (char *)malloc( length + 1 );
		
		if ( result ) {
			CFStringGetBytes( inString , CFRangeMake( 0 , length ) , kCFStringEncodingASCII , '?' , false , (UInt8 *)result , length , &used );
			
			result[used] = 0;
		}
	}
	
	return result;
}


long array_prepare( CFMutableArrayRef ioArray , int inCollapse ) {
	CFIndex						index , count = ioArray ? CFArrayGetCount( ioArray ) : 0;
	CFIndex						tally;
	CFStringRef					unique , string;
	
	CFArraySortValues( ioArray , CFRangeMake( 0 , count ) , (CFComparatorFunction)CFStringCompare , 0 );
	
	if ( inCollapse ) {
		for ( index = 0 ; index < count - 1 ; ++index ) {
			string = (CFStringRef)CFArrayGetValueAtIndex( ioArray , index );
			tally = 1;
			
			do {
				unique = (CFStringRef)CFArrayGetValueAtIndex( ioArray , index + 1 );
				
				if ( 0 == CFStringCompare( unique , string , 0 ) ) {
					tally += 1;
					count -= 1;
					
					CFArrayRemoveValueAtIndex( ioArray , index + 1 );
				} else {
					break;
				}
			} while ( index < count - 1 );
			
			if ( tally > 1 ) {
				unique = CFStringCreateWithFormat( NULL , NULL , CFSTR( "%@ x %d" ) , string , tally );
				
				CFArraySetValueAtIndex( ioArray , index , unique );
			}
		}
	}
	
	return count;
}


char *array_allocate_string( CFMutableArrayRef ioArray ) {
	char						*result = NULL;
	
	CFIndex						count = array_prepare( ioArray , 1 );
	CFStringRef					string;
	
	if ( count > 0 ) {
		string = CFStringCreateByCombiningStrings( NULL , ioArray , CFSTR( "|" ) );
		
		if ( string ) {
			result = string_allocate_string( string );
			
			CFRelease( string );
		}
	}
	
	return result;
}


char **array_allocate_strings( CFMutableArrayRef ioArray ) {
	char						**result = NULL;
	
	CFIndex						index , count = array_prepare( ioArray , 1 );
	CFStringRef					string;
	
	if ( count > 0 ) {
		result = (char **)malloc( sizeof(char *) * ( count + 1 ) );
	}
	
	if ( result ) {
		for ( index = 0 ; index < count ; ++index ) {
			string = (CFStringRef)CFArrayGetValueAtIndex( ioArray , index );
			
			result[index] = string_allocate_string( string );
		}
		
		result[count] = NULL;
	}
	
	return result;
}


char *pci_count( bool in_display_title ) {
	char						*result = NULL;
	
	io_iterator_t				objects = 0;
	io_object_t					object = 0;
	signed						count = 0;
	
	if ( noErr == IOServiceGetMatchingServices( kIOMasterPortDefault , IOServiceNameMatching( "pci" ) , &objects ) ) {
		while ( ( object = IOIteratorNext( objects ) ) != 0 ) {
			count += 1;
			
			IOObjectRelease( object );
		}
		
		IOObjectRelease( objects );
	}
	
#if 0
	if ( noErr == IOServiceGetMatchingServices( kIOMasterPortDefault , IOServiceNameMatching( "pci8086" ) , &objects ) ) {
		while ( ( object = IOIteratorNext( objects ) ) != 0 ) {
			count += 1;
			
			IOObjectRelease( object );
		}
		
		IOObjectRelease( objects );
	}
#endif
	
	if ( count > 0 ) {
		asprintf( &result , in_display_title ? "PCI Count: %d" : "%d" , count );
	}
	
	return result;
}


char *boot_rom_model( bool in_display_title ) {
	char						*result = NULL;
	
	io_object_t					rom = 0;
	
	CFMutableStringRef			string = NULL;
	
	rom = IORegistryEntryFromPath( kIOMasterPortDefault , kIODeviceTreePlane ":/rom/boot-rom" );
	
	if ( rom ) {
		string = CFStringCreateMutable( NULL , 0 );
		
		if ( string ) {
			if ( in_display_title ) CFStringAppend( string , CFSTR( "Boot ROM Model: " ) );
			string_append_string_property( string , rom , CFSTR( "model" ) , 0 );
			
			result = string_allocate_string( string );
			
			CFRelease( string );
		}
		
		IOObjectRelease( rom );
	}
	
	return result;
}


char *wireless_information( bool in_display_title ) {
	char						*result = NULL;
	
	io_service_t				object = 0;
	CFMutableStringRef			string = NULL;
	
	object = IORegistryEntryFromAlias( kIOMasterPortDefault , "wireless" );
	
	if ( object ) {
		string = CFStringCreateMutable( NULL , 0 );
		
		if ( string ) {
			if ( in_display_title ) CFStringAppend( string , CFSTR( "Wireless Card: " ) );
			string_append_string_property( string , object , CFSTR( "name" ) , 0 );
			string_append_string_property( string , object , CFSTR( "network-type" ) , '-' );
			string_append_number_property( string , object , CFSTR( "vendor-id" ) , '-' );
			string_append_number_property( string , object , CFSTR( "revision-id" ) , '-' );
			
			result = string_allocate_string( string );
			
			CFRelease( string );
		}
		
//		IOObjectDisplay( object );
		IOObjectRelease( object );
	}
	
	return result;
}


char **hardware_sensor_information( bool in_display_title ) {
	char						**result = NULL;
	
	io_iterator_t				sensors = 0;
	io_object_t					sensor = 0;
	
	CFMutableStringRef			string = NULL;
	CFMutableArrayRef			array = CFArrayCreateMutable( NULL , 0 , &kCFTypeArrayCallBacks );
	CFIndex						count = 0;
	
	if ( noErr == IOServiceGetMatchingServices( kIOMasterPortDefault , IOServiceMatching( "IOHWSensor" ) , &sensors ) ) {
		while ( ( sensor = IOIteratorNext( sensors ) ) != 0 ) {
			string = CFStringCreateMutable( NULL , 0 );
			
			if ( string ) {
				if ( in_display_title ) CFStringAppend( string , CFSTR( "Hardware Sensor: " ) );
				string_append_string_property( string , sensor , CFSTR( kHWSensorTypeKey ) , 0 );
//				string_append_number_property( string , sensor , CFSTR( kHWSensorIDKey ) , '-' );
				string_append_string_property( string , sensor , CFSTR( kHWSensorLocationKey ) , '-' );
				
				CFArrayAppendValue( array , string );
				CFRelease( string );
				
				count += 1;
			}
			
//			IOObjectDisplay( sensor );
			IOObjectRelease( sensor );
		}
		
		IOObjectRelease( sensors );
	}
	
	if ( array ) {
//		result = array_allocate_string( array );
		result = array_allocate_strings( array );
		
		CFRelease( array );
	}
	
	return result;
}


char **pci_information( bool in_display_title ) {
	char						**result = NULL;
	
	io_iterator_t				objects = 0;
	io_object_t					object = 0;
	
	CFMutableStringRef			string = NULL;
	CFMutableArrayRef			array = CFArrayCreateMutable( NULL , 0 , &kCFTypeArrayCallBacks );
	CFIndex						count = 0;
	
	if ( noErr == IOServiceGetMatchingServices( kIOMasterPortDefault , IOServiceMatching( "IOPCIDevice" ) , &objects ) ) {
		while ( ( object = IOIteratorNext( objects ) ) != 0 ) {
			string = CFStringCreateMutable( NULL , 0 );
			
			if ( string ) {
				if ( in_display_title ) CFStringAppend( string , CFSTR( "pci: " ) );
				string_append_string_property( string , object , CFSTR( "name" ) , 0 );
//				string_append_string_property( string , object , CFSTR( "device_type" ) , 0 );
				string_append_string_property( string , object , CFSTR( "model" ) , '-' );
				string_append_number_property( string , object , CFSTR( "vendor-id" ) , '-' );
				
				CFArrayAppendValue( array , string );
				CFRelease( string );
				
				count += 1;
			}
			
//			IOObjectDisplay( object );
			IOObjectRelease( object );
		}
		
		IOObjectRelease( objects );
	}
	
	if ( array ) {
//		result = array_allocate_string( array );
		result = array_allocate_strings( array );
		
		CFRelease( array );
	}
	
	return result;
}


char **ata_information( bool in_display_title ) {
	char						**result = NULL;
	
	io_iterator_t				objects = 0;
	io_object_t					object = 0;
	
	CFMutableStringRef			string = NULL;
	CFMutableArrayRef			array = CFArrayCreateMutable( NULL , 0 , &kCFTypeArrayCallBacks );
	CFIndex						count = 0;
	
	CFMutableDictionaryRef		matching = IOServiceMatching( "ATADeviceNub" );
	
	if ( matching ) {
		CFDictionarySetValue( matching , CFSTR( "socket type" ) , CFSTR( "internal" ) );
	}
	
	if ( noErr == IOServiceGetMatchingServices( kIOMasterPortDefault , matching , &objects ) ) {
		while ( ( object = IOIteratorNext( objects ) ) != 0 ) {
			string = CFStringCreateMutable( NULL , 0 );
			
			if ( string ) {
				if ( in_display_title ) CFStringAppend( string , CFSTR( "ata: " ) );
				string_append_string_property( string , object , CFSTR( "device model" ) , 0 );
				string_append_string_property( string , object , CFSTR( "device serial" ) , '-' );
				string_append_string_property( string , object , CFSTR( "device revision" ) , '-' );
				
				CFArrayAppendValue( array , string );
				CFRelease( string );
				
				count += 1;
			}
			
//			IOObjectDisplay( object );
			IOObjectRelease( object );
		}
		
		IOObjectRelease( objects );
	}
	
	if ( array ) {
//		result = array_allocate_string( array );
		result = array_allocate_strings( array );
		
		CFRelease( array );
	}
	
	return result;
}


char **usb_information( bool in_display_title ) {
	char						**result = NULL;
	
	io_iterator_t				objects = 0;
	io_object_t					object = 0;
	
	CFMutableStringRef			string = NULL;
	CFMutableArrayRef			array = CFArrayCreateMutable( NULL , 0 , &kCFTypeArrayCallBacks );
	CFIndex						count = 0;
	
	if ( noErr == IOServiceGetMatchingServices( kIOMasterPortDefault , IOServiceMatching( "IOUSBDevice" ) , &objects ) ) {
		while ( ( object = IOIteratorNext( objects ) ) != 0 ) {
			string = CFStringCreateMutable( NULL , 0 );
			
			if ( string ) {
				if ( in_display_title ) CFStringAppend( string , CFSTR( "usb: " ) );
//				string_append_string_property( string , object , CFSTR( "name" ) , 0 );
				string_append_string_property( string , object , CFSTR( "USB Product Name" ) , 0 );
//				string_append_string_property( string , object , CFSTR( "device_type" ) , 0 );
				string_append_string_property( string , object , CFSTR( "model" ) , '-' );
				string_append_number_property( string , object , CFSTR( "idVendor" ) , '-' );
				
				CFArrayAppendValue( array , string );
				CFRelease( string );
				
				count += 1;
			}
			
			IOObjectRelease( object );
		}
		
		IOObjectRelease( objects );
	}
	
	if ( array ) {
//		result = array_allocate_string( array );
		result = array_allocate_strings( array );
		
		CFRelease( array );
	}
	
	return result;
}


char **vsp_information( bool in_display_title ) {
	char						**result = NULL;
	
	io_iterator_t				objects , children;
	io_object_t					object , child;
	
	CFMutableStringRef			string = NULL;
	CFMutableArrayRef			array = CFArrayCreateMutable( NULL , 0 , &kCFTypeArrayCallBacks );
	CFIndex						count = 0;
	signed						tally;
	
	if ( noErr == IOServiceGetMatchingServices( kIOMasterPortDefault , IOServiceNameMatching( "vsp" ) , &objects ) ) {
		while ( ( object = IOIteratorNext( objects ) ) != 0 ) {
			string = CFStringCreateMutable( NULL , 0 );
			
			if ( string ) {
				if ( in_display_title ) CFStringAppend( string , CFSTR( "vsp: " ) );
				string_append_string_property( string , object , CFSTR( "model" ) , 0 );
				
				tally = 0;
				IORegistryEntryGetChildIterator( object , kIODeviceTreePlane , &children );
				
				if ( children ) {
					while ( ( child = IOIteratorNext( children ) ) != 0 ) {
						tally += 1;
						
						IOObjectRelease( child );
					}
					
					IOObjectRelease( children );
				}
				
				CFStringAppendFormat( string , NULL , CFSTR( "-%d" ) , tally );
				CFArrayAppendValue( array , string );
				CFRelease( string );
				
				count += 1;
			}
			
			IOObjectRelease( object );
		}
		
		IOObjectRelease( objects );
	}
	
	if ( array ) {
//		result = array_allocate_string( array );
		result = array_allocate_strings( array );
		
		CFRelease( array );
	}
	
	return result;
}


char **sep_information( bool in_display_title ) {
	char						**result = NULL;
	
	io_registry_entry_t			parent;
	io_iterator_t				objects , children;
	io_object_t					object , child;
	
	CFMutableStringRef			string = NULL;
	CFMutableArrayRef			array = CFArrayCreateMutable( NULL , 0 , &kCFTypeArrayCallBacks );
	CFIndex						count = 0;
	
	parent = IORegistryEntryFromPath( kIOMasterPortDefault , kIODeviceTreePlane ":/sep" );
	objects = 0;
	
	if ( parent ) {
		IORegistryEntryGetChildIterator( parent , kIODeviceTreePlane , &objects );
		IOObjectRelease( parent );
	}
	
	if ( objects ) {
		while ( ( object = IOIteratorNext( objects ) ) != 0 ) {
			children = 0;
			IORegistryEntryGetChildIterator( object , kIODeviceTreePlane , &children );
			
			if ( children ) {
				while ( ( child = IOIteratorNext( children ) ) != 0 ) {
					string = CFStringCreateMutable( NULL , 0 );
					
					if ( string ) {
						if ( in_display_title ) CFStringAppend( string , CFSTR( "sep: " ) );
						string_append_string_property( string , child , CFSTR( "device_type" ) , 0 );
						string_append_string_property( string , child , CFSTR( kHWSensorLocationKey ) , '-' );
//						string_append_number_property( string , child , CFSTR( kHWSensorIDKey ) , '-' );
						
						CFArrayAppendValue( array , string );
						CFRelease( string );
						
						count += 1;
					}
					
					IOObjectRelease( child );
				}
				
				IOObjectRelease( children );
			}
			
			IOObjectRelease( object );
		}
		
		IOObjectRelease( objects );
	}
	
	if ( array ) {
//		result = array_allocate_string( array );
		result = array_allocate_strings( array );
		
		CFRelease( array );
	}
	
	return result;
}


char **cpu_information( bool in_display_title ) {
	char						**result = NULL;
	
	io_registry_entry_t			parent;
	io_iterator_t				cpus , caches;
	io_object_t					cpu , cache;
	
	CFMutableStringRef			string = NULL;
	CFMutableArrayRef			array = CFArrayCreateMutable( NULL , 0 , &kCFTypeArrayCallBacks );
	CFIndex						count = 0;
	
	parent = IORegistryEntryFromPath( kIOMasterPortDefault , kIODeviceTreePlane ":/cpus" );
	cpus = 0;
	
	if ( parent ) {
		IORegistryEntryGetChildIterator( parent , kIODeviceTreePlane , &cpus );
		IOObjectRelease( parent );
	}
	
	if ( cpus ) {
		while ( ( cpu = IOIteratorNext( cpus ) ) != 0 ) {
			string = CFStringCreateMutable( NULL , 0 );
			
			if ( string ) {
				if ( in_display_title ) CFStringAppend( string , CFSTR( "cpu: " ) );
				string_append_string_property( string , cpu , CFSTR( "name" ) , 0 );
				string_append_number_property( string , cpu , CFSTR( "clock-frequency" ) , ',' );
				string_append_number_property( string , cpu , CFSTR( "bus-frequency" ) , ',' );
				string_append_number_property( string , cpu , CFSTR( "i-cache-size" ) , ',' );
				string_append_number_property( string , cpu , CFSTR( "d-cache-size" ) , ',' );
				
				caches = 0;
				IORegistryEntryGetChildIterator( cpu , kIODeviceTreePlane , &caches );
				
				if ( caches ) {
					while ( ( cache = IOIteratorNext( caches ) ) != 0 ) {
						string_append_string_property( string , cache , CFSTR( "name" ) , ',' );
						string_append_number_property( string , cache , CFSTR( "i-cache-size" ) , ',' );
						string_append_number_property( string , cache , CFSTR( "d-cache-size" ) , ',' );
						
//						IOObjectDisplay( cache );
						IOObjectRelease( cache );
					}
					
					IOObjectRelease( caches );
				}
				
				CFArrayAppendValue( array , string );
				CFRelease( string );
				
				count += 1;
			}
			
//			IOObjectDisplay( cpu );
			IOObjectRelease( cpu );
		}
		
		IOObjectRelease( cpus );
	}
	
	if ( array ) {
//		result = array_allocate_string( array );
		result = array_allocate_strings( array );
		
		CFRelease( array );
	}
	
	return result;
}


char **memory_information( bool in_display_title ) {
	char						**result = NULL;
	
	io_service_t				memory = 0;
	CFMutableArrayRef			array = NULL;
	
//	memory = IOServiceGetMatchingService( kIOMasterPortDefault , IOServiceNameMatching( "memory" ) );
	memory = IORegistryEntryFromPath( kIOMasterPortDefault , kIODeviceTreePlane ":/memory" );
	
	if ( memory ) {
		array = CFArrayCreateMutable( NULL , 0 , &kCFTypeArrayCallBacks );
		
		array_append_string_property( array , memory , CFSTR( "slot-names" ) );
		array_append_string_property( array , memory , CFSTR( "dimm-types" ) );
		array_append_string_property( array , memory , CFSTR( "dimm-speeds" ) );
//		array_append_string_property( array , memory , CFSTR( "dimm-info" ) );
		
//		IOObjectDisplay( memory );
		IOObjectRelease( memory );
	}
	
	if ( array ) {
//		result = array_allocate_string( array );
		result = array_allocate_strings( array );
		
		CFRelease( array );
	}
	
	return result;
}


char **alias_information( bool in_display_title ) {
	char						**result = NULL;
	
	io_registry_entry_t			entry = 0;
	CFMutableArrayRef			array = NULL;
	CFMutableDictionaryRef		dictionary = NULL;
	
//	entry = IOServiceGetMatchingService( kIOMasterPortDefault , IOServiceNameMatching( "aliases" ) );
	entry = IORegistryEntryFromPath( kIOMasterPortDefault , kIODeviceTreePlane ":/aliases" );
	
	if ( entry ) {
		IORegistryEntryCreateCFProperties( entry , &dictionary , NULL , 0 );
		IOObjectRelease( entry );
	}
	
	if ( dictionary ) {
		array = CFArrayCreateMutable( NULL , 0 , &kCFTypeArrayCallBacks );
		
		CFDictionaryApplyFunction( dictionary , array_append_dictionary , array );
		CFRelease( dictionary );
	}
	
	if ( array ) {
		result = array_allocate_strings( array );
		
		CFRelease( array );
	}
	
	return result;
}


char **service_information( bool in_display_title ) {
	char						**result = NULL;
	
	io_iterator_t				objects = 0;
	io_object_t					object = 0;
	
	CFMutableStringRef			string = NULL;
	CFMutableArrayRef			array = CFArrayCreateMutable( NULL , 0 , &kCFTypeArrayCallBacks );
	CFIndex						count = 0;
	
	if ( noErr == IOServiceGetMatchingServices( kIOMasterPortDefault , IOServiceMatching( "IOService" ) , &objects ) ) {
		while ( ( object = IOIteratorNext( objects ) ) != 0 ) {
			string = CFStringCreateMutable( NULL , 0 );
			
			if ( string ) {
				if ( in_display_title ) CFStringAppend( string , CFSTR( "service: " ) );
				string_append_string_property( string , object , CFSTR( "name" ) , 0 );
				string_append_string_property( string , object , CFSTR( "model" ) , '-' );
				string_append_string_property( string , object , CFSTR( "device_type" ) , '-' );
				
				CFArrayAppendValue( array , string );
				CFRelease( string );
				
				count += 1;
			}
			
			IOObjectRelease( object );
		}
		
		IOObjectRelease( objects );
	}
	
	if ( array ) {
//		result = array_allocate_string( array );
		result = array_allocate_strings( array );
		
		CFRelease( array );
	}
	
	return result;
}


#pragma mark -


#ifdef TESTING
/*
	gcc -DTESTING -fno-exceptions -framework IOKit -framework Cocoa machine_info_darwin.mm -o test
*/


#define TESTING_STRINGS(_s)\
	strings = _s( title );\
	printf( "%s: %p\n" , #_s , strings );\
	if ( strings ) for ( char **_t = strings ; ( string = *_t++ ) != NULL ; free( string ) ) { printf( "    \"%s\"\n" , string ); }\
	if ( strings ) free( strings )

#define TESTING_STRING(_s)\
	string = _s( title );\
	printf( "%s: \"%s\"\n" , #_s , string?string:"Missing" );\
	if ( string ) free( string )

#define TESTING_STRING_(_s,...)\
	string = _s( title , __VA_ARGS__ );\
	printf( "%s: \"%s\"\n" , #_s , string?string:"Missing" );\
	if ( string ) free( string )

int main( int argc , char const *argv[] ) {
	bool					title = 1;
	
	char					*string;
	char					**strings;
	
	if ( argc == 2 && argv[1][1] == 'n' ) title = 0;
	
	/*
		rated from 0 (changes often) to 9 (never changes)
		nothing under 5 should be relied upon
		
		to distinguish two different pieces of hardware use
			cpu_information
			sep_information/hardware_sensor_information
			
			wireless_information
			machine_model/boot_rom_model
		
		to distinguish otherwise identical hardware use
			machine_serial_number
			primary_ethernet_mac_address
		
		last ditch effort primarily for older machines use
			memory_information/physical_memory
			boot_volume_serial_number/boot_volume_model
		
	*/
	
	TESTING_STRING( pci_count );						//	6
	TESTING_STRING( boot_rom_model );					//	8	upgradeable?
	TESTING_STRING( boot_rom_version );					//	8	upgradeable?
	TESTING_STRING( boot_volume_model );				//	3
	TESTING_STRING( boot_volume_serial_number );		//	3
	
	//	these are now covered by cpu_information
	TESTING_STRING( bus_speed );						//	8
	TESTING_STRING( cpu_count );						//	9
	TESTING_STRING( cpu_speed );						//	9
	TESTING_STRING( cpu_type );							//	9
	
	TESTING_STRING( machine_model );					//	9
	TESTING_STRING( machine_serial_number );			//	9
	TESTING_STRING( kernel_version );					//	1
	TESTING_STRING_( operating_system_version , 1 );	//	1
	TESTING_STRING( physical_memory );					//	4
	TESTING_STRING( primary_ethernet_mac_address );		//	6
	TESTING_STRING( sudden_motion_sensor_version );		//	7
	TESTING_STRING( wireless_information );				//	7
	
	TESTING_STRINGS( ata_device_info );					//	2
	TESTING_STRINGS( hardware_sensor_information );		//	9
	TESTING_STRINGS( sep_information );					//	9
	TESTING_STRINGS( cpu_information );					//	9
	TESTING_STRINGS( pci_information );					//	4
	TESTING_STRINGS( ata_information );					//	3
	TESTING_STRINGS( vsp_information );					//	9	what is this
	TESTING_STRINGS( usb_information );					//	0
	TESTING_STRINGS( memory_information );				//	5
	
//	TESTING_STRINGS( service_information );				//	-
	TESTING_STRINGS( alias_information );				//	-
	
	return 0;
}

#endif
#endif	// 0
