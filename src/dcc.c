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
** $Id: dcc.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "nsevents.h"
#include "commands.h"

/** DCC subsystem
 *
 *  Handle incoming and outgoing CTCP DCC messages
 */

/* DCC command handler type */
typedef int( *dcc_cmd_handler )( CmdParams *cmdparams );

/* DCC command lookup table */
typedef struct dcc_cmd
{
	const char *cmd;
	dcc_cmd_handler req_handler;
} dcc_cmd;

/* DCC list pointer */
static list_t *dcclist;

/* DCC command type prototypes */
static int dcc_req_send( CmdParams *cmdparams );
static int dcc_req_chat( CmdParams *cmdparams );

/* DCC command lookup table */
static dcc_cmd dcc_cmds[]= 
{
	{"SEND", dcc_req_send},
	{"CHAT", dcc_req_chat},
	{NULL},
};

/** @brief AddDCCClient
 *
 *  Add DCC client
 *  DCC subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return pointer to client structure
 */

static Client *AddDCCClient( const CmdParams *cmdparams )
{
	Client *dcc;

	dcc = ns_calloc( sizeof( Client ) );
	if( dcc ) 
	{
		os_memcpy( dcc, cmdparams->source, sizeof( Client ) );
		lnode_create_append( dcclist, dcc );
		dcc->flags = CLIENT_FLAG_DCC;
		return dcc;
	}
	return NULL;
}

/** @brief DelDCCClient
 *
 *  Remove DCC client
 *  DCC subsystem use only.
 *
 *  @param pointer to client structure
 *
 *  @return none
 */

static void DelDCCClient( Client *dcc )
{
	lnode_t *dccnode;

	dccnode = lnode_find( dcclist, dcc->name, comparef );
	if( dccnode )
	{
		lnode_destroy( list_delete( dcclist, dccnode ) );
		ns_free( dcc );
	}
}

