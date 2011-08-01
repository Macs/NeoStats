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
** $Id: base64.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "servers.h"
#include "users.h"
#include "base64.h"

/** Base64 subsystem
 *
 *  base64 nick/server etc support functions 
 */

/*  TODO:
 *  - Create secondary user/server hash tables to speed up base64 lookups.
 *  - Extend base64 support to IRCds other than IRCu, e.g. Unreal.
 */


/** @brief set_server_base64
 *
 *  Set server base64 string
 *  NeoStats core use and protocol use only.
 *
 *  @param name of server to find
 *  @param base64name to set
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int set_server_base64( const char *name, const char *base64name )
{
	Client *s;

	s = FindServer( name );
	if( s == NULL )
	{
		dlog( DEBUG1, "set_server_base64: cannot find %s for %s", name, base64name );
		return NS_FAILURE;
	}
	dlog( DEBUG1, "set_server_base64: setting %s to %s", name, base64name );
	strlcpy( s->name64, base64name, 6 );
	return NS_SUCCESS;
}

/** @brief server_to_base64
 *
 *  Get server base64name from name
 *  NeoStats core use and protocol use only.
 *
 *  @param name of server to get base64 string for
 *
 *  @return base64 name or NULL if not found
 */

char *server_to_base64( const char *name )
{
	Client *s;

	dlog( DEBUG7, "server_to_base64: scanning for %s", name );
	s = FindServer( name );
	if( s != NULL )
	{
		return s->name64;
	}
	dlog( DEBUG1, "server_to_base64: cannot find %s", name );
	return NULL;
}

/** @brief base64_to_server
 *
 *  Get server name from base64 name
 *  NeoStats core use and protocol use only.
 *
 *  @param base64name to find
 *
 *  @return name or NULL if not found
 */

char *base64_to_server( const char *base64name )
{
	Client *s;

	dlog( DEBUG7, "base64_to_server: scanning for %s", base64name );
	s = find_server_base64( base64name );
	if( s != NULL )
	{
		return s->name;
	}
	dlog( DEBUG1, "base64_to_server: cannot find %s", base64name );
	return NULL;
}

/** @brief set_nick_base64
 *
 *  Set user base64 string
 *  NeoStats core use and protocol use only.
 *
 *  @param name of server to find
 *  @param base64name to set
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int set_nick_base64( const char *nick, const char *base64name )
{
	Client *u;

	u = FindUser( nick );
	if( u == NULL )
	{
		dlog( DEBUG1, "set_nick_base64: cannot find %s for %s", nick, base64name );
		return NS_FAILURE;
	}
	dlog( DEBUG1, "set_nick_base64: setting %s to %s", nick, base64name );
	strlcpy( u->name64, base64name, B64SIZE );
	return NS_SUCCESS;
}

/** @brief nick_to_base64
 *
 *  Get user base64name from name
 *  NeoStats core use and protocol use only.
 *
 *  @param name of user to get base64 string for
 *
 *  @return base64 name or NULL if not found
 */

char *nick_to_base64( const char *nick )
{
	Client *u;

	dlog( DEBUG1, "nick_to_base64: scanning for %s", nick );
	u = FindUser( nick );
	if( u != NULL )
	{
		return u->name64;
	}
	dlog( DEBUG1, "nick_to_base64: cannot find %s", nick );
	return NULL;
}

/** @brief base64_to_nick
 *
 *  Get user name from base64 name
 *  NeoStats core use and protocol use only.
 *
 *  @param base64name to find
 *
 *  @return name or NULL if not found
 */

char *base64_to_nick( const char *base64name )
{
	Client *u;

	dlog( DEBUG1, "base64_to_nick: scanning for %s", base64name );
	u = find_user_base64( base64name );
	if( u != NULL )
	{
		return u->name;
	}
	dlog( DEBUG1, "base64_to_nick: cannot find %s", base64name );
	return NULL;
}

/** @brief base64_to_name
 *
 *  Get client name (server or user) from base64 name
 *  NeoStats core use and protocol use only.
 *
 *  @param base64name to find
 *
 *  @return name or NULL if not found
 */

char *base64_to_name( const char *base64name )
{
	Client *c;

	dlog( DEBUG1, "base64_to_name: scanning for %s", base64name );
	c = find_user_base64( base64name );
	if( c != NULL )
	{
		return c->name;
	}
	c = find_server_base64( base64name );
	if( c != NULL )
	{
		return c->name;
	}
	dlog( DEBUG1, "base64_to_name: cannot find %s", base64name );
	return NULL;
}
