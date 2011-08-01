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
** $Id: support.c 3320 2008-03-05 09:22:44Z Fish $
*/

/* @file potentially missing C library functions
 */

#include "neostats.h"

#ifndef HAVE_STRNLEN
/** @brief strnlen
 *
 *  Find length of string up to count max
 *
 *  @param src string to find length of
 *  @param count limit of search
 *
 *  @return length of string excluding NULL or count if longer
 */

size_t strnlen( const char * src, size_t count )
{
	size_t len;

	/* Run through the string until we find NULL or reach count */
	for( len = 0; len < count; len++, src++ )
	{
		if( *src == 0 )
			return len;
	}
	/* src is longer or equal to count so return count */
	return count;
}
#endif /* HAVE_STRNLEN */

#ifndef HAVE_STRLCPY
/** @brief strlcpy
 *
 *  Copy up to size-1 characters from src to dst and NULL terminate
 *
 *  @param dst to copy to
 *  @param src to copy from
 *  @param count max size to copy
 *
 *  @return total characters written to string.
 */

size_t strlcpy( char *dst, const char *src, size_t size )
{
	size_t copycount;

    /* check size is safe */
	if ( size == 0 )
		return 0;
	/* NULL pointer checks */
	if( !dst || !src )
		return 0;
	/* use strnlen so huge strings do not hold us up */
	for( copycount = 0; copycount < size-1 && *src!=0 ; copycount++ )
		*dst++=*src++;
    /* Always null terminate */
	*dst = 0;
    /* count of characters written excluding NULL terminator */
	return copycount;
}
#endif /* HAVE_STRLCPY */

#ifndef HAVE_STRLCAT

/** @brief strlcat
 *
 *  Append at most size-len( dst )-1 chars from src to dst and NULL terminate
 *
 *  @param dst to copy to
 *  @param src to copy from
 *  @param count max size to copy
 *
 *  @return total characters written to string.
 */

size_t strlcat( char *dst, const char *src, size_t size )
{
	size_t lendst;
	size_t copycount;

    /* check size is safe */
	if ( size == 0 )
		return 0;
	/* NULL pointer checks */
	if( !dst || !src )
		return 0;
	/* if src contains NULL just NULL dst then quit to save a little CPU */
	if( *src == '\0' )
	{
		lendst = strnlen( dst, size );
		dst[lendst] = '\0';
		return 0;
	}
	/* use strnlen so huge strings do not hold us up */
	lendst = strnlen( dst, size );
	copycount = strnlen( src, size );
	/* bound copy size */
	if ( lendst + copycount >= size )
		copycount = size - lendst;
	/* memcpy the desired amount */
	if ( copycount > 0 ) 
	{
		os_memcpy( ( dst + lendst ), src, copycount );
		dst[lendst + copycount] = 0;
	}

    /* count of characters written excluding NULL terminator */
	return copycount;
}
#endif /* HAVE_STRLCAT */

#ifndef HAVE_STRNDUP
/** @brief strndup
 *
 *  allocate RAM and duplicate the passed string into the created buffer. 
 *  Always NULL terminates the new string.
 *  Suitable for partial string copies.
 *  Returned string will be count + 1 in length
 *
 *  @param src to copy from
 *  @param count max size to copy
 *
 *  @return pointer to new string or NULL if failed to allocate
 */

char *strndup( const char *src, size_t count )
{
	char *dst;
	
	/* validate inputs */
	if ( ( src == NULL ) || ( count == 0 ) )
		return NULL;
	/* Allocate count plus one for trailing NULL */
	dst = ( char * ) ns_malloc( count + 1 );
	/* Copy string into created buffer */
	os_memcpy( dst, src, count );
	dst[count] = 0;
	/* Return pointer to duplicated string */
	return dst;
}
#endif /* HAVE_STRNDUP */

#ifndef HAVE_STRDUP
/** @brief strdup
 *
 *  allocate RAM and duplicate the passed string into the created buffer. 
 *
 *  @param src to copy from
 *
 *  @return pointer to new string or NULL if failed to allocate
 */

char *strdup( const char *src )
{
	char *dst;
	
	/* validate inputs */
	if ( src == NULL )
		return NULL;
	/* Allocate count plus one for trailing NULL */
	dst = ( char * )ns_malloc( strlen( src ) + 1 );
	/* Copy string into created buffer */
	strlcpy( dst, src, strlen( src ) + 1 );
	/* Return pointer to duplicated string */
	return dst;
}
#endif /* HAVE_STRDUP */

#ifndef HAVE_STRCASESTR
/** @brief strcasestr
 *
 *  case insensitive sub string search
 *
 *  @param s1 string to search
 *  @param s2 substring to locate
 *
 *  @return pointer to location of substring or NULL if not found
 */

char *strcasestr( const char *s1, const char *s2 )
{
	while( *s1 != '\0' )
	{
		const char *ps1 = s1;
		const char *ps2 = s2;

		while( *ps2 != '\0' )
		{
			if( toupper( *ps2 ) != toupper( *ps1 ) ) 
				break;
			ps2++;
			ps1++;
		}
		if( !*ps2 ) 
			return (char *) s1;
		s1++;
	}
	return NULL;
}
#endif /* HAVE_STRCASESTR */

#ifndef HAVE_INET_NTOP
/** @brief inet_ntop
 *
 *  convert IPv4 addresses between binary and text form
 *
 *  @param af AF_INET
 *  @param src buffer containing IPv4 address in network byte order
 *  @param dst buffer to store result
 *  @param size of dst
 *
 *  @return pointer to the buffer containing the text string if succeeds else NULL
 */

char *inet_ntop( int af, const unsigned char *src, char *dst, size_t size )
{ 
	static const char *fmt = "%u.%u.%u.%u";
	char tmp[ sizeof( "255.255.255.255" )];

	/* only supports AF_INET at present */
	if ( af != AF_INET )
		return NULL;
	if ( ( size_t ) sprintf( tmp, fmt, src[0], src[1], src[2], src[3] ) >= size )
		return NULL;
	strlcpy( dst, tmp, size );
	return dst;
}
#endif /* HAVE_INET_NTOP */

#ifndef HAVE_INET_ATON
/** @brief inet_nton
 *
 *  Convert from "a.b.c.d" IP address string into an in_addr structure.  
 *
 *  @param 
 *  @param 
 *
 *  @return 0 on failure, 1 on success.
 */

int inet_aton( const char *name, struct in_addr *addr )
{
    addr->s_addr = inet_addr( name );
    return ( addr->s_addr == INADDR_NONE  ? 0 : 1 );
}
#endif /* HAVE_INET_ATON */
#ifndef HAVE_STRSEP
char *
strsep (char **stringp, const char *delim)
{
  char *begin, *end;

  begin = *stringp;
  if (begin == NULL)
    return NULL;

  /* A frequent case is when the delimiter string contains only one
     character.  Here we don't need to call the expensive `strpbrk'
     function and instead work using `strchr'.  */
  if (delim[0] == '\0' || delim[1] == '\0')
    {
      char ch = delim[0];

      if (ch == '\0')
	end = NULL;
      else
	{
	  if (*begin == ch)
	    end = begin;
	  else if (*begin == '\0')
	    end = NULL;
	  else
	    end = strchr (begin + 1, ch);
	}
    }
  else
    /* Find the end of the token.  */
    end = strpbrk (begin, delim);

  if (end)
    {
      /* Terminate the token and set *STRINGP past NUL character.  */
      *end++ = '\0';
      *stringp = end;
    }
  else
    /* No more delimiters; this is the last token.  */
    *stringp = NULL;

  return begin;
}
#endif
