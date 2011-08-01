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
** $Id: oscalls.c 3294 2008-02-24 02:45:41Z Fish $
*/

/* @file Portability wrapper functions
 */

#include "neostats.h"

/*
 *  Wrapper function for strftime
 */

size_t os_strftime( char *strDest, size_t maxsize, const char *format, const struct tm *timeptr )
{
	return strftime( strDest, maxsize, format, timeptr );
}

/*
 *  Wrapper function for localtime
 */

struct tm *os_localtime( const time_t *timer )
{
	return localtime( timer );
}

/*
 *  Wrapper function for memset
 */

void *os_memset( void *dest, int c, size_t count )
{
	return memset( dest, c, count );
}

/*
 *  Wrapper function for memcpy
 */

void *os_memcpy( void *dest, const void *src, size_t count )
{
	return memcpy( dest, src, count );
}

/*
 *  Wrapper function for malloc
 */

void *os_malloc( size_t size )
{
	return malloc( size );
}

/*
 *  Wrapper function for free
 */

void os_free( void *ptr )
{
	free( ptr );
}
