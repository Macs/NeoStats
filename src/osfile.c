/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2008 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
**  USA
**
** NeoStats CVS Identification
** $Id: oscalls.c 2386 2005-03-21 23:26:24Z Mark $
*/

/* @file Portability wrapper functions
 */

/*  TODO:
 *  - port file functions from CRT to Win32 native calls (CreateFile etc)
 */

#include "neostats.h"
#ifdef HAVE_FCNTL_H
#include <fcntl.h> 
#endif

int os_file_errno;
static char tempbuf[BUFSIZE*2];

/*
 *  Wrapper function for mkdir
 */

int os_mkdir( const char *filename, mode_t mode )
{
	int retval;

#ifdef WIN32
	retval = _mkdir( filename );
#else
	retval = mkdir( filename, mode );
#endif
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for check_create_dir
 */

int os_create_dir( const char *dirname )
{
	struct stat st;
	int res;

	/* first, make sure the logdir dir exists */
	res = stat( dirname, &st );
	if( res != 0 ) {
		/* hrm, error */
		if( errno == ENOENT ) {
			/* ok, it doesn't exist, create it */
			res = os_mkdir( dirname, 0700 );
			if( res != 0 ) {
				/* error */
				nlog( LOG_CRITICAL, "Couldn't create directory: %s", strerror( errno ) );
				return NS_FAILURE;
			}
			nlog( LOG_NOTICE, "Created directory: %s", dirname );
		} else {
			nlog( LOG_CRITICAL, "Stat returned error: %s", strerror( errno ) );
			return NS_FAILURE;
		}
	} else if( !S_ISDIR( st.st_mode ) )	{
		nlog( LOG_CRITICAL, "%s is not a directory", dirname );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/*
 *  Wrapper function for fopen
 */

FILE* os_fopen( const char *filename, const char *filemode )
{
	FILE *retval;

	retval = fopen( filename, filemode );
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for fclose
 */

int os_fclose( FILE *handle )
{
	int retval;

	retval = fclose( handle );
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for fseek
 */

int os_fseek( FILE *handle, long offset, int origin )
{
	int retval;

	retval = fseek( handle, offset, origin );
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for ftell
 */

long os_ftell( FILE *handle )
{
	int retval;

	retval = ftell( handle );
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for fprintf
 */

int os_fprintf( FILE *handle, const char *fmt, ... )
{
	int retval;
	va_list ap;

	va_start( ap, fmt );
	ircvsnprintf( tempbuf, BUFSIZE*2, fmt, ap );
	va_end( ap );
	retval = fprintf( handle, "%s", tempbuf );	
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for fputs
 */

int os_fputs( const char *string, FILE *handle )
{
	int retval;

	retval = fputs( string, handle );	
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for fread
 */

size_t os_fread( void *buffer, size_t size, size_t count, FILE* handle )
{
	size_t retval;

	retval = fread( buffer, size, count, handle );
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for fgets
 */

char *os_fgets( char *string, int n, FILE* handle )
{
	char *retval;

	retval = fgets( string, n, handle );
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for fwrite
 */

size_t os_fwrite( const void *buffer, size_t size, size_t count, FILE* handle )
{
	size_t retval;

	retval = fwrite( buffer, size, count, handle );
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for fflush
 */

int os_fflush( FILE *handle )
{
	int retval;

	retval = fflush( handle );
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for rename
 */

int os_rename( const char* oldname, const char* newname )
{
	int retval;
    
	/*  WIN32 does not allow rename if the file exists 
	 *  Behaviour is undefined on various systems so 
	 *  remove on all platforms
	 */
	remove( newname );
	retval = rename( oldname, newname );
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for stat
 */

int os_stat( const char *path, struct stat *buffer )
{
	int retval;

	retval = stat( path, buffer );
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for access
 */

int os_access( const char *path, int mode )
{
	int retval;

#ifdef WIN32
	retval = _access( path, mode );
#else /* WIN32 */
	retval = access( path, mode );
#endif /* WIN32 */
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for write
 */

int os_write( int fd, const void *buffer, unsigned int count )
{
	int retval;

#ifdef WIN32
	retval = _write( fd, buffer, count );
#else /* WIN32 */
	retval = write( fd, buffer, count );
#endif /* WIN32 */
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for chmod
 */

int os_chmod( const char *filename, int pmode )
{
	int retval;

#ifdef WIN32
	retval = _chmod( filename, pmode );
#else /* WIN32 */
	retval = chmod( filename, pmode );
#endif /* WIN32 */
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for close
 */

int os_close( int fd )
{
	int retval;

#ifdef WIN32
	retval = _close( fd );
#else /* WIN32 */
	retval = close( fd );
#endif /* WIN32 */
	os_file_errno = errno;
	return retval;
}

/*
 *  Wrapper function for mkstemp
 */

int os_mkstemp( char *ftemplate )
{
#ifdef WIN32
	int retval;
	char *name;

	name = _mktemp( ftemplate );
	if( name )
	{
		retval = _open( name, _O_CREAT, _S_IREAD | _S_IWRITE );
		os_file_errno = errno;
		return retval;
	}
	return -1;
#else
	return mkstemp( ftemplate );
#endif
}

/*
 *  Wrapper function for write_temp_file
 */

int os_write_temp_file( char *ftemplate, const void *buffer, unsigned int count )
{
#ifdef WIN32
	char *name;

	name = _mktemp( ftemplate );
	if( name )
	{
		FILE* file;
		file = fopen( name, "w" );
		if( file )
		{
			fwrite( buffer, count, 1, file );
			fclose( file );
			return 0;
		}
	}
	return -1;
#else
	int i;

	i = mkstemp( ftemplate );
	write(i, buffer, count );
	close(i);
	return 0;
#endif
}

/*
 *  Wrapper function for strerror
 */

char *os_strerror( void )
{
	return strerror( errno );
}

/*
 *  Wrapper function for file_get_size
 */

int os_file_get_size( const char* filename )
{
	struct stat st;
	int res;

	res = stat( filename, &st );
	if( res != 0 ) {
		if( errno == ENOENT ) {
			nlog( LOG_CRITICAL, "No such file: %s", filename );
			return -1;
		} else {
			nlog( LOG_CRITICAL, "File error: %s", os_strerror() );
			return -1;
		}
	}
	return st.st_size;
}
