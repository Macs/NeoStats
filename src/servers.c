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
** $Id: servers.c 3294 2008-02-24 02:45:41Z Fish $
*/

/*  TODO:
 *  - 
 */

#include "neostats.h"
#include "protocol.h"
#include "exclude.h"
#include "nsevents.h"
#include "servers.h"
#include "services.h"
#include "users.h"
#include "main.h"
#include "namedvars.h"
#include "namedvars-core.h"

#define SERVER_TABLE_SIZE	HASHCOUNT_T_MAX

static hash_t *serverhash;

nv_struct nv_server[] = {
	{ "name", NV_STR, offsetof(Client, name), NV_FLG_RO, -1, MAXNICK},
	{ "name64", NV_STR, offsetof(Client, name64), NV_FLG_RO, -1, B64SIZE},
	{ "uplinkname", NV_STR, offsetof(Client, uplinkname), NV_FLG_RO, -1, MAXHOST},
	{ "info", NV_STR, offsetof(Client, info), NV_FLG_RO, -1, MAXREALNAME},
	{ "version", NV_STR, offsetof(Client, version), NV_FLG_RO, -1, MAXHOST},
	{ "flags", NV_INT, offsetof(Client, flags), NV_FLG_RO, -1, -1},
	{ "hostip", NV_STR, offsetof(Client, hostip), NV_FLG_RO, -1, HOSTIPLEN},
	{ "users", NV_INT, offsetof(Server, users), NV_FLG_RO, offsetof(Client, server), -1},
	{ "awaycount", NV_INT, offsetof(Server, awaycount), NV_FLG_RO, offsetof(Client, server), -1},
	{ "hops", NV_INT, offsetof(Server, hops), NV_FLG_RO, offsetof(Client, server), -1},
	{ "numeric", NV_INT, offsetof(Server, numeric), NV_FLG_RO, offsetof(Client, server), -1},
	{ "ping", NV_INT, offsetof(Server, ping), NV_FLG_RO, offsetof(Client, server), -1},
	{ "uptime", NV_INT, offsetof(Server, uptime), NV_FLG_RO, offsetof(Client, server), -1},
	NV_STRUCT_END()
};


/** @brief new_server
 *
 *  Create a new server Client struct
 *  NeoStats core use only.
 *
 *  @param name of server to create
 *
 *  @return pointer to Client or NULL if fails
 */

static Client *new_server( const char *name )
{
	Client *s;

	SET_SEGV_LOCATION();
	if( hash_isfull( serverhash ) )
	{
		nlog( LOG_CRITICAL, "new_ban: server hash is full" );
		return NULL;
	}
	dlog( DEBUG2, "new_server: %s", name );
	s = ns_calloc( sizeof( Client ) );
	strlcpy( s->name, name, MAXHOST );
	s->server = ns_calloc( sizeof( Server ) );
	hnode_create_insert( serverhash, s, s->name );
	me.servercount++;
	return s;
}

/** @brief AddServer
 *
 *  Add a server to NeoStats
 *  NeoStats core use only.
 *
 *  @param name
 *  @param uplink
 *  @param hops
 *  @param numeric
 *  @param infoline
 *
 *  @return pointer to Client or NULL if fails
 */

Client *AddServer( const char *name, const char *uplink, const char *hops, const char *numeric, const char *infoline )
{
	CmdParams *cmdparams;
	Client *s;

	dlog( DEBUG1, "AddServer: %s", name );
	s = new_server( name );
	if( hops )
		s->server->hops = atoi( hops );
	if( uplink )
	{
		strlcpy( s->uplinkname, uplink, MAXHOST );
		s->uplink = FindServer( uplink );
	} 
	if( infoline )
	{
		strlcpy( s->info, infoline, MAXREALNAME );
	}
	if( numeric )
		s->server->numeric =  atoi( numeric );
	s->tsconnect = me.now;
	s->server->ping = 0;
	if( ircstrcasecmp( name, me.name ) == 0 )
		s->flags |= CLIENT_FLAG_ME;
	/* check exclusions */
	ns_do_exclude_server( s );
	/* if protocol supports EOB */
	if ((ircd_srv.protocol&PROTOCOL_EOB) == 1) 
		SetSynching( s );
		
	/* run the module event for a new server. */
	cmdparams = ( CmdParams * ) ns_calloc( sizeof( CmdParams ) );
	cmdparams->source = s;
	SendAllModuleEvent( EVENT_SERVER, cmdparams );
	ns_free( cmdparams );
	return( s );
}

