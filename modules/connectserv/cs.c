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
** $Id: cs.c 3294 2008-02-24 02:45:41Z Fish $
*/

/*  TODO:
 *  - Nothing at present
 */

#include "neostats.h"
#include "cs.h"

/** local structures */
/** Configuration structure */
static struct cs_cfg 
{ 
	unsigned int sign_watch;
	unsigned int kill_watch;
	unsigned int mode_watch;
	unsigned int nick_watch;
	unsigned int away_watch;
	unsigned int serv_watch;
	unsigned int exclusions;
	unsigned int logging;
	unsigned int colour;
} cs_cfg;

/** Message structure */
typedef struct msg
{
	const char *format;
	const char *formatcolour;
} msg;

/** Message format lookup indices */
enum
{
	MSG_NICKCHANGE = 0,
	MSG_AWAY,
	MSG_SIGNON,
	MSG_SIGNOFF,
	MSG_LOCALKILL,
	MSG_GLOBALKILL,
	MSG_SERVERKILL,
	MSG_MODE,
	MSG_MODE_SERV,
	MSG_BOT,
	MSG_SERVER,
	MSG_SQUIT
};

/** Message format lookup table */
static const msg msg_format[]=
{	
	{/* MSG_NICKCHANGE */
	 /* NORMAL */ "\2NICK\2 %s (%s@%s) changed their nick to %s",
	 /* COLOUR */ "\2\0037NICK\2 user: \2%s\2 (%s@%s) changed their nick to \2%s\2\003"
	},	
	{/* MSG_AWAY */
	 /* NORMAL */ "\2AWAY\2 %s (%s@%s) is %s away %s",
	 /* COLOUR */ "\2AWAY\2 %s (%s@%s) is %s away %s"
	},	
	{/* MSG_SIGNON */
	 /* NORMAL */ "\2SIGNON\2 %s (%s@%s %s) signed on at %s",
	 /* COLOUR */ "\2\0034SIGNON\2 user: \2%s\2 (%s@%s %s) at \2%s\2\003"
	},	
	{/* MSG_SIGNOFF */
	 /* NORMAL */ "\2SIGNOFF\2 %s (%s@%s %s) signed off at %s %s",
	 /* COLOUR */ "\2\0033SIGNOFF\2 user: %s (%s@%s %s) at %s %s\003"
	},	
	{/* MSG_LOCALKILL */
	 /* NORMAL */ "\2LOCAL KILL\2 %s (%s@%s) killed by %s for \2%s\2",
	 /* COLOUR */ "\2\00312LOCAL KILL\2 user: \2%s\2 (%s@%s) killed by \2%s\2 for \2%s\2\003"
	},	
	{/* MSG_GLOBALKILL */
	 /* NORMAL */ "\2GLOBAL KILL\2 %s (%s@%s) killed by %s for \2%s\2",
	 /* COLOUR */ "\2\00312GLOBAL KILL\2 user: \2%s\2 (%s@%s) killed by \2%s\2 for \2%s\2\003"
	},	
	{/* MSG_SERVERKILL */
	 /* NORMAL */ "\2SERVER KILL\2 %s (%s@%s) killed by %s for \2%s\2",
	 /* COLOUR */ "\2\00312SERVER KILL\2 user: \2%s\2 (%s@%s) killed by \2%s\2 for \2%s\2\003"
	},	
	{/* MSG_MODE */
	 /* NORMAL */ "\2MODE\2 %s is %s a %s (%c%c)",
	 /* COLOUR */ "\2\00313%s\2 is \2%s\2 a \2%s\2 (%c%c)\003"
	},	
	{/* MSG_MODE_SERV */
	 /* NORMAL */ "\2MODE\2 %s is %s a %s (%c%c) on %s",
	 /* COLOUR */ "\2\00313%s\2 is \2%s\2 a \2%s\2 (%c%c) on \2%s\2\003"
	},	
	{/* MSG_BOT */
	 /* NORMAL */ "\2BOT\2 %s is %s a Bot (%c%c)",
	 /* COLOUR */ "\2\00313BOT\2 %s is \2%s\2 a \2Bot\2 (%c%c)\003"
	},	
	{/* MSG_SERVER */
	 /* NORMAL */ "\2SERVER\2 %s joined the network at %s",
	 /* COLOUR */ "\2SERVER\2 %s joined the network at %s"
	},	
	{/* MSG_SQUIT */
	 /* NORMAL */ "\2SERVER\2 %s left the network at %s for %s",
	 /* COLOUR */ "\2SERVER\2 %s left the network at %s for %s"
	},
};