/** @brief dcc_write
 *
 *  Send message to a DCC client
 *  DCC subsystem use only.
 *
 *  @param dcc client to send to
 *  @param buf to send
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int dcc_write( Client *dcc, const char *buf )
{
	static char dcc_buf[BUFSIZE];

	dlog( DEBUG1, "DCCTX: %s", buf );
	strlcpy( dcc_buf, buf, BUFSIZE );
	strlcat( dcc_buf, "\n", BUFSIZE );
	if( send_to_sock( dcc->sock, dcc_buf, strnlen( dcc_buf, BUFSIZE ) ) == NS_FAILURE )
	{
		nlog( LOG_WARNING, "Got a write error when attempting to write %d", errno );
		DelDCCClient( dcc );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief dcc_parse
 *
 *  dcc_parse
 *  DCC subsystem use only.
 *
 *  @param arg Justin????
 *  @param rline Justin????
 *  @param len Justin????
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int dcc_parse( void *arg, void *rline, int len )
{
	static char buf[BUFSIZE];
	char *cmd;
	char *line = ( char * )rline;
	Client *dcc = ( Client * )arg;
	CmdParams *cmdparams;

	strlcpy( buf, line, BUFSIZE );
	dlog( DEBUG1, "DCCRX: %s", line );
	if( buf[0] == '.' )
	{
		cmd = strchr( buf, ' ' );
		if( !cmd ) {
	         dcc_write( dcc, "Error, You must specify a command to execute" );
	         return NS_SUCCESS;
		}
   		*cmd = 0;
   		cmd++;
		cmdparams = ( CmdParams *) ns_calloc( sizeof( CmdParams ) );
		cmdparams->source = dcc;
		if( cmdparams->source ) {
			cmdparams->target = FindUser( buf + 1 );
			if( cmdparams->target ) {
				cmdparams->bot = cmdparams->target->user->bot;
			} else {
				dcc_write( dcc, "Use .<botname> to send a command to a NeoStats bot" );
				return NS_SUCCESS;
			}
			if( cmdparams->bot->flags & BOT_FLAG_ROOT ) 
			{
				cmdparams->param = cmd;
				run_bot_cmd( cmdparams, 0 );
				return NS_SUCCESS;
			} 
		}
		ns_free( cmdparams );
		return NS_SUCCESS;
	}
	cmdparams = ( CmdParams *) ns_calloc( sizeof( CmdParams ) );
	cmdparams->source = dcc;
	cmdparams->param = buf;
	if( cmdparams->source )
		SendAllModuleEvent( EVENT_DCCCHATMSG, cmdparams );
	ns_free( cmdparams );
	return NS_SUCCESS;
}

/** @brief dcc_error
 *
 *  Justin???
 *  DCC subsystem use only.
 *
 *  @param sock_no Justin???
 *  @param name Justin???
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int dcc_error( int sock_no, void *name )
{
	Sock *sock = ( Sock * )name;
	if( sock->data )
		DelDCCClient( sock->data );
	else
		nlog( LOG_WARNING, "Problem, Sock->data is NULL, therefore we can't delete DCCClient!" );
	return NS_SUCCESS;
}

/** @brief DCCChatConnect
 *
 *  Initiate DCC connect
 *  DCC subsystem use only.
 *
 *  @param dcc pointer to client structure
 *  @param port to connect
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int DCCChatConnect( Client *dcc, int port ) 
{
	static char tmpname[BUFSIZE];
	OS_SOCKET socketfd;

	if( ( socketfd = sock_connect( SOCK_STREAM, dcc->ip, port ) ) == -1 )
	{
		nlog( LOG_WARNING, "Error Connecting to DCC Host %s (%s:%d)", dcc->user->hostname, inet_ntoa( dcc->ip ), port );
		DelDCCClient( dcc );
		return NS_FAILURE;
	}			
	/* ok, now add it as a linebuffered protocol */
	ircsnprintf( tmpname, BUFSIZE, "DCC-%s", dcc->name );
	if( ( dcc->sock = add_linemode_socket( tmpname, socketfd, dcc_parse, dcc_error, ( void * )dcc ) ) == NULL )
	{
		nlog( LOG_WARNING, "Can't add a Linemode Socket for DCC %s", dcc->user->hostname );
		os_sock_close( socketfd );
		DelDCCClient( dcc );
		return NS_FAILURE;
	}

	dcc->fd = socketfd;
	dcc->port = port;
	return NS_SUCCESS;
}

/** @brief DCCChatDisconnect
 *
 *  End DCC connect
 *  DCC subsystem use only.
 *
 *  @param dcc pointer to client structure
 *
 *  @return none
 */

static void DCCChatDisconnect( const Client *dcc )
{
	DelSock( dcc->sock );
}

/** @brief DCCGotAddr
 *
 *  DCCGotAddr
 *  DCC subsystem use only.
 *
 *  @param data
 *  @param a
 *
 *  @return none
 */

static void DCCGotAddr( void *data, adns_answer *a )
{
	Client *u = ( Client * )data;

	if( a && a->nrrs > 0 && u && a->status == adns_s_ok )
	{
		u->ip.s_addr = a->rrs.addr->addr.inet.sin_addr.s_addr;
		if( u->ip.s_addr > 0 )
		{
			if( DCCChatConnect( u, u->port ) == NS_SUCCESS )
				return;
		}
	}
	/* if we get here, there was something wrong */
	else
		nlog( LOG_WARNING, "DCC: Unable to connect to %s.%d: Unknown hostname", u->user->hostname, u->port );
	DelDCCClient( u );
	return;
}

/** @brief DCCChatStart
 *
 *  DCCChatStart
 *  DCC subsystem use only.
 *
 *  @param dcc
 *  @param port
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int DCCChatStart( Client *dcc, int port )
{
	dcc->port = port;
	if( dcc->ip.s_addr > 0 )
	{
		/* we have a valid IP address for this user, so just go and kick off the connection straight away */
		return DCCChatConnect( dcc, port );
	}
	else
	{
		/* we don't have a valid IP address, kick off a DNS lookup */
		dns_lookup( dcc->user->hostname, adns_r_addr, DCCGotAddr, ( void * )dcc );
	}
	return NS_SUCCESS;
}

