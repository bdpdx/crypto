#ifndef __file_h__
#define __file_h__



#if _WIN32
	#include "precompiled.h"
#else
	#include <unistd.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>


#ifndef O_BINARY
	#define O_BINARY	0
#endif

#define k_default_open_flags			O_RDONLY | O_BINARY
#define k_default_append_flags			O_WRONLY | O_CREAT | O_APPEND | O_BINARY
#define k_default_create_flags			O_WRONLY | O_CREAT | O_EXCL | O_BINARY
#define k_default_truncate_flags		O_WRONLY | O_CREAT | O_TRUNC | O_BINARY
#if _WIN32
	#define k_default_create_mode		0
#else
	#define k_default_create_mode		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#endif
#define k_default_FILE_read_mode		"r"
#define k_default_FILE_write_mode		"w"
#define k_default_FILE_read_write_mode	"r+"

#define open_create( in_path )			open( ( in_path ), k_default_create_flags, k_default_create_mode )
#define open_append( in_path )			open( ( in_path ), k_default_append_flags, k_default_create_mode )
#define open_truncate( in_path )		open( ( in_path ), k_default_truncate_flags, k_default_create_mode )

#define seek_begin()					seek( 0, SEEK_SET )
#define seek_end()						seek( 0, SEEK_END )
#define fseek_begin()					fseek( 0, SEEK_SET )
#define fseek_end()						fseek( 0, SEEK_END )


namespace balance {


class file {

public:

	file();
	file( const char *in_path, __s32 in_open_flags = k_default_open_flags, mode_t in_open_mode = k_default_create_mode, const char *in_FILE_mode = nil );
   ~file();

	void close();

	__s32 open( const char *in_path, __s32 in_open_flags = k_default_open_flags, mode_t in_mode = k_default_create_mode, const char *in_FILE_mode = nil );

	FILE *open_FILE( const char *in_path = nil, const char *in_open_mode = k_default_FILE_read_mode );

	off_t seek( off_t in_offset = 0, __s32 in_whence = SEEK_CUR );
	off_t fseek( off_t in_offset = 0, __s32 in_whence = SEEK_CUR );
	
	off_t size();

	__s32 read( void *out_buffer, __u32 in_length );
	__s32 write( const void *in_buffer, __u32 in_length );
	
	__u32 fread( void *out_buffer, __u32 in_length );
	__u32 fwrite( const void *in_buffer, __u32 in_length );

	operator int();
	operator FILE *();
	
	// acquire() opens in_file and gets in_size bytes to io_buffer, starting at
	// offset in_offset.  if io_buffer is nil, a buffer is allocated to contain
	// the data, and the caller is then responsible for delete[]ing the buffer
	// (which is returned).  if io_size is zero, acquire() reads the entire file
	// into the buffer.  either io_buffer or the allocated buffer is returned,
	// depending on whether io_buffer is nil on input or not.  the size of the
	// buffer is returned in io_size;
	static __u8 *acquire( const char *in_path, __u32 &io_size, __u8 *io_buffer = nil, off_t in_offset = 0 );
	static void copy( const char *in_src, const char *in_dst );
	static bool exists( const char *in_path );
	static void make( const char *in_path, const void *in_buffer, __u32 in_size, __u32 in_mode = k_default_create_mode );
	static void move( const char *in_src, const char *in_dst );
	static off_t size( const char *in_path );
	// temp creates a temporary file in /tmp and opens it for reading and writing (via mkstemp)
	// if out_filename is non-nil on output it will contain the filename of the temp file and must be
	// delete[]'ed by the caller.  the returned file object must be deleted by caller.
	static file *temp( char **out_filename = nil );
	static void touch( const char *in_path );
	static void unlink( const char *in_path );

	/*	make_path() concatenates in_filename to in_path, inserting a directory separator character
		if necessary and returns the result.  caller is responsible for delete[]ing result. */
	static char *make_path( const char *in_path, const char *in_filename );

protected:

