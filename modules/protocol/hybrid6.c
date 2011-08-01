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
** $Id: hybrid6.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "protocol.h"

char *MSG_PRIVATE = "PRIVMSG";
char *MSG_WHO = "WHO";
char *MSG_WHOIS = "WHOIS";
char *MSG_WHOWAS = "WHOWAS";
char *MSG_USER = "USER";
char *MSG_NICK = "NICK";
char *MSG_SERVER = "SERVER";
char *MSG_LIST = "LIST";
char *MSG_TOPIC = "TOPIC";
char *MSG_INVITE = "INVITE";
char *MSG_VERSION = "VERSION";
char *MSG_QUIT = "QUIT";
char *MSG_SQUIT = "SQUIT";
char *MSG_KILL = "KILL";
char *MSG_INFO = "INFO";
char *MSG_LINKS = "LINKS";
char *MSG_STATS = "STATS";
char *MSG_USERS = "USERS";
char *MSG_HELP = "HELP";
char *MSG_ERROR = "ERROR";
char *MSG_AWAY = "AWAY";
char *MSG_CONNECT = "CONNECT";
char *MSG_PING = "PING";
char *MSG_PONG = "PONG";
char *MSG_OPER = "OPER";
char *MSG_PASS = "PASS";
char *MSG_WALLOPS = "WALLOPS";
char *MSG_TIME = "TIME";
char *MSG_NAMES = "NAMES";
char *MSG_ADMIN = "ADMIN";
char *MSG_TRACE = "TRACE";
char *MSG_LTRACE = "LTRACE";
char *MSG_NOTICE = "NOTICE";
char *MSG_JOIN = "JOIN";
char *MSG_PART = "PART";
char *MSG_LUSERS = "LUSERS";
char *MSG_MOTD = "MOTD";
char *MSG_MODE = "MODE";
char *MSG_KICK = "KICK";
char *MSG_USERHOST = "USERHOST";
char *MSG_ISON = "ISON";
char *MSG_REHASH = "REHASH";
char *MSG_RESTART = "RESTART";
char *MSG_CLOSE = "CLOSE";
char *MSG_SVINFO = "SVINFO";
char *MSG_SJOIN = "SJOIN";
char *MSG_CAPAB = "CAPAB";
char *MSG_DIE = "DIE";
char *MSG_HASH = "HASH";
char *MSG_DNS = "DNS";
char *MSG_OPERWALL = "OPERWALL";
char *MSG_KLINE = "KLINE";
char *MSG_UNKLINE = "UNKLINE";
char *MSG_DLINE = "DLINE";
char *MSG_UNDLINE = "UNDLINE";
char *MSG_HTM = "HTM";
char *MSG_SET = "SET";
char *MSG_GLINE = "GLINE";
char *MSG_UNGLINE = "UNGLINE";
char *MSG_LOCOPS = "LOCOPS";
char *MSG_LWALLOPS = "LWALLOPS";
char *MSG_KNOCK = "KNOCK";
char *MSG_MAP = "MAP";
char *MSG_ETRACE = "ETRACE";
char *MSG_SINFO = "SINFO";
char *MSG_TESTLINE = "TESTLINE";
char *MSG_OPERSPY = "OPERSPY";
char *MSG_ENCAP = "ENCAP";
char *MSG_XLINE = "XLINE";
char *MSG_UNXLINE = "UNXLINE";
char *MSG_RESV = "RESV";
char *MSG_UNRESV = "UNRESV";

/* Umodes */
#define UMODE_SERVNOTICE   0x00000000 /* server notices such as kill */
#define UMODE_REJ          0x00000000 /* Bot Rejections */
#define UMODE_SKILL        0x00000000 /* Server Killed */
#define UMODE_FULL         0x00080000 /* Full messages */
#define UMODE_SPY          0x00100000 /* see STATS / LINKS */
#define UMODE_DEBUG        0x00200000 /* 'debugging' info */
#define UMODE_NCHANGE      0x00400000 /* Nick change notice */
#define UMODE_OPERWALL     0x00800000 /* Operwalls */
#define UMODE_BOTS         0x01000000 /* shows bots */
#define UMODE_EXTERNAL     0x02000000 /* show servers introduced and splitting */
#define UMODE_CALLERID     0x04000000 /* block unless caller id's */
#define UMODE_UNAUTH       0x08000000 /* show unauth connects here */
#define UMODE_STATSPHIDE   0x10000000 /* Oper hides from stats P */
#define UMODE_OSPYLOG      0x20000000 /* show Oper Spy being used */
#define UMODE_UNIDLE       0x40000000 /* Hide idle time with umode +u */
#define UMODE_LOCOPS       0x80000000 /* Oper see's LOCOPS */
 
/* Cmodes */
#define CMODE_EXCEPTION  0x0800
#define CMODE_DENY       0x1000

static void m_server( char *origin, char **argv, int argc, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	0,
	/* Protocol options negotiated at link by this IRCd */
	0,
	/* Features supported by this IRCd */
	0,
	/* Max host length */
	63,
	/* Max password length */
	32,
	/* Max nick length */
	32,
	/* Max user length */
	9,
	/* Max real name length */
	50,
	/* Max channel name length */
	200,
	/* Max topic length */
	120,
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
	IRC_CMD_END()
};

mode_init chan_umodes[] = 
{
	MODE_INIT_END()
};

mode_init chan_modes[] = 
{
	MODE_INIT_END()
};

mode_init user_umodes[] = 
{
	MODE_INIT_END()
};

void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
	send_cmd( "%s %s :TS", MSG_PASS, pass );
	/* CAP QS ZIP EX CHW KNOCK KLN UNKLN CLUSTER ENCAP IE */
	send_cmd( "CAPAB :EX CHW IE KLN KNOCK" );
	send_cmd( "%s %s :%s", MSG_SERVER, name, infoline );
}

void send_nick( const char *nick, const unsigned long ts, const char *newmode, const char *ident, const char *host, const char *server, const char *realname )
{
	send_cmd( "%s %s 1 %lu %s %s %s %s :%s", MSG_NICK, nick, ts, newmode, ident, host, server, realname );
}

/*  m_nick
 *   RX: SERVER servername 1 :hybrid6server
 *   argv[0] = servername
 *   argv[1] = hopcount
 *   argv[2] = serverinfo
 */

static void m_server( char *origin, char **argv, int argc, int srv )
{
	do_server( argv[0], origin, argv[1], NULL, argv[2], srv );
	if( !srv )
		do_synch_neostats();
}

/*  m_nick
 *   argv[0] = nickname
 *   argv[1] = optional hopcount when new user; TS when nick change
 *   argv[2] = optional TS
 *   argv[3] = optional umode
 *   argv[4] = optional username
 *   argv[5] = optional hostname
 *   argv[6] = optional server
 *   argv[7] = optional ircname
 */

static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( !srv ) {
		do_nick( argv[0], argv[1], argv[2], argv[4], argv[5], argv[6], 
			NULL, NULL, argv[3], NULL, argv[7], NULL, NULL );
	} else {
		do_nickchange( origin, argv[0], argv[1] );
	}
}

