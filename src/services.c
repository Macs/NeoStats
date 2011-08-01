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
** $Id: services.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "main.h"
#include "modules.h"
#include "nsevents.h"
#include "bots.h"
#include "timer.h"
#include "sock.h"
#include "helpstrings.h"
#include "users.h"
#include "servers.h"
#include "channels.h"
#include "ircprotocol.h"
#include "exclude.h"
#include "services.h"
#include "bans.h"
#include "auth.h"
#include "updates.h"
/* Command function prototypes */
static int ns_cmd_shutdown( const CmdParams *cmdparams );
#ifndef WIN32
static int ns_cmd_reload( const CmdParams *cmdparams );
#endif /* !WIN32 */
static int ns_cmd_jupe( const CmdParams *cmdparams );
#ifdef USE_RAW
static int ns_cmd_raw( const CmdParams *cmdparams );
#endif /* USE_RAW */
static int ns_cmd_status( const CmdParams *cmdparams );

#ifndef DEBUG
static int ns_set_debug_cb( const CmdParams *cmdparams, SET_REASON reason );
#endif /* DEBUG */
static int ns_set_servicecmode_cb(const CmdParams *cmdparams, SET_REASON reason);
static int ns_set_pingtime_cb(const CmdParams *cmdparams, SET_REASON reason);
config nsconfig;
tme me;

static char quitmsg[BUFSIZE];

static const char *ns_about[] = {
	"\2NeoStats\2 statistical services",
	NULL
};

/** Copyright info */
static const char *ns_copyright[] = {
	"Copyright (c) 1999-2008, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo ns_module_info = {
	"NeoStats",
	"NeoStats Statistical services", 	
	ns_copyright,
	ns_about,
	NEOSTATS_VERSION,
	NEOSTATS_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
	0,
};

/** Fake Module pointer for run level code */
Module ns_module = {
	MOD_TYPE_STANDARD,
	&ns_module_info,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	0,
	0,
	0,
	0,
};

/** Bot command table */
static bot_cmd ns_commands[] =
{
	{"STATUS",		ns_cmd_status,		0, 	0,					ns_help_status, 0, NULL, NULL},
	{"SHUTDOWN",	ns_cmd_shutdown,	1, 	NS_ULEVEL_ADMIN, 	ns_help_shutdown, 0, NULL, NULL},
#ifndef WIN32
	{"RELOAD",		ns_cmd_reload,		1, 	NS_ULEVEL_ADMIN, 	ns_help_reload, 0, NULL, NULL},
#endif /* !WIN32 */
	{"MODLIST",		ns_cmd_modlist,		0, 	NS_ULEVEL_ADMIN,  	ns_help_modlist, 0, NULL, NULL},
	{"LOAD",		ns_cmd_load,		1, 	NS_ULEVEL_ADMIN, 	ns_help_load, 0, NULL, NULL},
	{"UNLOAD",		ns_cmd_unload,		1,	NS_ULEVEL_ADMIN, 	ns_help_unload, 0, NULL, NULL},
	{"JUPE",		ns_cmd_jupe,		1, 	NS_ULEVEL_ADMIN, 	ns_help_jupe, 0, NULL, NULL},
#ifdef USE_RAW
	{"RAW",			ns_cmd_raw,			0, 	NS_ULEVEL_ADMIN, 	ns_help_raw, 0, NULL, NULL},
#endif /* USE_RAW */
	NS_CMD_END()
};

