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
** $Id: htmlstats.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "statserv.h"
#include "stats.h"
#include "network.h"
#include "server.h"
#include "channel.h"
#include "version.h"
#include "tld.h"
#ifdef HAVE_FCNTL_H
#include <fcntl.h> 
#endif /* HAVE_FCNTL_H */

/** HTML output handler type */
typedef void( *htmlhandler )( void );

/** HTML handler table struct */
typedef struct htmlfunc {
	char *directive;
	htmlhandler handler;
}htmlfunc;

#define TABLE_START( opf ) os_fprintf( ( opf ), "<table border = 0>" );
#define TABLE_END( opf ) os_fprintf( opf, "</table>" );
/** HTML handler prototypes */
static void html_map( void );
static void html_srvlist( void );
static void html_srvlistdet( void );
static void html_netstats( void );
static void html_dailystats( void );
static void html_weeklystats( void );
static void html_monthlystats( void );
static void html_channeltop10members( void );
static void html_channeltop10joins( void );
static void html_channeltop10kicks( void );
static void html_channeltop10topics( void );
static void html_tldmap( void );
static void html_version( void );
static void html_title( void );
static void html_clientstats( void );
static void html_copyright( void );
/** Output file pointer */
static FILE *opf;
/** Output file template */
static const char html_template[]="data/index.tpl";

/** HTML handler lookup table */
static htmlfunc htmlfuncs[]=
{
	{"!MAP!", html_map},
	{"!SRVLIST!", html_srvlist},
	{"!SRVLISTDET!", html_srvlistdet},
	{"!NETSTATS!", html_netstats},
	{"!DAILYSTATS!", html_dailystats},
	{"!WEEKLYSTATS!", html_weeklystats},
	{"!MONTHLYSTATS!", html_monthlystats},
	{"!TOP10CHAN!", html_channeltop10members},
	{"!TOP10JOIN!", html_channeltop10joins},
	{"!TOP10KICKS!", html_channeltop10kicks},
	{"!TOP10TOPICS!", html_channeltop10topics},
	{"!TLDMAP!", html_tldmap},
	{"!VERSION!", html_version},
	{"!TITLE!", html_title},
	{"!CLIENTSTATS!", html_clientstats},
	{"</body>", html_copyright},
	{NULL, NULL},
};

/** @brief html_title
 *
 *  HTML handler for title
 *
 *  @param none
 *
 *  @return none
 */

static void html_title( void )
{
	if( me.netname[0] != '\0' )
		os_fprintf( opf, "Network Statistics for %s", me.netname );
	else
		os_fprintf( opf, "Network Statistics" );
}

/** @brief html_version
 *
 *  HTML handler for version
 *
 *  @param none
 *
 *  @return none
 */

static void html_version( void )
{
	os_fputs( me.version, opf );
}

/** @brief html_copyright
 *
 *  HTML handler for copyright footer
 *
 *  @param none
 *
 *  @return none
 */

static void html_copyright( void )
{
	os_fprintf( opf, "<br><br><center>Statistics last updated at %s<br>", sftime( time( NULL ) ) );
	os_fprintf( opf, "<b>StatServ Information:</b>\n" );
	os_fprintf( opf, "<br> %s compiled on %s at %s\n", me.name, version_date, version_time );
	os_fprintf( opf, "<br> %s compiled on %s at %s\n", module_info.name,
		module_info.build_date, module_info.build_time );
	os_fprintf( opf, "<br><a href=\"http://www.neostats.net\">http://www.neostats.net</a>\n" );
	os_fprintf( opf, "</center>\n" );
	os_fprintf( opf, "</body>\n" );
}

/** @brief serverlisthandler
 *
 *  HTML handler helper for srvlist
 *
 *  @param none
 *
 *  @return none
 */

static void serverlisthandler( const serverstat *ss, const void *v )
{
	os_fprintf( opf, "<tr><td height=\"4\"></td>\n" );
	os_fprintf( opf, "<td height=\"4\"><a href=\"#%s\"> %s (%s)</a></td></tr>\n",
		ss->name, ss->name, ( ss->s ) ? "ONLINE" : "OFFLINE" );
}

/** @brief html_srvlist
 *
 *  HTML handler for srvlist
 *
 *  @param none
 *
 *  @return none
 */

