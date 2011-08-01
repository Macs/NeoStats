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
** $Id: commands.c 3316 2008-03-05 08:12:49Z Fish $
*/

/*  TODO:
 *  - Sub command processing, e.g. "EXCLUDE ADD" ???
 *  - More error processing
 *  - Fix LEVELS command for SET and other intrinsics
 */

#include "neostats.h"
#include "helpstrings.h"
#include "modules.h"
#include "services.h"
#include "commands.h"
#include "settings.h"

static int bot_cmd_help( const CmdParams *cmdparams );
static int bot_cmd_about( const CmdParams *cmdparams );
static int bot_cmd_version( const CmdParams *cmdparams );
static int bot_cmd_credits( const CmdParams *cmdparams );
static int bot_cmd_levels( const CmdParams *cmdparams );
static int bot_cmd_help_set( const CmdParams *cmdparams, int userlevel );

/* help title strings for different user levels */
static char *help_level_title[]=
{
	"Operators",
	"Service Admins",
	"Service Roots",
};

/*  Intrinsic commands 
 *  These are all automatically added to a bot for handling 
 *  by the core. A module can override these by defining them
 *  in it's local bot command array.
 */

/*  Simplified command table for handling intrinsic commands.
 *  We do not require all entries since we only use a few for
 *  intrinsic handling. 
 */
static bot_cmd intrinsic_commands[]=
{
	{"HELP",	bot_cmd_help,	0, 	0,	cmd_help_help,		0, NULL, NULL},	
	{"VERSION",	bot_cmd_version,0, 	0,	cmd_help_version,	0, NULL, NULL},
	{"ABOUT",	bot_cmd_about,	0, 	0,	cmd_help_about,		0, NULL, NULL},
	{"CREDITS",	bot_cmd_credits,0, 	0,	cmd_help_credits,	0, NULL, NULL},
	{"LEVELS",	bot_cmd_levels,	0, 	0,	cmd_help_levels,	0, NULL, NULL},
	NS_CMD_END()
};

/** @brief calc_cmd_ulevel calculate cmd ulevel
 *  done as a function so we can support potentially complex  
 *  ulevel calculations without impacting other code.
 *
 *  @param pointer to command structure
 *  @return command user level requires
 */
static int calc_cmd_ulevel( const bot_cmd *cmd_ptr )
{
	if( cmd_ptr->ulevel > NS_ULEVEL_ROOT )
	{
		/* int pointer rather than value */
		return( *( ( int * )cmd_ptr->ulevel ) );
	}
	/* use cmd entry directly */
	return( cmd_ptr->ulevel );
}

/** @brief getuserlevel 
 *
 *	calculate ulevel
 *
 *  @param pointer to command structure
 *
 *  @return command user level requires
 */

int getuserlevel( const CmdParams *cmdparams )
{
	int ulevel = 0;
	int modlevel = 0;

	/* Generally we just want the standard user level */
	ulevel = UserLevel( cmdparams->source );
	/* If less than a locop see if the module can give us a user level */
	if( ulevel < NS_ULEVEL_LOCOPER )
	{
		if( cmdparams->bot->moduleptr->authcb != NULL )
		{
			modlevel = cmdparams->bot->moduleptr->authcb( cmdparams->source );
			if( modlevel > ulevel )
				ulevel = modlevel;
		}
	}
	return ulevel;
}

/** common message handlers */
void CommandReport( const Bot *botptr, const char *fmt, ... )
{
	static char buf[BUFSIZE];
	va_list ap;

	va_start( ap, fmt );
	ircvsnprintf( buf, BUFSIZE, fmt, ap );
	va_end( ap );
	if( IsNeoStatsSynched() && botptr && !nsconfig.cmdreport )
		irc_chanalert( botptr, "%s", buf );
	nlog( LOG_NOTICE, "%s", buf );
}

void msg_permission_denied( const CmdParams *cmdparams, const char *subcommand )
{
	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "Permission Denied", cmdparams->source ) );
	irc_chanalert( cmdparams->bot, _( "%s tried to use %s %s, but is not authorised" ), 
		cmdparams->source->name, cmdparams->param, subcommand );
	nlog( LOG_NORMAL, "%s tried to use %s %s, but is not authorised", 
		cmdparams->source->name, cmdparams->param, subcommand );
}