/** Bot command table */
static bot_cmd ns_debug_commands[] =
{
	{"BOTLIST",		ns_cmd_botlist,		0, 	NS_ULEVEL_ROOT,  	ns_help_botlist, 0, NULL, NULL},
	{"SOCKLIST",	ns_cmd_socklist,	0, 	NS_ULEVEL_ROOT,  	ns_help_socklist, 0, NULL, NULL},
	{"TIMERLIST",	ns_cmd_timerlist,	0, 	NS_ULEVEL_ROOT,  	ns_help_timerlist, 0, NULL, NULL},
	{"USERLIST",	ns_cmd_userlist,	0, 	NS_ULEVEL_ROOT,  	ns_help_userlist, 0, NULL, NULL},
	{"CHANNELLIST",	ns_cmd_channellist,	0, 	NS_ULEVEL_ROOT,  	ns_help_channellist, 0, NULL, NULL},
	{"SERVERLIST",	ns_cmd_serverlist,	0, 	NS_ULEVEL_ROOT,  	ns_help_serverlist, 0, NULL, NULL},
	{"BANLIST",		ns_cmd_banlist,		0, 	NS_ULEVEL_ROOT,  	ns_help_banlist, 0, NULL, NULL},
	NS_CMD_END()
};

/** Bot setting table */
static bot_setting ns_settings[] =
{
	{"MSGSAMPLETIME",	&nsconfig.msgsampletime,	SET_TYPE_INT,		1,	100,		NS_ULEVEL_ADMIN, NULL,	ns_help_set_msgsampletime, NULL, ( void * )10 },
	{"MSGTHRESHOLD",	&nsconfig.msgthreshold,		SET_TYPE_INT,		1,	100,		NS_ULEVEL_ADMIN, NULL,	ns_help_set_msgthreshold, NULL, ( void * )5 },
	{"SPLITTIME",		&nsconfig.splittime,		SET_TYPE_INT,		0,	1000,		NS_ULEVEL_ADMIN, NULL,	ns_help_set_splittime, NULL, ( void * )300 },
	{"JOINSERVICESCHAN",&nsconfig.joinserviceschan, SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_joinserviceschan, NULL, ( void* )1 },
	{"PINGTIME",		&nsconfig.pingtime,			SET_TYPE_INT,		0, 65534, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_pingtime, ns_set_pingtime_cb, ( void* )120 },
	{"SERVICECMODE",	me.servicescmode,			SET_TYPE_STRING,	0, MODESIZE, 	NS_ULEVEL_ADMIN, NULL,	ns_help_set_servicecmode, ns_set_servicecmode_cb, NULL },
	{"SERVICEUMODE",	me.servicesumode,			SET_TYPE_STRING,	0, MODESIZE, 	NS_ULEVEL_ADMIN, NULL,	ns_help_set_serviceumode, NULL, NULL },
	{"CMDCHAR",			nsconfig.cmdchar,			SET_TYPE_STRING,	0, 2, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_cmdchar, NULL, ( void* )"!" },
	{"CMDREPORT",		&nsconfig.cmdreport,		SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_cmdreport, NULL, ( void* )1 },
	{"LOGLEVEL",		&nsconfig.loglevel,			SET_TYPE_INT,		1, 6, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_loglevel, NULL, ( void* )5 },
	{"RECVQ",			&nsconfig.recvq,			SET_TYPE_INT,		1024,10240000,		NS_ULEVEL_ADMIN, NULL,	ns_help_set_recvq, NULL, ( void*)2048}, 
	{"DEBUGCHAN",		nsconfig.debugchan,			SET_TYPE_STRING,	0, 	MAXCHANLEN, 	NS_ULEVEL_ADMIN, NULL,	ns_help_set_debugchan, NULL, ( void* )"#debug" },
	{"SENDHELP",		&nsconfig.sendhelp,			SET_TYPE_BOOLEAN,	0,	0,		NS_ULEVEL_ADMIN, NULL,  ns_help_set_sendhelp, NULL, (void *)1},
	{"ALLHELP",		&nsconfig.allhelp,			SET_TYPE_BOOLEAN,	0,	0,		NS_ULEVEL_ADMIN, NULL,  ns_help_set_allhelp, NULL, (void *)0},
	NS_SETTING_END()
};