static void html_srvlist( void )
{
	TABLE_START( opf );
	os_fprintf( opf, "<tr><th colspan = 2>Server name</th></tr>" );
	GetServerStats( serverlisthandler, NULL );
	TABLE_END( opf );
}

/** @brief serverlistdetailhandler
 *
 *  HTML handler helper for srvlistdet
 *
 *  @param none
 *
 *  @return none
 */

static void serverlistdetailhandler( const serverstat *ss, const void *v )
{
	os_fprintf( opf, "<tr><th><a name=\"%s\">Server:</a></th><th colspan = 2><b>%s</b></th></tr>\n",
		ss->name, ss->name );
	if( !ss->s ) {
		os_fprintf( opf, "<tr><td>Last Seen:</td><td colspan = 2>%s</td></tr>\n",
			sftime( ss->ts_lastseen ) );
	} else {
		os_fprintf( opf,"<tr><td>Current Users:</td><td>%d (%d%%)</td><td>Max %d at %s</td></tr>\n",
			ss->users.current, ( int )( ( ( float ) ss->users.current /( float ) networkstats.users.current ) * 100 ),
			ss->users.alltime.max, sftime( ss->users.alltime.ts_max ) );
		os_fprintf( opf,
			"<tr><td>Current Opers:</td><td>%d (%d%%)</td><td>Max %d at %s</td></tr>\n",
			ss->opers.current, ( int )( ( ( float ) ss->opers.current /( float ) networkstats.opers.current ) * 100 ),
			ss->opers.alltime.max, sftime( ss->opers.alltime.ts_max ) );
	}
	os_fprintf( opf, "<tr><td>Total Users Connected:</td><td colspan = 2>%d</td></tr>",
		ss->users.alltime.runningtotal );
	os_fprintf( opf, "<tr><td>IrcOp Kills</td><td colspan = 2>%d</td></tr>", 
		ss->operkills.alltime.runningtotal );
	os_fprintf( opf, "<tr><td>Server Kills</td><td colspan = 2>%d</td></tr>",
		ss->serverkills.alltime.runningtotal );
	os_fprintf( opf, "<tr><td>Highest Ping</td><td>%d</td><td>at %s</td></tr>",
		( int )ss->highest_ping, sftime( ss->ts_highest_ping ) );
	if( ss->s )
		os_fprintf( opf, "<tr><td>Current Ping</td><td colspan = 2>%d</td></tr>",
			(int)ss->s->server->ping );
	os_fprintf( opf, "<tr><td>Server Splits</td><td colspan = 2>%d</td></tr>",
		(int)ss->splits.alltime.runningtotal );
}

/** @brief html_srvlistdet
 *
 *  HTML handler for srvlistdet
 *
 *  @param none
 *
 *  @return none
 */

static void html_srvlistdet( void )
{
	TABLE_START( opf );
	GetServerStats( serverlistdetailhandler, NULL );
	TABLE_END( opf );
}

/** @brief html_netstats
 *
 *  HTML handler for netstats
 *
 *  @param none
 *
 *  @return none
 */