void msg_send_help( const CmdParams *cmdparams) 
{
	bot_cmd *cmd_ptr;
	int userlevel;
	int cmdlevel;

	/* space out message */
	irc_prefmsg(cmdparams->bot, cmdparams->source, " ");
	userlevel = getuserlevel( cmdparams );
	/* Process command list */
	if( cmdparams->bot->botcmds )
	{
		cmd_ptr = ( bot_cmd* )hnode_find( cmdparams->bot->botcmds, cmdparams->cmd );
		if( cmd_ptr )
		{
			cmdlevel = calc_cmd_ulevel( cmd_ptr );
			if( userlevel < cmdlevel )
			{
				msg_permission_denied( cmdparams, NULL );
				return;
			}		
			irc_prefmsg_list( cmdparams->bot, cmdparams->source, cmd_ptr->helptext + 1 );
			return;;
		}
	}

	/* Handle intrinsic commands */
	cmd_ptr = intrinsic_commands;
	while( cmd_ptr->cmd != NULL )
	{
		if( ircstrcasecmp( cmdparams->cmd, cmd_ptr->cmd ) == 0 )
		{
			irc_prefmsg_list( cmdparams->bot, cmdparams->source, cmd_ptr->helptext + 1 );
			return;
		}
		cmd_ptr++;
	}
	/* Handle SET if we have it */	
	if( cmdparams->bot->botsettings && userlevel >= cmdparams->bot->set_ulevel && ircstrcasecmp( cmdparams->cmd, "SET" ) == 0 )
	{
		bot_cmd_help_set( cmdparams, userlevel );		
		return;
	}
}
void msg_error_need_more_params( const CmdParams *cmdparams )
{
	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "Insufficient parameters", cmdparams->source ) );
	if (nsconfig.sendhelp) 
		msg_send_help(cmdparams);
	else
		irc_prefmsg( cmdparams->bot, cmdparams->source, __( "/msg %s HELP %s for more information", cmdparams->source ), 
			cmdparams->bot->name, cmdparams->cmd );
}

void msg_error_param_out_of_range( const CmdParams *cmdparams )
{
	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "Parameter out of range.", cmdparams->source ) );
	if (nsconfig.sendhelp) 
		msg_send_help(cmdparams);
	else
		irc_prefmsg( cmdparams->bot, cmdparams->source, __( "/msg %s HELP %s for more information", cmdparams->source ), 
			cmdparams->bot->name, cmdparams->cmd );
}

void msg_syntax_error( const CmdParams *cmdparams )
{
	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "Syntax error", cmdparams->source ) );
	if (nsconfig.sendhelp)
		msg_send_help(cmdparams);
	else
		irc_prefmsg( cmdparams->bot, cmdparams->source, __( "/msg %s HELP %s for more information", cmdparams->source ), 
			cmdparams->bot->name, cmdparams->cmd );
}

void msg_unknown_command( const CmdParams *cmdparams )
{
	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "Syntax error: unknown command: \2%s\2", cmdparams->source ), 
		cmdparams->param );
	if( nsconfig.cmdreport )
		irc_chanalert( cmdparams->bot, _( "%s requested %s, but that is an unknown command" ),
			cmdparams->source->name, cmdparams->param );
}

void msg_only_opers( const CmdParams *cmdparams )
{
	irc_prefmsg( cmdparams->bot, cmdparams->source, 
		__( "This service is only available to IRC operators.", cmdparams->source ) );
	irc_chanalert( cmdparams->bot, _( "%s requested %s, but is not an operator." ), 
		cmdparams->source->name, cmdparams->cmd );
	nlog( LOG_NORMAL, "%s requested %s, but is not an operator.", 
		cmdparams->source->name, cmdparams->cmd );
}

static void check_cmd_result( const CmdParams *cmdparams, int cmdret )
{
	switch( cmdret )
	{
		case NS_ERR_SYNTAX_ERROR:
			msg_syntax_error( cmdparams );
			break;
		case NS_ERR_NEED_MORE_PARAMS:
			msg_error_need_more_params( cmdparams );
			break;				
		case NS_ERR_PARAM_OUT_OF_RANGE:
			msg_error_param_out_of_range( cmdparams );
			break;				
		case NS_ERR_UNKNOWN_COMMAND:
		case NS_ERR_NO_PERMISSION:
			break;				
		case NS_FAILURE:
		case NS_SUCCESS:
		default:
			break;
	}
}