#define CS_MSG( x ) ( cs_cfg.colour == 1 ? msg_format[x].formatcolour : msg_format[x].format )

/** Bot event function prototypes */
static int cs_event_signon( const CmdParams *cmdparams );
static int cs_event_umode( const CmdParams *cmdparams );
static int cs_event_smode( const CmdParams *cmdparams );
static int cs_event_quit( const CmdParams *cmdparams );
static int cs_event_localkill( const CmdParams *cmdparams );
static int cs_event_globalkill( const CmdParams *cmdparams );
static int cs_event_serverkill( const CmdParams *cmdparams );
static int cs_event_nick( const CmdParams *cmdparams );
static int cs_event_away( const CmdParams *cmdparams );
static int cs_event_server( const CmdParams *cmdparams );
static int cs_event_squit( const CmdParams *cmdparams );

/** Set callbacks */
static int cs_set_exclusions_cb( const CmdParams *cmdparams, SET_REASON reason );
static int cs_set_sign_watch_cb( const CmdParams *cmdparams, SET_REASON reason );
static int cs_set_kill_watch_cb( const CmdParams *cmdparams, SET_REASON reason );
static int cs_set_mode_watch_cb( const CmdParams *cmdparams, SET_REASON reason );
static int cs_set_nick_watch_cb( const CmdParams *cmdparams, SET_REASON reason );
static int cs_set_away_watch_cb( const CmdParams *cmdparams, SET_REASON reason );
static int cs_set_serv_watch_cb( const CmdParams *cmdparams, SET_REASON reason );

/** Bot pointer */
static Bot *cs_bot;