/** @brief del_server_leaves
 *
 *  Remove a all leaves of this server from NeoStats for use with NOQUIT
 *  NeoStats core use only.
 *
 *  @param u pointer to hub Client to remove
 *
 *  @return none
 */

static void del_server_leaves( Client * hub )
{
	Client *s;
	hscan_t scan;
	hnode_t *node;

	dlog( DEBUG1, "del_server_leaves: %s", hub->name );
	hash_scan_begin( &scan, serverhash );
	while( ( node = hash_scan_next( &scan ) ) != NULL )
	{
		s = hnode_get( node );
		if( ircstrcasecmp( hub->name, s->uplinkname ) == 0 )
		{
			dlog( DEBUG1, "del_server_leaves: server %s had uplink %s", s->name, hub->name );
			DelServer( s->name, hub->name );
		}
	}
}

/** @brief DelServer
 *
 *  Remove a server from NeoStats
 *  NeoStats core use only.
 *
 *  @param name of server to remove
 *  @param reason
 *
 *  @return none
 */

void DelServer( const char *name, const char *reason )
{
	CmdParams *cmdparams;
	Client *s;
	hnode_t *node;

	dlog( DEBUG1, "DelServer: %s", name );
	node = hash_lookup( serverhash, name );
	if( !node )
	{
		nlog( LOG_WARNING, "DelServer: squit from unknown server %s", name );
		return;
	}
	s = hnode_get( node );
	if( ircd_srv.protocol & PROTOCOL_NOQUIT )
	{
		del_server_leaves( s );
		QuitServerUsers( s );
	}
	me.servercount--;
	/* run the event for delete server */
	cmdparams = ( CmdParams * ) ns_calloc( sizeof( CmdParams ) );
	cmdparams->source = s;
	if( reason )
		cmdparams->param = ( char *)reason;
	SendAllModuleEvent( EVENT_SQUIT, cmdparams );
	ns_free( cmdparams );
	hash_scan_delete_destroy_node( serverhash, node );
	ns_free( s->server );
	ns_free( s );
}

/** @brief find_server_base64
 *
 *  Find server based on base 64 representation
 *  NeoStats core use only.
 *
 *  @param num numeric to find
 *
 *  @return pointer to Client or NULL if fails
 */

Client *find_server_base64( const char *num )
{
	Client *s;
	hscan_t scan;
	hnode_t *node;

	hash_scan_begin( &scan, serverhash );
	while( ( node = hash_scan_next( &scan ) ) != NULL )
	{
		s = hnode_get( node );
		if( strncmp( s->name64, num, BASE64SERVERSIZE ) == 0 )
		{
			dlog( DEBUG1, "find_server_base64: %s -> %s", num, s->name );
			return s;
		}
	}
	dlog( DEBUG3, "find_server_base64: %s not found!", num );
	return NULL;
}

/** @brief FindServer
 *
 *  Find server based on name
 *  NeoStats core use only.
 *
 *  @param name to find
 *
 *  @return pointer to Client or NULL if fails
 */

Client *FindServer( const char *name )
{
	Client *s;

	s = (Client *)hnode_find( serverhash, name );
	if( s == NULL )
	{
		dlog( DEBUG3, "FindServer: %s not found!", name );
	}
	return s;
}

/** @brief ListServer
 *
 *  Report server information
 *  NeoStats core use only.
 *
 *  @param s pointer to server
 *  @param v not used
 *
 *  @return none
 */