static void html_netstats( void )
{
	TABLE_START( opf );
	os_fprintf( opf, "<tr><th><b></b></th><th><b>Total</b></th><th><b>Current</b></th><th><b>Average</b></th><th><b>Max</b></th><th><b>Max Time</b></th></tr>\n" );
	os_fprintf( opf, "<tr><td>Users:</td>\n" );
	os_fprintf( opf, "<td>%d</td>\n", networkstats.users.alltime.runningtotal );
	os_fprintf( opf, "<td>%d (%d%%)</td>\n", networkstats.users.current,
		GetAllTimePercent( &networkstats.users ) );
	os_fprintf( opf, "<td>%d</td>\n", networkstats.users.alltime.average );
	os_fprintf( opf, "<td>%d</td>\n", networkstats.users.alltime.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.users.alltime.ts_max ) );
	os_fprintf( opf, "<tr><td>Channels:</td>\n" );
	os_fprintf( opf, "<td>%d</td>\n", networkstats.channels.alltime.runningtotal );
	os_fprintf( opf, "<td>%i (%d%%)</td>\n", networkstats.channels.current,
		GetAllTimePercent( &networkstats.channels ) );
	os_fprintf( opf, "<td>%i</td>\n", networkstats.channels.alltime.average );
	os_fprintf( opf, "<td>%d</td>\n", networkstats.channels.alltime.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.channels.alltime.ts_max ) );
	os_fprintf( opf, "<tr><td>Opers:</td>\n" );
	os_fprintf( opf, "<td>%d</td>\n", networkstats.opers.alltime.runningtotal );
	os_fprintf( opf, "<td>%i (%d%%)</td>\n", networkstats.opers.current,
		GetAllTimePercent( &networkstats.opers ) );
	os_fprintf( opf, "<td>%i</td>\n", networkstats.opers.alltime.average );
	os_fprintf( opf, "<td>%i</td>\n", networkstats.opers.alltime.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.opers.alltime.ts_max ) );
	os_fprintf( opf, "<tr><td>Servers:</td>\n" );
	os_fprintf( opf, "<td>%d</td>\n", networkstats.servers.alltime.runningtotal );
	os_fprintf( opf, "<td>%d (%d%%)</td>\n", networkstats.servers.current,
		GetAllTimePercent( &networkstats.servers ) );
	os_fprintf( opf, "<td>%d</td>\n", networkstats.servers.alltime.average );
	os_fprintf( opf, "<td>%d</td>\n", networkstats.servers.alltime.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.servers.alltime.ts_max ) );
	os_fprintf( opf, "<tr><td colspan=\"3\">Users Set Away:</td>\n" );
	os_fprintf( opf, "<td colspan=\"3\">%d</td></tr>\n", NSGetAwayCount() );
	TABLE_END( opf );
}

/** @brief html_dailystats
 *
 *  HTML handler for daily
 *
 *  @param none
 *
 *  @return none
 */

static void html_dailystats( void )
{
	TABLE_START( opf );
	os_fprintf( opf, "<tr><th><b></b></th><th><b>Total</b><th><b>Current</b><th><b>Average</b></th><th><b>Max</b></th><th><b>Max Time</b></th></tr>\n" );
	os_fprintf( opf, "<tr><td>Users:</td>\n" );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.users.daily.runningtotal );
	os_fprintf( opf, "<td>%-2d (%d%%)</td>\n", networkstats.users.current,
		GetDailyPercent( &networkstats.users ) );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.users.daily.average );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.users.daily.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.users.daily.ts_max ) );
	os_fprintf( opf, "<tr><td>Channels:</td>\n" );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.channels.daily.runningtotal );
	os_fprintf( opf, "<td>%-2d (%d%%)</td>\n", networkstats.channels.current,
		GetDailyPercent( &networkstats.channels ) );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.channels.daily.average );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.channels.daily.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.channels.daily.ts_max ) );
	os_fprintf( opf, "<tr><td>Opers:</td>\n" );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.opers.daily.runningtotal );
	os_fprintf( opf, "<td>%-2d (%d%%)</td>\n", networkstats.opers.current,
		GetDailyPercent( &networkstats.opers ) );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.opers.daily.average );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.opers.daily.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.opers.daily.ts_max ) );
	os_fprintf( opf, "<tr><td>Servers:</td>\n" );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.servers.daily.runningtotal );
	os_fprintf( opf, "<td>%-2d (%d%%)</td>\n", networkstats.servers.current,
		GetDailyPercent( &networkstats.servers ) );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.servers.daily.average );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.servers.daily.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.servers.daily.ts_max ) );
	TABLE_END( opf );
}

/** @brief html_weeklystats
 *
 *  HTML handler for weekly
 *
 *  @param none
 *
 *  @return none
 */

