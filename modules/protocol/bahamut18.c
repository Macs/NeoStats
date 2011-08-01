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
** $Id: bahamut18.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "protocol.h"

/* Messages/Tokens */
char *MSG_PRIVATE = "PRIVMSG";		/* PRIV */
char *MSG_WHO = "WHO";	      	/* WHO  -> WHOC */
char *MSG_WHOIS = "WHOIS";	   	/* WHOI */
char *MSG_WHOWAS = "WHOWAS";	   	/* WHOW */
char *MSG_USER = "USER";	   	/* USER */
char *MSG_NICK = "NICK";	   	/* NICK */
char *MSG_SERVER = "SERVER";	   	/* SERV */
char *MSG_LIST = "LIST";	   	/* LIST */
char *MSG_TOPIC = "TOPIC";	   	/* TOPI */
char *MSG_INVITE = "INVITE";	   	/* INVI */
char *MSG_VERSION = "VERSION";		/* VERS */
char *MSG_QUIT = "QUIT";	   	/* QUIT */
char *MSG_SQUIT = "SQUIT";	   	/* SQUI */
char *MSG_KILL = "KILL";	   	/* KILL */
char *MSG_INFO = "INFO";	   	/* INFO */
char *MSG_LINKS = "LINKS";	   	/* LINK */
char *MSG_STATS = "STATS";	   	/* STAT */
char *MSG_USERS = "USERS";	   	/* USER -> USRS */
char *MSG_HELP = "HELP";	   	/* HELP */
char *MSG_ERROR = "ERROR";	   	/* ERRO */
char *MSG_AWAY = "AWAY";	   	/* AWAY */
char *MSG_CONNECT = "CONNECT";		/* CONN */
char *MSG_PING = "PING";	   	/* PING */
char *MSG_PONG = "PONG";	   	/* PONG */
char *MSG_OPER = "OPER";	   	/* OPER */
char *MSG_PASS = "PASS";	   	/* PASS */
char *MSG_WALLOPS = "WALLOPS";		/* WALL */
char *MSG_TIME = "TIME";	   	/* TIME */
char *MSG_NAMES = "NAMES";	   	/* NAME */
char *MSG_ADMIN = "ADMIN";	   	/* ADMI */
char *MSG_TRACE = "TRACE";	   	/* TRAC */
char *MSG_NOTICE = "NOTICE";	   	/* NOTI */
char *MSG_JOIN = "JOIN";	   	/* JOIN */
char *MSG_PART = "PART";	   	/* PART */
char *MSG_LUSERS = "LUSERS";	   	/* LUSE */
char *MSG_MOTD = "MOTD";	   	/* MOTD */
char *MSG_MODE = "MODE";	   	/* MODE */
char *MSG_KICK = "KICK";	   	/* KICK */
char *MSG_USERHOST = "USERHOST";		/* USER -> USRH */
char *MSG_USERIP = "USERIP";		/* USER -> USRH */
char *MSG_ISON = "ISON";	   	/* ISON */
char *MSG_REHASH = "REHASH";	   	/* REHA */
char *MSG_RESTART = "RESTART";		/* REST */
char *MSG_CLOSE = "CLOSE";	   	/* CLOS */
char *MSG_SVINFO = "SVINFO";	   	/* SVINFO */
char *MSG_SJOIN = "SJOIN";	   	/* SJOIN */
char *MSG_DIE = "DIE"; 		/* DIE */
char *MSG_HASH = "HASH";	   	/* HASH */
char *MSG_DNS = "DNS";   	   	/* DNS  -> DNSS */
char *MSG_OPERWALL = "OPERWALL";		/* OPERWALL */
char *MSG_GLOBOPS = "GLOBOPS";		/* GLOBOPS */
char *MSG_CHATOPS = "CHATOPS";		/* CHATOPS */
char *MSG_GOPER = "GOPER";	   	/* GOPER */
char *MSG_GNOTICE = "GNOTICE";		/* GNOTICE */
char *MSG_KLINE = "KLINE";	   	/* KLINE */
char *MSG_UNKLINE = "UNKLINE";		/* UNKLINE */
char *MSG_SET = "SET";	      	/* SET */
char *MSG_SAMODE = "SAMODE";    	/* SAMODE */
char *MSG_SAJOIN = "SAJOIN";		/* SAJOIN */
char *MSG_CHANSERV = "CHANSERV";		/* CHANSERV */
char *MSG_NICKSERV = "NICKSERV";		/* NICKSERV */
char *MSG_MEMOSERV = "MEMOSERV";		/* MEMOSERV */
char *MSG_ROOTSERV = "ROOTSERV";		/* MEMOSERV */
char *MSG_OPERSERV = "OPERSERV";		/* OPERSERV */
char *MSG_STATSERV = "STATSERV"; 	/* STATSERV */
char *MSG_HELPSERV = "HELPSERV"; 	/* HELPSERV */
char *MSG_SERVICES = "SERVICES";		/* SERVICES */
char *MSG_IDENTIFY = "IDENTIFY";		/* IDENTIFY */
char *MSG_CAPAB = "CAPAB";	   	/* CAPAB */ 
char *MSG_LOCOPS = "LOCOPS";	   	/* LOCOPS */
char *MSG_SVSNICK = "SVSNICK";   	/* SVSNICK */
char *MSG_SVSNOOP = "SVSNOOP";   	/* SVSNOOP */
char *MSG_SVSKILL = "SVSKILL";   	/* SVSKILL */
char *MSG_SVSMODE = "SVSMODE";   	/* SVSMODE */
char *MSG_SVSHOLD = "SVSHOLD";		/* SVSHOLD */
char *MSG_AKILL = "AKILL";     	/* AKILL */
char *MSG_RAKILL = "RAKILL";    	/* RAKILL */
char *MSG_SILENCE = "SILENCE";   	/* SILENCE */
char *MSG_WATCH = "WATCH";     	/* WATCH */
char *MSG_SQLINE = "SQLINE"; 		/* SQLINE */
char *MSG_UNSQLINE = "UNSQLINE"; 	/* UNSQLINE */
char *MSG_BURST = "BURST";     	/* BURST */
char *MSG_DCCALLOW = "DCCALLOW";		/* dccallow */
char *MSG_SGLINE = "SGLINE";           /* sgline */
char *MSG_UNSGLINE = "UNSGLINE";         /* unsgline */
char *MSG_DKEY = "DKEY";		/* diffie-hellman negotiation */
char *MSG_NS = "NS";            	/* NickServ commands */
char *MSG_CS = "CS";            	/* ChanServ commands */
char *MSG_MS = "MS";            	/* MemoServ commands */
char *MSG_RS = "RS";            	/* RootServ commands */
char *MSG_OS = "OS";            	/* OperServ commands */
char *MSG_SS = "SS";            	/* StatServ commands */
char *MSG_HS = "HS";            	/* StatServ commands */
char *MSG_RESYNCH = "RESYNCH";		/* RESYNCH */
char *MSG_LUSERSLOCK = "LUSERSLOCK";     /* Lusers LOCK */
char *MSG_LINKSCONTROL = "LINKSCONTROL"; /* LINKSCONTROL */
char *MSG_MODULE = "MODULE";		/* MODULE */
char *MSG_RWHO = "RWHO";         /* RWHO */
char *MSG_SVSCLONE = "SVSCLONE";     /* SVSCLONE */

