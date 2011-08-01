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
** $Id: ctcp.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "nsevents.h"
#include "ctcp.h"
#include "dcc.h"
#include "services.h"
#include "users.h"

/** CTCP subsystem
 *
 *  Handle incoming and outgoing CTCP messages
 */

/* CTCP master version request bot pointer */
static const Bot *CTCPVersionMasterBot = NULL;

/* CTCP command handler type */
typedef int( *ctcp_cmd_handler )( CmdParams *cmdparams );

/* CTCP command lookup table */
typedef struct ctcp_cmd {
	const char *cmd;
	ctcp_cmd_handler req_handler;
	ctcp_cmd_handler rpl_handler;
} ctcp_cmd;

/* CTCP command type prototypes */
static int ctcp_req_version( CmdParams *cmdparams );
static int ctcp_rpl_version( CmdParams *cmdparams );
static int ctcp_req_finger( CmdParams *cmdparams );
static int ctcp_rpl_finger( CmdParams *cmdparams );
static int ctcp_req_action( CmdParams *cmdparams );
static int ctcp_req_dcc( CmdParams *cmdparams );
static int ctcp_rpl_dcc( CmdParams *cmdparams );
static int ctcp_req_time( CmdParams *cmdparams );
static int ctcp_rpl_time( CmdParams *cmdparams );
static int ctcp_req_ping( CmdParams *cmdparams );
static int ctcp_rpl_ping( CmdParams *cmdparams );
static int ctcp_req_unhandled( CmdParams *cmdparams );
static int ctcp_rpl_unhandled( CmdParams *cmdparams );

/* CTCP command lookup table */
static ctcp_cmd ctcp_cmds[] =
{
	{"VERSION",	ctcp_req_version,	ctcp_rpl_version },
	{"FINGER",	ctcp_req_finger,	ctcp_rpl_finger },
	{"ACTION",	ctcp_req_action,	NULL },
	{"DCC",		ctcp_req_dcc,		ctcp_rpl_dcc },
	{"TIME",	ctcp_req_time,		ctcp_rpl_time },
	{"PING",	ctcp_req_ping,		ctcp_rpl_ping },
	{NULL,		NULL,				NULL}
};

/** @brief strip_ctcp_codes
 *
 *  strip ctcp codes from given message
 *  CTCP subsystem use only.
 *
 *  @param line to strip
 *
 *  @return none but line is modified
 */

static void strip_ctcp_codes( char *line )
{
	char *outline = line;
	while( *line != '\0' )
	{
		if( (*line ) != '\1' )
		{
			*outline = *line;
			outline++;
		}
		line++;
	}
	*outline = 0;
}

/** @brief ctcp_private
 *
 *  handle incoming CTCP PRIVMSG i.e. requests
 *  NeoStats core use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ctcp_private( CmdParams *cmdparams )
{
	ctcp_cmd* cmd;
	size_t len;
    
	if( cmdparams->param[0] == '\1' )
	{
		strip_ctcp_codes( cmdparams->param );
		cmd = ctcp_cmds;
		while( cmd->cmd != NULL )
		{	
			len = strlen( cmd->cmd );
			if( ircstrncasecmp( cmd->cmd, cmdparams->param, len  ) == 0 )
			{
				cmdparams->param += ( len + 1 );		
				if( cmd->req_handler )
				{
					/* Note ( void ) prefix to indicate 
					 * we do not care about return value */
					( void )cmd->req_handler( cmdparams );
				}
				return NS_SUCCESS;
			}
			cmd++;
		}
	}
	ctcp_req_unhandled( cmdparams );
	return NS_SUCCESS;
}

/** @brief ctcp_notice
 *
 *  handle incoming CTCP NOTICE i.e. replies
 *  NeoStats core use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ctcp_notice( CmdParams *cmdparams )
{
	ctcp_cmd* cmd;
	size_t len;
    
	if( cmdparams->param[0] == '\1' )
	{
		strip_ctcp_codes( cmdparams->param );
		cmd = ctcp_cmds;
		while( cmd->cmd != NULL )
		{
			len = strlen( cmd->cmd );
			if( ircstrncasecmp( cmd->cmd, cmdparams->param, len  ) == 0 )
			{
				cmdparams->param += ( len + 1 );		
				if( cmd->rpl_handler )
				{
					/* Note ( void ) prefix to indicate 
					 * we do not care about return value */
					( void )cmd->rpl_handler( cmdparams );
				}
				return NS_SUCCESS;
			}
			cmd++;
		}
	}
	ctcp_rpl_unhandled( cmdparams );
	return NS_SUCCESS;
}

