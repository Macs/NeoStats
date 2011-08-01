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
** $Id: bots.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "nsevents.h"
#include "ircprotocol.h"
#include "services.h"
#include "commands.h"
#include "botinfo.h"
#include "settings.h"
#include "bots.h"
#include "users.h"
#include "ctcp.h"
#include "exclude.h"
#include "namedvars.h"
#include "namedvars-core.h"

#define IS_CTCP_MSG( msg ) ( msg[0] == '\1' )

#define BOT_TABLE_SIZE		100		/* Max number of bots */
#define NICK_TRIES			5		/* Number of attempts for nick generation */

/* @brief Module Bot hash list */
static hash_t *bothash;

nv_struct nv_bots[] = {
	{ "name", NV_STR, offsetof(Bot, name), NV_FLG_RO, -1, MAXNICK},
	{ "flags", NV_INT, offsetof(Bot, flags), NV_FLG_RO, -1, -1},
	{ "set_ulevel", NV_INT, offsetof(Bot, set_ulevel), NV_FLG_RO, -1, -1},
/*	{ "nick", NV_STR, offsetof(BotInfo, nick), NV_FLG_RO, offsetof(Bot, botinfo), MAXNICK},
	{ "altnick", NV_STR, offsetof(BotInfo, altnick), NV_FLG_RO, offsetof(Bot, botinfo), MAXNICK}, 
	{ "user", NV_STR, offsetof(BotInfo, user), NV_FLG_RO, offsetof(Bot, botinfo), MAXUSER},
	{ "host", NV_STR, offsetof(BotInfo, host), NV_FLG_RO, offsetof(Bot, botinfo), MAXHOST},
	{ "realname", NV_STR, offsetof(BotInfo, realname), NV_FLG_RO, offsetof(Bot, botinfo), MAXREALNAME}, */
	NV_STRUCT_END()
};


/** @brief InitBots 
 *
 *  initialise bot subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitBots( void )
{
	bothash = nv_hash_create( BOT_TABLE_SIZE, 0, 0, "Bots", nv_bots, NV_FLAGS_RO, NULL);
	if( bothash == NULL )
	{
		nlog( LOG_CRITICAL, "Failed to create bot hash" );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief FiniBots
 *
 *  cleanup bot subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */

void FiniBots( void )
{
	hash_destroy( bothash );
}

/** @brief is_flood
 *
 *  calculate and test flood values to determine if a client is flooding
 *  Bot subsystem use only.
 *
 *  @param pointer to client to test
 *
 *  @return NS_TRUE if client flooded, NS_FALSE if not 
 */

static int is_flood( Client *u )
{
	/* locop or higher are exempt from flood checks */
	if( UserLevel( u ) >= NS_ULEVEL_OPER )	
		return NS_FALSE;
	if( ( me.now - u->user->tslastmsg ) > nsconfig.msgsampletime )
	{
		u->user->tslastmsg = me.now;
		u->user->flood = 0;
		return NS_FALSE;
	}
	if( u->user->flood >= nsconfig.msgthreshold )
	{
		nlog( LOG_NORMAL, "FLOODING: %s!%s@%s", u->name, u->user->username, u->user->hostname );
		irc_svskill( ns_botptr, u, _( "%s!%s (Flooding Services)" ), me.name, ns_botptr->name );
		return NS_TRUE;
	}
	u->user->flood++;
	return NS_FALSE;
}

/** @brief is_valid_origin
 *
 *  validate message origin and populate cmdparams structure
 *  Bot subsystem use only.
 *
 *  @param cmdparam structure to populate 
 *  @param origin nick or server name 
 *
 *  @return NS_TRUE if valid, NS_FALSE if not 
 */

static int is_valid_origin( CmdParams *cmdparams, const char *origin )
{
	cmdparams->source = FindUser( origin );
	if( cmdparams->source != NULL )
	{
		/* Since is_flood is a truth value, return its inverse */
		return !is_flood( cmdparams->source );
	}
	cmdparams->source = FindServer( origin );
	if( cmdparams->source != NULL )
		return NS_TRUE;
	return NS_FALSE;
}