/** Bot debug setting table */
static bot_setting ns_debugsettings[] =
{
#ifndef DEBUG
	{"DEBUG",			&nsconfig.debug,			SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_debug, ns_set_debug_cb, ( void* )0 },
#endif /* DEBUG */
	{"DEBUGMODULE",		nsconfig.debugmodule,		SET_TYPE_STRING,	0, MAX_MOD_NAME,NS_ULEVEL_ADMIN, NULL,	ns_help_set_debugmodule, NULL, ( void* )"all" },
	{"DEBUGLEVEL",		&nsconfig.debuglevel,		SET_TYPE_INT,		1, 10, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_debuglevel, NULL, ( void* )0 },
	{"DEBUGTOCHAN",		&nsconfig.debugtochan,		SET_TYPE_BOOLEAN,	0, 0, 			NS_ULEVEL_ADMIN, NULL,	ns_help_set_debugtochan, NULL, ( void* )0 },
	NS_SETTING_END()
};

/** Bot pointer */
Bot *ns_botptr = NULL;

/** Core bot info */
static BotInfo ns_botinfo =
{
	"NeoStats",
	"NeoStats1",
	"Neo",
	BOT_COMMON_HOST,
	"/msg NeoStats \2HELP\2",
	/* 0x80000000 is a "hidden" flag to identify the core bot */
	0x80000000|BOT_FLAG_ONLY_OPERS|BOT_FLAG_ROOT|BOT_FLAG_DEAF,
	ns_commands, 
	ns_settings,
};

/** Core event table */
static ModuleEvent neostats_events[] =
{
	NS_EVENT_END()
};

/** @brief InitServices
 *
 *  init NeoStats core
 *
 *  @param none
 *
 *  @return none
 */

void InitServices( void )
{
	InitExcludes( &ns_module );
	me.s = AddServer( me.name, NULL, 0, NULL, me.infoline );
	strlcpy( me.s->version, me.version, VERSIONSIZE );
	ModuleConfig( ns_settings );
}

/** @brief FiniServices
 *
 *  fini NeoStats core
 *
 *  @param none
 *
 *  @return none
 */

void FiniServices( void )
{
	FreeEventList( &ns_module );
}

/** @brief init_services_bot
 *
 *  init NeoStats bot
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int init_services_bot( void )
{
	SET_SEGV_LOCATION();
	strlcpy( ns_botinfo.nick, me.rootnick, MAXNICK );
	ircsnprintf( ns_botinfo.altnick, MAXNICK, "%s1", me.rootnick );
	ircsnprintf( ns_botinfo.realname, MAXREALNAME, "/msg %s \2HELP\2", ns_botinfo.nick );
	if( nsconfig.onlyopers ) 
		ns_botinfo.flags |= BOT_FLAG_ONLY_OPERS;
	SetModuleInSynch( &ns_module );
	ns_botptr = AddBot( &ns_botinfo );
	add_services_set_list( ns_debugsettings );
	add_services_cmd_list( ns_module.exclude_cmd_list );
	if( nsconfig.debug )
		add_services_cmd_list( ns_debug_commands );
	InitAuthCommands();
	AddEventList( neostats_events );
	SetModuleSynched( &ns_module );
	me.synched = 1;
	SynchAllModules();
	RequestServerUptimes();	
	return NS_SUCCESS;
}

/** @brief ns_cmd_shutdown
 *
 *  SHUTDOWN command handler
 *  Shutdown NeoStats
 *   
 *  @param cmdparams structure with command information
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ns_cmd_shutdown( const CmdParams *cmdparams )
{
	char *message;

	SET_SEGV_LOCATION();
	message = joinbuf( cmdparams->av, cmdparams->ac, 0 );
	irc_chanalert( ns_botptr, _( "%s requested SHUTDOWN for %s" ), cmdparams->source->name, message );
	ircsnprintf( quitmsg, BUFSIZE, _( "%s [%s] (%s) requested SHUTDOWN for %s." ), 
		cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->hostname, message );
	ns_free( message );
	irc_globops( ns_botptr, "%s", quitmsg );
	nlog( LOG_NOTICE, "%s", quitmsg );
	do_exit( NS_EXIT_NORMAL, quitmsg );
   	return NS_SUCCESS;
}

#ifndef WIN32
/** @brief ns_cmd_reload
 *
 *  RELOAD command handler
 *  Reload NeoStats
 *   
 *  @param cmdparams structure with command information
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ns_cmd_reload( const CmdParams *cmdparams )
{
	char *message;

	SET_SEGV_LOCATION();
	message = joinbuf( cmdparams->av, cmdparams->ac, 0 );
	irc_chanalert( ns_botptr, _( "%s requested RELOAD for %s" ), cmdparams->source->name, message );
	ircsnprintf( quitmsg, BUFSIZE, _( "%s [%s] (%s) requested RELOAD for %s." ), 
		cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->hostname, message );
	ns_free( message );
	irc_globops( ns_botptr, "%s", quitmsg );
	nlog( LOG_NOTICE, "%s", quitmsg );
	do_exit( NS_EXIT_RELOAD, quitmsg );
   	return NS_SUCCESS;
}
#endif /* !WIN32 */

