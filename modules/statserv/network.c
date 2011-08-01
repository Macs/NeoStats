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
** $Id: network.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "statserv.h"
#include "stats.h"
#include "network.h"

/** Network table name */
#define NETWORK_TABLE	"Network"

/** Network stats */
networkstat networkstats;

/** @brief AverageNetworkStatistics 
 *
 *  Average network statistics
 *
 *  @param none
 *
 *  @return none
 */

void AverageNetworkStatistics( void )
{
	AverageStatistic( &networkstats.servers );
	AverageStatistic( &networkstats.channels );
	AverageStatistic( &networkstats.users );
	AverageStatistic( &networkstats.opers );
	AverageStatistic( &networkstats.kills );
}

/** @brief ResetNetworkStatistics
 *
 *  Reset network statistics
 *
 *  @param none
 *
 *  @return none
 */

void ResetNetworkStatistics( void )
{
	ResetStatistic( &networkstats.servers );
	ResetStatistic( &networkstats.channels );
	ResetStatistic( &networkstats.users );
	ResetStatistic( &networkstats.opers );
	ResetStatistic( &networkstats.kills );
}

/** @brief AddNetworkServer
 *
 *  Add server to network stats
 *  Report new record if previous ones beaten
 *
 *  @param none
 *
 *  @return none
 */

void AddNetworkServer( void )
{
	if( IncStatistic( &networkstats.servers ) )
		announce_record( "\2NEW NETWORK RECORD\2 %d servers on the network",
			networkstats.servers.current );
}

/** @brief DelNetworkServer
 *
 *  Delete server from network stats
 *
 *  @param none
 *
 *  @return none
 */

void DelNetworkServer( void )
{
	DecStatistic( &networkstats.servers );
}

/** @brief AddNetworkChannel
 *
 *  Add channel to network stats
 *  Report new record if previous ones beaten
 *
 *  @param none
 *
 *  @return none
 */

void AddNetworkChannel( void )
{
	if( IncStatistic( &networkstats.channels ) )
		announce_record( "\2NEW NETWORK RECORD\2 %d channels on the network",
		    networkstats.channels.current );
}

/** @brief DelNetworkChannel
 *
 *  Delete channel from network stats
 *
 *  @param none
 *
 *  @return none
 */

void DelNetworkChannel( void )
{
	DecStatistic( &networkstats.channels );
}

/** @brief AddNetworkUser
 *
 *  Add user to network stats
 *  Report new record if previous ones beaten
 *
 *  @param none
 *
 *  @return none
 */

void AddNetworkUser( void )
{
	if( IncStatistic( &networkstats.users ) )
		announce_record( "\2NEW NETWORK RECORD!\2 %d users on the network",
			networkstats.users.current );
}

/** @brief DelNetworkUser
 *
 *  Delete user from network stats
 *
 *  @param none
 *
 *  @return none
 */

void DelNetworkUser( void )
{
	DecStatistic( &networkstats.users );
}

/** @brief AddNetworkOper
 *
 *  Add oper to network stats
 *
 *  @param none
 *
 *  @return none
 */

void AddNetworkOper( void )
{
	IncStatistic( &networkstats.opers );
}

/** @brief DelNetworkOper
 *
 *  Delete oper from network stats
 *
 *  @param none
 *
 *  @return none
 */

void DelNetworkOper( void )
{
	DecStatistic( &networkstats.opers );
}

/** @brief AddNetworkKill
 *
 *  Add kill to network stats
 *
 *  @param none
 *
 *  @return none
 */

void AddNetworkKill( void )
{
	IncStatistic( &networkstats.kills );
}

/** @brief DelNetworkKill
 *
 *  Delete kill from network stats
 *
 *  @param none
 *
 *  @return none
 */

void DelNetworkKill( void )
{
	DecStatistic( &networkstats.kills );
}