/** Copyright info */
static const char *cs_copyright[] = 
{
	"Copyright (c) 1999-2008, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo module_info = 
{
	"ConnectServ",
	"Connection monitoring service",
	cs_copyright,
	cs_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
	0,
};

/** Bot setting table */
static bot_setting cs_settings[] =
{
	{"SIGNWATCH",	&cs_cfg.sign_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	cs_help_set_signwatch, cs_set_sign_watch_cb, ( void* )1 },
	{"KILLWATCH",	&cs_cfg.kill_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	cs_help_set_killwatch, cs_set_kill_watch_cb, ( void* )1 },
	{"MODEWATCH",	&cs_cfg.mode_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	cs_help_set_modewatch, cs_set_mode_watch_cb, ( void* )1 },
	{"NICKWATCH",	&cs_cfg.nick_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	cs_help_set_nickwatch, cs_set_nick_watch_cb, ( void* )1 },
	{"AWAYWATCH",	&cs_cfg.away_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	cs_help_set_awaywatch, cs_set_away_watch_cb, ( void* )1 },
	{"SERVWATCH",	&cs_cfg.serv_watch,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	cs_help_set_servwatch, cs_set_serv_watch_cb, ( void* )1 },
	{"EXCLUSIONS",	&cs_cfg.exclusions,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	cs_help_set_exclusions, cs_set_exclusions_cb, ( void* )1 },
	{"LOGGING",	&cs_cfg.logging,		SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	cs_help_set_logging, NULL, ( void* )1 },
	{"COLOUR",	&cs_cfg.colour,		SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	cs_help_set_colour, NULL, ( void* )0 },
	NS_SETTING_END()
};

/** Bot command table */
static bot_cmd cs_commands[]=
{
	NS_CMD_END()
};


/** BotInfo */
static BotInfo cs_botinfo = 
{
	"ConnectServ", 
	"ConnectServ1", 
	"CS", 
	BOT_COMMON_HOST, 
	"Connection monitoring service", 	
	BOT_FLAG_ROOT|BOT_FLAG_RESTRICT_OPERS|BOT_FLAG_DEAF, 
	cs_commands, 
	cs_settings,
};

/** Module Events */
ModuleEvent module_events[] = 
{
	{EVENT_SIGNON,		cs_event_signon,	EVENT_FLAG_EXCLUDE_ME},
	{EVENT_UMODE,		cs_event_umode,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_SMODE,		cs_event_smode,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_QUIT,		cs_event_quit,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_LOCALKILL,	cs_event_localkill,	EVENT_FLAG_EXCLUDE_ME},
	{EVENT_GLOBALKILL,	cs_event_globalkill,EVENT_FLAG_EXCLUDE_ME},
	{EVENT_SERVERKILL,	cs_event_serverkill,EVENT_FLAG_EXCLUDE_ME},
	{EVENT_NICK,		cs_event_nick,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_AWAY,		cs_event_away,		EVENT_FLAG_EXCLUDE_ME},
	{EVENT_SERVER,		cs_event_server,	EVENT_FLAG_EXCLUDE_ME},
	{EVENT_SQUIT,		cs_event_squit,		EVENT_FLAG_EXCLUDE_ME},
	NS_EVENT_END()
};

/** @brief ModInit
 *
 *  Init handler
 *  Loads connectserv configuration
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit( void )
{
	SET_SEGV_LOCATION();
	/* Load stored configuration */
	ModuleConfig( cs_settings );
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
	/* Create module bot */
	cs_bot = AddBot( &cs_botinfo );
	/* If failed to create bot, module will terminate */
	if( !cs_bot ) 
		return NS_FAILURE;
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
	SET_SEGV_LOCATION();
	return NS_SUCCESS;
}

/** @brief cs_report
 *
 *  Handle event reporting and logging if enabled
 *
 *  @param none
 *
 *  @return none
 */

static void cs_report( const char *fmt, ... )
{
	static char buf[BUFSIZE];
	va_list ap;

	va_start( ap, fmt );
	ircvsnprintf( buf, BUFSIZE, fmt, ap );
	va_end( ap );
	irc_chanalert( cs_bot, "%s", buf );
	if( cs_cfg.logging ) {
		if (cs_cfg.colour == 1)
			strip_mirc_codes(buf);
		nlog( LOG_NORMAL, "%s", buf);
}	}

/** @brief cs_event_signon
 *
 *  signon event handler
 *  report signons
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_signon( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	/* Print Connection Notice */
	cs_report( CS_MSG( MSG_SIGNON ), cmdparams->source->name, 
		cmdparams->source->user->username, cmdparams->source->user->hostname, 
		cmdparams->source->info, cmdparams->source->uplink->name );
	return NS_SUCCESS;
}

/** @brief cs_event_quit
 *
 *  quit event handler
 *  report quits
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_quit( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	/* Print Disconnection Notice */
	cs_report( CS_MSG( MSG_SIGNOFF ), cmdparams->source->name, 
		cmdparams->source->user->username, cmdparams->source->user->hostname, 
		cmdparams->source->info, cmdparams->source->uplink->name, 
		cmdparams->param );
	return NS_SUCCESS;
}

/** @brief cs_event_localkill
 *
 *  local kill event handler
 *  report local kills
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_localkill( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	cs_report( CS_MSG( MSG_LOCALKILL ), cmdparams->target->name, 
		cmdparams->target->user->username, cmdparams->target->user->hostname,
		cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

/** @brief cs_event_globalkill
 *
 *  global kill event handler
 *  report global kills
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_globalkill( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	cs_report( CS_MSG( MSG_GLOBALKILL ), cmdparams->target->name, 
		cmdparams->target->user->username, cmdparams->target->user->hostname,
		cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

/** @brief cs_event_serverkill
 *
 *  server kill event handler
 *  report server kills
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_serverkill( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	cs_report( CS_MSG( MSG_SERVERKILL ), cmdparams->target->name, 
		cmdparams->target->user->username, cmdparams->target->user->hostname,
		cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

/** @brief cs_report_mode
 *
 *  report mode changes
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return none
 */

