/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2008 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2008 ^Enigma^
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
** $Id: base64.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _BASE64_H_
#define _BASE64_H_

EXPORTFUNC int set_server_base64( const char *name, const char *base64name );
EXPORTFUNC char *server_to_base64( const char *name );
EXPORTFUNC char *base64_to_server( const char *base64name );

EXPORTFUNC int set_nick_base64( const char *nick, const char *base64name );
EXPORTFUNC char *nick_to_base64( const char *nick );
EXPORTFUNC char *base64_to_nick( const char *base64name );

EXPORTFUNC char *base64_to_name( const char *base64name );

#endif /* _BASE64_H_ */