/** @brief ss_cmd_netstats
 *
 *  NETSTATS command handler
 *  Reports current network statistics to requesting user
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_netstats( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	irc_prefmsg( statbot, cmdparams->source, "Network Statistics:-----" );
	irc_prefmsg( statbot, cmdparams->source, "Current Users: %d", networkstats.users.current );
	irc_prefmsg( statbot, cmdparams->source, "Maximum Users: %d [%s]",
		networkstats.users.alltime.max, sftime( networkstats.users.alltime.ts_max ) );
	irc_prefmsg( statbot, cmdparams->source, "Total Users Connected: %d",
		networkstats.users.alltime.runningtotal );
	irc_prefmsg( statbot, cmdparams->source, "Current Channels %d", networkstats.channels.current );
	irc_prefmsg( statbot, cmdparams->source, "Maximum Channels %d [%s]",
		networkstats.channels.alltime.max, sftime( networkstats.channels.alltime.ts_max ) );
	irc_prefmsg( statbot, cmdparams->source, "Current Opers: %d", networkstats.opers.current );
	irc_prefmsg( statbot, cmdparams->source, "Maximum Opers: %d [%s]",
		networkstats.opers.alltime.max, sftime( networkstats.opers.alltime.ts_max ) );
	irc_prefmsg( statbot, cmdparams->source, "Users Set Away: %d", NSGetAwayCount() );
	irc_prefmsg( statbot, cmdparams->source, "Current Servers: %d", networkstats.servers.current );
	irc_prefmsg( statbot, cmdparams->source, "Maximum Servers: %d [%s]",
		networkstats.servers.alltime.max, sftime( networkstats.servers.alltime.ts_max ) );
	irc_prefmsg( statbot, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief ss_cmd_daily
 *
 *  DAILY command handler
 *  Reports current daily network statistics to requesting user
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_daily( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	irc_prefmsg( statbot, cmdparams->source, "Daily Network Statistics:" );
	irc_prefmsg( statbot, cmdparams->source, "Maximum Servers: %-2d %s",
		networkstats.servers.daily.max, sftime( networkstats.servers.daily.ts_max ) );
	irc_prefmsg( statbot, cmdparams->source, "Maximum Users: %-2d %s", 
		networkstats.users.daily.max, sftime( networkstats.users.daily.ts_max ) );
	irc_prefmsg( statbot, cmdparams->source, "Maximum Channel: %-2d %s", 
		networkstats.channels.daily.max, sftime( networkstats.channels.daily.ts_max ) );
	irc_prefmsg( statbot, cmdparams->source, "Maximum Opers: %-2d %s", 
		networkstats.opers.daily.max, sftime( networkstats.opers.daily.ts_max ) );
	irc_prefmsg( statbot, cmdparams->source, "Total Users Connected: %-2d",
		networkstats.users.daily.runningtotal );
	irc_prefmsg( statbot, cmdparams->source, "End of Information." );
	return NS_SUCCESS;
}

/** @brief LoadNetworkStats
 *
 *  Load network stats
 *
 *  @param none
 *
 *  @return none
 */

void LoadNetworkStats( void ) 
{
	if( DBAFetch( NETWORK_TABLE, NETWORK_TABLE, &networkstats, sizeof( networkstats ) ) == NS_SUCCESS ) 
	{
		PostLoadStatistic( &networkstats.servers );
		PostLoadStatistic( &networkstats.channels );
		PostLoadStatistic( &networkstats.users );
		PostLoadStatistic( &networkstats.opers );
		PostLoadStatistic( &networkstats.kills );
	}
}

/** @brief SaveNetworkStats
 *
 *  Save network stats
 *
 *  @param none
 *
 *  @return none
 */

void SaveNetworkStats( void )
{
	DBAStore( NETWORK_TABLE, NETWORK_TABLE, &networkstats, sizeof( networkstats ) );
	PreSaveStatistic( &networkstats.servers );
	PreSaveStatistic( &networkstats.channels );
	PreSaveStatistic( &networkstats.users );
	PreSaveStatistic( &networkstats.opers );
	PreSaveStatistic( &networkstats.kills );
}

/** @brief InitVersionStats
 *
 *  Init version stats
 *
 *  @param none
 *
 *  @return none
 */

void InitNetworkStats( void )
{
	DBAOpenTable( NETWORK_TABLE );
	LoadNetworkStats();
}

/** @brief FiniNetworkStats
 *
 *  Fini network stats
 *
 *  @param none
 *
 *  @return none
 */

void FiniNetworkStats( void )
{
	DBACloseTable( NETWORK_TABLE );
	SaveNetworkStats();
}
