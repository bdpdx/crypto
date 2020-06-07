#ifndef __common_headers_h__
	#include "common_headers.h"
#endif


// to list gcc predefined macros:
// gcc -dM -E - < /dev/null | sort

#ifndef __common_types_h__
#define __common_types_h__
#define __balance_types				1


// last used obfuscated identifier:	bscaaq


#ifdef __cplusplus
extern "C" {
#endif


#if __GNUC__
	#define __GCC_VERSION__			( __GNUC__ * 10000 + __GNUC_MINOR__ * 1000 + __GNUC_PATCHLEVEL__ )
#endif

#ifndef nil
	#define nil						0
#endif

#ifndef __cplusplus
	#if ! defined( __GNUC__ ) || __GNUC__ <= 3 || __GCC_VERSION__ >= 40101
		typedef int					bool;
	#endif
	
	#define	false					0
	#define true					1
	#define inline
#endif


#ifndef __bd_scalar_types__
	#define __bd_scalar_types__

	typedef char						__s8;
	typedef unsigned char				__u8;
	typedef short						__s16;
	typedef unsigned short				__u16;
	typedef int							__s32;
	typedef unsigned int				__u32;
	typedef long						__l32;
	typedef unsigned long				_ul32;
	typedef long long					__s64;
	typedef unsigned long long			__u64;

	typedef __u32						word;
	typedef __u64						dword;

	typedef __s32						err_t;
	#define __err_t						__s32
#endif


typedef bool (*b_proc_pv)( void * );

typedef void (*v_proc_i)( int );								// signal handler
typedef void (*v_proc_v)( void );
typedef void (*v_proc_pv)( void * );							// free
typedef void (*v_proc_pv_i)( void *, int );
typedef void (*v_proc_pv_f)( void *, float );
typedef void (*v_proc_pv_d_cpc)( void *, double, const char * );

typedef void *(*pv_proc_pv)( void * );							// pthread_entry_proc
typedef void *(*pv_proc_ul)( unsigned long );

typedef void *(*malloc_t)( size_t );

// result is to zero as in_lhs is to in_rhs
typedef __s32 (*comparison_proc)( const void *in_lhs, const void *in_rhs );

enum {
	k_word_bits					=	sizeof(word) * 8				,
	k_dword_bits				=	sizeof(dword) * 8				,

	k_1k						=	1 << 10							,
	k_2k						=	1 << 11							,
	k_4k						=	1 << 12							,
	k_8k						=	1 << 13							,
	k_16k						=	1 << 14							,
	k_32k						=	1 << 15							,
	k_64k						=	1 << 16							,
	k_128k						=	1 << 17							,
	k_256k						=	1 << 18							,
	k_512k						=	1 << 19							,
	k_1m						=	1 << 20							,
	k_2m						=	1 << 21							,
	k_4m						=	1 << 22							,
	k_8m						=	1 << 23							,
	k_16m						=	1 << 24							,
	k_32m						=	1 << 25							,
	k_64m						=	1 << 26							,
	k_128m						=	1 << 27							,
	k_256m						=	1 << 28							,
	k_512m						=	1 << 29							,
	k_1g						=	1 << 30							,
	k_2g						=	1 << 31							,

	k_bits_per_byte				=	8								,

	k_unlimited					=	-1
};


enum {
	k_descriptor_closed			=	-1								,
	k_socket_closed				=	-1
};


enum ascii_constant {
	k_ascii_null													,		// 0x00
	k_ascii_start_of_heading										,		// 0x01
	k_ascii_start_of_text											,		// 0x02
	k_ascii_end_of_text												,		// 0x03
	k_ascii_end_of_transmission										,		// 0x04
	k_ascii_enquiry													,		// 0x05
	k_ascii_acknowledge												,		// 0x06
	k_ascii_bell													,		// 0x07
	k_ascii_backspace												,		// 0x08
	k_ascii_horizontal_tab											,		// 0x09
	k_ascii_newline													,		// 0x0A
	k_ascii_vertical_tab											,		// 0x0B
	k_ascii_form_feed												,		// 0x0C
	k_ascii_carriage_return											,		// 0x0D
	k_ascii_shift_out												,		// 0x0E
	k_ascii_shift_in												,		// 0x0F
	k_ascii_data_link_escape										,		// 0x10
	k_ascii_device_control_1										,		// 0x11
	k_ascii_device_control_2										,		// 0x12
	k_ascii_device_control_3										,		// 0x13
	k_ascii_device_control_4										,		// 0x14
	k_ascii_negative_acknowledge									,		// 0x15
	k_ascii_synchronous_idle										,		// 0x16
	k_ascii_end_of_transmission_block								,		// 0x17
	k_ascii_cancel													,		// 0x18
	k_ascii_end_of_medium											,		// 0x19
	k_ascii_substitute												,		// 0x1A
	k_ascii_escape													,		// 0x1B
	k_ascii_file_separator											,		// 0x1C
	k_ascii_group_separator											,		// 0x1D
	k_ascii_record_separator										,		// 0x1E
	k_ascii_unit_separator											,		// 0x1F
	k_ascii_space													,		// 0x20  
	k_ascii_exclamation_point										,		// 0x21 !
	k_ascii_double_quote											,		// 0x22 "
	k_ascii_hash													,		// 0x23 #
	k_ascii_dollar													,		// 0x24 $
	k_ascii_percent													,		// 0x25 %
	k_ascii_ampersand												,		// 0x26 &
	k_ascii_single_quote											,		// 0x27 '
	k_ascii_left_parenthesis										,		// 0x28 (
	k_ascii_right_parenthesis										,		// 0x29 )
	k_ascii_asterisk												,		// 0x2A *
	k_ascii_plus													,		// 0x2B +
	k_ascii_comma													,		// 0x2C ,
	k_ascii_minus													,		// 0x2D -
	k_ascii_period													,		// 0x2E .
	k_ascii_forward_slash											,		// 0x2F /
	k_ascii_0														,		// 0x30 0
	k_ascii_1														,		// 0x31 1
	k_ascii_2														,		// 0x32 2
	k_ascii_3														,		// 0x33 3
	k_ascii_4														,		// 0x34 4
	k_ascii_5														,		// 0x35 5
	k_ascii_6														,		// 0x36 6
	k_ascii_7														,		// 0x37 7
	k_ascii_8														,		// 0x38 8
	k_ascii_9														,		// 0x39 9
	k_ascii_colon													,		// 0x3A :
	k_ascii_semicolon												,		// 0x3B ;
	k_ascii_less_than												,		// 0x3C <
	k_ascii_equals													,		// 0x3D =
	k_ascii_greater_than											,		// 0x3E >
	k_ascii_question_mark											,		// 0x3F ?
	k_ascii_at														,		// 0x40 @
	k_ascii_A														,		// 0x41 A
	k_ascii_B														,		// 0x42 B
	k_ascii_C														,		// 0x43 C
	k_ascii_D														,		// 0x44 D
	k_ascii_E														,		// 0x45 E
	k_ascii_F														,		// 0x46 F
	k_ascii_G														,		// 0x47 G
	k_ascii_H														,		// 0x48 H
	k_ascii_I														,		// 0x49 I
	k_ascii_J														,		// 0x4A J
	k_ascii_K														,		// 0x4B K
	k_ascii_L														,		// 0x4C L
	k_ascii_M														,		// 0x4D M
	k_ascii_N														,		// 0x4E N
	k_ascii_O														,		// 0x4F O
	k_ascii_P														,		// 0x50 P
	k_ascii_Q														,		// 0x51 Q
	k_ascii_R														,		// 0x52 R
	k_ascii_S														,		// 0x53 S
	k_ascii_T														,		// 0x54 T
	k_ascii_U														,		// 0x55 U
	k_ascii_V														,		// 0x56 V
	k_ascii_W														,		// 0x57 W
	k_ascii_X														,		// 0x58 X
	k_ascii_Y														,		// 0x59 Y
	k_ascii_Z														,		// 0x5A Z
	k_ascii_left_bracket											,		// 0x5B [
	k_ascii_backward_slash											,		// 0x5C \ /**/
	k_ascii_right_bracket											,		// 0x5D ]
	k_ascii_caret													,		// 0x5E ^
	k_ascii_underscore												,		// 0x5F _
	k_ascii_backtick												,		// 0x60 `
	k_ascii_a														,		// 0x61 a
	k_ascii_b														,		// 0x62 b
	k_ascii_c														,		// 0x63 c
	k_ascii_d														,		// 0x64 d
	k_ascii_e														,		// 0x65 e
	k_ascii_f														,		// 0x66 f
	k_ascii_g														,		// 0x67 g
	k_ascii_h														,		// 0x68 h
	k_ascii_i														,		// 0x69 i
	k_ascii_j														,		// 0x6A j
	k_ascii_k														,		// 0x6B k
	k_ascii_l														,		// 0x6C l
	k_ascii_m														,		// 0x6D m
	k_ascii_n														,		// 0x6E n
	k_ascii_o														,		// 0x6F o
	k_ascii_p														,		// 0x70 p
	k_ascii_q														,		// 0x71 q
	k_ascii_r														,		// 0x72 r
	k_ascii_s														,		// 0x73 s
	k_ascii_t														,		// 0x74 t
	k_ascii_u														,		// 0x75 u
	k_ascii_v														,		// 0x76 v
	k_ascii_w														,		// 0x77 w
	k_ascii_x														,		// 0x78 x
	k_ascii_y														,		// 0x79 y
	k_ascii_z														,		// 0x7A z
	k_ascii_left_brace												,		// 0x7B {
	k_ascii_vertical_bar											,		// 0x7C |
	k_ascii_right_brace												,		// 0x7D }
	k_ascii_tilde													,		// 0x7E ~
	k_ascii_delete													,		// 0x7F 

	// aliases
	
	k_ascii_tab					=	k_ascii_horizontal_tab			,		// 0x09
	k_ascii_new_line			=	k_ascii_newline					,		// 0x0A
	k_ascii_line_feed			=	k_ascii_newline					,		// 0x0A
	k_ascii_new_page			=	k_ascii_form_feed				,		// 0x0C
	k_ascii_control_z			=	k_ascii_substitute				,		// 0x1A
	k_ascii_bang				=	k_ascii_exclamation_point		,		// 0x21
	k_ascii_pound				=	k_ascii_hash					,		// 0x23
	k_ascii_pound_sign			=	k_ascii_hash					,		// 0x23
	k_ascii_dollar_sign			=	k_ascii_dollar					,		// 0x24
	k_ascii_percent_sign		=	k_ascii_percent					,		// 0x25
	k_ascii_open_parenthesis	=	k_ascii_left_parenthesis		,		// 0x28
	k_ascii_close_parenthesis	=	k_ascii_left_parenthesis		,		// 0x29
	k_ascii_dash				=	k_ascii_minus					,		// 0x2D
	k_ascii_hyphen				=	k_ascii_minus					,		// 0x2D
	k_ascii_dot					=	k_ascii_period					,		// 0x2E
	k_ascii_at_sign				=	k_ascii_at						,		// 0x40
	k_ascii_open_bracket		=	k_ascii_left_bracket			,		// 0x5B
	k_ascii_backslash			=	k_ascii_backward_slash			,		// 0x5C
	k_ascii_close_bracket		=	k_ascii_right_bracket			,		// 0x5D
	k_ascii_up_caret			=	k_ascii_caret					,		// 0x5E
	k_ascii_back_tick			=	k_ascii_backtick				,		// 0x60
	k_ascii_open_brace			=	k_ascii_left_brace				,		// 0x7B
	k_ascii_pipe				=	k_ascii_vertical_bar			,		// 0x7C
	k_ascii_close_brace			=	k_ascii_right_brace						// 0x7D
};


enum numeric_base {
	k_octal						=	8								,
	k_decimal					=	10								,
	k_hexadecimal				=	16								,
	
	k_oct						=	k_octal							,
	k_dec						=	k_decimal						,
	k_hex						=	k_hexadecimal
};


#define os_type( _a, _b, _c, _d )	( ( _a ) << 24 | ( _b ) << 16 | ( _c ) << 8 | ( _d ) )


enum operating_system_t {
	k_os_beos					=	os_type( 'b', 'e', 'o', 's' )	,
	k_os_bsd					=	os_type( 'b', 's', 'd', '!' )	,
	k_os_darwin					=	os_type( 'm', 'a', 'c', 'x' )	,
	k_os_free_bsd				=	os_type( 'f', 'b', 's', 'd' )	,
	k_os_hpux					=	os_type( 'h', 'p', 'u', 'x' )	,
	k_os_irix					=	os_type( 'i', 'r', 'i', 'x' )	,
	k_os_linux					=	os_type( 'l', 'i', 'n', 'x' )	,
	k_os_mac_os_classic			=	os_type( 'm', 'a', 'c', '9' )	,
	k_os_os_2					=	os_type( 'o', 's', '_', '2' )	,
	k_os_solaris				=	os_type( 's', 'o', 'l', 'r' )	,
	k_os_sun_os					=	os_type( 's', 'u', 'n', '!' )	,
	k_os_win32					=	os_type( 'w', 'n', '3', '2' )	,
	k_os_windows				=	os_type( 'w', 'n', 'd', 'z' )	,
	k_os_windows_31				=	os_type( 'w', 'n', '3', '1' )	,
	k_os_windows_95				=	os_type( 'w', 'n', '9', '5' )	,
	k_os_windows_98				=	os_type( 'w', 'n', '9', '8' )	,
	k_os_windows_2k				=	os_type( 'w', 'n', '2', 'k' )	,
	k_os_windows_me				=	os_type( 'w', 'n', 'm', 'e' )	,
	k_os_windows_nt				=	os_type( 'w', 'n', 'n', 't' )	,
	k_os_windows_xp				=	os_type( 'w', 'n', 'x', 'p' )	
};


#if __linux__
	#define k_os				k_os_linux
#elif __MACH__
	#define k_os				k_os_darwin
#elif __sun__
	#define k_os				k_os_solaris
#elif _WIN32
	#define k_os				k_os_windows_xp
#else
#endif


#if _WIN32
	#define k_path_separator	'\\'
#else
	#define k_path_separator	'/'
#endif


#ifdef __cplusplus
}
#endif



#endif	// __common_types_h__
