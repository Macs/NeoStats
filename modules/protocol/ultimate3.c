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
** $Id: ultimate3.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "protocol.h"

/* Messages/Tokens */
char *MSG_PRIVATE = "PRIVMSG";	/* PRIV */
char *MSG_WHO = "WHO";		/* WHO  -> WHOC */
char *MSG_WHOIS = "WHOIS";		/* WHOI */
char *MSG_WHOWAS = "WHOWAS";	/* WHOW */
char *MSG_USER = "USER";		/* USER */
char *MSG_NICK = "NICK";		/* NICK */
char *MSG_SERVER = "SERVER";	/* SERV */
char *MSG_LIST = "LIST";		/* LIST */
char *MSG_TOPIC = "TOPIC";		/* TOPI */
char *MSG_INVITE = "INVITE";	/* INVI */
char *MSG_VERSION = "VERSION";	/* VERS */
char *MSG_QUIT = "QUIT";		/* QUIT */
char *MSG_SQUIT = "SQUIT";		/* SQUI */
char *MSG_KILL = "KILL";		/* KILL */
char *MSG_INFO = "INFO";		/* INFO */
char *MSG_LINKS = "LINKS";		/* LINK */
char *MSG_STATS = "STATS";		/* STAT */
char *MSG_USERS = "USERS";		/* USER -> USRS */
char *MSG_HELP = "HELP";		/* HELP */
char *MSG_ERROR = "ERROR";		/* ERRO */
char *MSG_AWAY = "AWAY";		/* AWAY */
char *MSG_CONNECT = "CONNECT";	/* CONN */
char *MSG_PING = "PING";		/* PING */
char *MSG_PONG = "PONG";		/* PONG */
char *MSG_OPER = "OPER";		/* OPER */
char *MSG_PASS = "PASS";		/* PASS */
char *MSG_WALLOPS = "WALLOPS";	/* WALL */
char *MSG_TIME = "TIME";		/* TIME */
char *MSG_NAMES = "NAMES";		/* NAME */
char *MSG_ADMIN = "ADMIN";		/* ADMI */
char *MSG_TRACE = "TRACE";		/* TRAC */
char *MSG_NOTICE = "NOTICE";	/* NOTI */
char *MSG_JOIN = "JOIN";		/* JOIN */
char *MSG_PART = "PART";		/* PART */
char *MSG_LUSERS = "LUSERS";	/* LUSE */
char *MSG_MOTD = "MOTD";		/* MOTD */
char *MSG_MODE = "MODE";		/* MODE */
char *MSG_KICK = "KICK";		/* KICK */
char *MSG_USERHOST = "USERHOST";	/* USER -> USRH */
char *MSG_ISON = "ISON";		/* ISON */
char *MSG_REHASH = "REHASH";	/* REHA */
char *MSG_RESTART = "RESTART";	/* REST */
char *MSG_CLOSE = "CLOSE";		/* CLOS */
char *MSG_SVINFO = "SVINFO";	/* SVINFO */
char *MSG_SJOIN = "SJOIN";		/* SJOIN */
char *MSG_DIE = "DIE";		/* DIE */
char *MSG_HASH = "HASH";		/* HASH */
char *MSG_DNS = "DNS";		/* DNS  -> DNSS */
char *MSG_OPERWALL = "OPERWALL";	/* OPERWALL */
char *MSG_GLOBOPS = "GLOBOPS";	/* GLOBOPS */
char *MSG_CHATOPS = "CHATOPS";	/* CHATOPS */
char *MSG_GOPER = "GOPER";		/* GOPER */
char *MSG_GNOTICE = "GNOTICE";	/* GNOTICE */
char *MSG_KLINE = "KLINE";		/* KLINE */
char *MSG_UNKLINE = "UNKLINE";	/* UNKLINE */
char *MSG_HTM = "HTM";		/* HTM */
char *MSG_SET = "SET";		/* SET */
char *MSG_CAPAB = "CAPAB";		/* CAPAB */
char *MSG_LOCOPS = "LOCOPS";	/* LOCOPS */
char *MSG_SVSNICK = "SVSNICK";	/* SVSNICK */
char *MSG_SVSNOOP = "SVSNOOP";	/* SVSNOOP */
char *MSG_SVSKILL = "SVSKILL";	/* SVSKILL */
char *MSG_SVSMODE = "SVSMODE";	/* SVSMODE */
char *MSG_AKILL = "AKILL";		/* AKILL */
char *MSG_RAKILL = "RAKILL";	/* RAKILL */
char *MSG_SILENCE = "SILENCE";	/* SILENCE */
char *MSG_WATCH = "WATCH";		/* WATCH */
char *MSG_SQLINE = "SQLINE";	/* SQLINE */
char *MSG_UNSQLINE = "UNSQLINE";	/* UNSQLINE */
char *MSG_BURST = "BURST";		/* BURST */
char *MSG_DCCALLOW = "DCCALLOW";	/* dccallow */
char *MSG_SGLINE = "SGLINE";	/* sgline */
char *MSG_UNSGLINE = "UNSGLINE";	/* unsgline */
char *MSG_SETTINGS = "SETTINGS";	/* SETTINGS */
char *MSG_RULES = "RULES";		/* RULES */
char *MSG_OPERMOTD = "OPERMOTD";	/* OPERMOTD */
char *MSG_NETINFO = "NETINFO";	/* NETINFO */
char *MSG_NETGLOBAL = "NETGLOBAL";	/* NETGLOBAL */
char *MSG_SETHOST = "SETHOST";	/* SETHOST */
char *MSG_VHOST = "VHOST";		/* VHOST */
char *MSG_CREDITS = "CREDITS";	/* CREDITS */
char *MSG_COPYRIGHT = "COPYRIGHT";	/* COPYRIGHT */
char *MSG_ADCHAT = "ADCHAT";	/* ADCHAT */
char *MSG_GCONNECT = "GCONNECT";	/* GCONNECT */
char *MSG_IRCOPS = "IRCOPS";	/* IRCOPS */
char *MSG_KNOCK = "KNOCK";		/* KNOCK */
char *MSG_CHANNEL = "CHANNEL";	/* CHANNEL */
char *MSG_VCTRL = "VCTRL";		/* VCTRL */
char *MSG_CSCHAT = "CSCHAT";	/* CSCHAT */
char *MSG_MAP = "MAP";		/* MAP */
char *MSG_MAKEPASS = "MAKEPASS";	/* MAKEPASS */
char *MSG_DKEY = "DKEY";		/* diffie-hellman negotiation */
char *MSG_FJOIN = "FJOIN";		/* Forced Join's */
char *MSG_FMODE = "FMODE";		/* Forced Mode's */
char *MSG_IRCDHELP = "IRCDHELP";	/* IRCDHELP */
char *MSG_ADDOPER = "ADDOPER";	/* ADDOPER */
char *MSG_DELOPER = "DELOPER";	/* DELOPER */
char *MSG_ADDCNLINE = "ADDCNLINE";	/* ADDCNLINE */
char *MSG_DELCNLINE = "DELCNLINE";	/* DELCNLINE */
char *MSG_ADDQLINE = "ADDQLINE";	/* ADDQLINE */
char *MSG_DELQLINE = "DELQLINE";	/* DELQLINE */
char *MSG_ADDHLINE = "ADDHLINE";	/* ADDHLINE */
char *MSG_DELHLINE = "DELHLINE";	/* DELHLINE */
char *MSG_ADDULINE = "ADDULINE";	/* ADDULINE */
char *MSG_DELULINE = "DELULINE";	/* DELULINE */
char *MSG_CLIENT = "CLIENT";	/* CLIENT */
char *MSG_NETCTRL = "NETCTRL";	/* NETCTRL */
char *MSG_SMODE = "SMODE";		/* SMODE */
char *MSG_RESYNCH = "RESYNCH";	/* RESYNCH */
char *MSG_EOBURST = "EOBURST";	/* EOBURST */
char *MSG_CS = "CS";		/* CS */
char *MSG_CHANSERV = "CHANSERV";	/* CHANSERV */
char *MSG_NS = "NS";		/* NS */
char *MSG_NICKSERV = "NICKSERV";	/* NICKSERV */
char *MSG_MS = "MS";		/* MS */
char *MSG_MEMOSERV = "MEMOSERV";	/* MEMOSERV */
char *MSG_OS = "OS";		/* OS */
char *MSG_OPERSERV = "OPERSERV";	/* OPERSERV */
char *MSG_SS = "SS";		/* SS */
char *MSG_STATSERV = "STATSERV";	/* STATSERV */
char *MSG_BS = "BS";		/* BS */
char *MSG_BOTSERV = "BOTSERV";	/* BOTSERV */
char *MSG_RS = "RS";		/* RS */
char *MSG_HS = "HS";		/* HS aka HeadShot :) */
char *MSG_HOSTSERV = "HOSTSERV";	/* HOSTSERV */
char *MSG_ROOTSERV = "ROOTSERV";	/* ROOTSERV */
char *MSG_SERVICES = "SERVICES";	/* SERVICES */
char *MSG_IDENTIFY = "IDENTIFY";	/* IDENTIFY */
char *MSG_NMODE = "NMODE";		/* NMODE */
char *MSG_SVSJOIN = "SVSJOIN";	/* SVSJOIN */
char *MSG_CHANFIX = "CHANFIX";	/* CHANFIX */
char *MSG_SVSPART = "SVSPART";	/* SVSPART */
char *MSG_USERIP = "USERIP";	/* USERIP */

