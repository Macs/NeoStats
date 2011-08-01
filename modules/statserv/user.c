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
** $Id: user.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "statserv.h"
#include "stats.h"
#include "network.h"
#include "server.h"
#include "tld.h"

/** @brief AddUser
 *
 *  Add user to statserv
 *
 *  @param u pointer to client to add
 *  @param v not used
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int AddUser( Client *u, void *v )
{
	SET_SEGV_LOCATION();
	AddServerUser( u );
	AddNetworkUser();
	AddTLDUser( u );
	return NS_FALSE;
}

/** @brief DelUser
 *
 *  Delete user from statserv
 *
 *  @param u pointer to client to delete
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static void DelUser( const Client *u )
{
	if( IsOper( u ) )
	{
		dlog( DEBUG2, "Decreasing OperCount on %s due to signoff", u->uplink->name );
		DelNetworkOper();
		DelServerOper( u );
	}
	DelServerUser( u );
	DelNetworkUser();
	DelTLDUser( u );
}

/** @brief ss_event_mode
 *
 *  MODE event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_mode( const CmdParams *cmdparams )
{
	int add = 1;
	int operadded = 0;
	serverstat *ss;
	char *modes = cmdparams->param;

	SET_SEGV_LOCATION();
	ss = GetServerModValue( cmdparams->source->uplink );
	if( !ss )
	{
		nlog( LOG_WARNING, "Unable to find stats for %s", cmdparams->source->uplink->name );
		return NS_SUCCESS;
	}
	while( *modes != '\0' )
	{
		switch( *modes )
		{
			case '+':
				add = 1;
				break;
			case '-':
				add = 0;
				break;
			case 'O':
			case 'o':
				if( add != 0 )
				{
					/* only inc opercount if we have not already added during this parse */
					if( operadded == 0 )
					{
						dlog( DEBUG1, "Increasing OperCount for %s", ss->name );
						AddNetworkOper();
						AddServerOper( cmdparams->source );
						/* Flag oper added */
						operadded = 1;
					}
				}
				else
				{
					if( IsOper( cmdparams->source ) )
					{
						dlog( DEBUG1, "Decreasing OperCount for %s", ss->name );
						DelNetworkOper();
						DelServerOper( cmdparams->source );
						/* Clear oper added to support +o-o+o correctly */
						operadded = 0;
					}
				}
				break;
			default:
				break;
		}
		modes++;
	}
	return NS_SUCCESS;
}

/** @brief ss_event_globalkill
 *
 *  GLOBALKILL event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_globalkill( const CmdParams *cmdparams )
{
	serverstat *ss;

	ss = GetServerModValue( cmdparams->target->uplink );
	IncStatistic( &ss->operkills );
	return NS_SUCCESS;
}

/** @brief ss_event_serverkill
 *
 *  SERVERKILL event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_serverkill( const CmdParams *cmdparams )
{
	serverstat *ss;

	ss = GetServerModValue( cmdparams->target->uplink );
	IncStatistic( &ss->serverkills );
	return NS_SUCCESS;
}

/** @brief ss_event_quit
 *
 *  QUIT event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_quit( const CmdParams *cmdparams )
{
	DelUser( cmdparams->source );
	return NS_SUCCESS;
}

/** @brief ss_event_signon
 *
 *  SIGNON event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_signon( const CmdParams *cmdparams )
{
	AddUser( cmdparams->source, NULL );
	return NS_SUCCESS;
}

/** @brief operlist
 *
 *  OPERLIST helper function
 *  Process client information and report to requesting user
 *
 *  @param u pointer to client to report on
 *  @param v pointer to client to report to
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int operlistaway = 0;
static char *operlistserver;

static int operlist( Client *u, void * v )
{
	Client *listu;

	listu = ( Client * )v;
	if( !IsOper( u ) )
		return NS_FALSE;
	if( operlistaway && IsAway( u ) )
		return NS_FALSE;
	if( !operlistserver )
	{
		irc_prefmsg( statbot, listu, "%-15s %-15s %-10d",
			u->name, u->uplink->name, UserLevel( u ) );
	}
	else
	{
		if( ircstrcasecmp( operlistserver, u->uplink->name ) )
			return NS_FALSE;
		irc_prefmsg( statbot, listu, "%-15s %-15s %-10d", 
			u->name, u->uplink->name, UserLevel( u ) );
	}
	return NS_FALSE;
}

/** @brief ss_cmd_botlist
 *
 *  OPERLIST command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_operlist( CmdParams *cmdparams )
{
	char *flags = NULL;

	SET_SEGV_LOCATION();
	operlistaway = 0;
	operlistserver = NULL;
	if( cmdparams->ac == 0 )
	{
		irc_prefmsg( statbot, cmdparams->source, "Online IRCops:" );
		irc_prefmsg( statbot, cmdparams->source, "ID  %-15s %-15s %-10s", 
			"Nick", "Server", "Level" );
	}
	if( cmdparams->ac != 0 )
	{
		flags = cmdparams->av[0];
		operlistserver = cmdparams->av[1];
	}
	if( flags && ircstrcasecmp( flags, "NOAWAY" ) == 0 )
	{
		operlistaway = 1;
		flags = NULL;
		irc_prefmsg( statbot, cmdparams->source, "Online IRCops( not away ):" );
	}
	if( !operlistaway && flags && strchr( flags, '.' ) )
	{
		operlistserver = flags;
		irc_prefmsg( statbot, cmdparams->source, "Online IRCops on server %s", operlistserver );
	}
	ProcessUserList( operlist, ( void * )cmdparams->source );
	irc_prefmsg( statbot, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief botlist
 *
 *  BOTLIST helper function
 *  Process client information and report to requesting user
 *
 *  @param u pointer to client to report on
 *  @param v pointer to client to report to
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int botlist( Client *u, void * v )
{
	Client *listu;

	listu = ( Client * )v;
	if IsBot( u )
		irc_prefmsg( statbot, listu, "%-15s %s", u->name, u->uplink->name );
	return NS_FALSE;
}

/** @brief ss_cmd_botlist
 *
 *  BOTLIST command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_botlist( CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	irc_prefmsg( statbot, cmdparams->source, "Online bots:" );
	ProcessUserList( botlist, ( void * )cmdparams->source );
	irc_prefmsg( statbot, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief InitUserStats
 *
 *  Init user stats
 *  Requests current user list from core and adds to StatServ
 *
 *  @param none
 *
 *  @return NS_SUCCESS on success, NS_FAILURE on failure
 */

void InitUserStats( void )
{
	ProcessUserList( AddUser, NULL );
}