static void cs_report_mode( const char *modedesc, const int serverflag, const Client *u, const int add, const char mode )
{
	if( serverflag ) 
	{
		cs_report( CS_MSG( MSG_MODE_SERV ), u->name, 
			add ? "now" : "no longer", 
			modedesc,
			add ? '+' : '-',
			mode, u->uplink->name );
	} 
	else 
	{
		cs_report( CS_MSG( MSG_MODE ), u->name, 
			add ? "now" : "no longer", 
			modedesc,
			add ? '+' : '-',
			mode );
	}
}

/** @brief cs_event_umode
 *
 *  umode event handler
 *  report umode changes
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_umode( const CmdParams *cmdparams )
{
	/* Mask of modes we will handle */
	static const unsigned int OperUmodes = 
		UMODE_NETADMIN |
		UMODE_TECHADMIN |
		UMODE_ADMIN |
		UMODE_COADMIN |
		UMODE_SADMIN |
		UMODE_OPER |
		UMODE_LOCOP |
		UMODE_SERVICES;
	unsigned int mask;
	int add = 1;
	const char *modes;

	SET_SEGV_LOCATION();
	modes = cmdparams->param;
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
			default:
				mask = UmodeCharToMask( *modes );
				if( mask & UMODE_BOT )
					cs_report( CS_MSG( MSG_BOT ), cmdparams->source->name, add ? "now" : "no longer", add ? '+' : '-', *modes );
				else if( OperUmodes & mask )
					cs_report_mode( GetUmodeDesc( mask ), IsServerOperMode( mask ), cmdparams->source, add, *modes );
				break;
		}
		modes++;
	}
	return NS_SUCCESS;
}

/** @brief cs_event_smode
 *
 *  smode event handler
 *  report smode changes
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_smode( const CmdParams *cmdparams )
{
	/* Mask of modes we will handle */
	static const unsigned int OperSmodes =
		SMODE_NETADMIN |
		SMODE_CONETADMIN |
		SMODE_TECHADMIN |
		SMODE_COTECHADMIN |
		SMODE_ADMIN |
		SMODE_COADMIN |
		SMODE_GUESTADMIN;
	unsigned int mask;
	int add = 1;
	const char *modes;

	SET_SEGV_LOCATION();
	modes = cmdparams->param;
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
			default:
				mask = SmodeCharToMask( *modes );
				if( OperSmodes & mask )
					cs_report_mode( GetSmodeDesc( mask ), IsServerOperSMode( mask ), cmdparams->source, add, *modes );
				break;
		}
		modes++;
	}
	return NS_SUCCESS;
}

