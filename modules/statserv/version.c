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
** $Id: version.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "statserv.h"
#include "stats.h"
#include "version.h"
#include "namedvars.h"

/** CTCP version table name */
#define CTCPVERSION_TABLE "CTCPVERSION"

/** Client version list */
static list_t *ctcp_version_list;

/** namedvars for version list */
nv_struct nv_ss_version[] = {
	{"name", NV_STR, offsetof(ss_ctcp_version, name), NV_FLG_RO, -1, BUFSIZE},
	{"users-daily", NV_INT, offsetof(statistic, day), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	{"users-weekly", NV_INT, offsetof(statistic, week), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	{"users-month", NV_INT, offsetof(statistic, month), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	{"users-current", NV_INT, offsetof(statistic, current), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	{"users-alltime-runningtotal", NV_INT, offsetof(statistic, alltime.runningtotal), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	{"users-alltime-average", NV_INT, offsetof(statistic, alltime.average), NV_FLG_RO, offsetof(ss_ctcp_version,users), -1},
	{"users-alltime-max", NV_INT, offsetof(statistic, alltime.max), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	{"users-alltime-ts_max", NV_INT, offsetof(statistic, alltime.ts_max), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	{"users-daily-runningtotal", NV_INT, offsetof(statistic, daily.runningtotal), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	{"users-daily-average", NV_INT, offsetof(statistic, daily.average), NV_FLG_RO, offsetof(ss_ctcp_version,users), -1},
	{"users-daily-max", NV_INT, offsetof(statistic, daily.max), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	{"users-daily-ts_max", NV_INT, offsetof(statistic, daily.ts_max), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	{"users-weekly-runningtotal", NV_INT, offsetof(statistic, weekly.runningtotal), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	{"users-weekly-average", NV_INT, offsetof(statistic, weekly.average), NV_FLG_RO, offsetof(ss_ctcp_version,users), -1},
	{"users-weekly-max", NV_INT, offsetof(statistic, weekly.max), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	{"users-weekly-ts_max", NV_INT, offsetof(statistic, weekly.ts_max), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	{"users-monthly-runningtotal", NV_INT, offsetof(statistic, monthly.runningtotal), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	{"users-monthly-average", NV_INT, offsetof(statistic, monthly.average), NV_FLG_RO, offsetof(ss_ctcp_version,users), -1},
	{"users-monthly-max", NV_INT, offsetof(statistic, monthly.max), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	{"users-monthly-ts_max", NV_INT, offsetof(statistic, monthly.ts_max), NV_FLG_RO, offsetof(ss_ctcp_version, users), -1},
	NV_STRUCT_END()
};


/** @brief topcurrentversions
 *
 *  list sorting helper
 *
 *  @param key1
 *  @param key2
 *
 *  @return results of comparison
 */

static int topcurrentversions( const void *key1, const void *key2 )
{
	const ss_ctcp_version *ver1 = key1;
	const ss_ctcp_version *ver2 = key2;
	return( ver2->users.current - ver1->users.current );
}

/** @brief findctcpversion
 *
 *  Check list for ctcp version
 *
 *  @param name ctcp version to search for
 *
 *  @return pointer to statistic found or NULL if none
 */

static ss_ctcp_version *findctcpversion( const char *name )
{
	ss_ctcp_version *cv;

	cv = lnode_find( ctcp_version_list, name, comparef );
	if( cv )
	{
		dlog( DEBUG7, "findctcpversion: found version: %s", name );
	}
	else
	{
		dlog( DEBUG7, "findctcpversion: %s not found", name );	
	}
	return cv;
}

/** @brief SaveClientVersion
 *
 *  Save client version statistic
 *
 *  @param none
 *
 *  @return none
 */
static void SaveClientVersion( const ss_ctcp_version *cv, const void *v )
{
	DBAStore( CTCPVERSION_TABLE, cv->name, ( void *)cv, sizeof( ss_ctcp_version) );
	dlog( DEBUG7, "Save version %s", cv->name );
}

/** @brief SaveClientVersions
 *
 *  Save client version statistics
 *
 *  @param none
 *
 *  @return none
 */

static void SaveClientVersions( void )
{
	GetClientStats( SaveClientVersion, -1, NULL );
}

/** @brief new_ctcpversion
 *
 *  Table load handler
 *
 *  @param data pointer to table row data
 *  @param size of loaded data
 *
 *  @return NS_TRUE to abort load or NS_FALSE to continue loading
 */

static int new_ctcpversion( void *data, int size )
{
	ss_ctcp_version *cv;
	
	if( size != sizeof( ss_ctcp_version ) )
	{
		nlog( LOG_CRITICAL, "CTCP version data size invalid" );		
		return NS_FALSE;
	}
	cv = ns_calloc( sizeof( ss_ctcp_version ) );
	os_memcpy( cv, data, sizeof( ss_ctcp_version ) );
	lnode_create_append( ctcp_version_list, cv );
	return NS_FALSE;
}

/** @brief LoadVersionStats
 *
 *  Load version stats
 *
 *  @param none
 *
 *  @return none
 */

static void LoadVersionStats( void )
{
	DBAFetchRows( CTCPVERSION_TABLE, new_ctcpversion );
}

/** @brief ClientVersionReport
 *
 *  Report client version
 *
 *  @param cv pointer to version to report
 *  @param v pointer to client to send to
 *
 *  @return none
 */

static void ClientVersionReport( const ss_ctcp_version *cv, const void *v )
{
	irc_prefmsg( statbot, ( Client * ) v, "%d -> %s", cv->users.current, cv->name );
}

/** @brief GetClientStats
 *
 *  Walk through list passing each client version to handler
 *
 *  @param handler pointer to handler function
 *  @param limit max entries to handle
 *  @param v pointer to client to send to
 *
 *  @return none
 */

void GetClientStats( const CTCPVersionHandler handler, int limit, const void *v )
{
	ss_ctcp_version *cv;
	lnode_t *cn;
	int count = 0;
	
	if( !list_is_sorted( ctcp_version_list, topcurrentversions ) )
	{
		list_sort( ctcp_version_list, topcurrentversions );
	}
	cn = list_first( ctcp_version_list );
	while( cn != NULL )
	{
		cv = ( ss_ctcp_version * ) lnode_get( cn );
		handler( cv, v );	
		cn = list_next( ctcp_version_list, cn );
		count++;
		if( limit != -1 && count >= limit )
		{
			break;
		}
	}

}

/** @brief ss_cmd_ctcpversion
 *
 *  CTCPVERSION command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *    cmdparams->av[0] = optional limit
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_ctcpversion( const CmdParams *cmdparams )
{
	int limit;

	limit = cmdparams->ac > 0 ? atoi( cmdparams->av[0] ) : 10;
	if( limit < 10 )
	{
		limit = 10;
	}
	if( list_count( ctcp_version_list ) == 0 )
	{
		irc_prefmsg( statbot, cmdparams->source, "No Stats Available." );
		return NS_SUCCESS;
	}
	irc_prefmsg( statbot, cmdparams->source, "Top %d Client Versions:", limit );
	irc_prefmsg( statbot, cmdparams->source, "======================" );
	GetClientStats( ClientVersionReport, limit, ( void * )cmdparams->source );
	irc_prefmsg( statbot, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief ss_event_ctcpversionbc
 *
 *  CTCP VERSION event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_ctcpversionbc( const CmdParams *cmdparams )
{
	static char nocols[BUFSIZE];
	ss_ctcp_version *cv;

	/* drop multiple replies and additional lines in multi line replies */
	if( cmdparams->source->flags & CLIENT_FLAG_GOTVERSION )
	{
		return NS_SUCCESS;
	}
	strlcpy( nocols, cmdparams->param, BUFSIZE );
	strip_mirc_codes( nocols );
	cv = findctcpversion( nocols );
	if( cv == NULL )
	{
		cv = ns_calloc( sizeof( ss_ctcp_version ) );
		strlcpy( cv->name, nocols, BUFSIZE );
		lnode_create_append( ctcp_version_list, cv );
		dlog( DEBUG7, "Added version: %s", cv->name );
	}
	IncStatistic( &cv->users );
	return NS_SUCCESS;
}

/** @brief InitVersionStats
 *
 *  Init version stats
 *
 *  @param none
 *
 *  @return NS_SUCCESS on success, NS_FAILURE on failure
 */

int InitVersionStats( void )
{
	ctcp_version_list = nv_list_create( LISTCOUNT_T_MAX, "StatServ-Version", nv_ss_version, NV_FLAGS_RO, NULL );
	if( ctcp_version_list == NULL )
	{
		nlog( LOG_CRITICAL, "Unable to create version statistic list" );
		return NS_FAILURE;
	}
	DBAOpenTable( CTCPVERSION_TABLE );
	LoadVersionStats();
	return NS_SUCCESS;
}

/** @brief FiniVersionStats
 *
 *  Fini version stats
 *
 *  @param none
 *
 *  @return none
 */

void FiniVersionStats( void )
{
	SaveClientVersions();
	DBACloseTable( CTCPVERSION_TABLE );
	list_destroy_auto( ctcp_version_list );
}
