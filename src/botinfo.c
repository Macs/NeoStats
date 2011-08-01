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
** $Id: botinfo.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "helpstrings.h"

static int bot_set_nick_cb( const CmdParams *cmdparams, SET_REASON reason );
static int bot_set_user_cb( const CmdParams *cmdparams, SET_REASON reason );
static int bot_set_host_cb( const CmdParams *cmdparams, SET_REASON reason );
static int bot_set_realname_cb( const CmdParams *cmdparams, SET_REASON reason );

static bot_setting bot_info_settings[] =
{
	{"NICK",	NULL,	SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, NULL,	ns_help_set_nick, bot_set_nick_cb, NULL },
	{"ALTNICK",	NULL,	SET_TYPE_NICK,		0, MAXNICK, 	NS_ULEVEL_ADMIN, NULL,	ns_help_set_altnick, NULL, NULL },
	{"USER",	NULL,	SET_TYPE_USER,		0, MAXUSER, 	NS_ULEVEL_ADMIN, NULL,	ns_help_set_user, bot_set_user_cb, NULL },
	{"HOST",	NULL,	SET_TYPE_HOST,		0, MAXHOST, 	NS_ULEVEL_ADMIN, NULL,	ns_help_set_host, bot_set_host_cb, NULL },
	{"REALNAME",NULL,	SET_TYPE_REALNAME,	0, MAXREALNAME, NS_ULEVEL_ADMIN, NULL,	ns_help_set_realname, bot_set_realname_cb, NULL },
	NS_SETTING_END()
};

/** @brief bot_set_nick_cb
 *
 *  SET nick callback
 *  Command subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_set_nick_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	/* Ignore bootup and list callback */
	if( reason == SET_CHANGE )
	{
		irc_nickchange( cmdparams->bot, cmdparams->av[1] );
	}
	return NS_SUCCESS;
}

/** @brief bot_set_user_cb
 *
 *  SET user callback
 *  Command subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_set_user_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	/* Ignore bootup and list callback */
	if( reason == SET_CHANGE )
	{
		irc_setident( cmdparams->bot, cmdparams->av[1] );
	}
	return NS_SUCCESS;
}

/** @brief bot_set_host_cb
 *
 *  SET host callback
 *  Command subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_set_host_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	/* Ignore bootup and list callback */
	if( reason == SET_CHANGE )
	{
		irc_sethost( cmdparams->bot, cmdparams->av[1] );
	}
	return NS_SUCCESS;
}

/** @brief bot_set_realname_cb
 *
 *  SET realname callback
 *  Command subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_set_realname_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	char *buf;

	/* Ignore bootup and list callback */
	if( reason == SET_CHANGE )
	{
		buf = joinbuf( cmdparams->av, cmdparams->ac, 1 );
		irc_setname( cmdparams->bot, buf );
		ns_free( buf );
	}
	return NS_SUCCESS;
}

/** @brief add_bot_info_settings
 *
 *  Add bot info settings to SET command
 *  Command subsystem use only.
 *
 *  @params cmdparams pointer to bot
 *  @params cmdparams pointer to bot info structure
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int add_bot_info_settings( Bot *bot_ptr, BotInfo* botinfo )
{
	bot_ptr->bot_info_settings = ns_calloc( sizeof( bot_info_settings ) );
	if( bot_ptr->bot_info_settings != NULL )
	{
		os_memcpy( bot_ptr->bot_info_settings, bot_info_settings, sizeof( bot_info_settings ) );
		bot_ptr->bot_info_settings[0].varptr = botinfo->nick;
		bot_ptr->bot_info_settings[1].varptr = botinfo->altnick;
		bot_ptr->bot_info_settings[2].varptr = botinfo->user;
		bot_ptr->bot_info_settings[3].varptr = botinfo->host;
		bot_ptr->bot_info_settings[4].varptr = botinfo->realname;
		ModuleConfig( bot_ptr->bot_info_settings );
		if( (*botinfo->host ) == 0 )
			strlcpy( botinfo->host, me.servicehost, MAXHOST );
		add_bot_setting_list( bot_ptr, bot_ptr->bot_info_settings );
	}
	return NS_SUCCESS;
}

/** @brief del_bot_info_settings
 *
 *  Delete bot info settings from SET command
 *  Command subsystem use only.
 *
 *  @params cmdparams pointer to bot
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int del_bot_info_settings( Bot *bot_ptr )
{
	if( bot_ptr->bot_info_settings != NULL )
	{
		del_bot_setting_list( bot_ptr, bot_ptr->bot_info_settings );
		ns_free( bot_ptr->bot_info_settings );
	}
	return NS_SUCCESS;
}
