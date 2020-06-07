#include "file.h"


__u8 *balance::file::acquire( const char *in_path, __u32 &io_size, __u8 *io_buffer, off_t in_offset ) {
	__u8				   *buffer = nil;
	file					f;
	__s32					n, size;

	if ( ! io_size ) io_size = file::size( in_path );

	buffer = io_buffer ? io_buffer : new __u8[ io_size ];
	
	_try {
		f.open( in_path );

		if ( in_offset ) f.seek( in_offset, SEEK_SET );

		n = size = 0;

		for ( size = 0; size < __s32(io_size); size += n ) {
			if ( ! ( n = f.read( buffer + size, io_size - size ) ) ) _throw( err_read_failure );
		}
	} _catch
	
	if ( _err && ! io_buffer ) delete[] buffer;

	_return buffer;
}


void balance::file::copy( const char *in_src, const char *in_dst ) {
	char			   *buf = nil;
	err_t_exception		err;
	int					fdi = -1, fdo = -1, n;
	struct stat			sb;
	
	_throw_errno_if( stat( in_src, &sb ) == -1 );
	_throw_errno_if( ( fdi = ::open( in_src, O_RDONLY, 0 ) ) == -1 );

	try {
		_throw_errno_if( ( fdo = ::open( in_dst, O_WRONLY | O_CREAT | O_TRUNC, sb.st_mode & 07777 ) ) == -1 );

		buf = new char[ k_512k ];

		for ( ;; ) {
			_throw_errno_if( ( n = ::read( fdi, buf, k_512k ) ) == -1 );

			if ( ! n ) break;
			
			_throw_errno_if( ::write( fdo, buf, n ) == -1 );
		}
	} catch ( err_t_exception in_err ) {
		err = in_err;
	}
	
	delete[] buf;
	
	if ( fdo != -1 ) ::close( fdo );
	if ( fdi != -1 ) ::close( fdi );
	
	if ( err.err ) {
		_no_throw( unlink( in_dst ) );
		throw err;
	}
}


void balance::file::make( const char *in_path, const void *in_buffer, __u32 in_size, __u32 in_mode ) {
	int						fd;

	_throw_errno_if( ( fd = ::open( in_path, k_default_truncate_flags, in_mode ) ) == -1 );
	_try { _throw_errno_if( ::write( fd, in_buffer, in_size ) == -1 ); } _catch
	::close( fd );
	_return;
}


balance::file *balance::file::temp( char **out_filename ) {
	char				   *filename;
	file				   *result;

	if ( out_filename ) *out_filename = nil;
	
	result = new file;
	
	_try {
		filename = new char[ 16 ];

		try {
#if _WIN32
	#error fix me
#else
			snprintf( filename, 16, "/tmp/XXXXXXXXXX" );

			_throw_errno_if( ( result->m_fd = mkstemp( filename  ) ) == -1 );
#endif
		} _catch
		
		if ( _err ) delete filename;
		
		_throw_now();
		
		if ( out_filename ) *out_filename = filename;
		else delete filename;
	} _catch
	
	_if_err delete result;
	
	_return result;
	
	
}