/** @brief ns_cmd_jupe
 *
 *  JUPE command handler
 *  Jupiter a server
 *   
 *  @param cmdparams structure with command information
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ns_cmd_jupe( const CmdParams *cmdparams )
{
	static char infoline[255];

	SET_SEGV_LOCATION();
	ircsnprintf( infoline, 255, "[jupitered by %s]", cmdparams->source->name );
	irc_server( cmdparams->av[0], 1, infoline );
	nlog( LOG_NOTICE, "%s!%s@%s jupitered %s", cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->hostname, cmdparams->av[0] );
	irc_chanalert( ns_botptr, _( "%s jupitered %s" ), cmdparams->source->name, cmdparams->av[0] );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "%s has been jupitered", cmdparams->source ), cmdparams->av[0] );
   	return NS_SUCCESS;
}

/** @brief ns_cmd_status
 *
 *  STATUS command handler
 *  Display NeoStats status
 *   
 *  @param cmdparams structure with command information
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int ns_cmd_status( const CmdParams *cmdparams )
{
	time_t uptime = me.now - me.ts_boot;

	SET_SEGV_LOCATION();
	irc_prefmsg( ns_botptr, cmdparams->source, __( "%s status:", cmdparams->source ), ns_botptr->name );
	if( uptime > TS_ONE_DAY )
		irc_prefmsg( ns_botptr, cmdparams->source, __( "%s up \2%ld\2 day%s, \2%02ld:%02ld\2", cmdparams->source ), ns_botptr->name, uptime / TS_ONE_DAY, ( uptime / TS_ONE_DAY == 1 ) ? "" : "s", ( uptime / TS_ONE_HOUR ) % 24, ( uptime / TS_ONE_MINUTE ) % TS_ONE_MINUTE );
	else if( uptime > TS_ONE_HOUR )
		irc_prefmsg( ns_botptr, cmdparams->source, __( "%s up \2%ld hour%s, %ld minute%s\2", cmdparams->source ), ns_botptr->name, uptime / TS_ONE_HOUR, uptime / TS_ONE_HOUR == 1 ? "" : "s", ( uptime / TS_ONE_MINUTE ) % TS_ONE_MINUTE, ( uptime / 60 ) % TS_ONE_MINUTE == 1 ? "" : "s" );
	else if( uptime > TS_ONE_MINUTE )
		irc_prefmsg( ns_botptr, cmdparams->source, __( "%s up \2%ld minute%s, %ld second%s\2", cmdparams->source ), ns_botptr->name, uptime / TS_ONE_MINUTE, uptime / TS_ONE_MINUTE == 1 ? "" : "s", uptime % TS_ONE_MINUTE, uptime % TS_ONE_MINUTE == 1 ? "" : "s" );
	else
		irc_prefmsg( ns_botptr, cmdparams->source, __( "%s up \2%d second%s\2", cmdparams->source ), ns_botptr->name, ( int )uptime, uptime == 1 ? "" : "s" );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Sent %ld messages, %ld bytes", cmdparams->source ), me.SendM, me.SendBytes );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Received %ld messages, %ld bytes", cmdparams->source ), me.RcveM, me.RcveBytes );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Reconnect time: %d", cmdparams->source ), nsconfig.r_time );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Requests: %d",cmdparams->source ), me.requests );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Max sockets: %d( in use: %d )", cmdparams->source ), me.maxsocks, me.cursocks );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Current servers: %d", cmdparams->source ), NSGetServerCount() );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Current channels: %d", cmdparams->source ), NSGetChannelCount() );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Current users: %d( Away: %d )", cmdparams->source ), NSGetUserCount(), NSGetAwayCount() );
	if( nsconfig.debug )
		irc_prefmsg( ns_botptr, cmdparams->source, __( "Debugging mode enabled", cmdparams->source ) );
	else
		irc_prefmsg( ns_botptr, cmdparams->source, __( "Debugging mode disabled", cmdparams->source ) );
#ifndef WIN32
	MQStatusMsg(ns_botptr, cmdparams);
#endif
	return NS_SUCCESS;
}

#ifdef USE_RAW
/** @brief ns_cmd_raw
 *
 *  RAW command handler
 *   
 *  @param cmdparams structure with command information
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
/* forward decleration */
void send_cmd( const char *fmt, ... );