/** @brief ctcp_cprivate
 *
 *  handle incoming CTCP channel PRIVMSG
 *  NeoStats core use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ctcp_cprivate( CmdParams *cmdparams )
{
	dlog( DEBUG5, "Channel CTCP requests currently not supported" );
	return NS_SUCCESS;
}

/** @brief ctcp_cnotice
 *
 *  handle incoming CTCP channel NOTICE
 *  NeoStats core use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ctcp_cnotice( CmdParams *cmdparams )
{
	dlog( DEBUG5, "Channel CTCP replies currently not supported" );
	return NS_SUCCESS;
}

/** @brief ctcp_req_version
 *
 *  handle incoming CTCP VERSION request
 *  CTCP subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ctcp_req_version( CmdParams *cmdparams )
{
	dlog( DEBUG5, "RX: CTCP VERSION request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPVERSIONREQ, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

/** @brief ctcp_rpl_version
 *
 *  handle incoming CTCP VERSION reply
 *  CTCP subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ctcp_rpl_version( CmdParams *cmdparams )
{
	dlog( DEBUG5, "RX: CTCP VERSION reply from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendAllModuleEvent( EVENT_CTCPVERSIONRPLBC, cmdparams );
	SendModuleEvent( EVENT_CTCPVERSIONRPL, cmdparams, cmdparams->bot->moduleptr );
	SetUserVersion( cmdparams->source, cmdparams->param );
	return NS_SUCCESS;
}

/** @brief irc_ctcp_version_req
 *
 *  Send a correctly formatted CTCP VERSION request
 *  Module call.
 *
 *  @param botptr, pointer to bot sending request
 *  @param target, pointer to client to send to
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_ctcp_version_req( const Bot *botptr, const Client *target ) 
{
	dlog( DEBUG5, "TX: CTCP VERSION request from %s to %s", botptr->name, target->name );
	irc_privmsg( botptr, target, "\1VERSION\1" );
	return NS_SUCCESS;
}

/** @brief master_ctcp_version_req
 *
 *  Send a correctly formatted CTCP VERSION request
 *  Module call.
 *
 *  @param target, pointer to client to send to
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int master_ctcp_version_req( const Client *target ) 
{
	const Bot *botptr;

	botptr = CTCPVersionMasterBot;
	if( botptr == NULL )
		botptr = ns_botptr;
	dlog( DEBUG5, "TX: CTCP VERSION request from %s to %s", botptr->name, target->name );
	irc_privmsg( botptr, target, "\1VERSION\1" );
	return NS_SUCCESS;
}

/** @brief SetCTCPVersionMaster
 *
 *  Set CTCP version master
 *  Module call.
 *
 *  @param botptr, pointer to bot sending request
 *
 *  @return none
 */

void SetCTCPVersionMaster( const Bot *bot )
{
	CTCPVersionMasterBot = bot;
}

