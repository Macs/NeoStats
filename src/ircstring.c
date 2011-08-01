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
** $Id: ircstring.c 3294 2008-02-24 02:45:41Z Fish $
*/

/* @file irc optimised [v]s[n]printf routines
 * Currently only supports: %s, %c and %d
 * will call lib vsnprintf if unknown formats are encountered.
 * Approximately 50-75% performance increase (i.e. 2-300% faster) 
 * than lib functions
 */

#include "neostats.h"

/* buffer for itoa conversion 
 * int ranges from 
 *    -2,147,483,647 to 2,147,483,647 signed
 *    0 to 4,294,967,295 unsigned
 * so buffer size is 10 + 1 + 1 ( digits + sign + NULL )
 */
#define SCRATCHPAD_SIZE	( 10 + 1 + 1 )

static char scratchpad[SCRATCHPAD_SIZE] = 
{ 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 
};

static char nullstring[] = "(null)";

/** @brief ircvsnprintf
 *
 *  vsnprintf replacement optimised for IRC
 *
 *  @param buf Storage location for output. 
 *  @param size Maximum number of characters to write.
 *  @param fmt Format specification. 
 *  @param args Pointer to list of arguments. 
 *
 *  @return number of characters written excluding terminating null
 */

int ircvsnprintf( char *buf, size_t size, const char *fmt, va_list args ) 
{
	va_list saveargs;
	size_t len = 0;
	unsigned int i;
    char *str;
    char c;
	const char *format = fmt;

	/* save args in case we need to call vsnprintf */
	va_copy( saveargs, args );
	while( ( c = *format++ ) != 0 && ( len < size ) )
	{
		/* Is it a format string character? */
	    if( c == '%' ) 
		{
			switch( *format ) 
			{
				/* handle %s (string) */
				case 's': 
					str = va_arg( args, char * );
					/* If NULL string point to our null output string */
					if( str == NULL ) {
						str = nullstring;
					}
					/* copy string to output observing limit */
					while( *str != '\0' && len < size ) {
						buf[len++] = *str++;
					}
					/* next char... */
					format++;
					break;
				/* handle %c( char ) */
				case 'c':
					/* copy character to output */
					buf[len++] = ( char ) va_arg( args, int );
					/* next char... */
					format++;
					break;
				/* handle %d( int ) */
				case 'd':
					i = ( unsigned ) va_arg( args, int );
					/* treat as unsigned int then convert to +ve after we output the - */
					if( i & 0x80000000 ) {
						buf[len++] = '-'; 
						i = 0x80000000 - ( i & ~0x80000000 );
					}
					/* generate the number string in temp buffer */
					str = &scratchpad[SCRATCHPAD_SIZE-1];
					do {
						*--str = ( '0' + ( i % 10 ) );
						i /= 10;
					} while( i != 0 );
					/* copy number string from temp buffer to output observing limit */
					while( *str != '\0' && len < size )
						buf[len++] = *str++;
					/* next char... */
					format++;
					break;
				default:
					/* in the event of an unknown type call lib version of vs[n]printf */
#ifdef WIN32
					return _vsnprintf( buf, size, fmt, saveargs );
#else /* WIN32 */
					return vsnprintf( buf, size, fmt, saveargs );
#endif /* WIN32 */
			}
		}
		/* just copy char from src */
		else 
		{
			buf[len++] = c;
	    }
	}
	/* NULL terminate */
	if( len < size ) 
		buf[len] = 0;
	else
		buf[size -1] = 0;
	/* return count chars written */
	return ( int )len;
}

/** @brief ircvsprintf
 *
 *  vsprintf replacement optimised for IRC
 *
 *  @param buf Storage location for output. 
 *  @param fmt Format specification. 
 *  @param args Pointer to list of arguments. 
 *
 *  @return number of characters written excluding terminating null
 */

int ircvsprintf( char *buf, const char *fmt, va_list args ) 
{
    return ircvsnprintf( buf, 0xffffffffUL, fmt, args );
}

/** @brief ircsprintf
 *
 *  sprintf replacement optimised for IRC
 *
 *  @param buf Storage location for output. 
 *  @param fmt Format specification. 
 *  @param ... to list of arguments. 
 *
 *  @return number of characters written excluding terminating null
 */