/* Umodes */
#define UMODE_SERVNOTICE    0x00020000	/* umode +s - Server notices */
#define UMODE_KILLS     	0x00040000	/* umode +k - Server kill messages */
#define UMODE_FLOOD     	0x00080000	/* umode +f - Server flood messages */
#define UMODE_SPY			0x00100000	/* umode +y - Stats/links */
#define UMODE_DCC     		0x00200000	/* umode +D - pseudo/hidden, has seen dcc warning message */
#define UMODE_GLOBOPS     	0x00400000	/* umode +g - Globops */
#define UMODE_CHATOPS     	0x00800000	/* umode +C - Chatops */
#define UMODE_SERVICESOPER  0x01000000	/* umode +a - Services Operator - Should be moved to smode */
#define UMODE_REJ			0x02000000	/* umode +j - Reject notices */
#define UMODE_ROUTE     	0x04000000	/* umode +n - Routing Notices */
#define UMODE_SPAM     		0x08000000	/* umode +m - spambot notices */
#define UMODE_SRA			0x10000000	/* umode +Z - Services Root Admin - Should be moved to smode */
#define UMODE_DEBUG			0x20000000	/* umode +d - Debug Info */
#define UMODE_DCCWARN		0x40000000	/* umode +e - See DCC send warnings */
#define UMODE_WHOIS			0x80000000	/* umode +W - Opers can see when a user /whois's them */