/** @brief add_bot_cmd adds a single command to the command hash
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int add_bot_cmd( hash_t *cmd_hash, bot_cmd *cmd_ptr ) 
{
	bot_cmd *hashentry;
	char confcmd[32];
	int ulevel = 0;

	/* Verify the command is OK before we add it so we do not have to 
	 * check validity during processing. Only check critical elements.
	 * For now we verify help during processing since it is not critical. */
	/* No command, we cannot recover from this */
	if( cmd_ptr->cmd == NULL )
	{
		nlog( LOG_ERROR, "add_bot_cmd: missing command" );
		return NS_FAILURE;
	}
	if( hash_lookup( cmd_hash, cmd_ptr->cmd ) != NULL )
	{
		nlog( LOG_ERROR, "add_bot_cmd: attempt to add duplicate command %s", cmd_ptr->cmd );
		return NS_FAILURE;
	}
	/* No handler, we cannot recover from this */
	if( cmd_ptr->handler == NULL )
	{
		nlog( LOG_ERROR, "add_bot_cmd: missing command handler, command %s not added", 
			cmd_ptr->cmd );
		return NS_FAILURE;
	}
	if( cmd_ptr->helptext == NULL )
	{
		nlog( LOG_ERROR, "add_bot_cmd: missing help text, command %s not added", 
			cmd_ptr->cmd );
		return NS_FAILURE;
	}
	/* Seems OK, add the command */
	hnode_create_insert( cmd_hash, cmd_ptr, cmd_ptr->cmd );
	ircsnprintf( confcmd, 32, "command%s", cmd_ptr->cmd );
	cmd_ptr->modptr = GET_CUR_MODULE();
	if( DBAFetchConfigInt( confcmd, &ulevel ) == NS_SUCCESS )
	{
		hashentry = ( bot_cmd * ) hnode_find( cmd_hash, cmd_ptr->cmd );
		hashentry->ulevel = ulevel;
	}
	dlog( DEBUG3, "add_bot_cmd: added command %s", cmd_ptr->cmd );
	return NS_SUCCESS;
}

/** @brief find_bot_cmd finds the bot command structure
 *
 * @return bot_cmd structure or NULL if not found
 */
 

bot_cmd *find_bot_cmd( const Bot *bot_ptr, const char *cmd) 
{
	hnode_t *cmdnode;
	
	/* Find the command */
	if( bot_ptr->botcmds == NULL )
		return NULL;
	cmdnode = hash_lookup( bot_ptr->botcmds, cmd );
	if( cmdnode != NULL )
		return hnode_get(cmdnode);
	return NULL;
}	


/** @brief del_bot_cmd deltes a single command to the command hash
 *
 * @return none
 */
void del_bot_cmd( hash_t *cmd_hash, const bot_cmd *cmd_ptr ) 
{
	hnode_t *cmdnode;
	
	/* Delete the command */
	cmdnode = hash_lookup( cmd_hash, cmd_ptr->cmd );
	if( cmdnode != NULL )
	{
		dlog( DEBUG3, "deleting command %s from services bot", ( ( bot_cmd* )hnode_get( cmdnode ) )->cmd );
		hash_delete_destroy_node( cmd_hash, cmdnode );
#ifdef USE_PERL
		if (IS_PERL_MOD(cmd_ptr->modptr))
		{
			ns_free(cmd_ptr->cmd);
			ns_free(cmd_ptr->moddata);
			/* XXX is this correct on a array of strings? */
			/* Mark: depends on how the array is allocated */
			ns_free(cmd_ptr->helptext);
		}
#endif /* USE_PERL */
	}
}

/** @brief add_bot_cmd_list adds a list of commands to the command hash
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int add_bot_cmd_list( Bot *bot_ptr, bot_cmd *bot_cmd_list ) 
{
	if( bot_cmd_list == NULL )
		return NS_FAILURE;
	/* If no hash create */
	if( bot_ptr->botcmds == NULL )
	{
		bot_ptr->botcmds = hash_create( HASHCOUNT_T_MAX, 0, 0 );
		if( bot_ptr->botcmds == NULL )
		{
			nlog( LOG_CRITICAL, "Unable to create botcmds hash" );
			return NS_FAILURE;
		}
	}
	/* Cycle through command list and add them */
	while( bot_cmd_list->cmd != NULL )
	{
		add_bot_cmd( bot_ptr->botcmds, bot_cmd_list );
		bot_cmd_list++;
	}
	return NS_SUCCESS;
}