/* Umodes */
#define UMODE_SERVNOTICE	0x00100000	/* umode +s - Server notices */
#define UMODE_KILLS			0x00200000	/* umode +k - Server kill messages */
#define UMODE_FLOOD			0x00400000	/* umode +f - Server flood messages */
#define UMODE_SPY			0x00800000	/* umode +y - Stats/links */
#define UMODE_DEBUG			0x00100000	/* umode +d - Debug info */
#define UMODE_GLOBOPS		0x00200000	/* umode +g - Globops */
#define UMODE_CHATOPS		0x00400000	/* umode +b - Chatops */
#define UMODE_ROUTE			0x00800000	/* umode +n - Routing Notices */
#define UMODE_SPAM			0x01000000	/* umode +m - spambot notices */
#define UMODE_OPERNOTICE	0x02000000	/* umode +e - oper notices for the above +D */
#define UMODE_SQUELCH		0x04000000	/* umode +x - Squelch with notice */
#define UMODE_SQUELCHN		0x08000000	/* umode +X - Squelch without notice */
#define UMODE_HIDDENDCC     0x10000000	/* umode +D - Hidden dccallow umode */
#define UMODE_THROTTLE		0x20000000	/* umode +F - no cptr->since message rate throttle */
#define UMODE_REJ			0x40000000	/* umode +j - client rejection notices */
#define UMODE_ULINEKILL     0x80000000	/* umode +K - U: lined server kill messages */