static void html_weeklystats( void )
{
	TABLE_START( opf );
	os_fprintf( opf, "<tr><th><b></b></th><th><b>Total</b><th><b>Current</b><th><b>Average</b></th><th><b>Max</b></th><th><b>Max Time</b></th></tr>\n" );
	os_fprintf( opf, "<tr><td>Users:</td>\n" );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.users.weekly.runningtotal );
	os_fprintf( opf, "<td>%-2d (%d%%)</td>\n", networkstats.users.current,
		GetWeeklyPercent( &networkstats.users ) );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.users.weekly.average );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.users.weekly.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.users.weekly.ts_max ) );
	os_fprintf( opf, "<tr><td>Channels:</td>\n" );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.channels.weekly.runningtotal );
	os_fprintf( opf, "<td>%-2d (%d%%)</td>\n", networkstats.channels.current,
		GetWeeklyPercent( &networkstats.channels ) );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.channels.weekly.average );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.channels.weekly.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.channels.weekly.ts_max ) );
	os_fprintf( opf, "<tr><td>Opers:</td>\n" );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.opers.weekly.runningtotal );
	os_fprintf( opf, "<td>%-2d (%d%%)</td>\n", networkstats.opers.current,
		GetWeeklyPercent( &networkstats.opers ) );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.opers.weekly.average );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.opers.weekly.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.opers.weekly.ts_max ) );
	os_fprintf( opf, "<tr><td>Servers:</td>\n" );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.servers.weekly.runningtotal );
	os_fprintf( opf, "<td>%-2d (%d%%)</td>\n", networkstats.servers.current,
		GetWeeklyPercent( &networkstats.servers ) );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.servers.weekly.average );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.servers.weekly.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.servers.weekly.ts_max ) );
	TABLE_END( opf );
}

/** @brief html_monthlystats
 *
 *  HTML handler for monthly
 *
 *  @param none
 *
 *  @return none
 */

static void html_monthlystats( void )
{
	TABLE_START( opf );
	os_fprintf( opf, "<tr><th><b></b></th><th><b>Total</b><th><b>Current</b><th><b>Average</b></th><th><b>Max</b></th><th><b>Max Time</b></th></tr>\n" );
	os_fprintf( opf, "<tr><td>Users:</td>\n" );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.users.monthly.runningtotal );
	os_fprintf( opf, "<td>%-2d (%d%%)</td>\n", networkstats.users.current,
		GetMonthlyPercent( &networkstats.users ) );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.users.monthly.average );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.users.monthly.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.users.monthly.ts_max ) );
	os_fprintf( opf, "<tr><td>Channels:</td>\n" );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.channels.monthly.runningtotal );
	os_fprintf( opf, "<td>%-2d (%d%%)</td>\n", networkstats.channels.current,
		GetMonthlyPercent( &networkstats.channels ) );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.channels.monthly.average );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.channels.monthly.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.channels.monthly.ts_max ) );
	os_fprintf( opf, "<tr><td>Opers:</td>\n" );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.opers.monthly.runningtotal );
	os_fprintf( opf, "<td>%-2d (%d%%)</td>\n", networkstats.opers.current,
		GetMonthlyPercent( &networkstats.opers ) );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.opers.monthly.average );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.opers.monthly.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.opers.monthly.ts_max ) );
	os_fprintf( opf, "<tr><td>Servers:</td>\n" );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.servers.monthly.runningtotal );
	os_fprintf( opf, "<td>%-2d (%d%%)</td>\n", networkstats.servers.current,
		GetMonthlyPercent( &networkstats.servers ) );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.servers.monthly.average );
	os_fprintf( opf, "<td>%-2d</td>\n", networkstats.servers.monthly.max );
	os_fprintf( opf, "<td>%s</td></tr>\n", sftime( networkstats.servers.monthly.ts_max ) );
	TABLE_END( opf );
}

/** @brief top10membershandler
 *
 *  HTML handler helper for top10members
 *
 *  @param none
 *
 *  @return none
 */

static void top10membershandler( channelstat *cs, const void *v )
{
	os_fprintf( opf, "<tr><td>%s</td><td align=right>%d</td></tr>\n",
		cs->name, cs->c->users );
}

/** @brief html_channeltop10members
 *
 *  HTML handler for channeltop10members
 *
 *  @param none
 *
 *  @return none
 */

static void html_channeltop10members( void )
{
	TABLE_START( opf );
	os_fprintf( opf, "<tr><th>Channel</th><th align=right>Members</th></tr>" );
	GetChannelStats( top10membershandler, CHANNEL_SORT_MEMBERS, 10, 1, NULL );
	TABLE_END( opf );
}

/** @brief top10joinshandler
 *
 *  HTML handler helper for top10joins
 *
 *  @param none
 *
 *  @return none
 */

static void top10joinshandler( channelstat *cs, const void *v )
{
	os_fprintf( opf, "<tr><td>%s</td><td align=right>%d</td></tr>\n",
		cs->name, cs->users.alltime.runningtotal );
}

/** @brief html_channeltop10joins
 *
 *  HTML handler for channeltop10joins
 *
 *  @param none
 *
 *  @return none
 */

