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
** $Id: tld.c 3294 2008-02-24 02:45:41Z Fish $
*/

/*  TODO:
 *  - AverageTLDStatistics
 */

#include "neostats.h"
#include "statserv.h"
#include "stats.h"
#include "network.h"
#include "tld.h"
#include "namedvars.h"
#include "GeoIP.h"
#include "GeoIPCity.h"

/** TLD table name */
#define TLD_TABLE		"TLD"
/** Unknown entry */
#define UNKNOWN_COUNTRY_CODE	"???"

/** TLD list */
static list_t *tldstatlist;
static GeoIP *gi;

nv_struct nv_ss_tld[] = {
	{"tld", NV_STR, offsetof(TLD, tld), NV_FLG_RO, -1, -1},
	{"country", NV_STR, offsetof(TLD, tld), NV_FLG_RO, -1, -1},
	{"user-day", NV_INT, offsetof(statistic, day), NV_FLG_RO, offsetof(TLD, users), -1},
	{"user-week", NV_INT, offsetof(statistic, week), NV_FLG_RO, offsetof(TLD, users), -1},
	{"user-month", NV_INT, offsetof(statistic, month), NV_FLG_RO, offsetof(TLD, users), -1},
	{"users", NV_INT, offsetof(statistic, current), NV_FLG_RO, offsetof(TLD, users), -1},
	{"users-alltime-runningtotal", NV_INT, offsetof(statistic, alltime.runningtotal), NV_FLG_RO, offsetof(TLD, users), -1},
	{"users-alltime-average", NV_INT, offsetof(statistic, alltime.average), NV_FLG_RO, offsetof(TLD,users), -1},
	{"users-alltime-max", NV_INT, offsetof(statistic, alltime.max), NV_FLG_RO, offsetof(TLD, users), -1},
	{"users-alltime-ts_max", NV_INT, offsetof(statistic, alltime.ts_max), NV_FLG_RO, offsetof(TLD, users), -1},
	{"users-daily-runningtotal", NV_INT, offsetof(statistic, daily.runningtotal), NV_FLG_RO, offsetof(TLD, users), -1},
	{"users-daily-average", NV_INT, offsetof(statistic, daily.average), NV_FLG_RO, offsetof(TLD,users), -1},
	{"users-daily-max", NV_INT, offsetof(statistic, daily.max), NV_FLG_RO, offsetof(TLD, users), -1},
	{"users-daily-ts_max", NV_INT, offsetof(statistic, daily.ts_max), NV_FLG_RO, offsetof(TLD, users), -1},
	{"users-weekly-runningtotal", NV_INT, offsetof(statistic, weekly.runningtotal), NV_FLG_RO, offsetof(TLD, users), -1},
	{"users-weekly-average", NV_INT, offsetof(statistic, weekly.average), NV_FLG_RO, offsetof(TLD,users), -1},
	{"users-weekly-max", NV_INT, offsetof(statistic, weekly.max), NV_FLG_RO, offsetof(TLD, users), -1},
	{"users-weekly-ts_max", NV_INT, offsetof(statistic, weekly.ts_max), NV_FLG_RO, offsetof(TLD, users), -1},
	{"users-monthly-runningtotal", NV_INT, offsetof(statistic, monthly.runningtotal), NV_FLG_RO, offsetof(TLD, users), -1},
	{"users-monthly-average", NV_INT, offsetof(statistic, monthly.average), NV_FLG_RO, offsetof(TLD,users), -1},
	{"users-monthly-max", NV_INT, offsetof(statistic, monthly.max), NV_FLG_RO, offsetof(TLD, users), -1},
	{"users-monthly-ts_max", NV_INT, offsetof(statistic, monthly.ts_max), NV_FLG_RO, offsetof(TLD, users), -1},
	NV_STRUCT_END()
};	

/** @brief ResetTLDStatistics
 *
 *  Reset TLD statistics
 *
 *  @param none
 *
 *  @return none
 */

