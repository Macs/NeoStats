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
** $Id: plexus.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "protocol.h"
#include "services.h"

/* Messages/Tokens */
char *MSG_EOB = "EOB";	/* end of burst */
char *MSG_PRIVATE = "PRIVMSG";	/* PRIV */
char *MSG_NICK = "NICK";	/* NICK */
char *MSG_SERVER = "SERVER";	/* SERV */
char *MSG_TOPIC = "TOPIC";	/* TOPI */
char *MSG_INVITE = "INVITE";	/* INVI */
char *MSG_VERSION = "VERSION";	/* VERS */
char *MSG_QUIT = "QUIT";	/* QUIT */
char *MSG_SQUIT = "SQUIT";	/* SQUI */
char *MSG_KILL = "KILL";	/* KILL */
char *MSG_STATS = "STATS";	/* STAT */
char *MSG_ERROR = "ERROR";	/* ERRO */
char *MSG_AWAY = "AWAY";	/* AWAY */
char *MSG_PING = "PING";	/* PING */
char *MSG_PONG = "PONG";	/* PONG */
char *MSG_PASS = "PASS";	/* PASS */
char *MSG_WALLOPS = "WALLOPS";	/* WALL */
char *MSG_ADMIN = "ADMIN";	/* ADMI */
char *MSG_NOTICE = "NOTICE";	/* NOTI */
char *MSG_JOIN = "JOIN";	/* JOIN */
char *MSG_PART = "PART";	/* PART */
char *MSG_MOTD = "MOTD";	/* MOTD */
char *MSG_MODE = "MODE";	/* MODE */
char *MSG_KICK = "KICK";	/* KICK */
char *MSG_KLINE = "KLINE";	/* KLINE */
char *MSG_UNKLINE = "UNKLINE";	/* UNKLINE */
char *MSG_CHATOPS = "CHATOPS";	/* CHATOPS */
char *MSG_NETINFO = "NETINFO";	/* NETINFO */
char *MSG_CREDITS = "CREDITS";
char *MSG_SNETINFO = "SNETINFO";	/* SNetInfo */
char *MSG_SVINFO = "SVINFO";
char *MSG_CAPAB = "CAPAB";
char *MSG_SJOIN = "SJOIN";

/* Umodes */
#define UMODE_SERVNOTICE   0x00100000 /* server notices such as kill */
#define UMODE_REJ          0x00200000 /* Bot Rejections */
#define UMODE_SKILL        0x00400000 /* Server Killed */
#define UMODE_FULL         0x00800000 /* Full messages */
#define UMODE_SPY          0x01000000 /* see STATS / LINKS */
#define UMODE_DEBUG        0x02000000 /* 'debugging' info */
#define UMODE_NCHANGE      0x04000000 /* Nick change notice */
#define UMODE_OPERWALL     0x08000000 /* Operwalls */
#define UMODE_BOTS         0x10000000 /* shows bots */
#define UMODE_EXTERNAL     0x20000000 /* show servers introduced and splitting */
#define UMODE_CALLERID     0x40000000 /* block unless caller id's */
#define UMODE_UNAUTH       0x80000000 /* show unauth connects here */
 
/* Channel Visibility macros */
#define CMODE_INVEX		0x02000000
#define CMODE_HIDEOPS	0x04000000

static void m_server( char *origin, char **argv, int argc, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );
static void m_topic( char *origin, char **argv, int argc, int srv );
static void m_sjoin( char *origin, char **argv, int argc, int srv );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN,
	/* Protocol options negotiated at link by this IRCd */
	PROTOCOL_UNKLN,
	/* Features supported by this IRCd */
	0,
	/* Max host length */
	128,
	/* Max password length */
	32,
	/* Max nick length */
	32,
	/* Max user length */
	10,
	/* Max real name length */
	50,
	/* Max channel name length */
	200,
	/* Max topic length */
	512,
	/* Default operator modes for NeoStats service bots */
	"+o",
	/* Default channel mode for NeoStats service bots */
	"+o",
};

/* this is the command list and associated functions to run */
irc_cmd cmd_list[] = 
{
	/* Command Token Function usage */
	{&MSG_SERVER, 0, m_server, 0},
	{&MSG_NICK, 0, m_nick, 0},
	{&MSG_TOPIC, 0, m_topic, 0},
	{&MSG_SJOIN, 0, m_sjoin, 0},
	IRC_CMD_END()
};

mode_init chan_umodes[] = 
{
	{'h', CUMODE_HALFOP, 0, '%'},
	MODE_INIT_END()
};