/** @brief is_valid_target
 *
 *  validate message target and populate cmdparams structure
 *  Bot subsystem use only.
 *
 *  @param cmdparam structure to populate 
 *  @param target nick
 *
 *  @return NS_TRUE if valid, NS_FALSE if not 
 */

static int is_valid_target( CmdParams *cmdparams, const char *target )
{
	cmdparams->target = FindUser( target );
	if( cmdparams->target != NULL )
	{
		cmdparams->bot = cmdparams->target->user->bot;
		if( cmdparams->bot != NULL )
			return NS_TRUE;
	}
	dlog( DEBUG1, "is_valid_target: user %s not found", target );
	return NS_FALSE;
}

/** @brief is_valid_target_chan
 *
 *  validate message target and populate cmdparams structure
 *  Bot subsystem use only.
 *
 *  @param cmdparam structure to populate 
 *  @param target channel
 *
 *  @return NS_TRUE if valid, NS_FALSE if not 
 */

static int is_valid_target_chan( CmdParams *cmdparams, const char *target )
{
	cmdparams->channel = FindChannel( target );
	if( cmdparams->channel != NULL )
		return NS_TRUE;
	dlog( DEBUG1, "cmdparams->channel: chan %s not found", target );
	return NS_FALSE;
}

/** @brief bot_chan_event
 *
 *  Process event regarding to bot channel
 *  Bot subsystem use only.
 *
 *  @param cmdparams pointer to command parameters
 *
 *  @return none
 */

static void bot_chan_event( Event event, CmdParams *cmdparams )
{
	lnode_t *cm;
	Bot *botptr;
	hnode_t *bn;
	hscan_t bs;
	int cmdflag = 0;
	char *chan;

	SET_SEGV_LOCATION();
	if( cmdparams->param[0] == nsconfig.cmdchar[0] )
	{
		/* skip over command char */
		cmdparams->param ++;
		cmdflag = 1;
	}
	hash_scan_begin( &bs, bothash );
	while( ( bn = hash_scan_next( &bs ) ) != NULL )
	{
		botptr = hnode_get( bn );
		/* Use an internal flag for handling DEAF so we can fake support
		 * on IRCd's which do not have the mode natively and prohibit
		 * channel commands in services channel due to potentially
		 * many bots trying to handle the command.
		 */
		if( !( botptr->flags & BOT_FLAG_DEAF ) && !IsServicesChannel( cmdparams->channel ) )
		{
			cm = list_first( botptr->u->user->chans );
			while( cm != NULL )
			{	
				chan = ( char * ) lnode_get( cm );
				cmdparams->bot = botptr;
				if( ircstrcasecmp( cmdparams->channel->name, chan ) == 0 )
				{
					if( cmdflag == 0 || run_bot_cmd( cmdparams, cmdflag ) != NS_SUCCESS || (botptr->botcmds == NULL) )
					{
						/* Reset message if we have stripped cmdchar */
						if( cmdflag != 0 )
							cmdparams->param --;
						SendModuleEvent( event, cmdparams, botptr->moduleptr );
						/* Reset message if we have unstripped cmdchar */
						if( cmdflag != 0 )
							cmdparams->param ++;
					}
				}
				cm = list_next( botptr->u->user->chans, cm );
			}
		}
	}
}

/** @brief bot_notice
 *
 *  send a notice to a bot
 *  NeoStats core use only.
 *
 *  @param origin 
 *  @param av 
 *  @param ac
 * 
 *  @return none
 */

void bot_notice( const char *origin, char *const *av, int ac )
{
	CmdParams *cmdparams;

	SET_SEGV_LOCATION();
	cmdparams = ( CmdParams* ) ns_calloc( sizeof( CmdParams ) );
	if( is_valid_origin( cmdparams, origin ) )
	{
		if( is_valid_target( cmdparams, av[0] ) )
		{
			cmdparams->param = av[ac - 1];
			if( IS_CTCP_MSG( cmdparams->param ) )
			{
				ctcp_notice( cmdparams );
			}
			else
			{
				SendModuleEvent( EVENT_NOTICE, cmdparams, cmdparams->bot->moduleptr );
			}
		}		
	}
	if (cmdparams->ac > 0)
		ns_free(cmdparams->av);
	ns_free( cmdparams );
}