static void html_channeltop10joins( void )
{
	TABLE_START( opf );
	os_fprintf( opf, "<tr><th>Channel</th><th align=right>Total Joins</th></tr>" );
	GetChannelStats( top10joinshandler, CHANNEL_SORT_JOINS, 10, 1, NULL );
	TABLE_END( opf );
}

/** @brief top10kickshandler
 *
 *  HTML handler helper for top10kicks
 *
 *  @param none
 *
 *  @return none
 */

static void top10kickshandler( channelstat *cs, const void *v )
{
	os_fprintf( opf, "<tr><td>%s</td><td align=right>%d</td></tr>\n",
		cs->name, cs->kicks.alltime.runningtotal );
}

/** @brief html_channeltop10kicks
 *
 *  HTML handler for channeltop10kicks
 *
 *  @param none
 *
 *  @return none
 */

static void html_channeltop10kicks( void )
{
	TABLE_START( opf );
	os_fprintf( opf, "<tr><th>Channel</th><th>Total Kicks</th></tr>" );
	GetChannelStats( top10kickshandler, CHANNEL_SORT_KICKS, 10, 1, NULL );
	TABLE_END( opf );
}

/** @brief top10topicshandler
 *
 *  HTML handler helper for top10topics
 *
 *  @param none
 *
 *  @return none
 */

static void top10topicshandler( channelstat *cs, const void *v )
{
	os_fprintf( opf, "<tr><td>%s</td><td align=right>%d</td></tr>\n",
		cs->name, cs->topics.alltime.runningtotal );
}

/** @brief html_channeltop10topics
 *
 *  HTML handler for channeltop10topics
 *
 *  @param none
 *
 *  @return none
 */

static void html_channeltop10topics( void )
{
	TABLE_START( opf );
	os_fprintf( opf, "<tr><th>Channel</th><th>Total Topics</th></tr>" );
	GetChannelStats( top10topicshandler, CHANNEL_SORT_TOPICS, 10, 1, NULL );
	TABLE_END( opf );
}

/** @brief HTMLClientVersionReport
 *
 *  HTML handler helper for ClientVersionReport
 *
 *  @param none
 *
 *  @return none
 */

static void HTMLClientVersionReport( const ss_ctcp_version *cv, const void *v )
{
	os_fprintf( opf, "<tr><td>%s</td><td align=right>%d</td></tr>\n",
		cv->name, cv->users.current );
}

/** @brief html_clientstats
 *
 *  HTML handler for clientstats
 *
 *  @param none
 *
 *  @return none
 */

static void html_clientstats( void )
{
	TABLE_START( opf );
	os_fprintf( opf, "<tr><th>Version</th><th align=right>Count</th></tr>" );
	GetClientStats( HTMLClientVersionReport, 10, NULL );
	TABLE_END( opf );
}

/** @brief HTMLTLDReport
 *
 *  HTML handler helper for TLDReport
 *
 *  @param none
 *
 *  @return none
 */

static void HTMLTLDReport( const TLD *tld, const void *v )
{
	os_fprintf( opf, "<tr><td>%s</td><td>%s</td><td>%3d</td><td>%3d</td><td>%3d</td><td>%3d</td><td>%3d</td></tr>",
		tld->tld, tld->country, tld->users.current, tld->users.daily.runningtotal, tld->users.weekly.runningtotal, tld->users.monthly.runningtotal, tld->users.alltime.runningtotal );
}

/** @brief html_tldmap
 *
 *  HTML handler for tldmap
 *
 *  @param none
 *
 *  @return none
 */

static void html_tldmap( void )
{
	TABLE_START( opf );
	os_fprintf( opf, "<tr><th>tld</th><th>Country</th><th>Current</th><th>Day</th><th>Week</th><th>Month</th><th>All Time</th></tr>" );
	GetTLDStats( HTMLTLDReport, NULL );
	TABLE_END( opf );
}

/** @brief HTMLMapHandler
 *
 *  HTML handler to output network map
 *
 *  @param s pointer to server
 *  @param isroot whether this server is root
 *  @param depth of server in map
 *  @param v not used
 *
 *  @return none
 */