static int ListServer( Client *s, void *v )
{
	CmdParams *cmdparams;
	time_t uptime;

	/* Calculate uptime as uptime from server plus uptime of NeoStats */
	uptime = s->server->uptime  + ( me.now - me.ts_boot );
	cmdparams = ( CmdParams * ) v;
	if( ircd_srv.protocol & PROTOCOL_B64SERVER )
		irc_prefmsg( ns_botptr, cmdparams->source, _( "Server: %s (%s)" ), s->name, s->name64 );
	else
		irc_prefmsg( ns_botptr, cmdparams->source, _( "Server: %s" ), s->name );
	irc_prefmsg( ns_botptr, cmdparams->source, _( "Version: %s" ), s->version );
	irc_prefmsg( ns_botptr, cmdparams->source, _( "Uptime:  %ld day%s, %02ld:%02ld:%02ld" ), ( uptime / TS_ONE_DAY ), ( uptime / TS_ONE_DAY == 1 ) ? "" : "s", ( ( uptime / TS_ONE_HOUR ) % 24 ), ( ( uptime / TS_ONE_MINUTE ) % TS_ONE_MINUTE ), ( uptime % 60 ) );
	irc_prefmsg( ns_botptr, cmdparams->source, _( "Flags:   %x" ), s->flags );
	irc_prefmsg( ns_botptr, cmdparams->source, _( "Uplink:  %s" ), s->uplink ? s->uplink->name : "" );
	irc_prefmsg( ns_botptr, cmdparams->source, "========================================" );
	return NS_FALSE;
}

/** @brief ns_cmd_serverlist
 *
 *  SERVERLIST command handler
 *  Dump server list
 *   
 *  @param cmdparams structure with command information
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ns_cmd_serverlist( const CmdParams *cmdparams )
{
	Client *s;

	SET_SEGV_LOCATION();
	irc_prefmsg( ns_botptr, cmdparams->source, _( "===============SERVERLIST===============" ) );
	if( cmdparams->ac < 1 )
	{
		ProcessServerList( ListServer, ( void * )cmdparams );
   		return NS_SUCCESS;
	}
	s = FindServer( cmdparams->av[0] );
	if( s )
		ListServer( s, ( void * )cmdparams );
	else
		irc_prefmsg( ns_botptr, cmdparams->source, _( "can't find server %s" ), cmdparams->av[0] );
   	return NS_SUCCESS;
}

/** @brief InitServers
 *
 *  Init server subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitServers( void )
{
	serverhash = nv_hash_create( SERVER_TABLE_SIZE, 0, 0, "Servers", nv_server, NV_FLAGS_RO, NULL);
	if( !serverhash )
	{
		nlog( LOG_CRITICAL, "Unable to create server hash" );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}


/** @brief PingServer
 *
 *  Ping server
 *  NeoStats core use only.
 *
 *  @param s pointer to server
 *  @param v not used
 *
 *  @return none
 */

static int PingServer( Client *s, void *v )
{
	if( !IsMe( s ) )
	{
		irc_ping( me.name, me.name, s->name );
	}
	return NS_FALSE;
}

/** @brief PingServers
 *
 *  Ping each server
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */

int PingServers(void *arg)
{
	if( !IsNeoStatsSynched() )
		return NS_SUCCESS;
	dlog( DEBUG3, "Sending pings..." );
	me.ulag = 0;
	me.tslastping = me.now;	
	ProcessServerList( PingServer, NULL );
	return NS_SUCCESS;
}

/** @brief SetServersTime
 *
 *  use SVSTIME on the servers
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */

int SetServersTime( void *arg)
{
	if( !IsNeoStatsSynched() )
		return NS_SUCCESS;
	irc_svstime(ns_botptr, NULL, me.now);
	return NS_SUCCESS;
}

/** @brief FiniServers
 *
 *  Fini server subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */

void FiniServers( void )
{
	Client *s;
	hnode_t *node;
	hscan_t hs;

	hash_scan_begin( &hs, serverhash );
	while( ( node = hash_scan_next( &hs ) ) != NULL )
	{
		s = hnode_get( node );
		hash_scan_delete_destroy_node( serverhash, node );
		ns_free( s->server );
		ns_free( s );
	}
	hash_destroy( serverhash );
}