void ResetTLDStatistics( void ) 
{
	lnode_t *tn, *tn2;
	TLD *t;
	
	tn = list_first( tldstatlist );
	while( tn != NULL )
	{
		t = lnode_get( tn );
		ResetStatistic( &t->users );
		if( t->users.current == 0 )
		{
			/* don't delete the tld entry ??? as its our "unknown" entry */
			if( ircstrcasecmp( t->tld, UNKNOWN_COUNTRY_CODE ) )
			{
				tn2 = list_next( tldstatlist, tn );
				ns_free( t );
				list_delete_destroy_node( tldstatlist, tn );
				tn = tn2;
				continue;
			}
		}
		tn = list_next( tldstatlist, tn );
	}
}

/** @brief AverageTLDStatistics
 *
 *  Average TLD statistics
 *
 *  @param none
 *
 *  @return none
 */

void AverageTLDStatistics( void )
{
}

/** @brief FindTLD
 *
 *  list find helper
 *
 *  @param key1
 *  @param key2
 *
 *  @return results of comparison
 */

static int FindTLD( const void *v, const void *cc )
{
	const TLD *t = ( void * )v;
	return( ircstrcasecmp( t->tld, ( char * )cc ) );
}

/** @brief sortusers
 *
 *  list sorting helper
 *
 *  @param key1
 *  @param key2
 *
 *  @return results of comparison
 */

static int sortusers( const void *v, const void *v2 )
{
	const TLD *t = ( void * )v;
	const TLD *t2 = ( void * )v2;
	return( t2->users.daily.max - t->users.daily.max );
}

/** @brief TLDReport
 *
 *  Report TLD
 *
 *  @param cv pointer to version to report
 *  @param v pointer to client to send to
 *
 *  @return none
 */

void TLDReport( const TLD *tld, const void *v )
{
	irc_prefmsg( statbot, ( Client * ) v, 
		"%3s \2%3d\2 (%d%%) -> %s ---> Daily Total: %d",
		tld->tld, tld->users.alltime.max, ( int )( ( float ) tld->users.current / ( float ) networkstats.users.current ) * 100,
		tld->country, tld->users.current );
}

/** @brief GetTLDStats
 *
 *  Walk through list passing each TLD to handler
 *
 *  @param handler pointer to handler function
 *  @param v pointer to client to send to
 *
 *  @return none
 */

void GetTLDStats( const TLDStatHandler handler, const void *v )
{
	const TLD *t;
	lnode_t *tn;
	
	list_sort( tldstatlist, sortusers );
	tn = list_first( tldstatlist );
	while( tn != NULL )
	{
		t = lnode_get( tn );
		handler( t, v );
		tn = list_next( tldstatlist, tn );
	}
}

/** @brief ss_cmd_tldmap
 *
 *  TLDMAP command handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_tldmap( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	irc_prefmsg( statbot, cmdparams->source, "Top Level Domain Statistics:" );
	GetTLDStats( TLDReport, ( void * )cmdparams->source );
	irc_prefmsg( statbot, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief DelTLDUser
 *
 *  Delete a TLD from the current stats
 *
 *  @param client to delete from TLD list
 *
 *  @return none
 */

void DelTLDUser( const Client * u )
{
	const char *country_code;
	TLD *t = NULL;
	
	SET_SEGV_LOCATION();
	if( !gi )
		return;
	country_code = GeoIP_country_code_by_addr( gi, u->hostip );
	if( country_code )
		t = lnode_find( tldstatlist, country_code, FindTLD );
	else
		t = lnode_find( tldstatlist, UNKNOWN_COUNTRY_CODE, FindTLD );
	DecStatistic( &t->users );		
}

/** @brief AddTLDUser
 *
 *  Add a TLD to the current stats
 *
 *  @param client to add to TLD list
 *
 *  @return none
 */