static void HTMLMapHandler( const Client *s, int isroot, int depth, void *v )
{
#define MAPBUFSIZE 512
	static char buf[MAPBUFSIZE];
	serverstat *ss;

	ss = ( serverstat * ) GetServerModValue( s );
	if( isroot )
	{
		/* its the root server */
		os_fprintf( opf, "<tr><td>%s</td><td>%d/%d</td><td>%d/%d</td><td>%d/%d</td></tr>\n",
			ss->name, s->server->users, ss->users.alltime.max, ss->opers.current, ss->opers.alltime.max,
			(int)s->server->ping, ( int )ss->highest_ping );
	}
	else
	{
		/* its not the root server */
		if( StatServ.flatmap )
		{
			os_fprintf( opf, "<tr><td>%s</td><td>%d/%d</td><td>%d/%d</td><td>%d/%d</td></tr>\n",
				ss->name, s->server->users, ss->users.alltime.max, ss->opers.current, ss->opers.alltime.max,
				(int)s->server->ping, ( int )ss->highest_ping );
		}
		else
		{
			buf[0]='\0';
			for( ; depth > 1; depth-- )
				strlcat( buf, "&nbsp&nbsp&nbsp&nbsp&nbsp|", MAPBUFSIZE );
			os_fprintf( opf, "<tr><td>%s\\_%s</td><td>%d/%d</td><td>%d/%d</td><td>%d/%d</td></tr>\n",
				buf, ss->name, s->server->users, ss->users.alltime.max, ss->opers.current, ss->opers.alltime.max,
				(int)s->server->ping, ( int )ss->highest_ping );
		}
	}
}

/** @brief html_map
 *
 *  HTML handler for map
 *
 *  @param none
 *
 *  @return none
 */

static void html_map( void )
{
	TABLE_START( opf );
	os_fprintf( opf, "<tr><th>Server Name</th><th>Users/Max</th><th>Opers/Max</th><th>Ping/Max</th></tr>" );
	ProcessServerMap( HTMLMapHandler, StatServ.exclusions, NULL );
	TABLE_END( opf );
}

/** @brief HTMLOutput
 *
 *  output HTML file
 *
 *  @param none
 *
 *  @return none
 */

void HTMLOutput( void )
{
#define HTMLREADBUFSIZE 512
	static char buf[HTMLREADBUFSIZE];
	FILE *tpl;
	char *buftemp;
	char *bufptr;
	htmlfunc* htmlfuncptr;

	tpl = os_fopen( html_template, "rt" );
	if( !tpl ) {
		nlog( LOG_WARNING, "Failed to open StatServ HTML template %s.", html_template );
		irc_chanalert( statbot, "Failed to open StatServ HTML template %s.", html_template );
		return;
	}
	opf = os_fopen( StatServ.htmlpath, "wt" );
	if( !opf ) {
		nlog( LOG_WARNING, "Failed to open HTML output file %s. Check file permissions.", StatServ.htmlpath );
		irc_chanalert( statbot, "Failed to open HTML output file %s. Check file permissions.", StatServ.htmlpath );
		return;
	}
	while( os_fgets( buf, HTMLREADBUFSIZE, tpl ) != NULL )
	{
		bufptr = buf;
		htmlfuncptr = htmlfuncs;
		while( htmlfuncptr->directive != NULL )
		{
			buftemp = strstr( bufptr, htmlfuncptr->directive );
			if( buftemp ) {
				os_fwrite( bufptr, ( int )buftemp -( int )bufptr, 1, opf );
				htmlfuncptr->handler();
				bufptr = buftemp + strlen( htmlfuncptr->directive );
			}		
			htmlfuncptr++;
		}
		os_fputs( bufptr, opf );
	}
	os_fclose( tpl );
	os_fclose( opf );
    /* update the umode so others can read it and owner can overwrite it */
    os_chmod( StatServ.htmlpath, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
}

/** @brief HTMLOutputTimer
 *
 *  Timer handler to output HTML file
 *
 *  @param none
 *
 *  @return none
 */
int HTMLOutputTimer( void *userptr )
{
	HTMLOutput();
	return NS_SUCCESS;
}

/** @brief ss_cmd_forcehtml
 *
 *  FORCEHTML command handler
 *  Reports current statistics to requesting user
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ss_cmd_forcehtml( const CmdParams *cmdparams )
{
	nlog( LOG_NOTICE, "%s!%s@%s forced an update of the HTML file.",
		cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->hostname );
	HTMLOutput();
	return NS_SUCCESS;
}