/* Cmodes */
#define CMODE_MODREG	0x02000000
#define CMODE_LISTED	0x04000000

static void m_server( char *origin, char **argv, int argc, int srv );
static void m_svsmode( char *origin, char **argv, int argc, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );
static void m_burst( char *origin, char **argv, int argc, int srv );
static void m_sjoin( char *origin, char **argv, int argc, int srv );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN | PROTOCOL_NICKIP | PROTOCOL_NOQUIT,
	/* Protocol options negotiated at link by this IRCd */
	0,
	/* Features supported by this IRCd */
	0,
	/* Max host length */
	128,
	/* Max password length */
	63,
	/* Max nick length */
	30,
	/* Max user length */
	10,
	/* Max real name length */
	50,
	/* Max channel name length */
	32,
	/* Max topic length */
	307,
	/* Default operator modes for NeoStats service bots */
	"+oS",
	/* Default channel mode for NeoStats service bots */
	"+o",
};

irc_cmd cmd_list[] = 
{
	/* Command Token Function usage */
	{&MSG_SERVER, 0, m_server, 0},
	{&MSG_SVSMODE, 0, m_svsmode, 0},
	{&MSG_NICK, 0, m_nick, 0},
	{&MSG_BURST, 0, m_burst, 0},
	{&MSG_SJOIN, 0, m_sjoin, 0},
	IRC_CMD_END()
};

mode_init chan_umodes[] = 
{
	MODE_INIT_END()
};

mode_init chan_modes[] = 
{
	{'r', CMODE_RGSTR, 0, 0},
	{'R', CMODE_RGSTRONLY, 0, 0},
	{'x', CMODE_NOCOLOR, 0, 0},
	{'O', CMODE_OPERONLY, 0, 0},
	{'M', CMODE_MODREG, 0, 0},
	{'L', CMODE_LISTED, 0, 0},
	MODE_INIT_END()
};

mode_init user_umodes[] = 
{
	{'a', UMODE_SADMIN, 0, 0},
	{'A', UMODE_ADMIN, 0, 0},
	{'O', UMODE_LOCOP, 0, 0},
	{'r', UMODE_REGNICK, 0, 0},
	{'w', UMODE_WALLOP, 0, 0},
	{'s', UMODE_SERVNOTICE, 0, 0},
	{'c', UMODE_CLIENT, 0, 0},
	{'k', UMODE_KILLS, 0, 0},
	{'f', UMODE_FLOOD, 0, 0},
	{'y', UMODE_SPY, 0, 0},
	{'d', UMODE_DEBUG, 0, 0},
	{'g', UMODE_GLOBOPS, 0, 0},
	{'b', UMODE_CHATOPS, 0, 0},
	{'n', UMODE_ROUTE, 0, 0},
	{'h', UMODE_HELPOP, 0, 0},
	{'m', UMODE_SPAM, 0, 0},
	{'R', UMODE_RGSTRONLY, 0, 0},
	{'e', UMODE_OPERNOTICE, 0, 0},
	{'x', UMODE_SQUELCH, 0, 0},
	{'X', UMODE_SQUELCHN, 0, 0},
	{'D', UMODE_HIDDENDCC, 0, 0},
	{'F', UMODE_THROTTLE, 0, 0},
	{'j', UMODE_REJ, 0, 0},
	{'K', UMODE_ULINEKILL, 0, 0},
	MODE_INIT_END()
};