/** @brief irc_dccmsgall
 *
 *  Send message to all DCC clients
 *  DCC subsystem use only.
 *
 *  @param fmt format of message to send
 *  @param ... parameters to format string
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_dccmsgall( const char *fmt, ...)
{
	va_list ap;
	static char buf[BUFSIZE];
	Client *todcc;
	lnode_t *dccnode;

	va_start( ap, fmt );
	ircvsnprintf( buf, BUFSIZE, fmt, ap );
	va_end( ap );

	dccnode = list_first( dcclist );
	while( dccnode != NULL )
	{
		todcc = ( Client * )lnode_get( dccnode );
		dcc_write( todcc, buf );
		dccnode = list_next( dcclist, dccnode );
	}
	return NS_SUCCESS;
}

/** @brief dcc_send_msg
 *
 *  Send message to a DCC client
 *  DCC subsystem use only.
 *
 *  @param dcc client to send to
 *  @param buf to send
 *
 *  @return none
 */

void dcc_send_msg( const Client* dcc, char * buf )
{
	dcc_write( ( Client * )dcc, buf );
}

/** @brief dcc_req
 *
 *  Handle DCC requests
 *  DCC subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int dcc_req( CmdParams *cmdparams )
{
	dcc_cmd* cmd;
	size_t len;
    
	cmd = dcc_cmds;
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
	return NS_SUCCESS;
}

/** @brief dcc_req_send
 *
 *  Send DCC CHAT request
 *  DCC subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int dcc_req_send( CmdParams *cmdparams )
{
	dlog( DEBUG5, "DCC SEND request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	SendModuleEvent( EVENT_DCCSEND, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

/** @brief dcc_req_chat
 *
 *  Handle DCC CHAT request
 *  RX: 
 *    :Mark ! neostats :\1DCC CHAT chat 2130706433 1028\1
 *  DCC subsystem use only.
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int dcc_req_chat( CmdParams *cmdparams )
{
	int userlevel;
	Client *dcc;
	char **av;
	int ac;

	dlog( DEBUG5, "DCC CHAT request from %s to %s", cmdparams->source->name, cmdparams->bot->name );
	userlevel = UserLevel( cmdparams->source ); 
	if( userlevel < NS_ULEVEL_ROOT )
	{
		dlog( DEBUG5, "Dropping DCC CHAT request from unauthorised user %s", cmdparams->source->name );
		return NS_FAILURE;
	}
	ac = split_buf( cmdparams->param, &av );
	if( ac == 3 )
	{
		dcc = AddDCCClient( cmdparams );
		if( !dcc )
		{
			dlog( DEBUG5, "DCC CHAT unable to add user %s", cmdparams->source->name );
			return NS_FAILURE;
		}
		if( DCCChatStart( dcc, atoi( av[2] ) ) != NS_SUCCESS ) 
		{
			DelDCCClient( dcc );
		}
	}
	ns_free( av );
	SendModuleEvent( EVENT_DCCCHAT, cmdparams, cmdparams->bot->moduleptr );
	return NS_SUCCESS;
}

/** @brief InitDCC
 *
 *  Init DCC subsystem
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitDCC( void )
{
	dcclist = list_create( LISTCOUNT_T_MAX );
	if( !dcclist )
	{
		nlog( LOG_CRITICAL, "Unable to create DCC list" );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief FiniDCC
 *
 *  Fini DCC subsystem
 *
 *  @param none
 *
 *  @return none
 */

void FiniDCC( void )
{
	Client *dcc;
	lnode_t *dccnode;
 
	dccnode = list_first( dcclist );
	while( dccnode != NULL )
	{
		dcc = ( Client * )lnode_get( dccnode );
		DCCChatDisconnect( dcc );
		ns_free( dcc );
		dccnode = list_next( dcclist, dccnode );
	}
	list_destroy_nodes( dcclist );
	list_destroy( dcclist );
}
