#if _WIN32
	#include "precompiled.h"
#endif

#include "common_errors.h"


const char *error_string( err_t in_error ) {
	const char			   *result;

	if ( in_error > 0 ) return strerror( in_error );
	if ( in_error == -1 && errno ) return strerror( errno );
	
	switch ( in_error ) {
		case no_err:											result = "no error";												break;

		case err_assertion_failure:								result = "assertion failure";										break;
		case err_bad_parameter:									result = "bad parameter";											break;
		case err_file_not_found:								result = "file not found";											break;
		case err_fubar:											result = "internal error";											break;
		case err_mem_full:										result = "out of memory";											break;
		case err_range:											result = "out of range";											break;
		case err_read_failure:									result = "read failed";												break;
		case err_unexpected_end_of_file:						result = "unexpected end of file";									break;
		case err_unimplemented:									result = "unimplemented";											break;
		case err_uninitialized:									result = "uninitialized";											break;
		case err_unspecified_exception:							result = "unspecified exception";									break;
		case err_unsupported:									result = "unsupported";												break;
		case err_write_failure:									result = "write failed";											break;

#if ENABLE_CGI_ERRORS
		case err_cgi_invalid_index:								result = "invalid index";											break;
		case err_cgi_invalid_method:							result = "invalid cgi method";										break;
		case err_cgi_key_not_found:								result = "key not found";											break;
		case err_cgi_no_equal_character_in_argument:			result = "no '=' character in cgi argument";						break;
		case err_cgi_not_invoked_as_cgi:						result = "cgi not invoked as cgi";									break;
		case err_cgi_unexpected_end_of_content:					result = "unexpected end of cgi content";							break;
		case err_cgi_unspecified_content:						result = "unspecified cgi content";									break;
#endif

#if ENABLE_CONTROL_FLOW_ERRORS
		case err_timeout:										result = "timeout";													break;
		case err_task_aborted:									result = "task aborted";											break;
		case err_task_aborted_no_completion:					result = "task aborted without completion";							break;
#endif

#if ENABLE_CRYPTO_ERRORS
		case err_decryption_failure:							result = "decryption failure";										break;
		case err_encryption_failure:							result = "encryption failure";										break;
		case err_invalid_signature:								result = "invalid signature";										break;
		case err_prng_broken:									result = "pseduo-random number generator failed";					break;
#endif

#if ENABLE_DATA_ERRORS
		case err_bad_data:										result = "bad data";												break;
		case err_item_not_found:								result = "item not found";											break;
		case err_no_data:										result = "no data available";										break;
		case err_query_failed:									result = "query failed";											break;
		case err_wrong_type:									result = "wrong type";												break;
#endif

#if ENABLE_LIBRARY_ERRORS
		case err_unable_to_initialize_library:					result = "unable to initialize library";							break;
#endif

#if ENABLE_MATH_ERRORS
		case err_divide_by_zero:								result = "divide by zero";											break;
		case err_unsolvable:									result = "unsolvable";												break;
#endif

#if ENABLE_MISCELLANEOUS_ERRORS
		case err_censored:										result = "censored";												break;
#endif

#if ENABLE_TCP_ERRORS
		case err_already_connected:								result = "already connected";										break;
		case err_could_not_connect:								result = "unable to connect";										break;
		case err_could_not_line_buffer_stream:					result = "unable to line buffer stream";							break;
		case err_local_address_unavailable:						result = "local address unavailable";								break;
		case err_not_connected:									result = "not connected";											break;
		case err_not_listening:									result = "not listening";											break;
		case err_remote_address_unavailable:					result = "remote address unobtainable";								break;
		case err_select_failed:									result = "select() failed";											break;
		case err_socket_closed:									result = "socket closed";											break;
#endif

#if ENABLE_TLS_ERRORS
		case err_tls_cannot_create_context:						result = "tls cannot create context";								break;
		case err_tls_cannot_create_session:						result = "tls cannot create session";								break;
		case err_tls_cannot_create_ssl:							result = "tls cannot create ssl";									break;
		case err_tls_cannot_read_certificate_file:				result = "tls cannot read certificate file";						break;
		case err_tls_cannot_read_rsa_private_key_file:			result = "tls cannot read rsa private key file";					break;
		case err_tls_could_not_read_trusted_ca_file:			result = "tls cannot read trusted ca file";							break;
		case err_tls_not_connected:								result = "tls not connected";										break;
		case err_tls_peer_shutdown_cleanly:						result = "tls peer shutdown cleanly";								break;
		case err_tls_prng_seed_failure:							result = "tls unable to seed pseudo-random number generator";		break;
		case err_tls_system_uninitialized:						result = "tls uninitialized";										break;
		case err_tls_unable_to_generate_rsa_key:				result = "tls unable to generate rsa key";							break;
		case err_tls_unable_to_install_cipher:					result = "tls unable to install cipher";							break;
		case err_tls_unknown_error:								result = "unknown tls error";										break;
		case err_tls_unknown_method:							result = "unknown tls method";										break;
#endif

#if ENABLE_VALIDATION_ERRORS
		case err_expired:										result = "expired";													break;
		case err_not_enough_information:						result = "not enough information";									break;
		case err_serial_number_in_use:							result = "serial number already in use";							break;
		case err_validation_failure:							result = "validation failed";										break;
#endif


#if 0
		case err_bad_credentials:								result = "bad credentials";											break;
		case err_boot_volume_inconsistent:						result = "boot volume inconsistent";								break;
		case err_cannot_create_mutex:							result = "cannot create mutex";										break;
		case err_cannot_create_semaphore:						result = "cannot create semaphore";									break;
		case err_cannot_create_thread:							result = "cannot create thread";									break;
		case err_cannot_reprioritize_task:						result = "cannot reprioritize task";								break;
		case err_checksum_mismatch:								result = "checksum mismatch";										break;
		case err_client_disconnected:							result = "client disconnected";										break;
		case err_client_unavailable:							result = "client unavailable";										break;
		case err_command_failed:								result = "command failed";											break;
		case err_could_not_get_interface_address:				result = "unable to get interface address";							break;
		case err_could_not_get_interface_info:					result = "unable to get interface info";							break;
		case err_disabled:										result = "disabled";												break;
		case err_duplicate:										result = "duplicate";												break;
		case err_insufficient_funds:							result = "insufficient funds";										break;
//		case err_invalid_argument:								result = "invalid argument";										break;
		case err_invalid_base_class_access:						result = "invalid base class access";								break;
		case err_invalid_display:								result = "invalid display";											break;
		case err_invalid_journal_header:						result = "invalid journal header";									break;
		case err_invalid_partition_size:						result = "invalid partition size";									break;
		case err_item_found:									result = "item found";												break;
		case err_journal_needs_replay:							result = "journal needs replay";									break;
		case err_limit_exceeded:								result = "limit exceeded";											break;
		case err_line_too_long:									result = "line too long";											break;
		case err_listening:										result = "already listening";										break;
		case err_login_disabled:								result = "login disabled";											break;
		case err_mutex:											result = "mutex failure";											break;
		case err_no_authorization:								result = "no authorization";										break;
		case err_no_configured_interfaces:						result = "no configured interfaces";								break;
		case err_no_displays:									result = "no display available";									break;
		case err_no_entry:										result = "no entry";												break;
		case err_no_iterator:									result = "no iterator";												break;
		case err_no_port_specified:								result = "no port specified";										break;
		case err_no_property:									result = "no property";												break;
		case err_no_reply_from_server:							result = "no reply from server";									break;
		case err_no_resources:									result = "no resources";											break;
		case err_no_space_on_device:							result = "no space left on device";									break;
		case err_no_such_process:								result = "no such process";											break;
		case err_no_value:										result = "no value";												break;
		case err_not_unique:									result = "not unique";												break;
		case err_object_in_use:									result = "object in use";											break;
		case err_partition_too_small:							result = "partition too small";										break;
		case err_peer_disconnected:								result = "peer disconnected";										break;
		case err_program_too_old:								result = "program version is too old to continue, please upgrade";	break;
		case err_program_corrupt:								result = "program corruption detected, please reinstall";			break;
		case err_pthread:										result = "pthread failure";											break;
		case err_read_failure:									result = "read failed";												break;
		case err_receive_timeout:								result = "receive timeout";											break;
		case err_requeue_task:									result = "requeue task";											break;
		case err_required_parameter_omitted:					result = "required parameter omitted";								break;
		case err_secure_failed:									result = "secure failed";											break;
		case err_see_message:									result = "additional details follow";								break;
		case err_seek:											result = "seek failed";												break;
		case err_select_interrupted:							result = "select() interrupted";									break;
		case err_send_timeout:									result = "send timeout";											break;
		case err_size_mismatch:									result = "size mismatch";											break;
		case err_socket_not_readable:							result = "socket not readable";										break;
		case err_socket_not_secure:								result = "socket not secure";										break;
		case err_socket_not_writable:							result = "socket not writable";										break;
		case err_symbol_not_found:								result = "symbol not found";										break;
		case err_task_aborted_no_completion:					result = "task aborted without completion";							break;
		case err_task_aborted:									result = "task aborted";											break;
		case err_task_queue_shutdown:							result = "task queue shutdown";										break;
		case err_transport_failure:								result = "transport failure";										break;
		case err_uncaught_exception:							result = "uncaught exception";										break;
		case err_unexpected_type:								result = "unexpected type";											break;
		case err_unknown_client:								result = "unknown client";											break;
		case err_unknown_select_result:							result = "unknown select() result";									break;
		case err_unknown_user:									result = "unknown user";											break;
		case err_unsupported_sector_size:						result = "unsupported sector size";									break;
		case err_unsupported:									result = "unsupported operation";									break;
		case err_user_cancelled:								result = "user canceled";											break;
		case err_volume_inconsistent:							result = "volume inconsistent";										break;
		case err_volume_mounted:								result = "volume already mounted";									break;
		case err_volume_write_protected:						result = "volume write protected";									break;
#endif
		
		default:												result = "unknown error";											break;
	}

	return result;
}