/** @brief del_bot_cmd_list delete a list of commands to the command hash
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
void del_bot_cmd_list( const Bot *bot_ptr, const bot_cmd *bot_cmd_list ) 
{
	if( bot_ptr != NULL && bot_ptr->botcmds != NULL )
	{
		/* Cycle through command list and delete them */
		while( bot_cmd_list->cmd != NULL )
		{
			del_bot_cmd( bot_ptr->botcmds, bot_cmd_list );
			bot_cmd_list++;
		}
	}
}

/** @brief del_all_bot_cmds delete all commands from the bot
 *
 * @return none
 */
void del_all_bot_cmds( Bot *bot_ptr ) 
{
	hnode_t *cmdnode;
	hscan_t hs;

	/* Check we have a command hash */
	if( bot_ptr->botcmds != NULL )
	{
		/* Cycle through command hash and delete each command */
		hash_scan_begin( &hs, bot_ptr->botcmds );
		while( ( cmdnode = hash_scan_next( &hs ) ) != NULL )
		{
			dlog( DEBUG3, "deleting command %s from services bot", ( ( bot_cmd* )hnode_get( cmdnode ) )->cmd );
			hash_scan_delete_destroy_node( bot_ptr->botcmds, cmdnode );
		}
		/* Destroy command */
		hash_destroy( bot_ptr->botcmds );
		bot_ptr->botcmds = NULL;
	}
}

/** @brief add_services_cmd_list adds a list of commands to the services bot
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int add_services_cmd_list( bot_cmd *bot_cmd_list ) 
{
	if( !IsModuleInSynch( GET_CUR_MODULE() ) )
	{
		SetModuleError( GET_CUR_MODULE() );
		return NS_FAILURE;
	}	
	return add_bot_cmd_list( ns_botptr, bot_cmd_list );
}

/** @brief del_services_cmd_list delete a list of commands from the services bot
 *
 * @return none
 */
void del_services_cmd_list( const bot_cmd *bot_cmd_list ) 
{
	del_bot_cmd_list( ns_botptr, bot_cmd_list );
}

/** @brief intrinsic_handler
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int intrinsic_handler( const CmdParams *cmdparams, const bot_cmd_handler handler )
{
	int cmdret;

	SET_RUN_LEVEL( cmdparams->bot->moduleptr );
	cmdret = handler( cmdparams );
	RESET_RUN_LEVEL();
	check_cmd_result( cmdparams, cmdret );
	return NS_SUCCESS;
}

/** @brief run_intrinsic_cmds process bot intrinsic command list
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int run_intrinsic_cmds( const char *cmd, const CmdParams *cmdparams )
{
	bot_cmd *cmd_ptr;

	/* Handle SET if we have it */
	if( cmdparams->bot->botsettings && ircstrcasecmp( cmd, "SET" ) == 0 )
	{
		intrinsic_handler( cmdparams, bot_cmd_set );
		return NS_SUCCESS;
	}
	/* Handle intrinsic commands */
	cmd_ptr = intrinsic_commands;
	if( ircstrcasecmp( cmd, "LEVELS" ) == 0 && !( cmdparams->bot->flags & BOT_FLAG_ROOT ) ) 
		return NS_FAILURE;
	while( cmd_ptr->cmd != NULL )
	{
		if( ircstrcasecmp( cmd, cmd_ptr->cmd ) == 0 )
		{
			intrinsic_handler( cmdparams, cmd_ptr->handler );
			return NS_SUCCESS;
		}
		cmd_ptr++;
	}
	return NS_FAILURE;
}

