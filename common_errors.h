#ifndef __common_errors_h__
#define __common_errors_h__



#ifdef __cplusplus
extern "C" {
#endif


/*

// error enable macros:

#define ENABLE_CGI_ERRORS							0
#define ENABLE_CONTROL_FLOW_ERRORS					0
#define ENABLE_CRYPTO_ERRORS						0
#define ENABLE_DATA_ERRORS							0
#define ENABLE_LIBRARY_ERRORS						0
#define ENABLE_MATH_ERRORS							0
#define ENABLE_MISCELLANEOUS_ERRORS					0
#define ENABLE_TCP_ERRORS							0
#define ENABLE_TLS_ERRORS							0
#define ENABLE_VALIDATION_ERRORS					0

*/


// english language descriptions of error messages in this file
const char *error_string( err_t in_error );


#define k_error_version_base			-1000000

#define error_base( _top )				( k_error_version_base - _top - 1 )

enum common_errors {
	no_err								=	0					,
	
	k_first_common_error				=	error_base( 1000 )	,
	
	err_assertion_failure										,
	err_bad_parameter											,
	err_file_not_found											,
	err_fubar													,
	err_mem_full												,
	err_range													,
	err_read_failure											,
	err_unexpected_end_of_file									,
	err_unimplemented											,
	err_uninitialized											,
	err_unspecified_exception									,
	err_unsupported												,
	err_write_failure											,
};


#if ENABLE_CGI_ERRORS
enum cgi_errors {
	k_first_cgi_error					=	error_base( 2000 )	,

	err_cgi_invalid_index										,
	err_cgi_invalid_method										,
	err_cgi_key_not_found										,
	err_cgi_no_equal_character_in_argument						,
	err_cgi_not_invoked_as_cgi									,
	err_cgi_unexpected_end_of_content							,
	err_cgi_unspecified_content									,
};
#endif // ENABLE_CGI_ERRORS


#if ENABLE_CONTROL_FLOW_ERRORS
enum control_flow_errors {
	k_first_control_flow_error			=	error_base( 3000 )	,

	err_task_aborted											,
	err_task_aborted_no_completion								,
	err_timeout													,
};
#endif // ENABLE_CONTROL_FLOW_ERRORS


#if ENABLE_CRYPTO_ERRORS
enum crypto_errors {
	k_first_crypto_error				=	error_base( 4000 )	,

	err_decryption_failure										,
	err_encryption_failure										,
	err_invalid_signature										,
	err_prng_broken												,		// pseudo random number generator is misbehaving
};
#endif // ENABLE_CRYPTO_ERRORS


#if ENABLE_DATA_ERRORS
enum data_errors {
	k_first_data_error					=	error_base( 5000 )	,

	err_bad_data												,
	err_item_not_found											,
	err_no_data													,
	err_query_failed											,
	err_wrong_type												,
};
#endif // ENABLE_DATA_ERRORS


#if ENABLE_LIBRARY_ERRORS
enum library_errors {
	k_first_library_error				=	error_base( 6000 )	,

	err_unable_to_initialize_library							,
};
#endif // ENABLE_LIBRARY_ERRORS


#if ENABLE_MATH_ERRORS
enum math_errors {
	k_first_math_error					=	error_base( 7000 )	,

	err_divide_by_zero											,
	err_unsolvable												,
};
#endif // ENABLE_MATH_ERRORS


#if ENABLE_MISCELLANEOUS_ERRORS
enum miscellaneous_errors {
	k_first_miscellaneous_error			=	error_base( 8000 )	,

	err_censored												,
};
#endif // ENABLE_MISCELLANEOUS_ERRORS


#if ENABLE_TCP_ERRORS
enum tcp_errors {
	k_first_tcp_error					=	error_base( 9000 )	,

	err_already_connected										,
	err_could_not_connect										,
	err_could_not_line_buffer_stream							,
	err_local_address_unavailable								,
	err_not_connected											,
	err_not_listening											,
	err_remote_address_unavailable								,
	err_select_failed											,
	err_socket_closed											,
};
#endif // ENABLE_TCP_ERRORS


#if ENABLE_TLS_ERRORS
enum tls_errors {
	k_first_tls_error					=	error_base( 10000 )	,

	err_tls_unknown_method										,
	err_tls_cannot_create_context								,
	err_tls_cannot_read_certificate_file						,
	err_tls_cannot_read_rsa_private_key_file					,
	err_tls_could_not_read_trusted_ca_file						,
	err_tls_prng_seed_failure									,
	err_tls_cannot_create_session								,
	err_tls_unable_to_generate_rsa_key							,
	err_tls_unable_to_install_cipher							,
	err_tls_cannot_create_ssl									,
	err_tls_not_connected										,
	err_tls_peer_shutdown_cleanly								,
	err_tls_unknown_error										,
	err_tls_system_uninitialized								,
};
#endif // ENABLE_TLS_ERRORS


#if ENABLE_VALIDATION_ERRORS
enum validation_errors {
	k_first_validation_error			=	error_base( 11000 )	,

	err_expired													,
	err_not_enough_information									,
	err_serial_number_in_use									,
	err_validation_failure										,
};
#endif // ENABLE_VALIDATION_ERRORS



#if 0
enum filesystem_errors {
	err_unknown_user							=	-7001		,
	err_no_space_on_device						=	-7004		,
	err_volume_inconsistent						=	-7005		,
	err_unsupported_sector_size					=	-7006		,
	err_partition_too_small						=	-7007		,
	err_invalid_partition_size					=	-7008		,
	err_volume_mounted							=	-7009		,
	err_boot_volume_inconsistent				=	-7010		,
	err_volume_write_protected					=	-7011		,
	err_invalid_journal_header					=	-7012		,
	err_journal_needs_replay					=	-7013		,
	err_seek									=	-7015		,
};


enum pthread_errors {
	err_pthread									=	-8000		,
	err_mutex									=	-8001		,
	err_cannot_create_thread					=	-8002		,
	err_cannot_create_semaphore					=	-8003		,
	err_cannot_create_mutex						=	-8004		,
};


enum system_errors {
	err_no_displays								=	-8201		,		// no displays attached to system
	err_invalid_display							=	-8202		,
};


enum miscellaneous_runtime_errors {
	err_cannot_reprioritize_task				=	-8300		,
	err_task_queue_shutdown						=	-8303		,
//	err_invalid_argument						=	-8304		,
	err_no_authorization						=	-8305		,
	err_no_such_process							=	-8306		,
	err_user_cancelled							=	-8311		,
	err_object_in_use							=	-8314		,
	err_unsupported								=	-8315		,
	err_uncaught_exception						=	-8316		,
	err_requeue_task							=	-8317		,
	err_item_found								=	-8318		,
	err_checksum_mismatch						=	-8319		,
	err_command_failed							=	-8321		,
	err_login_disabled							=	-8322		,
	err_limit_exceeded							=	-8323		,
	err_disabled								=	-8324		,
	err_not_unique								=	-8325		,
	err_size_mismatch							=	-8326		,
	err_no_entry								=	-8327		,
	err_no_iterator								=	-8328		,
	err_no_property								=	-8329		,
	err_no_value								=	-8330		,
	err_unexpected_type							=	-8331		,
	err_no_resources							=	-8332		,
	err_program_too_old							=	-8336		,
	err_program_corrupt							=	-8337		,
};


enum commerce_errors {
	err_insufficient_funds						=	-8500		,
};


enum communication_errors {
	err_peer_disconnected						=	-9000		,
	err_send_timeout							=	-9002		,
	err_receive_timeout							=	-9003		,
	err_transport_failure						=	-9004		,
	err_unknown_client							=	-9005		,
	err_client_unavailable						=	-9006		,
	err_line_too_long							=	-9007		,
	err_select_interrupted						=	-9009		,
	err_could_not_get_interface_info			=	-9011		,
	err_no_configured_interfaces				=	-9012		,
	err_could_not_get_interface_address			=	-9013		,
	err_socket_not_writable						=	-9017		,
	err_socket_not_readable						=	-9018		,
	err_unknown_select_result					=	-9019		,
	err_listening								=	-9021		,
	err_no_port_specified						=	-9022		,
	err_socket_not_secure						=	-9026		,
	err_bad_credentials							=	-9028		,
	err_secure_failed							=	-9029		,
	err_client_disconnected						=	-9039		,
	err_no_reply_from_server					=	-9040		,
};


enum programmer_errors {
	err_invalid_base_class_access				=	-10000		,
	err_symbol_not_found						=	-10001		,
	err_see_message								=	-10003		,
	err_duplicate								=	-10006		,
	err_required_parameter_omitted				=	-10009		,
};
#endif


#ifdef __cplusplus
}
#endif



#endif // __common_errors_h__