/** @brief cs_event_nick
 *
 *  nick change event handler
 *  report nick changes
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_nick( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	cs_report( CS_MSG( MSG_NICKCHANGE ), cmdparams->param, 
		cmdparams->source->user->username, cmdparams->source->user->hostname, 
		cmdparams->source->name );
	return NS_SUCCESS;
}

/** @brief cs_event_away
 *
 *  away event handler
 *  report away
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_away( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	cs_report( CS_MSG( MSG_AWAY ), cmdparams->source->name, 
		cmdparams->source->user->username, cmdparams->source->user->hostname, 
		IsAway( cmdparams->source ) ? "now" : "no longer", cmdparams->source->user->awaymsg );
	return NS_SUCCESS;
}

/** @brief cs_event_server
 *
 *  server connect event handler
 *  report server connects
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_server( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	cs_report( CS_MSG( MSG_SERVER ), cmdparams->source->name, cmdparams->source->uplink->name );
	return NS_SUCCESS;
}

/** @brief cs_event_squit
 *
 *  server quit event handler
 *  report server quits
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_event_squit( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	cs_report( CS_MSG( MSG_SQUIT ), cmdparams->source->name, cmdparams->source->uplink->name, 
		cmdparams->param ? cmdparams->param : "reason unknown" );
	return NS_SUCCESS;
}

/** @brief cs_set_exclusions_cb
 *
 *  Set callback for exclusions
 *  Enable or disable exclude event flag
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_set_exclusions_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_LOAD || reason == SET_CHANGE )
	{
		SetAllEventFlags( EVENT_FLAG_USE_EXCLUDE, cs_cfg.exclusions );
	}
	return NS_SUCCESS;
}

/** @brief cs_set_sign_watch_cb
 *
 *  Set callback for sign watch
 *  Enable or disable events associated with sign on/off
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_set_sign_watch_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_LOAD || reason == SET_CHANGE )
	{
		if( cs_cfg.sign_watch )
		{
			EnableEvent( EVENT_SIGNON );
			EnableEvent( EVENT_QUIT );
		} 
		else 
		{
			DisableEvent( EVENT_SIGNON );
			DisableEvent( EVENT_QUIT );
		}
	}
	return NS_SUCCESS;
}

/** @brief cs_set_kill_watch_cb
 *
 *  Set callback for kill watch
 *  Enable or disable events associated with kills
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_set_kill_watch_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_LOAD || reason == SET_CHANGE )
	{
		if( cs_cfg.kill_watch ) 
		{
			EnableEvent( EVENT_GLOBALKILL );
			EnableEvent( EVENT_SERVERKILL );
			EnableEvent( EVENT_LOCALKILL );
		} 
		else 
		{
			DisableEvent( EVENT_GLOBALKILL );
			DisableEvent( EVENT_SERVERKILL );
			DisableEvent( EVENT_LOCALKILL );
		}
	}
	return NS_SUCCESS;
}

/** @brief cs_set_mode_watch_cb
 *
 *  Set callback for mode watch
 *  Enable or disable events associated with modes
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_set_mode_watch_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_LOAD || reason == SET_CHANGE )
	{
		if( cs_cfg.mode_watch ) 
		{
			EnableEvent( EVENT_UMODE );
			EnableEvent( EVENT_SMODE );
		} 
		else 
		{
			DisableEvent( EVENT_UMODE );
			DisableEvent( EVENT_SMODE );
		}
	}
	return NS_SUCCESS;
}

/** @brief cs_set_nick_watch_cb
 *
 *  Set callback for nick watch
 *  Enable or disable events associated with nick changes
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_set_nick_watch_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_LOAD || reason == SET_CHANGE )
	{
		if( cs_cfg.nick_watch )
		{
			EnableEvent( EVENT_NICK );
		}
		else
		{
			DisableEvent( EVENT_NICK );
		}
	}
	return NS_SUCCESS;
}

/** @brief cs_set_away_watch_cb
 *
 *  Set callback for away watch
 *  Enable or disable events associated with away events
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_set_away_watch_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_LOAD || reason == SET_CHANGE )
	{
		if( cs_cfg.away_watch ) 
		{
			EnableEvent( EVENT_AWAY );
		} 
		else 
		{
			DisableEvent( EVENT_AWAY );
		}
	}
	return NS_SUCCESS;
}

/** @brief cs_set_serv_watch_cb
 *
 *  Set callback for server watch
 *  Enable or disable events associated with server connects/quits
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int cs_set_serv_watch_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_LOAD || reason == SET_CHANGE )
	{
		if( cs_cfg.serv_watch )
		{
			EnableEvent( EVENT_SERVER );
			EnableEvent( EVENT_SQUIT );
		}
		else 
		{
			DisableEvent( EVENT_SERVER );
			DisableEvent( EVENT_SQUIT );
		}
	}
	return NS_SUCCESS;
}