static int ns_cmd_raw( const CmdParams *cmdparams )
{
	char *message;

	SET_SEGV_LOCATION();
	message = joinbuf( cmdparams->av, cmdparams->ac, 0 );
	irc_chanalert( ns_botptr, _( "\2RAW COMMAND\2 \2%s\2 issued a raw command! (%s)" ), cmdparams->source->name, message );
	nlog( LOG_NORMAL, "RAW COMMAND %s issued a raw command! (%s)", cmdparams->source->name, message );
	send_cmd( "%s", message );
	ns_free( message );
   	return NS_SUCCESS;
}
#endif /* USE_RAW */

#ifndef DEBUG
/** @brief ns_set_debug_cb
 *
 *  Set callback for debug
 *  Enable or disable debug commands
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int ns_set_debug_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_LOAD || reason == SET_CHANGE )
	{
		if( nsconfig.debug )
			add_services_cmd_list( ns_debug_commands );
		else 
			add_services_cmd_list( ns_debug_commands );
	}
	return NS_SUCCESS;
}
#endif /* DEBUG */

/** @brief ns_set_servicecmode_cb
 *
 *  Set callback for SERVICECMODE
 *  Checks the CMODE string starts with + and only contains 1 mode (as currently we don't support multiple modes
 *  @TODO support multiple modes
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int ns_set_servicecmode_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_VALIDATE )
	{
		if (strlen(cmdparams->av[1]) > (MODESIZE-1)) {
			irc_prefmsg(ns_botptr, cmdparams->source, "SERVICECMODE only supports %d modes", (MODESIZE-1));
			return NS_FAILURE;
		} else if (cmdparams->av[1][0] != '+') {
			irc_prefmsg(ns_botptr, cmdparams->source, "SERVICECMODE must start with a +");
			return NS_FAILURE;
		}
	}
	return NS_SUCCESS;
}

static int ns_set_pingtime_cb(const CmdParams *cmdparams, SET_REASON reason)
{
	if( reason == SET_LOAD || reason == SET_CHANGE ) {
		SetTimerInterval("PingServers", nsconfig.pingtime);
		SetTimerInterval("FlushLogs", nsconfig.pingtime);
	}
	return NS_SUCCESS;
}
