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
** $Id: server.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "statserv.h"
#include "stats.h"
#include "network.h"
#include "server.h"
#include "namedvars.h"

/** Server table name */
#define SERVER_TABLE	"Server"

/** Server hash */
static hash_t *serverstathash;


nv_struct nv_ss_servers[] = {
	{"name", NV_STR, offsetof(serverstat, name), NV_FLG_RO, -1, MAXHOST},
	{"ts_start", NV_INT, offsetof(serverstat, ts_start), NV_FLG_RO, -1, -1},
	{"ts_lastseen", NV_INT, offsetof(serverstat, ts_lastseen), NV_FLG_RO, -1, -1},
	{"users-day", NV_INT, offsetof(statistic, day), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"users-week", NV_INT, offsetof(statistic, week), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"users-month", NV_INT, offsetof(statistic, month), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"users", NV_INT, offsetof(statistic, current), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"users-alltime-runningtotal", NV_INT, offsetof(statistic, alltime.runningtotal), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"users-alltime-average", NV_INT, offsetof(statistic, alltime.average), NV_FLG_RO, offsetof(serverstat,users), -1},
	{"users-alltime-max", NV_INT, offsetof(statistic, alltime.max), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"users-alltime-ts_max", NV_INT, offsetof(statistic, alltime.ts_max), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"users-daily-runningtotal", NV_INT, offsetof(statistic, daily.runningtotal), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"users-daily-average", NV_INT, offsetof(statistic, daily.average), NV_FLG_RO, offsetof(serverstat,users), -1},
	{"users-daily-max", NV_INT, offsetof(statistic, daily.max), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"users-daily-ts_max", NV_INT, offsetof(statistic, daily.ts_max), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"users-weekly-runningtotal", NV_INT, offsetof(statistic, weekly.runningtotal), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"users-weekly-average", NV_INT, offsetof(statistic, weekly.average), NV_FLG_RO, offsetof(serverstat,users), -1},
	{"users-weekly-max", NV_INT, offsetof(statistic, weekly.max), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"users-weekly-ts_max", NV_INT, offsetof(statistic, weekly.ts_max), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"users-monthly-runningtotal", NV_INT, offsetof(statistic, monthly.runningtotal), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"users-monthly-average", NV_INT, offsetof(statistic, monthly.average), NV_FLG_RO, offsetof(serverstat,users), -1},
	{"users-monthly-max", NV_INT, offsetof(statistic, monthly.max), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"users-monthly-ts_max", NV_INT, offsetof(statistic, monthly.ts_max), NV_FLG_RO, offsetof(serverstat, users), -1},
	{"opers-day", NV_INT, offsetof(statistic, day), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"opers-week", NV_INT, offsetof(statistic, week), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"opers-month", NV_INT, offsetof(statistic, month), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"opers", NV_INT, offsetof(statistic, current), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"opers-alltime-runningtotal", NV_INT, offsetof(statistic, alltime.runningtotal), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"opers-alltime-average", NV_INT, offsetof(statistic, alltime.average), NV_FLG_RO, offsetof(serverstat,opers), -1},
	{"opers-alltime-max", NV_INT, offsetof(statistic, alltime.max), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"opers-alltime-ts_max", NV_INT, offsetof(statistic, alltime.ts_max), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"opers-daily-runningtotal", NV_INT, offsetof(statistic, daily.runningtotal), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"opers-daily-average", NV_INT, offsetof(statistic, daily.average), NV_FLG_RO, offsetof(serverstat,opers), -1},
	{"opers-daily-max", NV_INT, offsetof(statistic, daily.max), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"opers-daily-ts_max", NV_INT, offsetof(statistic, daily.ts_max), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"opers-weekly-runningtotal", NV_INT, offsetof(statistic, weekly.runningtotal), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"opers-weekly-average", NV_INT, offsetof(statistic, weekly.average), NV_FLG_RO, offsetof(serverstat,opers), -1},
	{"opers-weekly-max", NV_INT, offsetof(statistic, weekly.max), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"opers-weekly-ts_max", NV_INT, offsetof(statistic, weekly.ts_max), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"opers-monthly-runningtotal", NV_INT, offsetof(statistic, monthly.runningtotal), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"opers-monthly-average", NV_INT, offsetof(statistic, monthly.average), NV_FLG_RO, offsetof(serverstat,opers), -1},
	{"opers-monthly-max", NV_INT, offsetof(statistic, monthly.max), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"opers-monthly-ts_max", NV_INT, offsetof(statistic, monthly.ts_max), NV_FLG_RO, offsetof(serverstat, opers), -1},
	{"operkills-day", NV_INT, offsetof(statistic, day), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"operkills-week", NV_INT, offsetof(statistic, week), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"operkills-month", NV_INT, offsetof(statistic, month), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"operkills", NV_INT, offsetof(statistic, current), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"operkills-alltime-runningtotal", NV_INT, offsetof(statistic, alltime.runningtotal), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"operkills-alltime-average", NV_INT, offsetof(statistic, alltime.average), NV_FLG_RO, offsetof(serverstat,operkills), -1},
	{"operkills-alltime-max", NV_INT, offsetof(statistic, alltime.max), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"operkills-alltime-ts_max", NV_INT, offsetof(statistic, alltime.ts_max), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"operkills-daily-runningtotal", NV_INT, offsetof(statistic, daily.runningtotal), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"operkills-daily-average", NV_INT, offsetof(statistic, daily.average), NV_FLG_RO, offsetof(serverstat,operkills), -1},
	{"operkills-daily-max", NV_INT, offsetof(statistic, daily.max), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"operkills-daily-ts_max", NV_INT, offsetof(statistic, daily.ts_max), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"operkills-weekly-runningtotal", NV_INT, offsetof(statistic, weekly.runningtotal), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"operkills-weekly-average", NV_INT, offsetof(statistic, weekly.average), NV_FLG_RO, offsetof(serverstat,operkills), -1},
	{"operkills-weekly-max", NV_INT, offsetof(statistic, weekly.max), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"operkills-weekly-ts_max", NV_INT, offsetof(statistic, weekly.ts_max), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"operkills-monthly-runningtotal", NV_INT, offsetof(statistic, monthly.runningtotal), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"operkills-monthly-average", NV_INT, offsetof(statistic, monthly.average), NV_FLG_RO, offsetof(serverstat,operkills), -1},
	{"operkills-monthly-max", NV_INT, offsetof(statistic, monthly.max), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"operkills-monthly-ts_max", NV_INT, offsetof(statistic, monthly.ts_max), NV_FLG_RO, offsetof(serverstat, operkills), -1},
	{"serverkills-day", NV_INT, offsetof(statistic, day), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"serverkills-week", NV_INT, offsetof(statistic, week), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"serverkills-month", NV_INT, offsetof(statistic, month), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"serverkills", NV_INT, offsetof(statistic, current), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"serverkills-alltime-runningtotal", NV_INT, offsetof(statistic, alltime.runningtotal), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"serverkills-alltime-average", NV_INT, offsetof(statistic, alltime.average), NV_FLG_RO, offsetof(serverstat,serverkills), -1},
	{"serverkills-alltime-max", NV_INT, offsetof(statistic, alltime.max), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"serverkills-alltime-ts_max", NV_INT, offsetof(statistic, alltime.ts_max), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"serverkills-daily-runningtotal", NV_INT, offsetof(statistic, daily.runningtotal), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"serverkills-daily-average", NV_INT, offsetof(statistic, daily.average), NV_FLG_RO, offsetof(serverstat,serverkills), -1},
	{"serverkills-daily-max", NV_INT, offsetof(statistic, daily.max), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"serverkills-daily-ts_max", NV_INT, offsetof(statistic, daily.ts_max), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"serverkills-weekly-runningtotal", NV_INT, offsetof(statistic, weekly.runningtotal), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"serverkills-weekly-average", NV_INT, offsetof(statistic, weekly.average), NV_FLG_RO, offsetof(serverstat,serverkills), -1},
	{"serverkills-weekly-max", NV_INT, offsetof(statistic, weekly.max), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"serverkills-weekly-ts_max", NV_INT, offsetof(statistic, weekly.ts_max), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"serverkills-monthly-runningtotal", NV_INT, offsetof(statistic, monthly.runningtotal), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"serverkills-monthly-average", NV_INT, offsetof(statistic, monthly.average), NV_FLG_RO, offsetof(serverstat,serverkills), -1},
	{"serverkills-monthly-max", NV_INT, offsetof(statistic, monthly.max), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"serverkills-monthly-ts_max", NV_INT, offsetof(statistic, monthly.ts_max), NV_FLG_RO, offsetof(serverstat, serverkills), -1},
	{"splits-day", NV_INT, offsetof(statistic, day), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"splits-week", NV_INT, offsetof(statistic, week), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"splits-month", NV_INT, offsetof(statistic, month), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"splits", NV_INT, offsetof(statistic, current), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"splits-alltime-runningtotal", NV_INT, offsetof(statistic, alltime.runningtotal), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"splits-alltime-average", NV_INT, offsetof(statistic, alltime.average), NV_FLG_RO, offsetof(serverstat,splits), -1},
	{"splits-alltime-max", NV_INT, offsetof(statistic, alltime.max), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"splits-alltime-ts_max", NV_INT, offsetof(statistic, alltime.ts_max), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"splits-daily-runningtotal", NV_INT, offsetof(statistic, daily.runningtotal), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"splits-daily-average", NV_INT, offsetof(statistic, daily.average), NV_FLG_RO, offsetof(serverstat,splits), -1},
	{"splits-daily-max", NV_INT, offsetof(statistic, daily.max), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"splits-daily-ts_max", NV_INT, offsetof(statistic, daily.ts_max), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"splits-weekly-runningtotal", NV_INT, offsetof(statistic, weekly.runningtotal), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"splits-weekly-average", NV_INT, offsetof(statistic, weekly.average), NV_FLG_RO, offsetof(serverstat,splits), -1},
	{"splits-weekly-max", NV_INT, offsetof(statistic, weekly.max), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"splits-weekly-ts_max", NV_INT, offsetof(statistic, weekly.ts_max), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"splits-monthly-runningtotal", NV_INT, offsetof(statistic, monthly.runningtotal), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"splits-monthly-average", NV_INT, offsetof(statistic, monthly.average), NV_FLG_RO, offsetof(serverstat,splits), -1},
	{"splits-monthly-max", NV_INT, offsetof(statistic, monthly.max), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"splits-monthly-ts_max", NV_INT, offsetof(statistic, monthly.ts_max), NV_FLG_RO, offsetof(serverstat, splits), -1},
	{"lowest_ping", NV_INT, offsetof(serverstat, lowest_ping), NV_FLG_RO, -1, -1},
	{"ts_lowest_ping", NV_INT, offsetof(serverstat, ts_lowest_ping), NV_FLG_RO, -1, -1},
	{"highest_ping", NV_INT, offsetof(serverstat, highest_ping), NV_FLG_RO, -1, -1},
	{"ts_highest_ping", NV_INT, offsetof(serverstat, ts_highest_ping), NV_FLG_RO, -1, -1},
	NV_STRUCT_END()
};	