/** @brief bot_chan_notice
 *
 *  send a channel notice to a bot
 *  NeoStats core use only.
 *
 *  @param origin 
 *  @param av 
 *  @param ac
 * 
 *  @return none
 */

void bot_chan_notice( const char *origin, char *const *av, int ac )
{
	CmdParams *cmdparams;

	SET_SEGV_LOCATION();
	cmdparams = ( CmdParams* ) ns_calloc( sizeof(CmdParams ) );
	if( is_valid_origin( cmdparams, origin ) )
	{
		if( is_valid_target_chan( cmdparams, av[0] ) )
		{
			cmdparams->param = av[ac - 1];
			if( IS_CTCP_MSG( cmdparams->param ) )
			{
				ctcp_cnotice( cmdparams );
			}
			else
			{
				bot_chan_event( EVENT_CNOTICE, cmdparams );
			}
		}
	}
	if (cmdparams->ac > 0)
		ns_free(cmdparams->av);
	ns_free( cmdparams );
}

/** @brief bot_private
 *
 *  send a private message to a bot
 *  NeoStats core use only.
 *
 *  @param origin 
 *  @param av 
 *  @param ac
 * 
 *  @return none
 */

void bot_private( const char *origin, char *const *av, int ac )
{
	CmdParams *cmdparams;

	SET_SEGV_LOCATION();
	cmdparams = ( CmdParams* ) ns_calloc( sizeof(CmdParams ) );
	if( is_valid_origin( cmdparams, origin ) )
	{
		if( is_valid_target( cmdparams, av[0] ) )
		{
			cmdparams->param = av[ac - 1];
			if( IS_CTCP_MSG( cmdparams->param ) )
			{
				ctcp_private( cmdparams );
			}
			else
			{
				if( run_bot_cmd( cmdparams, 0 ) == NS_FAILURE )
				{
					SendModuleEvent( EVENT_PRIVATE, cmdparams, cmdparams->bot->moduleptr );
				}
			}
		}
	}
	if (cmdparams->ac > 0)
		ns_free(cmdparams->av);
	ns_free( cmdparams );
}

/** @brief bot_chan_private
 *
 *  send a channel private message to a bot
 *  NeoStats core use only.
 *
 *  @param origin 
 *  @param av 
 *  @param ac
 * 
 *  @return none
 */

void bot_chan_private( const char *origin, char *const *av, int ac )
{
	CmdParams *cmdparams;

	SET_SEGV_LOCATION();
	cmdparams = ( CmdParams* ) ns_calloc( sizeof(CmdParams ) );
	if( is_valid_origin( cmdparams, origin ) )
	{
		if( is_valid_target_chan( cmdparams, av[0] ) )
		{
			cmdparams->param = av[ac - 1];
			if( IS_CTCP_MSG( cmdparams->param ) )
			{
				ctcp_cprivate( cmdparams );
			}
			else
			{
				bot_chan_event( EVENT_CPRIVATE, cmdparams );
			}
		}
	}
	if (cmdparams->ac > 0)
		ns_free(cmdparams->av);
	ns_free( cmdparams );
}

/** @brief FindBot
 *
 *  find bot
 *
 *  @param bot_name name of bot to find
 *
 *  @return pointer to bot or NULL if not found
 */

Bot *FindBot( const char *bot_name )
{
	Bot *bot;

	SET_SEGV_LOCATION(); 
	bot = ( Bot * ) hnode_find( bothash, bot_name );
	if( bot == NULL )
		dlog( DEBUG3, "FindBot: %s not found", bot_name );
	return bot;
}