/** @brief run_cmd
 *
 *  process bot command
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int run_cmd( CmdParams *cmdparams, const bot_cmd *cmd_ptr )
{
	if( setjmp( sigvbuf ) == 0 )
		return cmd_ptr->handler( cmdparams );
	return NS_FAILURE;
}

/** @brief run_bot_cmd
 *
 *  process bot command list
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int run_bot_cmd( CmdParams *cmdparams, int ischancmd )
{
	static char privmsgbuffer[BUFSIZE];
	int userlevel;
	bot_cmd *cmd_ptr;
	int cmdret = NS_FAILURE;
	int cmdlevel;
	char **av;
	unsigned int ac = 0;
	unsigned int i;
	int processed = 0;

	SET_SEGV_LOCATION();
	strlcpy( privmsgbuffer, cmdparams->param, BUFSIZE );
	ac = split_buf( privmsgbuffer, &av );
	cmdparams->cmd = av[0];
	cmdparams->ac = 0;
	for( i = 1; i < ac; i++ )
		AddStringToList( &cmdparams->av, av[i], &cmdparams->ac );
	userlevel = getuserlevel( cmdparams ); 
	/* Check user authority to use this command set */
	if( ( ( cmdparams->bot->flags & BOT_FLAG_RESTRICT_OPERS ) && ( userlevel < NS_ULEVEL_OPER ) ) ||
		( ( cmdparams->bot->flags & BOT_FLAG_ONLY_OPERS ) && nsconfig.onlyopers && ( userlevel < NS_ULEVEL_OPER ) ) ) 
	{
		msg_only_opers( cmdparams );
		cmdret = NS_SUCCESS;
		processed = 1;
	} 
	else if( ac && cmdparams->bot->botcmds ) 
	{
		/* Process command list */
		cmd_ptr = ( bot_cmd * ) hnode_find( cmdparams->bot->botcmds, av[0] );
		if( cmd_ptr ) 
		{
			cmdparams->cmd_ptr = cmd_ptr;
			processed = 1;
			if( ischancmd && ( cmd_ptr->flags & CMD_FLAG_PRIVMSGONLY ) ) 
			{
				dlog( DEBUG2, "dropping channel command %s since it is flagged privmsg only ", cmdparams->cmd );
				cmdret = NS_FAILURE;
			} 
			else if( !ischancmd && ( cmd_ptr->flags & CMD_FLAG_CHANONLY ) ) 
			{
				dlog( DEBUG2, "dropping privmsg command %s since it is flagged channel only ", cmdparams->cmd );
				cmdret = NS_FAILURE;
			} 
			else 
			{
				cmdlevel = calc_cmd_ulevel( cmd_ptr );
				/* Is user authorised to issue this command? */
				if( userlevel < cmdlevel ) 
				{
					msg_permission_denied( cmdparams, NULL );
				} 
				/* Check parameter count */
				else if( cmdparams->ac < cmd_ptr->minparams ) 
				{
					msg_error_need_more_params( cmdparams );
				} 
				else 
				{
					/* Seems OK so report the command call so modules do not have to */
					SET_RUN_LEVEL( cmd_ptr->modptr );
					if( nsconfig.cmdreport )
						irc_chanalert( cmdparams->bot, _( "%s used %s" ), cmdparams->source->name, cmd_ptr->cmd );
					/* Log command message */
					nlog( LOG_NORMAL, "%s used %s", cmdparams->source->name, cmdparams->param );
					/* call handler */
					cmdret = run_cmd( cmdparams, cmd_ptr );
					check_cmd_result( cmdparams, cmdret );
					RESET_RUN_LEVEL();
				}
				cmdret = NS_SUCCESS;
			}
		}
	} 
	/* if the bot doesn't have any commands, then don't run the intrinsic commands either */
	if( ac && processed != 1 && cmdparams->bot->botcmds != NULL) {
		cmdret = run_intrinsic_cmds( av[0], cmdparams );
		if ((cmdret != NS_SUCCESS) && !ischancmd) msg_unknown_command(cmdparams);
	}
	if( ac == 0)
	{
		cmdret = NS_FAILURE;
	}
	if( ac )
		ns_free( av );
	return cmdret;
}

/** @brief bot_cmd_help_set process bot help command
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int bot_cmd_help_set( const CmdParams *cmdparams, int userlevel )
{
	hnode_t *setnode;
	hscan_t hs;
	bot_setting* set_ptr;

	/* Display HELP SET intro text and LIST command */
	irc_prefmsg_list( cmdparams->bot, cmdparams->source, cmd_help_set );
	/* Display option specific text for current user level */
	hash_scan_begin( &hs, cmdparams->bot->botsettings );
	while( ( setnode = hash_scan_next( &hs ) ) != NULL )
	{
		set_ptr = hnode_get( setnode );
		if( set_ptr->helptext && userlevel >= set_ptr->ulevel )
		{
			irc_prefmsg_list( cmdparams->bot, cmdparams->source, set_ptr->helptext );
			irc_prefmsg( cmdparams->bot, cmdparams->source, " " );
		}
	}
	return NS_SUCCESS;
}