/** @brief AverageServerStatistics
 *
 *  Average server statistics
 *
 *  @param none
 *
 *  @return none
 */

void AverageServerStatistics( void )
{
	serverstat *ss;
	hscan_t scan;
	hnode_t *node;

	hash_scan_begin( &scan, serverstathash );
	while( ( node = hash_scan_next( &scan ) ) != NULL )
	{
		ss = hnode_get( node );
		AverageStatistic( &ss->users );
		AverageStatistic( &ss->opers );
	}
}

/** @brief ResetServerStatistics
 *
 *  Reset server statistics
 *
 *  @param none
 *
 *  @return none
 */

void ResetServerStatistics( void )
{
	serverstat *ss;
	hscan_t scan;
	hnode_t *node;

	hash_scan_begin( &scan, serverstathash );
	while( ( node = hash_scan_next( &scan ) ) != NULL )
	{
		ss = hnode_get( node );
		ResetStatistic( &ss->users );
		ResetStatistic( &ss->opers );
	}
}

/** @brief new_server_stat
 *
 *  Create new server stat
 *
 *  @param name of server to search for
 *
 *  @return pointer to stat created or NULL if unable
 */

static serverstat *new_server_stat( const char *name )
{
	serverstat *ss;

	SET_SEGV_LOCATION();
	dlog( DEBUG2, "new_server_stat (%s)", name );
	if( hash_isfull( serverstathash ) )
	{
		nlog( LOG_CRITICAL, "StatServ Server hash is full!" );
		return NULL;
	}
	ss = ns_calloc( sizeof( serverstat ) );
	memcpy( ss->name, name, MAXHOST );
	hnode_create_insert( serverstathash, ss, ss->name );
	return ss;
}