	__s32			m_fd;
	FILE		   *m_fp;

};


inline file::file() { m_fd = k_descriptor_closed; m_fp = nil; }
inline file::file( const char *in_path, __s32 in_open_flags, mode_t in_open_mode, const char *in_FILE_mode ) { m_fp = nil; m_fd = k_descriptor_closed; open( in_path, in_open_flags, in_open_mode, in_FILE_mode ); }
inline file::~file() { close(); }

inline void file::close() {
	if ( m_fp ) {
		fclose( m_fp );
		m_fp = nil;
	}

	if ( m_fd != k_descriptor_closed ) {
		::close( m_fd );
		m_fd = k_descriptor_closed;
	}
}

inline __s32 file::open( const char *in_path, __s32 in_open_flags, mode_t in_mode, const char *in_FILE_mode ) {
	close();
	
	_throw_errno_if( ( m_fd = ::open( in_path, in_open_flags, in_mode ) ) == -1 );

	if ( in_FILE_mode ) open_FILE( m_fd == k_descriptor_closed ? in_path : nil, in_FILE_mode );
	
	return m_fd;
}

inline FILE *file::open_FILE( const char *in_path, const char *in_mode ) {
	if ( m_fp ) fclose( m_fp );
	
	_throw_errno_if( ! ( m_fp = in_path ? fopen( in_path, in_mode ) : fdopen( m_fd, in_mode ) ) );
	clearerr( m_fp );
	
	return m_fp;
}

inline off_t file::seek( off_t in_offset, __s32 in_whence ) { off_t result; _throw_errno_if( ( result = lseek( m_fd, in_offset, in_whence ) ) == -1 ); return result; }
inline off_t file::fseek( off_t in_offset, __s32 in_whence ) { off_t result; _throw_errno_if( fseeko( m_fp, in_offset, in_whence ) == -1 ); _throw_errno_if( ( result = ftello( m_fp ) ) == -1 ); return result; }
inline off_t file::size() { struct stat info; _throw_errno_if( fstat( m_fd, &info ) == -1 ); return info.st_size; }

inline __s32 file::read( void *out_buffer, __u32 in_length ) { __s32 result; _throw_errno_if( ( result = ::read( m_fd, out_buffer, in_length ) ) == -1 ); return result; }
inline __s32 file::write( const void *in_buffer, __u32 in_length ) { __s32 result; _throw_errno_if( ( result = ::write( m_fd, in_buffer, in_length ) ) == -1 ); return result; }

inline __u32 file::fread( void *out_buffer, __u32 in_length ) { __u32 result; result = (__u32) ::fread( out_buffer, 1, in_length, m_fp ); _throw_if( ferror( m_fp ) ); return result; }
inline __u32 file::fwrite( const void *in_buffer, __u32 in_length ) {
	__s32	err;
	__u32	result;

	if ( ( result = (__u32) ::fwrite( in_buffer, 1, in_length, m_fp ) ) != in_length ) {
		if ( ! ( err = ferror( m_fp ) ) ) err = err_write_failure;
		_throw( err );
	}

	return result;
}

inline file::operator int() { if ( m_fd ) return m_fd; _throw( err_file_not_found ); }
inline file::operator FILE *() { if ( m_fp ) return m_fp; _throw( err_file_not_found ); }

inline off_t file::size( const char *in_path ) { struct stat info; _throw_errno_if( ::stat( in_path, &info ) == -1 ); return info.st_size; }
inline bool file::exists( const char *in_path ) { struct stat info; return ! ::stat( in_path, &info ); }
inline void file::unlink( const char *in_path ) { _throw_errno_if( ::unlink( in_path ) == -1 ); }
inline void file::touch( const char *in_path ) { int fd = ::open( in_path, O_RDONLY | O_CREAT, k_default_create_mode ); _throw_errno_if( fd == -1 ); ::close( fd ); }
inline void file::move( const char *in_src, const char *in_dst ) { _throw_errno_if( rename( in_src, in_dst ) == -1 ); }

inline char *file::make_path( const char *in_path, const char *in_filename ) {
	char	   *path;
	bool		needs_slash;
	size_t		path_length, name_length;
	
	path_length = strlen( in_path );
	name_length = strlen( in_filename );
	
	needs_slash = in_path[ path_length - 1 ] != '/';

	path = new char[ path_length = path_length + ( needs_slash ? 1 : 0 ) + name_length + 1 ];

	snprintf( path, path_length, "%s%s%s", in_path, needs_slash ? "/" : "", in_filename );
	
	return path;
}


} // namespace balance


#endif // __file_hpp__
