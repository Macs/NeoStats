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
** $Id: statserv.c 3294 2008-02-24 02:45:41Z Fish $
*/

/*  TODO:
 *  - Identify bootup and netjoin stats and handle differently
 *    to improve accuracy
 */

#include "neostats.h"
#include "statserv.h"
#include "stats.h"
#include "network.h"
#include "server.h"
#include "channel.h"
#include "user.h"
#include "version.h"
#include "tld.h"
#include "htmlstats.h"

tStatServ StatServ;

/** SET callback prototypes */
static int ss_set_htmltime_cb( const CmdParams *cmdparams, SET_REASON reason );
static int ss_set_exclusions_cb( const CmdParams *cmdparams, SET_REASON reason );
static int ss_set_html_cb( const CmdParams *cmdparams, SET_REASON reason );
static int ss_set_htmlpath_cb( const CmdParams *cmdparams, SET_REASON reason );

/** Bot pointer */
Bot *statbot;

/** Module Events */
ModuleEvent module_events[] = {
	{EVENT_PONG,			ss_event_pong,			EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_SERVER,			ss_event_server,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_SQUIT,			ss_event_squit,			EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_SIGNON,			ss_event_signon,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_NICKIP,			ss_event_nickip,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_UMODE,			ss_event_mode,			EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_QUIT,			ss_event_quit,			EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_GLOBALKILL,		ss_event_globalkill,	EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_SERVERKILL,		ss_event_serverkill,	EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_NEWCHAN,			ss_event_newchan,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_DELCHAN,			ss_event_delchan,		EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_JOIN,			ss_event_join,			EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_PART,			ss_event_part,			EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_KICK,			ss_event_kick,			EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_TOPIC,			ss_event_topic,			EVENT_FLAG_IGNORE_SYNCH},
	{EVENT_CTCPVERSIONRPLBC,ss_event_ctcpversionbc,	EVENT_FLAG_IGNORE_SYNCH},
	NS_EVENT_END()
};

/** Copyright info */
static const char *ns_copyright[] = {
	"Copyright (c) 1999-2008, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo module_info = {
	"StatServ",
	"Network statistical service",
	ns_copyright,
	ss_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	MODULE_FLAG_CTCP_VERSION,
	0,
	0,
};

/** Bot command table */
static bot_cmd ss_commands[]=
{
	{"SERVER",		ss_cmd_server,		1, 	0,		ss_help_server},
	{"MAP",			ss_cmd_map,			0, 	0,		ss_help_map},
	{"CHANNEL",		ss_cmd_channel,		0, 	0,		ss_help_channel},
	{"NETSTATS",	ss_cmd_netstats,	0, 	0,		ss_help_netstats},
	{"DAILY",		ss_cmd_daily,		0, 	0,		ss_help_daily},
	{"TLDMAP",		ss_cmd_tldmap,		0, 	0,		ss_help_tldmap},
	{"OPERLIST",	ss_cmd_operlist,	0, 	0,		ss_help_operlist},
	{"BOTLIST",		ss_cmd_botlist,		0, 	0,		ss_help_botlist},
	{"CTCPVERSION",	ss_cmd_ctcpversion,	0,	0,		ss_help_ctcpversion},
	{"FORCEHTML",	ss_cmd_forcehtml,	0, 	NS_ULEVEL_ADMIN,	ss_help_forcehtml},
	NS_CMD_END()
};

/** Bot setting table */
static bot_setting ss_settings[]=
{
	{"HTML",		&StatServ.html,			SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, NULL,		ss_help_set_html, ss_set_html_cb, ( void * )0},
	{"HTMLPATH",	&StatServ.htmlpath,		SET_TYPE_STRING,	0, MAXPATH,		NS_ULEVEL_ADMIN, NULL,		ss_help_set_htmlpath, ss_set_htmlpath_cb, ( void * )""},
	{"HTMLTIME",	&StatServ.htmltime,		SET_TYPE_INT,		600, TS_ONE_HOUR,		NS_ULEVEL_ADMIN, "seconds",	ss_help_set_htmltime, ss_set_htmltime_cb, ( void* )TS_ONE_HOUR},
	{"CHANNELTIME",	&StatServ.channeltime,	SET_TYPE_INT,		TS_ONE_DAY, 18144000,NS_ULEVEL_ADMIN, "seconds",	ss_help_set_channeltime, NULL, ( void* )604800},
	{"MSGINTERVAL",	&StatServ.msginterval,	SET_TYPE_INT,		1, 99, 			NS_ULEVEL_ADMIN, "seconds",	ss_help_set_msginterval, NULL, ( void * )TS_ONE_MINUTE},
	{"MSGLIMIT",	&StatServ.msglimit,		SET_TYPE_INT,		1, 99, 			NS_ULEVEL_ADMIN, NULL,		ss_help_set_msglimit, NULL, ( void * )5},
	{"LAGTIME",		&StatServ.lagtime,		SET_TYPE_INT,		1, 256,			NS_ULEVEL_ADMIN, "seconds",	ss_help_set_lagtime, NULL, ( void * )30},
	{"LAGALERT",	&StatServ.lagalert,		SET_TYPE_INT,		0, 3, 			NS_ULEVEL_ADMIN, NULL,		ss_help_set_lagalert, NULL, ( void * )1},
	{"RECORDALERT", &StatServ.recordalert,	SET_TYPE_INT,		0, 3, 			NS_ULEVEL_ADMIN, NULL,		ss_help_set_recordalert, NULL, ( void * )1},
	{"EXCLUSIONS",	&StatServ.exclusions,	SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, NULL,		ss_help_set_exclusions, ss_set_exclusions_cb, ( void * )1},
	{"FLATMAP",		&StatServ.flatmap,		SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, NULL,		ss_help_set_flatmap, NULL, ( void * )0},
	NS_SETTING_END()
};