mode_init chan_modes[] = 
{
	{'e', CMODE_EXCEPT, MODEPARAM, 0},
	{'I', CMODE_INVEX, MODEPARAM, 0},
	{'a', CMODE_HIDEOPS, 0, 0},
	MODE_INIT_END()
};

mode_init user_umodes[] = 
{
	{'d', UMODE_DEBUG, 0, 0},
	{'a', UMODE_ADMIN, 0, 0},
	{'l', UMODE_LOCOP, 0, 0},
	{'b', UMODE_BOTS, 0, 0},
	{'c', UMODE_CLIENT, 0, 0},
	{'f', UMODE_FULL, 0, 0},
	{'g', UMODE_CALLERID, 0, 0},
	{'k', UMODE_SKILL, 0, 0},
	{'n', UMODE_NCHANGE, 0, 0},
	{'r', UMODE_REJ, 0, 0},
	{'s', UMODE_SERVNOTICE, 0, 0},
	{'u', UMODE_UNAUTH, 0, 0},
	{'w', UMODE_WALLOP, 0, 0},
	{'x', UMODE_EXTERNAL, 0, 0},
	{'y', UMODE_SPY, 0, 0},
	{'z', UMODE_OPERWALL, 0, 0},
	MODE_INIT_END()
};

void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
	send_cmd( "%s %s :TS", MSG_PASS, pass );
	send_cmd( "CAPAB :TS EX CHW IE EOB KLN GLN KNOCK HOPS HUB AOPS MX" );
	send_cmd( "%s %s %d :%s", MSG_SERVER, name, numeric, infoline );
}

void send_sjoin( const char *source, const char *target, const char *chan, const unsigned long ts )
{
	send_cmd( ":%s %s %lu %s + :%s", source, MSG_SJOIN, ts, chan, target );
}

void send_cmode( const char *sourceserver, const char *sourceuser, const char *chan, const char *mode, const char *args, const unsigned long ts )
{
	send_cmd( ":%s %s %s %s %s %lu", sourceuser, MSG_MODE, chan, mode, args, ts );
}

void send_nick( const char *nick, const unsigned long ts, const char *newmode, const char *ident, const char *host, const char *server, const char *realname )
{
	send_cmd( "%s %s 1 %lu %s %s %s %s %s %s%lu :%s", MSG_NICK, nick, ts, newmode, ident, host, host, server, nick, ts, realname );
}

/* there isn't an akill on Hybrid, so we send a kline to all servers! */
void send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts )
{
	send_cmd( ":%s %s * %lu %s %s :%s", setby, MSG_KLINE, length, ident, host, reason );
}

void send_rakill( const char *source, const char *host, const char *ident )
{
	if( ircd_srv.protocol & PROTOCOL_UNKLN ) {
		send_cmd( ":%s %s %s %s", source, MSG_UNKLINE, ident, host );
	} else {
		irc_chanalert( ns_botptr, "Please Manually remove KLINES using /unkline on each server" );
	}
}

/* source SJOIN unsigned long chan modes param:" */
static void m_sjoin( char *origin, char **argv, int argc, int srv )
{
	do_sjoin( argv[0], argv[1], ( ( argc <= 2 ) ? argv[1] : argv[2] ), argv[4], argv, argc );
}

static void m_server( char *origin, char **argv, int argc, int srv )
{
	do_server( argv[0], origin, argv[1], NULL, argv[2], srv );
}

/*  m_nick
 *    argv[0] = nickname
 *    argv[1] = hop count
 *    argv[2] = TS
 *    argv[3] = umode
 *    argv[4] = username
 *    argv[5] = hostname
 *    argv[6] = vhost
 *    argv[7] = server
 *    argv[8] = svsid
 *    argv[9] = ircname
 */
static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( !srv ) {
		do_nick( argv[0], argv[1], argv[2], argv[4], argv[5], argv[7], NULL, NULL, argv[3], argv[6], argv[9], NULL, NULL );
	} else {
		do_nickchange( origin, argv[0], NULL );
	}
}

static void m_topic( char *origin, char **argv, int argc, int srv )
{
	/*
	** Hybrid uses two different formats for the topic change protocol... 
	** :user TOPIC channel :topic 
	** and 
	** :server TOPIC channel author topicts :topic 
	** Both forms must be accepted.
	** - Hwy
	*/	
	if( FindUser( origin ) ) {
		do_topic( argv[0], origin, NULL, argv[1] );
	} else if( FindServer( origin ) ) {
		do_topic( argv[0], argv[1], argv[2], argv[3] );
	} else {
		nlog( LOG_WARNING, "m_topic: can't find topic setter %s for topic %s", origin, argv[1] ); 
	}
}