/** @brief bot_cmd_help_on_help 
 *
 *  display help on help text
 *
 *  @param pointer to command parameters
 *
 *  @return none
 */
static void bot_cmd_help_on_help( const CmdParams *cmdparams, int helptype, int othercmds )
{
	/* Generate help on help footer text */
		irc_prefmsg( cmdparams->bot, cmdparams->source, " " );
		irc_prefmsg( cmdparams->bot, cmdparams->source, __( "To execute a command:", cmdparams->source ) );
		if (helptype == 0) 
			irc_prefmsg( cmdparams->bot, cmdparams->source, "    \2/msg %s command\2", cmdparams->bot->name);
		else 
			irc_prefmsg( cmdparams->bot, cmdparams->source, "    \2!command\2 (in the channel)");
		irc_prefmsg( cmdparams->bot, cmdparams->source, __( "For help on a command:", cmdparams->source ) );
		if (helptype == 0)
			irc_prefmsg( cmdparams->bot, cmdparams->source, "    \2/msg %s HELP command\2", cmdparams->bot->name);
		else
			irc_prefmsg( cmdparams->bot, cmdparams->source, "    \2!HELP command\2 (in the channel)");
		if (othercmds) {
			irc_prefmsg( cmdparams->bot, cmdparams->source, " ");
			irc_prefmsg( cmdparams->bot, cmdparams->source, "Other Commands are available.");
			if (helptype == 0) 
				irc_prefmsg( cmdparams->bot, cmdparams->source, "Type \2!help\2 in the same channel as me for a list of those commands");
			else 
				irc_prefmsg( cmdparams->bot, cmdparams->source, "Type \2/msg %s help\2 for a list of those commands", cmdparams->bot->name);	
		}
}