/** BotInfo */
static BotInfo statbotinfo = 
{
	"StatServ", 
	"StatServ1", 
	"SS", 
	BOT_COMMON_HOST, 
	"Statistics service", 
	BOT_FLAG_ROOT|BOT_FLAG_ONLY_OPERS|BOT_FLAG_DEAF, 
	ss_commands, 
	ss_settings,
};

/** @brief SaveStatsTimer
 *
 *  Save stats timer handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int SaveStatsTimer( void *userptr )
{
	SET_SEGV_LOCATION();
	SaveServerStats();
	SaveChanStatsProgressive();
	SaveNetworkStats();
	return NS_SUCCESS;
}

/** @brief ModInit
 *
 *  Init handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit( void )
{
	SET_SEGV_LOCATION();
	ModuleConfig( ss_settings );
	if( StatServ.html && StatServ.htmlpath[0] == 0 )
	{
		nlog( LOG_NOTICE, "HTML stats disabled as HTML_PATH is not set" );
		StatServ.html = 0;
	}
	InitNetworkStats();
	if( InitChannelStats() == NS_FAILURE )
		return NS_FAILURE;
	if( InitServerStats() == NS_FAILURE )
		return NS_FAILURE;
	if( InitVersionStats() == NS_FAILURE )
		return NS_FAILURE;
	if( InitTLDStatistics() == NS_FAILURE )
		return NS_FAILURE;
	InitUserStats();	
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *  Introduce bot onto network
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch( void )
{
	SET_SEGV_LOCATION();
	statbot = AddBot( &statbotinfo );
	if( !statbot )
		return NS_FAILURE;
	/* Timer to save the database */
	AddTimer( TIMER_TYPE_INTERVAL, SaveStatsTimer, "SaveStatsTimer", DBSAVETIME, NULL );
	/* Timer to output html */
	if( StatServ.html )
	{
		AddTimer( TIMER_TYPE_INTERVAL, HTMLOutputTimer, "HTMLOutputTimer", StatServ.htmltime, NULL );
		/* Initial output at load */
		HTMLOutput();
	}
	/* Timer to reset timeslice stats */
	AddTimer( TIMER_TYPE_DAILY, ResetStatisticsTimer, "ResetStatisticsTimer", 0, NULL );
	/* Timer to average stats */
	AddTimer( TIMER_TYPE_INTERVAL, AverageStatisticsTimer, "AverageStatisticsTimer", TS_ONE_HOUR, NULL );
	/* Initial average at load */
	AverageStatistics();
	/* Timer to delete old channels */
	AddTimer( TIMER_TYPE_INTERVAL, DelOldChanTimer, "DelOldChanTimer", TS_ONE_HOUR, NULL );
	return NS_SUCCESS;
}

/** @brief ModFini
 *
 *  Fini handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModFini( void )
{
	FiniServerStats();
	FiniChannelStats();
	FiniTLDStatistics();
	FiniVersionStats();
	FiniNetworkStats();
	return NS_SUCCESS;
}

/** @brief ss_set_html_cb
 *
 *  Set callback for SET HTML
 *  Enable or disable html output
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int ss_set_html_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_CHANGE )
	{
		if( StatServ.html && StatServ.htmlpath[0] == 0 )
		{
			irc_prefmsg ( statbot, cmdparams->source, 
				"You need to SET HTMLPATH. HTML output disabled." );
			StatServ.html = 0;
			return NS_SUCCESS;
		}
		if( StatServ.html )
			AddTimer( TIMER_TYPE_INTERVAL, HTMLOutputTimer, "HTMLOutputTimer", StatServ.htmltime, NULL );
		else
			DelTimer( "HTMLOutputTimer" );
	}
	return NS_SUCCESS;
}

/** @brief ss_set_htmlpath_cb
 *
 *  Set callback for SET HTMLPATH
 *  Change html output path
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int ss_set_htmlpath_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	FILE *opf;

	if( reason == SET_CHANGE )
	{
		opf = os_fopen( StatServ.htmlpath, "wt" );
		if( !opf )
		{
			irc_prefmsg( statbot, cmdparams->source, 
				"Failed to open HTML output file %s. Check file permissions. HTML output disabled.", StatServ.htmlpath );
			return NS_SUCCESS;
		}
		os_fclose( opf );
		HTMLOutput();
	}
	return NS_SUCCESS;
}

/** @brief ss_set_htmltime_cb
 *
 *  Set callback for SET HTMLTIME
 *  Change html output time
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int ss_set_htmltime_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_CHANGE )
	{
		SetTimerInterval( "HTMLOutputTimer", StatServ.htmltime );
	}
	return NS_SUCCESS;
}

/** @brief ss_set_exclusions_cb
 *
 *  Set callback for exclusions
 *  Enable or disable exclude event flag
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int ss_set_exclusions_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_LOAD || reason == SET_CHANGE )
	{
		SetAllEventFlags( EVENT_FLAG_USE_EXCLUDE, StatServ.exclusions );
	}
	return NS_SUCCESS;
}