/** @brief findserverstats
 *
 *  Check list for server
 *
 *  @param name of server to search for
 *
 *  @return pointer to stat found or NULL if none
 */

static serverstat *findserverstats( const char *name )
{
	serverstat *stats;

	stats = ( serverstat * )hnode_find( serverstathash, name );
	if( !stats )
		dlog( DEBUG2, "findserverstats (%s) - not found", name );
	return stats;
}

/** @brief AddServerUser
 *
 *  Add user stat
 *
 *  @param u pointer to client to update
 *
 *  @return none
 */

void AddServerUser( const  Client *u )
{
	serverstat *ss;

	ss = GetServerModValue( u->uplink );
	if( IncStatistic( &ss->users ) )
	{
		announce_record( "\2NEW SERVER RECORD!\2 %d users on server %s",
			ss->s->server->users, ss->name );
	}
}

/** @brief DelServerUser
 *
 *  Delete server user
 *
 *  @param u pointer to client to update
 *
 *  @return none
 */

void DelServerUser( const Client *u )
{
	serverstat *ss;

	ss = GetServerModValue( u->uplink );
	DecStatistic( &ss->users );
}

/** @brief AddServerOper
 *
 *  Add server oper
 *
 *  @param u pointer to client to update
 *
 *  @return none
 */