/** @brief DelBot
 *
 *  delete bot
 *  NeoStats core use only.
 *
 *  @param bot_name name of bot to delete
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DelBot( const char *bot_name )
{
	Bot *botptr;
	hnode_t *bn;

	SET_SEGV_LOCATION();
	bn = hash_lookup( bothash, bot_name );
	if( bn == NULL )
	{
		nlog( LOG_WARNING, "DelBot: %s not found", bot_name );
		return NS_FAILURE;
	}
	botptr = hnode_get( bn );
	if( botptr->flags & BOT_FLAG_CTCPVERSIONMASTER )
		SetCTCPVersionMaster( NULL );
	del_all_bot_cmds( botptr );
	del_bot_info_settings( botptr );
	del_all_bot_settings( botptr );
	hash_delete_destroy_node( bothash, bn );
	ns_free( botptr );
	return NS_SUCCESS;
}

/** @brief BotNickChange
 *
 *  change bot nick
 *  NeoStats core use only.
 *
 *  @param botptr pointer to bot
 *  @param newnick new nick
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int BotNickChange( const Bot *botptr, const char *newnick )
{
	hnode_t *bn;

	SET_SEGV_LOCATION();
	bn = hash_lookup( bothash, botptr->name );
	if( bn == NULL )
	{
		nlog( LOG_WARNING, "BotNickChange: %s not found", botptr->name );
		return NS_FAILURE;
	}
	/* remove old hash entry */
	hash_delete( bothash, bn );
	dlog( DEBUG3, "Bot %s changed nick to %s", botptr->name, newnick );
	strlcpy( (char *)botptr->name, newnick, MAXNICK );
	/* insert new hash entry */
	hash_insert( bothash, bn, botptr->name );
	return NS_SUCCESS;
}

/** @brief ns_cmd_botlist
 *
 *  list all neostats bots
 *  NeoStats core use only.
 *
 *  @param cmdparams pointer to command parameters
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ns_cmd_botlist( const CmdParams *cmdparams )
{
	lnode_t *cm;
	Bot *botptr;
	hnode_t *bn;
	hscan_t bs;

	SET_SEGV_LOCATION();
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Module Bot List:", cmdparams->source ) );
	hash_scan_begin( &bs, bothash );
	while( ( bn = hash_scan_next( &bs ) ) != NULL )
	{
		botptr = hnode_get( bn );
		if( ( botptr->flags & 0x80000000 ) )
			irc_prefmsg( ns_botptr, cmdparams->source, __( "NeoStats", cmdparams->source ) );
		else
			irc_prefmsg( ns_botptr, cmdparams->source, __( "Module: %s", cmdparams->source ), botptr->moduleptr->info->name );
		irc_prefmsg( ns_botptr, cmdparams->source, __( "Bot: %s", cmdparams->source ), botptr->name );
		cm = list_first( botptr->u->user->chans );
		irc_prefmsg( ns_botptr, cmdparams->source, __( "Channels:", cmdparams->source ) );
		while( cm != NULL )
		{
			irc_prefmsg( ns_botptr, cmdparams->source, "    %s", ( char * ) lnode_get( cm ) );
			cm = list_next( botptr->u->user->chans, cm );
		}
	}
	irc_prefmsg( ns_botptr, cmdparams->source, __( "End of Module Bot List", cmdparams->source ) );
	return NS_SUCCESS;
}

/** @brief DelModuleBots
 *
 *  delete all bots associated with a given module
 *  NeoStats core use only.
 *
 *  @param mod_ptr pointer to module
 *
 *  @return none
 */

void DelModuleBots( const Module *mod_ptr )
{
	Bot *botptr;
	hnode_t *modnode;
	hscan_t hscan;

	hash_scan_begin( &hscan, bothash );
	while( ( modnode = hash_scan_next( &hscan ) ) != NULL )
	{
		botptr = hnode_get( modnode );
		if( botptr->moduleptr == mod_ptr )
		{
			dlog( DEBUG1, "Deleting module %s bot %s", mod_ptr->info->name, botptr->name );
			irc_quit( botptr, _( "Module Unloaded" ) );
		}
	}
	return;
}

/** @brief new_bot
 *
 *  allocate a new bot
 *  Bot subsystem use only.
 *
 *  @param bot_name string containing bot name
 * 
 *  @return none
 */