/** @brief ctcp_req_finger
 *
 *  handle incoming CTCP FINGER request
 *  CTCP subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ctcp_req_finger( CmdParams *cmdparams )
{
	dlog( DEBUG5, "RX: CTCP FINGER request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPFINGERREQ, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

/** @brief ctcp_rpl_finger
 *
 *  handle incoming CTCP FINGER reply
 *  CTCP subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ctcp_rpl_finger( CmdParams *cmdparams )
{
	dlog( DEBUG5, "RX: CTCP FINGER reply from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendAllModuleEvent( EVENT_CTCPFINGERRPLBC, cmdparams );
	SendModuleEvent( EVENT_CTCPFINGERRPL, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

/** @brief irc_ctcp_finger_req
 *
 *  Send a correctly formatted CTCP FINGER request
 *  Module call.
 *
 *  @param botptr, pointer to bot sending request
 *  @param target, pointer to client to send to
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_ctcp_finger_req( const Bot *botptr, const Client *target ) 
{
	dlog( DEBUG5, "TX: CTCP FINGER request from %s to %s", botptr->name, target->name );
	irc_privmsg( botptr, target, "\1FINGER\1" );
	return NS_SUCCESS;
}

/** @brief ctcp_req_action
 *
 *  handle incoming CTCP ACTION request
 *  CTCP subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ctcp_req_action( CmdParams *cmdparams )
{
	dlog( DEBUG5, "RX: CTCP ACTION request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPACTIONREQ, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

/** @brief irc_ctcp_action_req
 *
 *  Send a correctly formatted CTCP ACTION request
 *  Module call.
 *
 *  @param botptr, pointer to bot sending request
 *  @param target, pointer to client to send to
 *  @param action to send
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_ctcp_action_req( const Bot *botptr, const Client *target, const char *action ) 
{
	dlog( DEBUG5, "TX: Sending CTCP ACTION request from %s to %s", botptr->name, target->name );
	irc_privmsg( botptr, target, "\1ACTION %s\1", action );
	return NS_SUCCESS;
}

/** @brief irc_ctcp_action_req_channel
 *
 *  Send a correctly formatted CTCP ACTION request
 *  Module call.
 *
 *  @param botptr, pointer to bot sending request
 *  @param channel to send to
 *  @param action to send
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_ctcp_action_req_channel( const Bot *botptr, const Channel* channel, const char *action ) 
{
	dlog( DEBUG5, "TX: Sending CTCP ACTION request from %s to %s", botptr->name, channel->name );
	irc_chanprivmsg( botptr, channel->name, "\1ACTION %s\1", action );
	return NS_SUCCESS;
}

/** @brief ctcp_req_dcc
 *
 *  handle incoming CTCP DCC request
 *  CTCP subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ctcp_req_dcc( CmdParams *cmdparams )
{
	dlog( DEBUG5, "RX: CTCP DCC request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	dcc_req( cmdparams  );
	return NS_SUCCESS;
}

/** @brief ctcp_rpl_dcc
 *
 *  handle incoming CTCP DCC reply
 *  CTCP subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ctcp_rpl_dcc( CmdParams *cmdparams )
{
	dlog( DEBUG5, "RX: CTCP DCC reply from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	dlog( DEBUG5, "CTCP DCC replies currently not supported" );
	return NS_SUCCESS;
}

/** @brief ctcp_req_time
 *
 *  handle incoming CTCP TIME request
 *  CTCP subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ctcp_req_time( CmdParams *cmdparams )
{
	dlog( DEBUG5, "RX: CTCP TIME request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPTIMEREQ, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

/** @brief ctcp_rpl_time
 *
 *  handle incoming CTCP TIME reply
 *  CTCP subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ctcp_rpl_time( CmdParams *cmdparams )
{
	dlog( DEBUG5, "RX: CTCP TIME reply from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendAllModuleEvent( EVENT_CTCPTIMERPLBC, cmdparams );
	SendModuleEvent( EVENT_CTCPTIMERPL, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

/** @brief irc_ctcp_time_req
 *
 *  Send a correctly formatted CTCP TIME request
 *  Module call.
 *
 *  @param botptr, pointer to bot sending request
 *  @param target, pointer to client to send to
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_ctcp_time_req( const Bot *botptr, const Client *target ) 
{
	dlog( DEBUG5, "TX: CTCP TIME request from %s to %s", botptr->name, target->name );
	irc_privmsg( botptr, target, "\1TIME\1" );
	return NS_SUCCESS;
}

/** @brief ctcp_req_ping
 *
 *  handle incoming CTCP PING request
 *  CTCP subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ctcp_req_ping( CmdParams *cmdparams )
{
	dlog( DEBUG5, "RX: CTCP PING request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPPINGREQ, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

/** @brief ctcp_rpl_ping
 *
 *  handle incoming CTCP PING reply
 *  CTCP subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ctcp_rpl_ping( CmdParams *cmdparams )
{
	dlog( DEBUG5, "RX: CTCP PING reply from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendAllModuleEvent( EVENT_CTCPPINGRPLBC, cmdparams );
	SendModuleEvent( EVENT_CTCPPINGRPL, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

/** @brief irc_ctcp_ping_req
 *
 *  Send a correctly formatted CTCP PING request
 *  Module call.
 *
 *  @param botptr, pointer to bot sending request
 *  @param target, pointer to client to send to
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_ctcp_ping_req( const Bot *botptr, const Client *target ) 
{
	dlog( DEBUG5, "TX: CTCP PING request from %s to %s", botptr->name, target->name );
	irc_privmsg( botptr, target, "\1PING\1" );
	return NS_SUCCESS;
}

/** @brief ctcp_req_unhandled
 *
 *  handle incoming CTCP request that we have no specific handler for
 *  CTCP subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ctcp_req_unhandled( CmdParams *cmdparams )
{
	dlog( DEBUG5, "RX: CTCP UNHANDLED request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPUNHANDLEDREQ, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

/** @brief ctcp_rpl_unhandled
 *
 *  handle incoming CTCP reply that we have no specific handler for
 *  CTCP subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ctcp_rpl_unhandled( CmdParams *cmdparams )
{
	dlog( DEBUG5, "RX: CTCP UNHANDLED reply from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_CTCPUNHANDLEDRPL, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

/** @brief irc_ctcp_ping_req
 *
 *  Send a correctly formatted CTCP request that we have no specific handler for
 *  Module call.
 *
 *  @param botptr, pointer to bot sending request
 *  @param target, pointer to client to send to
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_ctcp_unhandled_req( const Bot *botptr, const Client *target, const char *ctcp_command )
{
	dlog( DEBUG5, "TX: CTCP UNHANDLED request from %s to %s: %s", botptr->name, target->name, ctcp_command );
	irc_privmsg( botptr, target, "\1%s\1", ctcp_command );
	return NS_SUCCESS;
}

/** @brief irc_ctcp_unhandled_rpl
 *
 *  Send a correctly formatted CTCP reply that we have no specific handler for
 *  Module call.
 *
 *  @param botptr, pointer to bot sending reply
 *  @param target, pointer to client to send to
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_ctcp_unhandled_rpl( const Bot *botptr, const Client *target, const char *ctcp_command, const char *ctcp_parameters )
{
	dlog( DEBUG5, "TX: CTCP UNHANDLED reply from %s to %s: %s %s", botptr->name, target->name, ctcp_command, ctcp_parameters );
	irc_privmsg( botptr, target, "\1%s %s\1", ctcp_command, ctcp_parameters );
	return NS_SUCCESS;
}