int ircsprintf( char *buf, const char *fmt, ... )
{
    int ret;
    va_list args;

	va_start( args, fmt );
    ret = ircvsprintf( buf, fmt, args );
    va_end( args );
    return ret;
}

/** @brief ircsnprintf
 *
 *  snprintf replacement optimised for IRC
 *
 *  @param buf Storage location for output. 
 *  @param size Maximum number of characters to write.
 *  @param fmt Format specification. 
 *  @param ... list of arguments. 
 *
 *  @return number of characters written excluding terminating null
 */

int ircsnprintf( char *buf, size_t size, const char *fmt, ... )
{
    int ret;
    va_list args;

	va_start( args, fmt );
    ret = ircvsnprintf( buf, size, fmt, args );
    va_end( args );
    return ret;
}

/* tolower for RFC compliant nicknames and channel names
 * RFC dictates:
 * '[' == '{'  '\' == '|' ']' == '}'
 * Not all IRCds follow this so we might need to replace this table
 * on an IRCd specific basis. c* in table indicates comparison that are 
 * RFC compliant but may vary between IRCds.
 */

static const unsigned char irctolowertable[256] = 
{
    /*NUL SOH  STX  ETX  EOT  ENQ  ACK  BEL   BS   HT   LF   VT   FF   CR   SO   SI*/
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    /*DLE DC1  DC2  DC3  DC4  NAK  SYN  ETB   CAN  EM   SUB  ESC  FS   GS   RS   US*/
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
	/*SPC  !    "    #    $    %    &    '   (      ) *    +    ,    -    .    / */
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
	/*0    1    2    3    4    5    6    7    8    9    :    ;    <    =    >    ? */
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
	/*@    A    B    C    D    E    F    G    H    I    J    K    L    M    N    O */
    0x40,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
	/*P    Q    R    S    T    U    V    W    X    Y    Z    [*   \*   ]*   ^    _ */
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x5e,0x5f,
	/*`    a    b    c    d    e    f    g    h    i    j    k    l    m    n    o */
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
	/*p    q    r    s    t    u    v    w    x    y    z    {*   |*   }*   ~   DEL*/
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
    /*                                                                             */
    0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
    /*                                                                             */
    0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
    /*                                                                             */
    0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
    /*                                                                             */
    0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
    /*                                                                             */
    0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
    /*                                                                             */
    0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
    /*                                                                             */
    0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
    /*                                                                             */
    0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff,
};


/* IRCTOLOWER macro to perform unsigned char lookup in above table  */

#define IRCTOLOWER( c ) irctolowertable[( unsigned char )( c )]

/** @brief ircstrcasecmp
 *
 *  strcasecmp replacement optimised for IRC
 *
 *  @param s1 Null-terminated string to compare
 *  @param s2 Null-terminated string to compare
 *
 *  @return lexicographic relation of s1 to s2.
 *    < 0 s1 less than s2 
 *    0   s1 identical to s2 
 *    > 0 s1 greater than s2 
 */

int ircstrcasecmp( const char *s1, const char *s2 )
{
	/* How should we handle nulls... hmmm */
	if( s1 == 0 || s2 == 0 )
		return 0;
	while( IRCTOLOWER( *s1 ) == IRCTOLOWER( *s2 ) ) {
		if( *s1 == 0 )
			return 0;
		s1++;
		s2++;
	}
	return IRCTOLOWER( *s1 ) - IRCTOLOWER( *s2 );
}

/** @brief ircstrncasecmp
 *
 *  strncasecmp replacement optimised for IRC
 *
 *  @param s1 Null-terminated string to compare
 *  @param s2 Null-terminated string to compare
 *  @param size Maximum number of characters to compare.
 *
 *  @return lexicographic relation of s1 to s2.
 *    < 0 s1 less than s2 
 *    0   s1 identical to s2 
 *    > 0 s1 greater than s2 
 */

int ircstrncasecmp( const char *s1, const char *s2, size_t size )
{
	/* How should we handle nulls... hmmm */
	if( s1 == 0 || s2 == 0 )
		return 0;
	if( size == 0 )
		return 0;
	while( size && ( IRCTOLOWER( *s1 ) == IRCTOLOWER( *s2 ) ) )
	{
		if( *s1 == 0 )
			return 0;
		s1++;
		s2++;
		size--;
	}
	if( size )
		return IRCTOLOWER( *s1 ) - IRCTOLOWER( *s2 );
	return 0;
}