/** @brief ProcessServerList
 *
 *  Walk server list and call handler for each server
 *  NeoStats core use only.
 *
 *  @param handler to call
 *  @param v optional pointer
 *
 *  @return NS_SUCCESS
 */

int ProcessServerList( const ServerListHandler handler, void *v )
{
	hnode_t *node;
	hscan_t scan;
	Client *s;
	int ret = 0;

	SET_SEGV_LOCATION();
	hash_scan_begin( &scan, serverhash );
	while( ( node = hash_scan_next( &scan ) ) != NULL )
	{
		s = hnode_get( node );
		ret = handler( s, v );
		if( ret != 0 )
			break;
	}
	return ret;
}

/** @brief TransverseMap
 *
 *  Recursively transverse map and send results to handler
 *
 *  @param handler to call with map entry
 *  @param useexclusions whether to observe global exclusions
 *  @param uplink current point in map
 *  @param depth in map
 *  @param v optional handler parameter
 *
 *  @return none
 */

static void TransverseMap( const ServerMapHandler handler, int useexclusions, const char *uplink, int depth, void *v )
{
	hscan_t hs;
	hnode_t *node;
	Client *s;

	hash_scan_begin( &hs, serverhash );
	while( ( node = hash_scan_next( &hs ) ) != NULL )
	{
		s = hnode_get( node );
		/*printf( "%d %s %s (%s)\n", depth, s->name, s->uplink ? s->uplink->name : "", uplink );*/
		if( ( depth == 0 ) && ( s->uplinkname[0] == 0 ) )
		{
			/* its the root server */
			if( !useexclusions || !IsExcluded( s ) )
				handler( s, 1, depth, v );
			TransverseMap( handler, useexclusions, s->name, depth + 1, v );
		}
		else if( ( depth > 0 ) && ( s->uplink ) &&  ircstrcasecmp( s->uplink->name, uplink ) == 0 )
		{
			/* its not the root server */
			if( !useexclusions || !IsExcluded( s ) )
				handler( s, 0, depth, v );
			TransverseMap( handler, useexclusions, s->name, depth + 1, v );
		}
	}
}

/** @brief ProcessServerMap
 *
 *  Process server map
 *  Recursively transverse map and send results to handler
 *
 *  @param handler to call with map entry
 *  @param useexclusions whether to observe global exclusions
 *  @param v optional handler parameter
 *
 *  @return none
 */

void ProcessServerMap( const ServerMapHandler handler, int useexclusions, void *v )
{
	TransverseMap( handler, useexclusions, "", 0, v );
}

/** @brief RequestServerUptime
 *
 *  Request uptime of a server
 *
 *  @param s pointer to server
 *  @param v not used
 *
 *  @return none
 */

static int RequestServerUptime( Client *s, void *v )
{
	if( !IsMe( s ) )
		irc_stats( ns_botptr->u->name, 'u', s->name );
	return NS_FALSE;
}

/** @brief RequestServerUptimes
 *
 *  Request uptime of all servers
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */

void RequestServerUptimes( void )
{
	ProcessServerList( RequestServerUptime, NULL );
}

/** @brief AllocServerModPtr
 *
 *  Allocate memory for a module pointer for a server
 *  NeoStats core use only.
 *
 *  @param u pointer to client to add pointer for
 *  @param size to allocate
 *
 *  @return pointer to allocated memory
 */

void *AllocServerModPtr( Client *s, size_t size )
{
	void *ptr;

	ptr = ns_calloc( size );
	s->modptr[GET_CUR_MODULE_INDEX()] = ptr;
	GET_CUR_MODULE()->serverdatacnt++;
	return ptr;
}

/** @brief FreeServerModPtr
 *
 *  Free memory for a module pointer for a server
 *  NeoStats core use only.
 *
 *  @param u pointer to client to free pointer for
 *
 *  @return none
 */

void FreeServerModPtr( Client *s )
{
	if( s )
	{
		ns_free( s->modptr[GET_CUR_MODULE_INDEX()] );
		GET_CUR_MODULE()->serverdatacnt--;
	}
}