/** @brief bot_cmd_help process bot help command
 *
 * @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int bot_cmd_help( const CmdParams *cmdparams )
{
	char *curlevelmsg=NULL;
	int donemsg=0;
	bot_cmd *cmd_ptr;
	int curlevel, lowlevel;
	hscan_t hs;
	int userlevel;
	int cmdlevel;
	int helptype = 0;
	int othercmds = 0;;

	userlevel = getuserlevel( cmdparams );

	/* If no parameter to help, generate main help text */
	if( cmdparams->ac < 1 )
	{
		lowlevel = 0;
		curlevel = 30;
		if( nsconfig.cmdreport )
			irc_chanalert( cmdparams->bot, _( "%s requested %s help" ), cmdparams->source->name, cmdparams->bot->name );
		nlog( LOG_NORMAL, "%s requested %s help", cmdparams->source->name, cmdparams->bot->name );
		irc_prefmsg( cmdparams->bot, cmdparams->source, __( "\2The following commands can be used with %s:\2",cmdparams->source ), cmdparams->bot->name );

		/* Handle intrinsic commands */
		cmd_ptr = intrinsic_commands;
		while( cmd_ptr->cmd != NULL )
		{
			/* Levels are only intrinsic for root module bot */
			if( ircstrcasecmp( cmd_ptr->cmd, "LEVELS" ) == 0 && !( cmdparams->bot->flags & BOT_FLAG_ROOT ) )
			{
				cmd_ptr++;
				continue;
			}
			/* Check for module override */	
			if( !cmdparams->bot->botcmds || !hash_lookup( cmdparams->bot->botcmds, cmd_ptr->cmd ) )
			{
				irc_prefmsg( cmdparams->bot, cmdparams->source, "    %-20s %s", cmd_ptr->cmd, cmd_ptr->helptext[0] );
			}
			cmd_ptr++;
		}
		/* Do we have a set command? */
		if( cmdparams->bot->botsettings && userlevel >= cmdparams->bot->set_ulevel )
		{
			irc_prefmsg( cmdparams->bot, cmdparams->source, "    %-20s Configure %s", "SET", cmdparams->bot->name );
		}
		if( cmdparams->bot->botcmds )
		{
			for(;;)
			{
				hnode_t* cmdnode;
				if (cmdparams->channel)
					helptype = 1;

				hash_scan_begin( &hs, cmdparams->bot->botcmds );
				while( ( cmdnode = hash_scan_next( &hs ) ) != NULL )
				{
					cmd_ptr = hnode_get( cmdnode );
					cmdlevel = calc_cmd_ulevel( cmd_ptr );
					/* display relevent help */
					if (nsconfig.allhelp == 0) {
						if ((!cmdparams->channel) && (cmd_ptr->flags & CMD_FLAG_CHANONLY)) {
							othercmds = 1;
							continue;
						}
						if ((cmdparams->channel) && (cmd_ptr->flags & CMD_FLAG_PRIVMSGONLY)) {
							othercmds = 1;
							continue;  
						}
					}
					if( ( cmdlevel < curlevel ) && ( cmdlevel >= lowlevel ) )
					{
						if( curlevelmsg && !donemsg )
						{
							irc_prefmsg( cmdparams->bot, cmdparams->source, __( "\2Additional commands available to %s:\2", cmdparams->source ), curlevelmsg );
							donemsg = 1;
						}
						irc_prefmsg( cmdparams->bot, cmdparams->source, "    %-20s %s", cmd_ptr->cmd, cmd_ptr->helptext[0] );
					}
				}
				if( lowlevel >= userlevel )
				{
					break;
				}
				if( userlevel >= curlevel )
				{
					switch( curlevel )
					{
						case 30:
							curlevel = NS_ULEVEL_OPER;
							lowlevel = 30;
							curlevelmsg=NULL;
							donemsg=0;
							break;
						case NS_ULEVEL_OPER:
							curlevel = NS_ULEVEL_ADMIN;
							lowlevel = NS_ULEVEL_OPER;
							curlevelmsg=help_level_title[0];
							donemsg=0;
							break;
						case NS_ULEVEL_ADMIN:
							curlevel = NS_ULEVEL_ROOT;
							lowlevel = NS_ULEVEL_ADMIN;
							curlevelmsg=help_level_title[1];
							donemsg=0;
							break;
						case NS_ULEVEL_ROOT:
							curlevel = ( NS_ULEVEL_ROOT + 1 );
							lowlevel = NS_ULEVEL_ROOT;
							curlevelmsg=help_level_title[2];
							donemsg=0;
							break;
						default:	
							break;
					}
				}						
				else
					break;
			}
		}
		bot_cmd_help_on_help( cmdparams, helptype, othercmds);
		return NS_SUCCESS;
	}
	if( nsconfig.cmdreport )
	{
		irc_chanalert( cmdparams->bot, _( "%s requested %s help on %s" ), cmdparams->source->name, cmdparams->bot->name, cmdparams->av[0] );
	}
	nlog( LOG_NORMAL, "%s requested %s help on %s", cmdparams->source->name, cmdparams->bot->name, cmdparams->av[0] );

	/* Process command list */
	if( cmdparams->bot->botcmds )
	{
		cmd_ptr = ( bot_cmd* )hnode_find( cmdparams->bot->botcmds, cmdparams->av[0] );
		if( cmd_ptr )
		{
			cmdlevel = calc_cmd_ulevel( cmd_ptr );
			if( userlevel < cmdlevel )
			{
				msg_permission_denied( cmdparams, NULL );
				return NS_ERR_NO_PERMISSION;
			}		
			irc_prefmsg_list( cmdparams->bot, cmdparams->source, cmd_ptr->helptext + 1 );
			return NS_SUCCESS;
		}
	}

	/* Handle intrinsic commands */
	cmd_ptr = intrinsic_commands;
	while( cmd_ptr->cmd != NULL )
	{
		if( ircstrcasecmp( cmdparams->av[0], cmd_ptr->cmd ) == 0 )
		{
			irc_prefmsg_list( cmdparams->bot, cmdparams->source, cmd_ptr->helptext + 1 );
			return NS_SUCCESS;
		}
		cmd_ptr++;
	}
	/* Handle SET if we have it */	
	if( cmdparams->bot->botsettings && userlevel >= cmdparams->bot->set_ulevel && ircstrcasecmp( cmdparams->av[0], "SET" ) == 0 )
	{
		bot_cmd_help_set( cmdparams, userlevel );		
		return NS_SUCCESS;
	}

	/* Command not found so report as unknown */
	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "No help available or unknown help topic: \2%s\2", cmdparams->source ), cmdparams->av[2] );
	return NS_ERR_UNKNOWN_COMMAND;
}

/**	Support function for command handlers to call to check that target nick 
 *	is not the bot and is on IRC. Done in core to avoid module code bloat.
 */ 