void AddServerOper( const Client *u )
{
	serverstat *ss;

	ss = GetServerModValue( u->uplink );
	if( IncStatistic( &ss->opers ) )
	{
		announce_record( "\2NEW SERVER RECORD!\2 %d opers on %s",
			ss->opers.alltime.runningtotal, ss->name );
	}
}

/** @brief DelServerOper
 *
 *  Delete server oper
 *
 *  @param s pointer to client to update
 *
 *  @return none
 */

void DelServerOper( const Client *u )
{
	serverstat *ss;

	ss = GetServerModValue( u->uplink );
	DecStatistic( &ss->opers );
}

/** @brief AddServerStat
 *
 *  Add server stat
 *
 *  @param s pointer to client to update
 *  @param v not used
 *
 *  @return none
 */

static int AddServerStat( Client *s, void *v )
{
	serverstat *ss;

	dlog( DEBUG2, "AddServerStat (%s)", s->name );
	ss = findserverstats( s->name );
	if( !ss )
		ss = new_server_stat( s->name );
	ss->ts_start = ss->ts_lastseen = me.now;
	AddNetworkServer();
	SetServerModValue( s, ( void * )ss );
	ss->s = s;
	return NS_FALSE;
}

/** @brief ss_event_server
 *
 *  SERVER event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_server( const CmdParams *cmdparams )
{
	AddServerStat( cmdparams->source, NULL );
	return NS_SUCCESS;
}

/** @brief DelServerStat
 *
 *  Delete server stat
 *
 *  @param s pointer to client to update
 *
 *  @return none
 */