static void m_server( char *origin, char **argv, int argc, int srv );
static void m_svsmode( char *origin, char **argv, int argc, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );
static void m_sethost( char *origin, char **argv, int argc, int srv );
static void m_burst( char *origin, char **argv, int argc, int srv );
static void m_sjoin( char *origin, char **argv, int argc, int srv );
static void m_client( char *origin, char **argv, int argc, int srv );
static void m_smode( char *origin, char **argv, int argc, int srv );
static void m_vctrl( char *origin, char **argv, int argc, int srv );
static void m_netinfo( char *origin, char **argv, int argc, int srv );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN | PROTOCOL_NICKIP,
	/* Protocol options negotiated at link by this IRCd */
	PROTOCOL_CLIENT,
	/* Features supported by this IRCd */
	0,
	/* Max host length */
	128,
	/* Max password length */
	32,
	/* Max nick length */
	32,
	/* Max user length */
	15,
	/* Max real name length */
	50,
	/* Max channel name length */
	50,
	/* Max topic length */
	512,
	/* Default operator modes for NeoStats service bots */
	"+oS",
	/* Default channel mode for NeoStats service bots */
	"+a",
};

irc_cmd cmd_list[] = 
{
	/* Command Token Function usage */
	{&MSG_SETHOST,   0, m_sethost,   0},
	{&MSG_SERVER,    0, m_server,	0},
	{&MSG_SVSMODE,   0, m_svsmode,   0},
	{&MSG_NICK,      0, m_nick,      0},
	{&MSG_BURST,     0, m_burst,     0},
	{&MSG_SJOIN,     0, m_sjoin,     0},
	{&MSG_CLIENT,    0, m_client,    0},
	{&MSG_SMODE,     0, m_smode,     0},
	{&MSG_VCTRL,     0, m_vctrl,     0},
	{&MSG_NETINFO,   0, m_netinfo,   0},
	IRC_CMD_END()
};

mode_init chan_umodes[] = 
{
	{'h', CUMODE_HALFOP, 0, '%'},
	{'a', CUMODE_CHANADMIN, 0, '!'},
	MODE_INIT_END()
};

mode_init chan_modes[] = 
{
	{'e', CMODE_EXCEPT, MODEPARAM, 0},
	{'f', CMODE_FLOODLIMIT, MODEPARAM, 0},
	{'r', CMODE_RGSTR, 0, 0},
	{'x', CMODE_NOCOLOR, 0, 0},
	{'A', CMODE_ADMONLY, 0, 0},
	{'I', CMODE_NOINVITE, 0, 0},
	{'K', CMODE_NOKNOCK, 0, 0},
	{'L', CMODE_LINK, MODEPARAM, 0},
	{'O', CMODE_OPERONLY, 0, 0},
	{'R', CMODE_RGSTRONLY, 0, 0},
	{'S', CMODE_STRIP, 0, 0},	
	MODE_INIT_END()
};

