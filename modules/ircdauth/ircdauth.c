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
** $Id: serviceroots.c 1721 2004-04-09 22:17:19Z Mark $
*/

#include "neostats.h"

/** IRCDAuth Module
 *
 *  User authentication based on ircd user modes
 */


/** User mode lookup struct */
typedef struct UserAuthModes {
	unsigned long umode;
	int level;
} UserAuthModes;

/** Copyright info */
static const char *ircdauth_copyright[] =
{
	"Copyright (c) 1999-2008, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** About info */
static const char *ircdauth_about[] = 
{
	"\2IRCDAuth\2 authorises users based on their ircd mode flags.",
	NULL
};

/** Help text */
const char *auth_help_authmodelist[] = 
{
	"User mode auth list",
	"Syntax: \2AUTHMODELIST\2",
	"",
	"Lists the user modes and their level",
	NULL
};

/** Module info */
ModuleInfo module_info = 
{
	"IRCDAuth",
	"IRCD User Mode Authentication Module",
	ircdauth_copyright,
	ircdauth_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	MODULE_FLAG_AUTH,
	0,
	0,
};

/** User mode lookup tables */
static UserAuthModes user_auth_modes[] = 
{
	{UMODE_TECHADMIN,	NS_ULEVEL_ADMIN},
	{UMODE_SERVICES,	NS_ULEVEL_ROOT},
	{UMODE_NETADMIN,	NS_ULEVEL_ADMIN},
	{UMODE_SADMIN,		NS_ULEVEL_ADMIN},
	{UMODE_ADMIN,		NS_ULEVEL_OPER},
	{UMODE_COADMIN,		NS_ULEVEL_OPER},
	{UMODE_OPER,		NS_ULEVEL_OPER},
	{UMODE_LOCOP,		NS_ULEVEL_LOCOPER},
	{UMODE_REGNICK,		NS_ULEVEL_REG},
};

const int user_auth_mode_count = ( ( sizeof( user_auth_modes ) / sizeof( user_auth_modes[0] ) ) );

static UserAuthModes user_auth_smodes[] = 
{
	{SMODE_NETADMIN,	NS_ULEVEL_ADMIN},
	{SMODE_CONETADMIN,	175},
	{SMODE_TECHADMIN,	150},
	{SMODE_COTECHADMIN,	125},
	{SMODE_ADMIN,		100},
	{SMODE_GUESTADMIN,	100},
	{SMODE_COADMIN,		NS_ULEVEL_OPER},
};

const int user_auth_smode_count = ( ( sizeof( user_auth_smodes ) / sizeof( user_auth_smodes[0] ) ) );

/** @brief auth_cmd_authmodelist
 *
 *  Auth mode level list command handler
 *
 *  @param cmdparam struct
 *
 *  @return NS_SUCCESS if suceeds else result of command
 */

static int auth_cmd_authmodelist( const CmdParams *cmdparams )
{
	int i;

	irc_prefmsg( NULL, cmdparams->source, "User mode auth levels:" );
	for( i = 0; i < user_auth_mode_count; i++ ) 
	{
		irc_prefmsg( NULL, cmdparams->source, "%s: %d", 
			GetUmodeDesc( user_auth_modes[i].umode ), user_auth_modes[i].level );
	}
	if( HaveFeature( FEATURE_USERSMODES ) ) 
	{
		for( i = 0; i < user_auth_smode_count; i++ ) 
		{
			irc_prefmsg( NULL, cmdparams->source, "%s: %d", 
				GetSmodeDesc( user_auth_smodes[i].umode ), user_auth_smodes[i].level );
		}
	}
	return NS_SUCCESS;
}

/** Bot command table */
static bot_cmd ircdauth_commands[] =
{
	{"AUTHMODELIST",	auth_cmd_authmodelist,	0,	NS_ULEVEL_OPER, auth_help_authmodelist, 0, NULL, NULL},
	NS_CMD_END()
};

/** @brief ModInit
 *
 *  Init handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit( void )
{
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch( void )
{
	if( add_services_cmd_list( ircdauth_commands ) != NS_SUCCESS ) 
	{
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief ModFini
 *
 *  Fini handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModFini( void )
{
	del_services_cmd_list( ircdauth_commands );
	return NS_SUCCESS;
}

/** @brief ModAuthUser
 *
 *  Lookup authentication level for user
 *
 *  @param pointer to user
 *
 *  @return authentication level for user
 */

int ModAuthUser( const Client *u )
{
	int i, authlevel;

	/* Check umodes */
	authlevel = 0;
	for( i = 0; i < user_auth_mode_count; i++ ) 
	{
		if( u->user->Umode & user_auth_modes[i].umode ) 
		{
			if( user_auth_modes[i].level > authlevel ) 
			{
				authlevel = user_auth_modes[i].level;
			}
		}
	}
	dlog( DEBUG1, "UmodeAuth: level after umode for %s is %d", u->name, authlevel );
	/* Check smodes if we have them */
	if( HaveFeature( FEATURE_USERSMODES ) ) 
	{
		for( i = 0; i < user_auth_smode_count; i++ ) 
		{
			if( u->user->Smode & user_auth_smodes[i].umode ) 
			{
				if( user_auth_smodes[i].level > authlevel ) 
				{
					authlevel = user_auth_smodes[i].level;
				}
			}
		}
		dlog( DEBUG1, "UmodeAuth: level after smode for %s is %d", u->name, authlevel );
	}
	/* Return new level */
	return authlevel;
}