static void DelServerStat( Client* s )
{
	serverstat *ss;
	
	ss = ( serverstat * ) GetServerModValue( s );
	ss->ts_lastseen = me.now;
	IncStatistic( &ss->splits );
	ClearServerModValue( s );
	ss->s = NULL;
}

/** @brief ss_event_squit
 *
 *  SQUIT event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_squit( const CmdParams *cmdparams )
{
	DelServerStat( cmdparams->source );
	DelNetworkServer();
	return NS_SUCCESS;
}

/** @brief UpdatePingStats
 *
 *  Update ping stats
 *
 *  @param s pointer to client to update
 *
 *  @return none
 */

static void UpdatePingStats( const Client* s )
{
	serverstat *ss;

	ss = ( serverstat * ) GetServerModValue( s );
	if( !ss )
		return;
	if( s->server->ping > ss->highest_ping )
	{
		ss->highest_ping = s->server->ping;
		ss->ts_highest_ping = me.now;
	}
	if( s->server->ping < ss->lowest_ping )
	{
		ss->lowest_ping = s->server->ping;
		ss->ts_lowest_ping = me.now;
	}
	/* ok, updated the statistics, now lets see if this server is "lagged out" */
	if( s->server->ping > StatServ.lagtime )
		announce_lag( "\2%s\2 is lagged out with a ping of %d", s->name, s->server->ping );
}

/** @brief ss_event_pong
 *
 *  PONG event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_pong( const CmdParams *cmdparams )
{
	/* we don't want negative pings! */
	if( cmdparams->source->server->ping > 0 )
		UpdatePingStats( cmdparams->source );
	return NS_SUCCESS;
}

/** @brief MapHandler
 *
 *  MAP process callback
 *  report results of map process
 *
 *  @param s pointer to server
 *  @param isroot whether this server is root
 *  @param depth of server in map
 *  @param v user to report to
 *
 *  @return none
 */

static void MapHandler( const Client *s, int isroot, int depth, void *v )
{
#define MAPBUFSIZE 512
	static char buf[MAPBUFSIZE];
	Client *u = (Client *)v;
	serverstat *ss;

	ss = ( serverstat * ) GetServerModValue( s );
	if( isroot )
	{
		/* its the root server */
		irc_prefmsg( statbot, u, "\2%-45s      [ %d/%d ]   [ %d/%d ]   [ %d/%ld ]",
			ss->name, s->server->users, ( int )ss->users.alltime.max, ss->opers.current, ss->opers.alltime.max,
			( int )s->server->ping, ss->highest_ping );
	}
	else
	{
		/* its not the root server */
		if( StatServ.flatmap )
		{
			irc_prefmsg( statbot, u, "\2%-40s      [ %d/%d ]   [ %d/%d ]   [ %d/%ld ]", 
				ss->name, s->server->users, ( int )ss->users.alltime.max,
				ss->opers.current, ss->opers.alltime.max, ( int )s->server->ping, ss->highest_ping );
		}
		else
		{
			buf[0]='\0';
			for( ; depth > 1; depth-- )
				strlcat( buf, "     |", 256 );
			irc_prefmsg( statbot, u, "%s \\_\2%-40s      [ %d/%d ]   [ %d/%d ]   [ %d/%ld ]",
				buf, ss->name, s->server->users, ( int )ss->users.alltime.max,
				ss->opers.current, ss->opers.alltime.max, ( int )s->server->ping, ss->highest_ping );
		}
	}
}