static Bot *new_bot( const char *bot_name )
{
	Bot *botptr;

	SET_SEGV_LOCATION();
	if( hash_isfull( bothash ) )
	{
		nlog( LOG_CRITICAL, "new_bot: Failed to create bot %s, bot list is full", bot_name );
		return NULL;
	}
	dlog( DEBUG2, "new_bot: %s", bot_name );
	botptr = ns_calloc( sizeof( Bot ) );
	strlcpy( botptr->name, bot_name, MAXNICK );
	botptr->moduleptr = GET_CUR_MODULE();
	botptr->set_ulevel = NS_ULEVEL_ROOT;
	hnode_create_insert( bothash, botptr, botptr->name );
	return botptr;
}

/** @brief GenerateBotNick
 *
 *  find a new nick based on the passed nick
 *
 *  @param pointer to nick buffer
 *  @param length of passed nick stub
 *  @param number of characters to add to nick
 *  @param number of digits to add to nick
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int GenerateBotNick( char *nickbuf, size_t stublen, int alphacount, int numcount)
{
	int i;
	
	for ( i = 0 ; i < alphacount ; i++ )
	{
		/* if room, add random letter */
		if( ( stublen + 1 ) < MAXNICK )
		{
			nickbuf[stublen++] = ( ( rand() % 26 ) + 97 );
			nickbuf[stublen] = '\0';
		}
	}
	for ( i = 0 ; i < numcount ; i++ )
	{
		/* if room, add random number between 0 and 9 */
		if( ( stublen + 1 ) < MAXNICK )
		{
			nickbuf[stublen++] = ( ( rand() % 10 ) + 48 );
			nickbuf[stublen] = '\0';
		}
	}
	if( FindUser( nickbuf ) != NULL )
	{
		nlog( LOG_WARNING, "GenerateBotNick, %s already in use", nickbuf );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief GetBotNick
 *
 *  check the requested nick
 *  Bot subsystem use only.
 *
 *  @param botinfo pointer to bot description
 *  @param pointer to nick buffer
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int GetBotNick( const BotInfo *botinfo, char *nickbuf )
{
	size_t stublen;
	int i;

	/* Check primary nick */
	strlcpy( nickbuf, botinfo->nick, MAXNICK );
	if( FindUser( nickbuf ) == NULL )
		return NS_SUCCESS;
	nlog( LOG_WARNING, "Bot nick %s already in use", nickbuf );
	/* Check alternate nick */
	if( botinfo->altnick[0] )
	{
		strlcpy( nickbuf, botinfo->altnick, MAXNICK );
		if( FindUser( nickbuf ) == NULL )
			return NS_SUCCESS;
	}
	nlog( LOG_WARNING, "Bot alt nick %s already in use", nickbuf );
	/* Try to auto generate a nick from bot nick */
	strlcpy(nickbuf, botinfo->nick, MAXNICK);
	stublen = strlen( nickbuf );
	for( i = 0 ; i < NICK_TRIES ; i++ )
	{
		if( GenerateBotNick( nickbuf, stublen , (i + 1) , (i + 1)) == NS_SUCCESS )
			return NS_SUCCESS;
	}
	/* Try to auto generate a nick from bot alt nick */
	if( botinfo->altnick[0] )
	{
		strlcpy(nickbuf, botinfo->altnick, MAXNICK);
		stublen = strlen( nickbuf );
		for( i = 0 ; i < NICK_TRIES ; i++ )
		{
			if( GenerateBotNick( nickbuf, stublen , (i + 1) , (i + 1)) == NS_SUCCESS )
				return NS_SUCCESS;
		}
	}
	/* Give up */
	return NS_FAILURE;
}

/** @brief ConnectBot
 *
 *  Connect bot to IRC
 *  Bot subsystem use only.
 *
 *  @param botptr pointer to bot 
 *
 *  @return none
 */

static void ConnectBot( const Bot *botptr )
{
	if( botptr->flags & BOT_FLAG_ROOT )
	{
		irc_nick( botptr->name, botptr->u->user->username, botptr->u->user->hostname, botptr->u->info, me.servicesumode );
		UserMode( botptr->name, me.servicesumode );
		if( nsconfig.joinserviceschan )
			irc_join( botptr, me.serviceschan, me.servicescmode );
	} 
	else
	{
		irc_nick( botptr->name, botptr->u->user->username, botptr->u->user->hostname, botptr->u->info, "+" );
	}	
	if( botptr->flags & BOT_FLAG_DEAF )
	{
		/* Set deaf mode at IRCd level if we can */
		if( HaveUmodeDeaf() )
			irc_umode( botptr, botptr->name, UMODE_DEAF );
	}
}

/** @brief AddBot
 *
 *  Add a new bot
 *  NeoStats core use and Module API call.
 *
 *  @param botinfo pointer to bot description
 *
 *  @return pointer to bot or NULL if failed
 */

Bot *AddBot( BotInfo *botinfo )
{
	static char nick[MAXNICK];
	Bot *botptr; 
	Module *modptr;

	SET_SEGV_LOCATION();
	modptr = GET_CUR_MODULE();
	if( !IsModuleInSynch( modptr ) )
	{
		nlog( LOG_WARNING, "Module %s attempted to init a bot %s but is not yet synched", modptr->info->name, botinfo->nick );
		SetModuleError( modptr );
		return NULL;
	}
	/* Only one root bot allowed per module so prevent modules trying to add multiple roots */
	if( botinfo->flags & BOT_FLAG_ROOT && IsModuleRootBot( modptr ) )
	{
		/* Warn user of multiple root condition */
		nlog( LOG_WARNING, "Module %s attempted to init root bot %s but root bot already present", modptr->info->name, botinfo->nick );
		/* Clear flag and continue as if flag not set */
		botinfo->flags &= ~BOT_FLAG_ROOT;
	}
	/* In single bot mode, just add all commands and settings to main bot */
	if( nsconfig.singlebotmode && ns_botptr != NULL )
	{
		add_bot_cmd_list( ns_botptr, botinfo->bot_cmd_list );
		add_bot_setting_list( ns_botptr, botinfo->bot_setting_list );
		return(ns_botptr );
	}
	/* create the bot record so we can load the settings and get the bots nicks/user/realname and host from DB */
	botptr = new_bot( botinfo->nick );
	if( botptr == NULL )
		return NULL;
	/* Copy flags from bot definition */
	botptr->flags = botinfo->flags;
	/* Add commands if defined by bot */
	if ( botinfo->bot_cmd_list != NULL )
	{
		add_bot_cmd_list( botptr, botinfo->bot_cmd_list );
	}
	/* Add settings if defined by bot */
	if ( botinfo->bot_setting_list != NULL )
	{		
		add_bot_setting_list( botptr, botinfo->bot_setting_list );
	}
	if( botptr->flags & BOT_FLAG_ROOT )
	{
		SetModuleRootBot( modptr );

		/* Do not add set botinfo options for root bot */
		if( !( botptr->flags & 0x80000000 ) )
		{
			add_bot_info_settings( botptr, botinfo );
		}
		/* Create module exclusion command handlers if needed */
		if( botptr->moduleptr->info->flags & MODULE_FLAG_LOCAL_EXCLUDES )
		{
			AddBotExcludeCommands( botptr );
		}
	}
	if( botptr->flags & BOT_FLAG_CTCPVERSIONMASTER )
	{
		SetCTCPVersionMaster( botptr );
	}
	/* ok, now botinfo should be loaded from the DB, get the nickname */
	if( GetBotNick( botinfo, nick ) == NS_FAILURE )
	{
		nlog( LOG_WARNING, "Failed to find free nick for bot %s", botinfo->nick );
		return NULL;
	}
	/* if the generated nick doesn't match what we created this bot as, rename it */
	if ( ircstrcasecmp( nick, botptr->name ) )
	{
		BotNickChange(botptr, nick);
	}
	if( botptr->flags & BOT_FLAG_ROOT && !( botptr->flags & 0x80000000 ) )
	{
		botptr->u = AddUser( botptr->name, 
			botptr->bot_info_settings[2].varptr, //user
			botptr->bot_info_settings[3].varptr, //host
			botptr->bot_info_settings[4].varptr, //realname
			me.name, NULL, NULL, NULL );
	}
	else
	{
		botptr->u = AddUser( botptr->name, botinfo->user, ( (*botinfo->host ) == 0 ? me.servicehost : botinfo->host ), botinfo->realname, me.name, NULL, NULL, NULL );
	}
	/* For more efficient transversal of bot/user lists, link 
	 * associated user struct to bot and link bot into user struct */
	botptr->u->user->bot = botptr;
	ConnectBot( botptr );
	return botptr;
}

/** @brief handle_dead_channel
 *
 *  remove bots from dead channel
 *  NeoStats core use only.
 *
 *  @param channel to process
 *
 *  @return none
 */

void handle_dead_channel( Channel *c )
{
	CmdParams *cmdparams;
	hnode_t *bn;
	hscan_t bs;
	lnode_t *cm;
	char *chan;

	SET_SEGV_LOCATION();
	/* If services channel ignore it */
	if( IsServicesChannel ( c ) )
		return;
	/* If channel has persistent bot(s) ignore it */
	if( c->persistentusers > 0 )
		return;
	hash_scan_begin( &bs, bothash );
	cmdparams = ns_calloc( sizeof( CmdParams ) );
	cmdparams->channel = c;
	while( ( bn = hash_scan_next( &bs ) ) != NULL )
	{
		cmdparams->bot = hnode_get( bn );
		cm = list_first( cmdparams->bot->u->user->chans );
		while( cm != NULL )
		{
			chan = ( char * ) lnode_get( cm );
			if( ircstrcasecmp( cmdparams->channel->name, chan ) == 0 )
			{
				/* Force the bot to leave the channel */
				irc_part( cmdparams->bot, cmdparams->channel->name, NULL );
				/* Tell the module we kicked them out */
				SendModuleEvent( EVENT_EMPTYCHAN, cmdparams, cmdparams->bot->moduleptr );
				break;
			}
			cm = list_next( cmdparams->bot->u->user->chans, cm );
		}
	}
	ns_free( cmdparams );
}

/** @brief AllocBotModPtr
 *
 *  Allocate memory for a module pointer for a bot
 *  NeoStats core use only.
 *
 *  @param pBot pointer to bot to lookup pointer for
 *  @param size to allocate
 *
 *  @return pointer to allocated memory
 */

void *AllocBotModPtr( Bot *pBot, size_t size )
{
	void *ptr;

	ptr = ns_calloc( size );
	pBot->moddata = ptr;
	return ptr;
}

/** @brief FreeBotModPtr
 *
 *  Free memory for a module pointer for a bot
 *  NeoStats core use only.
 *
 *  @param pBot pointer to bot to lookup pointer for
 *
 *  @return none
 */

void FreeBotModPtr( Bot *pBot )
{
	if( pBot != NULL )
		ns_free( pBot->moddata );
}

/** @brief GetBotModPtr
 *
 *  Retrieve module pointer for a bot
 *  NeoStats core use only.
 *
 *  @param pBot pointer to bot to lookup pointer for
 *
 *  @return none
 */

void *GetBotModPtr( const Bot *pBot )
{
	if( pBot != NULL )
		return pBot->moddata;
	return NULL;
}

/** @brief ClearBotModValue
 *
 *  Clear module value for a bot
 *  NeoStats core use only.
 *
 *  @param pBot pointer to bot to lookup pointer for
 *
 *  @return none
 */

void ClearBotModValue( Bot *pBot )
{
	if( pBot != NULL )
		pBot->moddata = NULL;
}

/** @brief SetBotModValue
 *
 *  Set module value for a bot
 *  NeoStats core use only.
 *
 *  @param pBot pointer to bot to lookup pointer for
 *  @param data pointer to set
 *
 *  @return none
 */

void SetBotModValue( Bot *pBot, void *data )
{
	if( pBot != NULL )
		pBot->moddata = data;
}

/** @brief GetBotModValue
 *
 *  Retrieve module value for a bot
 *  NeoStats core use only.
 *
 *  @param pBot pointer to bot to lookup pointer for
 *
 *  @return none
 */

void *GetBotModValue( const Bot *pBot )
{
	if( pBot != NULL )
		return pBot->moddata;
	return NULL;	
}