/** @brief GetServerModPtr
 *
 *  Retrieve module pointer for a server
 *  NeoStats core use only.
 *
 *  @param u pointer to client to lookup pointer for
 *
 *  @return none
 */

void *GetServerModPtr( const Client *s )
{
	if( s )
		return s->modptr[GET_CUR_MODULE_INDEX()];
	return NULL;	
}

/** @brief ClearServerModValue
 *
 *  Clear module value for a server
 *  NeoStats core use only.
 *
 *  @param u pointer to client to clear
 *
 *  @return none
 */

void ClearServerModValue( Client *s )
{
	if( s )
	{
		s->modvalue[GET_CUR_MODULE_INDEX()] = NULL;
		GET_CUR_MODULE()->serverdatacnt--;
	}
}

/** @brief SetServerModValue 
 *
 *  Set module value for a server
 *  NeoStats core use only.
 *
 *  @param u pointer to client to set
 *  @param data pointer to set
 *
 *  @return none
 */

void SetServerModValue( Client *s, void *data )
{
	if( s )
	{
		s->modvalue[GET_CUR_MODULE_INDEX()] = data;
		GET_CUR_MODULE()->serverdatacnt++;
	}
}

/** @brief GetServerModValue 
 *
 *  Retrieve module value for a server
 *  NeoStats core use only.
 *
 *  @param u pointer to client to lookup pointer for
 *
 *  @return none
 */

void *GetServerModValue( const Client *s )
{
	if( s )
		return s->modvalue[GET_CUR_MODULE_INDEX()];
	return NULL;	
}

/** @brief CleanupServerModdataHandler
 *
 *  Cleanup server moddata
 *
 *  @param s pointer to server
 *  @param v not used
 *
 *  @return none
 */

static int CleanupServerModdataHandler( Client *s, void *v )
{
	if( s->modptr[GET_CUR_MODULE_INDEX()] )
		ns_free( s->modptr[GET_CUR_MODULE_INDEX()] );		
	s->modvalue[GET_CUR_MODULE_INDEX()] = NULL;
	return NS_FALSE;
}

/** @brief CleanupServerModdata
 *
 *  Clear module data values and pointer left set by an unloaded module
 *  NeoStats core use only.
 *
 *  @param index of module to clear
 *
 *  @return none
 */

void CleanupServerModdata( void )
{
	SET_SEGV_LOCATION();
	if( GET_CUR_MODULE()->serverdatacnt > 0 )
	{
		nlog( LOG_WARNING, "Cleaning up servers after dirty module!" );
		ProcessServerList( CleanupServerModdataHandler, NULL );
	}
	GET_CUR_MODULE()->serverdatacnt = 0;
}

/** @brief sync_server_leafs
 *
 *  Mark all the servers as Synced 
 *  NeoStats core use only.
 *
 *  @param u pointer to server to sync
 *
 *  @return none
 */

static void sync_server_leaf( Client * Server )
{
	Client *s;
	hscan_t scan;
	hnode_t *node;

	dlog( DEBUG1, "sync_server_leaf: %s", Server->name );
	hash_scan_begin( &scan, serverhash );
	while( ( node = hash_scan_next( &scan ) ) != NULL )
	{
		s = hnode_get( node );
		if( ircstrcasecmp( Server->name, s->uplinkname ) == 0 )
		{
			dlog( DEBUG1, "sync_server_leaf: server %s (%s) is Synced", s->name, s->uplinkname);
			SetServerSynched(s);
			SyncServerClients(s);
			sync_server_leaf(s);
		}
	}
}

/** @brief SycnServer
 *
 *  Sync's a server and its leafs
 *  NeoStats core use only.
 *
 *  @param name of server to remove
 *  @param reason
 *
 *  @return none
 */

void SyncServer( const char *name)
{
	Client *s;

	dlog( DEBUG1, "SyncServer: %s", name );
	s = FindServer(name);
	if (s) {
		SetServerSynched(s);
		SyncServerClients(s);
		sync_server_leaf(s);
	}
}