/** @brief ss_cmd_map
 *
 *  MAP command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_map( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	irc_prefmsg( statbot, cmdparams->source, "%-40s      %-10s %-10s %-10s",
		"\2[NAME]\2", "\2[USERS/MAX]\2", "\2[OPERS/MAX]\2", "\2[LAG/MAX]\2" );
	ProcessServerMap( MapHandler, StatServ.exclusions, (void *)cmdparams->source );
	irc_prefmsg( statbot, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief ss_cmd_server_list
 *
 *  SERVER LIST command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_server_list( const CmdParams *cmdparams )
{
	serverstat *ss;
	hscan_t scan;
	hnode_t *node;

	irc_prefmsg( statbot, cmdparams->source, "Server listing:" );
	hash_scan_begin( &scan, serverstathash );
	while( ( node = hash_scan_next( &scan ) ) != NULL )
	{
		ss = hnode_get( node );
		irc_prefmsg( statbot, cmdparams->source, "%s (%s)", ss->name, 
			( ss->s ) ? "ONLINE" : "OFFLINE" );
	}
	irc_prefmsg( statbot,cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief ss_server_del
 *
 *  SERVER DEL command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ss_server_del( const CmdParams *cmdparams )
{
	serverstat *ss;
	hnode_t *node;

	if( cmdparams->ac < 2 )
		return NS_ERR_NEED_MORE_PARAMS;
	ss = findserverstats( cmdparams->av[1] );
	if( !ss )
	{
		irc_prefmsg( statbot, cmdparams->source, "%s is not in the database", cmdparams->av[1] );
		return NS_SUCCESS;
	}
	if( ss->s )
	{
		irc_prefmsg( statbot, cmdparams->source, 
			"Cannot remove %s from the database, it is online!!", cmdparams->av[1] );
		return NS_SUCCESS;
	}
	node = hash_lookup( serverstathash, cmdparams->av[1] );
	if( node )
	{
		ss = ( serverstat * )hnode_get( node );
		hash_delete_destroy_node( serverstathash, node );
		ns_free( ss );
		irc_prefmsg( statbot, cmdparams->source, "Removed %s from the database.",
			cmdparams->av[1] );
		nlog( LOG_NOTICE, "%s deleted stats for %s", cmdparams->source->name, cmdparams->av[1] );
	}
	return NS_SUCCESS;
}

/** @brief ss_server_rename
 *
 *  SERVER RENAME command handler
 *  Renames a server statistic entry
 *
 *  @param cmdparams
 *    cmdparams->av[1] = old name
 *    cmdparams->av[2] = new name
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ss_server_rename( const CmdParams *cmdparams )
{
	serverstat *dest;
	serverstat *src;

	if( UserLevel( cmdparams->source ) < NS_ULEVEL_ADMIN )
		return NS_ERR_NO_PERMISSION;
	if( cmdparams->ac < 3 )
		return NS_ERR_NEED_MORE_PARAMS;
	dest = findserverstats( cmdparams->av[2] );
	if( dest )
	{
		if( dest->s )
		{
			irc_prefmsg( statbot, cmdparams->source, "Server %s is online!", cmdparams->av[2] );
			return NS_SUCCESS;
		}
		ns_free( dest );
	}
	src = findserverstats( cmdparams->av[1] );
	if( !src )
	{
		irc_prefmsg( statbot, cmdparams->source, "%s is not in the database", 
			cmdparams->av[1] );
		return NS_SUCCESS;
	}
	strlcpy( dest->name, cmdparams->av[2], MAXHOST );
	irc_prefmsg( statbot, cmdparams->source, "Renamed database entry for %s to %s", 
		cmdparams->av[1], cmdparams->av[2] );
	nlog( LOG_NOTICE, "%s requested STATS RENAME %s to %s", cmdparams->source->name, 
		cmdparams->av[1], cmdparams->av[2] );
	return NS_SUCCESS;
}

/** @brief ss_cmd_server_stats
 *
 *  SERVER command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int ss_cmd_server_stats( const CmdParams *cmdparams )
{
	serverstat *ss;
	Client *s;
	char *server;

	if( cmdparams->ac == 0 )
		return NS_ERR_SYNTAX_ERROR;
	server = cmdparams->av[0];
	ss = findserverstats( server );
	if( !ss )
	{
		nlog( LOG_CRITICAL, "Unable to find server statistics for %s", server );
		irc_prefmsg( statbot, cmdparams->source, "Unable to find server statistics for %s", server );
		return NS_SUCCESS;
	}
	irc_prefmsg( statbot, cmdparams->source, "Statistics for \2%s\2 since %s",
		ss->name, sftime( ss->ts_start ) );
	s = ss->s;
	if( !s )
	{
		irc_prefmsg( statbot, cmdparams->source, "Server Last Seen: %s", 
			sftime( ss->ts_lastseen ) );
	}
	else
	{
		/* Calculate uptime as uptime from server plus uptime of NeoStats */
		time_t uptime;

		uptime = s->server->uptime + ( me.now - me.ts_boot );
		irc_prefmsg( statbot, cmdparams->source, "Version: %s", s->version );
		irc_prefmsg( statbot, cmdparams->source, "Uptime:  %ld day%s, %02ld:%02ld:%02ld", ( uptime / TS_ONE_DAY ), ( ( uptime / TS_ONE_DAY ) == 1 ) ? "" : "s", ( ( uptime / TS_ONE_HOUR ) % 24 ), ( ( uptime / TS_ONE_MINUTE ) % TS_ONE_MINUTE ), ( uptime % 60 ) );
		irc_prefmsg( statbot, cmdparams->source, "Current Users: %-3d (%d%%)", 
			s->server->users, 
			( int )( ( float ) s->server->users /( float ) networkstats.users.current * 100 ) );
	}
	irc_prefmsg( statbot, cmdparams->source, "Maximum users: %-3d at %s",
		ss->users.alltime.max, sftime( ss->users.alltime.ts_max ) );
	irc_prefmsg( statbot, cmdparams->source, "Total users connected: %-3d", ss->users.alltime.runningtotal );
	if( s )
		irc_prefmsg( statbot, cmdparams->source, "Current opers: %-3d", ss->opers.current );
	irc_prefmsg( statbot, cmdparams->source, "Maximum opers: %-3d at %s",
		ss->opers.alltime.max, sftime( ss->opers.alltime.ts_max ) );
	irc_prefmsg( statbot, cmdparams->source, "IRCop kills: %d", ss->operkills.alltime.runningtotal );
	irc_prefmsg( statbot, cmdparams->source, "Server kills: %d", ss->serverkills.alltime.runningtotal );
	irc_prefmsg( statbot, cmdparams->source, "Lowest ping: %-3d at %s",
		( int )ss->lowest_ping, sftime( ss->ts_lowest_ping ) );
	irc_prefmsg( statbot, cmdparams->source, "Highest ping: %-3d at %s",
		( int )ss->highest_ping, sftime( ss->ts_highest_ping ) );
	if( s )
		irc_prefmsg( statbot, cmdparams->source, "Current Ping: %-3d", ( int )s->server->ping );
	if( ss->splits.alltime.runningtotal >= 1 )
	{
		irc_prefmsg( statbot, cmdparams->source, 
			"%s has split from the network %d time%s",
			ss->name, ss->splits.alltime.runningtotal, ( ss->splits.alltime.runningtotal == 1 ) ? "" : "s" );
	}
	else
	{
		irc_prefmsg( statbot, cmdparams->source, "%s has never split from the network.", 
			ss->name );
	}
	irc_prefmsg( statbot, cmdparams->source, "***** End of Statistics *****" );
	return NS_SUCCESS;
}