void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
	send_cmd( "%s %s :TS", MSG_PASS, pass );
	send_cmd( "CAPAB TS3 SSJOIN BURST NICKIP" );
	send_cmd( "%s %s %d :%s", MSG_SERVER, name, numeric, infoline );
}

void send_sjoin( const char *source, const char *target, const char *chan, const unsigned long ts )
{
	send_cmd( ":%s %s %lu %s + :%s", source, MSG_SJOIN, ts, chan, target );
}

void send_nick( const char *nick, const unsigned long ts, const char *newmode, const char *ident, const char *host, const char *server, const char *realname )
{
	send_cmd( "%s %s 1 %lu %s %s %s %s 0 %lu :%s", MSG_NICK, nick, ts, newmode, ident, host, server, ts, realname );
}

void send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts )
{
	send_cmd( ":%s %s %s %s %lu %s %lu :%s", source, MSG_AKILL, host, ident, length, setby, ts, reason );
}

void send_rakill( const char *source, const char *host, const char *ident )
{
	send_cmd( ":%s %s %s %s", source, MSG_RAKILL, host, ident );
}

void send_burst( int b )
{
	if( b == 0 ) {
		send_cmd( "BURST 0" );
	} else {
		send_cmd( "BURST" );
	}
}

/* from SJOIN TS TS chan modebuf parabuf :nicks */
/* from SJOIN TS TS chan modebuf :nicks */
/* from SJOIN TS TS chan modebuf parabuf : */
/* from SJOIN TS TS chan + :@nicks */
/* from SJOIN TS TS chan 0 : */
/* from SJOIN TS chan modebuf parabuf :nicks */
/* from SJOIN TS chan modebuf :nicks */
/* from SJOIN TS chan modebuf parabuf : */
/* from SJOIN TS chan + :@nicks */
/* from SJOIN TS chan 0 : */
/* from SJOIN TS chan */

static void m_sjoin( char *origin, char **argv, int argc, int srv )
{
	do_sjoin( argv[0], argv[1], ( ( argc <= 2 ) ? argv[1] : argv[2] ), origin, argv, argc );
}

static void m_burst( char *origin, char **argv, int argc, int srv )
{
	do_burst( origin, argv, argc );
}

static void m_server( char *origin, char **argv, int argc, int srv )
{
	if( argc > 2 ) {
		do_server( argv[0], origin, argv[1], argv[2], NULL, srv );
	} else {
		do_server( argv[0], origin, NULL, argv[1], NULL, srv );
	}
}

static void m_svsmode( char *origin, char **argv, int argc, int srv )
{
	if( argv[0][0] == '#' ) {
		do_svsmode_channel( origin, argv, argc );
	} else {
		do_svsmode_user( argv[0], argv[2], NULL );
	}
}

/* m_nick 
 * argv[0] = nickname 
 * argv[1] = hopcount when new user; TS when nick change 
 * argv[2] = TS
 * ---- new user only below ---- 
 * argv[3] = umode 
 * argv[4] = username 
 * argv[5] = hostname 
 * argv[6] = server 
 * argv[7] = serviceid
 * -- If NICKIP
 * argv[8] = IP
 * argv[9] = ircname
 * -- else
 * argv[8] = ircname
 * -- endif
 */
static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( !srv ) {
		do_nick( argv[0], argv[1], argv[2], argv[4], argv[5], argv[6], 
			argv[8], NULL, argv[3], NULL, argv[9], NULL, NULL );
	} else {
		do_nickchange( origin, argv[0], argv[1] );
	}
}