Client *FindValidUser( const Bot *botptr, const Client *sourceuser, const char *target_nick )
{
	Client *target;
	
	/* Check target user is on IRC */
	target = FindUser( target_nick );
	if( !target )
	{
		irc_prefmsg( botptr, sourceuser, 
			__( "%s cannot be found on IRC, message not sent.", sourceuser ), target_nick );
		return NULL;
	}
	/* Check for message to self */
	if( IsMe( target ) )
	{
		irc_prefmsg( botptr, sourceuser, __( "Cannot send message to a service bot.", sourceuser ) );
		return NULL;
	}
	/* User OK */
	return target;
}

/** @brief bot_cmd_about process bot about command
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int bot_cmd_about( const CmdParams *cmdparams )
{
	if( cmdparams->bot->moduleptr )
	{
		if( IS_STANDARD_MOD( cmdparams->bot->moduleptr ) )
		{
			irc_prefmsg_list( cmdparams->bot, cmdparams->source, cmdparams->bot->moduleptr->info->about_text );
#ifdef USE_PERL
		}
		else
		{
			irc_prefmsg(cmdparams->bot, cmdparams->source, "Not Available");
#endif /* USE_PERL */
		}
	}
	return NS_SUCCESS;
}

/** @brief bot_cmd_version process bot version command
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int bot_cmd_version( const CmdParams *cmdparams )
{
	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "\2%s version\2", cmdparams->source ), 
		cmdparams->bot->moduleptr->info->name );
	irc_prefmsg( cmdparams->bot, cmdparams->source, "%s %s %s", 
		cmdparams->bot->moduleptr->info->version, 
		cmdparams->bot->moduleptr->info->build_date, 
		cmdparams->bot->moduleptr->info->build_time );
	return NS_SUCCESS;
}

/** @brief bot_cmd_credits process bot credits command
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int bot_cmd_credits( const CmdParams *cmdparams )
{
	if( cmdparams->bot->moduleptr )
	{
		if( IS_STANDARD_MOD( cmdparams->bot->moduleptr ) )
		{
			irc_prefmsg_list( cmdparams->bot, cmdparams->source, 
				cmdparams->bot->moduleptr->info->copyright );
#ifdef USE_PERL
		}
		else
		{
			irc_prefmsg(cmdparams->bot, cmdparams->source, "Not Available");
#endif /* USE_PERL */
		}
	}
	return NS_SUCCESS;
}

/** @brief bot_cmd_levels process bot level command
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
static int bot_cmd_levels( const CmdParams *cmdparams )
{
	char confcmd[32];
	bot_cmd *cmd_ptr;
	int userlevel;

	if( cmdparams->ac < 1 )
		return NS_ERR_NEED_MORE_PARAMS;
	if( !cmdparams->bot->botcmds )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, "No commands found." );
		return NS_SUCCESS;
	}
	if( ircstrcasecmp( cmdparams->av[0], "LIST" ) == 0 )
	{
		hnode_t *cmdnode;
		hscan_t hs;

		/* Cycle through command hash and list each command */
		hash_scan_begin( &hs, cmdparams->bot->botcmds );
		while( ( cmdnode = hash_scan_next( &hs ) ) != NULL )
		{
			cmd_ptr = ( ( bot_cmd* )hnode_get( cmdnode ) );
			irc_prefmsg( cmdparams->bot, cmdparams->source, "%s %d", cmd_ptr->cmd, cmd_ptr->ulevel );
		}
		return NS_SUCCESS;
	}
	if( cmdparams->ac < 2 )
		return NS_ERR_NEED_MORE_PARAMS;
	userlevel = getuserlevel( cmdparams );
	if( userlevel < NS_ULEVEL_ROOT )
	{
		msg_permission_denied( cmdparams, cmdparams->cmd );
		return NS_ERR_NO_PERMISSION;
	}
	cmd_ptr = ( bot_cmd * )hnode_find( cmdparams->bot->botcmds, cmdparams->av[0] );
	if( cmd_ptr )
	{
		int newlevel = 0;

		newlevel = atoi( cmdparams->av[1] );
		if( newlevel >= 0 && newlevel <= NS_ULEVEL_ROOT )
		{
			cmd_ptr->ulevel = newlevel;
			ircsnprintf( confcmd, 32, "command%s", cmd_ptr->cmd );
			DBAStoreConfigInt( confcmd, &newlevel );
			return NS_SUCCESS;
		}
	}
	return NS_ERR_SYNTAX_ERROR;
}