void AddTLDUser( const Client *u )
{
	const char *country_name;
	const char *country_code;
	TLD *t = NULL;

	if( !gi )
		return;
	country_code = GeoIP_country_code_by_addr( gi, u->hostip );
	if( country_code )
	{
		t = lnode_find( tldstatlist, country_code, FindTLD );
		if( !t )
		{
			country_name = GeoIP_country_name_by_addr( gi, u->hostip );
			t = ns_calloc( sizeof( TLD ) );
			strlcpy( t->tld, country_code, 5 );
			strlcpy( t->country, country_name, 32 );
			lnode_create_append( tldstatlist, t );
		}
	}
	else
	{
		t = lnode_find( tldstatlist, UNKNOWN_COUNTRY_CODE, FindTLD );
	}
	IncStatistic( &t->users );
}
	
/** @brief ss_event_nickip
 *
 *  NICKIP event handler
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_event_nickip( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	AddTLDUser( cmdparams->source );
	return NS_SUCCESS;
}

/** @brief SaveTLDStat
 *
 *  Save TLD statistic
 *
 *  @param none
 *
 *  @return none
 */

static void SaveTLDStat( const TLD *tld, const void *v )
{
	DBAStore( TLD_TABLE, tld->tld, ( void * )tld, sizeof( TLD ) );
}

/** @brief SaveTLDStats
 *
 *  Save TLD stats
 *
 *  @param none
 *
 *  @return none
 */

void SaveTLDStats( void )
{
	GetTLDStats( SaveTLDStat, NULL );
}

/** @brief new_tld
 *
 *  Table load handler
 *
 *  @param data pointer to table row data
 *  @param size of loaded data
 *
 *  @return NS_TRUE to abort load or NS_FALSE to continue loading
 */

static int new_tld( void *data, int size )
{
	TLD *t;

	if( size != sizeof( TLD ) )
	{
		nlog( LOG_CRITICAL, "TLD data size invalid" );		
		return NS_FALSE;
	}
	t = ns_calloc( sizeof( TLD ) );
	os_memcpy( t, data, sizeof( TLD ) );
	lnode_create_append( tldstatlist, t );
	return NS_FALSE;
}

/** @brief LoadTLDStats
 *
 *  Load TLD statistics 
 *
 *  @param none
 *
 *  @return NS_SUCCESS
 */

int LoadTLDStats( void )
{
	DBAFetchRows( TLD_TABLE, new_tld );
	return NS_SUCCESS;
}

/** @brief InitTLDStatistics
 *
 *  Init TLD lists
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int InitTLDStatistics( void )
{
	TLD *t;

	SET_SEGV_LOCATION();
	tldstatlist = nv_list_create( LISTCOUNT_T_MAX, "StatServ-TLD", nv_ss_tld, NV_FLAGS_RO, NULL );
	if( !tldstatlist )
	{
		nlog( LOG_CRITICAL, "Unable to create TLD list" );
		return NS_FAILURE;
	}
	gi = NULL;
	/* now open the various DB's */
	if( GeoIP_db_avail( GEOIP_COUNTRY_EDITION ) )
	{
		gi = GeoIP_open_type( GEOIP_COUNTRY_EDITION, GEOIP_STANDARD );
		if( gi != NULL )
			nlog( LOG_NOTICE, "Loaded %s GeoIP Database", GeoIPDBDescription[GEOIP_COUNTRY_EDITION] );
		else
			nlog( LOG_WARNING, "%s Database may be corrupt", GeoIPDBDescription[GEOIP_COUNTRY_EDITION] );
	}
	else
	{
		nlog( LOG_WARNING, "GeoIP Database is not available. TLD stats will not be available" );
	}
	t = ns_calloc( sizeof( TLD ) );
	ircsnprintf( t->tld, 5, UNKNOWN_COUNTRY_CODE );
	strlcpy( t->country, "Unknown", 8 );
	lnode_create_append( tldstatlist, t );
	DBAOpenTable( TLD_TABLE );
	LoadTLDStats();
	return NS_SUCCESS;
}

/** @brief FiniTLDStatistics
 *
 *  Clean up TLD lists
 *
 *  @param none
 *
 *  @return none
 */

void FiniTLDStatistics( void ) 
{
	SaveTLDStats();
	DBACloseTable( TLD_TABLE );
	if( gi )
	{
		GeoIP_delete( gi );
		gi = NULL;
	}
	list_destroy_auto( tldstatlist );
}