/** @brief ss_cmd_server
 *
 *  SERVER command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *    cmdparams->av[0] = optionally LIST, DEL, RENAME
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_server( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	if( ircstrcasecmp( cmdparams->av[0], "LIST" ) == 0 )
		return ss_cmd_server_list( cmdparams );
	if( ircstrcasecmp( cmdparams->av[0], "DEL" ) == 0 )
		return ss_server_del( cmdparams );
	if( ircstrcasecmp( cmdparams->av[0], "RENAME" ) == 0 )
		return ss_server_rename( cmdparams );	
	return ss_cmd_server_stats( cmdparams );
}

/** @brief SaveServer
 *
 *  Save server stat
 *
 *  @param ss pointer to server stat to save
 *
 *  @return none
 */

static void SaveServer( serverstat *ss )
{
	dlog( DEBUG1, "Writing statistics to database for %s", ss->name );
	PreSaveStatistic( &ss->users );
	PreSaveStatistic( &ss->opers );
	PreSaveStatistic( &ss->operkills );
	PreSaveStatistic( &ss->serverkills );
	PreSaveStatistic( &ss->splits );
	DBAStore( SERVER_TABLE, ss->name, ( void * )ss, sizeof( serverstat ) );
}

/** @brief SaveServerStats
 *
 *  Save server stats
 *
 *  @param none
 *
 *  @return none
 */