mode_init user_umodes[] = 
{
	{'Z', UMODE_SRA, 0, 0},
	{'S', UMODE_SERVICES, 0, 0},
	{'P', UMODE_SADMIN, 0, 0},
	{'a', UMODE_SERVICESOPER, 0, 0},
	{'O', UMODE_LOCOP, 0, 0},
	{'r', UMODE_REGNICK, 0, 0},
	{'w', UMODE_WALLOP, 0, 0},
	{'s', UMODE_SERVNOTICE, 0, 0},
	{'c', UMODE_CLIENT, 0, 0},
	{'k', UMODE_KILLS, 0, 0},
	{'h', UMODE_HELPOP, 0, 0},
	{'f', UMODE_FLOOD, 0, 0},
	{'y', UMODE_SPY, 0, 0},
	{'D', UMODE_DCC, 0, 0},
	{'g', UMODE_GLOBOPS, 0, 0},
	{'c', UMODE_CHATOPS, 0, 0},
	{'j', UMODE_REJ, 0, 0},
	{'n', UMODE_ROUTE, 0, 0},
	{'m', UMODE_SPAM, 0, 0},
	{'x', UMODE_HIDE, 0, 0},
	{'p', UMODE_KIX, 0, 0},
	{'F', UMODE_FCLIENT, 0, 0},
	/* useless modes, ignore them as services use these modes for services ID */
	/* {'d', UMODE_DEBUG, 0, 0}, */
	{'e', UMODE_DCCWARN, 0, 0},
	{'W', UMODE_WHOIS, 0, 0},
	MODE_INIT_END()
};

mode_init user_smodes[] = 
{
	{'N', SMODE_NETADMIN, 0, 0},
	{'n', SMODE_CONETADMIN, 0, 0},
	{'T', SMODE_TECHADMIN, 0, 0},
	{'t', SMODE_COTECHADMIN, 0, 0},
	{'A', SMODE_ADMIN, 0, 0},
	{'G', SMODE_GUESTADMIN, 0, 0},
	{'a', SMODE_COADMIN, 0, 0},
	{'s', SMODE_SSL, 0, 0},
	MODE_INIT_END()
};

void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
	send_cmd( "%s %s :TS", MSG_PASS, pass );
	send_cmd( "CAPAB TS5 BURST SSJ5 NICKIP CLIENT" );
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
	send_cmd( "%s %s 1 %lu %s %s %s %s 0 %lu :%s", MSG_NICK, nick, ts, newmode, ident, host, server, ts, realname );
}

void send_vctrl( const int uprot, const int nicklen, const int modex, const int gc, const char *netname )
{
	send_cmd( "%s %d %d %d %d 0 0 0 0 0 0 0 0 0 0 :%s", MSG_VCTRL, uprot, nicklen, modex, gc, netname );
}

void send_svshost( const char *source, const char *target, const char *vhost )
{
	send_cmd( ":%s %s %s %s", source, MSG_SETHOST, target, vhost );
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

void send_netinfo( const char *source, const char *maxglobalcnt, const unsigned long ts, const int prot, const char *cloak, const char *netname ) {
	dlog(DEBUG3, "Warning, Ultimate3 Protocol Handler was asked to send Netinfo, Not supported");
}

/* :source SJOIN TS #chan modebuf  :nickbuf */
/* :source SJOIN TS #chan modebuf parabuf :nickbuf */
/* :source SJOIN TS #chan */
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

static void m_sethost( char *origin, char **argv, int argc, int srv )
{
	do_vhost( argv[0], argv[1] );
}
static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( !srv ) {
		do_nick( argv[0], argv[1], argv[2], argv[4], argv[5], argv[6], 
			argv[8], NULL, argv[3], NULL, argv[9], NULL, NULL );
	} else {
		do_nickchange( origin, argv[0], NULL );
	}
}

static void m_vctrl( char *origin, char **argv, int argc, int srv )
{
	do_vctrl( argv[0], argv[1], argv[2], argv[3], argv[14] );
}

/* Ultimate3 Client Support */
static void m_client( char *origin, char **argv, int argc, int srv )
{
	do_client( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], 
		 argv[7], argv[8], NULL, argv[10], argv[11] );
}

static void m_smode( char *origin, char **argv, int argc, int srv )
{
	do_smode( argv[0], argv[1] );
}

static void m_netinfo( char *origin, char **argv, int argc, int srv) {
	/* netinfo is essentially the same as globops */
	do_globops(origin, argv[0]);
}