void SaveServerStats( void )
{
	serverstat *ss;
	hnode_t *node;
	hscan_t scan;

	/* run through stats and save them */
	hash_scan_begin( &scan, serverstathash );
	while( ( node = hash_scan_next( &scan ) ) != NULL )
	{
		ss = hnode_get( node );
		SaveServer( ss );
	}
}

/** @brief LoadVersionStats
 *
 *  Table load handler
 *
 *  @param data pointer to table row data
 *  @param size of loaded data
 *
 *  @return NS_TRUE to abort load or NS_FALSE to continue loading
 */

int LoadServerStats( void *data, int size ) 
{
	serverstat *ss;

	if( size != sizeof( serverstat ) )
	{
		nlog( LOG_CRITICAL, "server data size invalid" );		
		return NS_FALSE;
	}
	ss = ns_calloc( sizeof( serverstat ) );
	os_memcpy( ss, data, sizeof( serverstat ) );
	dlog( DEBUG1, "Loaded statistics for %ss", ss->name );
	hnode_create_insert( serverstathash, ss, ss->name );
	PostLoadStatistic( &ss->users );
	PostLoadStatistic( &ss->opers );
	PostLoadStatistic( &ss->operkills );
	PostLoadStatistic( &ss->serverkills );
	PostLoadStatistic( &ss->splits );
	ss->s = NULL;
	return NS_FALSE;
}

/** @brief GetServerStats
 *
 *  Walk through list passing each server to handler
 *
 *  @param handler pointer to handler function
 *  @param v pointer to client to send to
 *
 *  @return none
 */

void GetServerStats( const ServerStatHandler handler, const void *v )
{
	serverstat *ss;
	hnode_t *node;
	hscan_t scan;

	hash_scan_begin( &scan, serverstathash );
	while( ( node = hash_scan_next( &scan ) ) != NULL )
	{
		ss = hnode_get( node );
		handler( ss, v );
	}
}

/** @brief InitServerStats
 *
 *  Init server stats
 *
 *  @param none
 *
 *  @return NS_SUCCESS on success, NS_FAILURE on failure
 */

int InitServerStats( void )
{
	serverstathash = nv_hash_create( HASHCOUNT_T_MAX, 0, 0, "StatServ-Servers", nv_ss_servers, NV_FLAGS_RO, NULL);
	if( !serverstathash )
	{
		nlog( LOG_CRITICAL, "Unable to create server hash list" );
		return NS_FAILURE;
	}
	DBAOpenTable( SERVER_TABLE );
	DBAFetchRows( SERVER_TABLE, LoadServerStats );
	ProcessServerList( AddServerStat, NULL );
	return NS_SUCCESS;
}

/** @brief FiniServerStats
 *
 *  Fini server stats
 *
 *  @param none
 *
 *  @return none
 */

void FiniServerStats( void )
{
	serverstat *ss;
	hnode_t *node;
	hscan_t scan;

	SaveServerStats();
	DBACloseTable( SERVER_TABLE );
	hash_scan_begin( &scan, serverstathash );
	while( ( node = hash_scan_next( &scan ) ) != NULL )
	{
		ss = hnode_get( node );
		ClearServerModValue( ss->s );
		hash_scan_delete_destroy_node( serverstathash, node );
		ns_free( ss );
	}
	hash_destroy( serverstathash );
}
