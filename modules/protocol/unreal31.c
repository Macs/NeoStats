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
** $Id: unreal31.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "protocol.h"

/* Messages/Tokens */
char *MSG_PRIVATE = "PRIVMSG";
char *TOK_PRIVATE = "!";
char *MSG_WHO = "WHO";
char *TOK_WHO = "\"";
char *MSG_WHOIS = "WHOIS";
char *TOK_WHOIS = "#";
char *MSG_WHOWAS = "WHOWAS";
char *TOK_WHOWAS = "$";
char *MSG_USER = "USER";
char *TOK_USER = "%";
char *MSG_NICK = "NICK";
char *TOK_NICK = "&";
char *MSG_SERVER = "SERVER";
char *TOK_SERVER = "'";
char *MSG_LIST = "LIST";
char *TOK_LIST = "(";
char *MSG_TOPIC = "TOPIC";
char *TOK_TOPIC = ")";
char *MSG_INVITE = "INVITE";
char *TOK_INVITE = "*";
char *MSG_VERSION = "VERSION";
char *TOK_VERSION = "+";
char *MSG_QUIT = "QUIT";
char *TOK_QUIT = ",";
char *MSG_SQUIT = "SQUIT";
char *TOK_SQUIT = "-";
char *MSG_KILL = "KILL";
char *TOK_KILL = ".";
char *MSG_INFO = "INFO";
char *TOK_INFO = "/";
char *MSG_LINKS = "LINKS";
char *TOK_LINKS = "0";
char *MSG_SUMMON = "SUMMON";
char *TOK_SUMMON = "1";
char *MSG_STATS = "STATS";
char *TOK_STATS = "2";
char *MSG_USERS = "USERS";
char *TOK_USERS = "3";
char *MSG_HELP = "HELP";
char *MSG_HELPOP = "HELPOP";
char *TOK_HELP = "4";
char *MSG_ERROR = "ERROR";
char *TOK_ERROR = "5";
char *MSG_AWAY = "AWAY";
char *TOK_AWAY = "6";
char *MSG_CONNECT = "CONNECT";
char *TOK_CONNECT = "7";
char *MSG_PING = "PING";
char *TOK_PING = "8";
char *MSG_PONG = "PONG";
char *TOK_PONG = "9";
char *MSG_OPER = "OPER";
char *TOK_OPER = ";";
char *MSG_PASS = "PASS";
char *TOK_PASS = "<";
char *MSG_WALLOPS = "WALLOPS";
char *TOK_WALLOPS = "=";
char *MSG_TIME = "TIME";
char *TOK_TIME = ">";
char *MSG_NAMES = "NAMES";
char *TOK_NAMES = "?";
char *MSG_ADMIN = "ADMIN";
char *TOK_ADMIN = "@";
char *MSG_NOTICE = "NOTICE";
char *TOK_NOTICE = "B";
char *MSG_JOIN = "JOIN";
char *TOK_JOIN = "C";
char *MSG_PART = "PART";
char *TOK_PART = "D";
char *MSG_LUSERS = "LUSERS";
char *TOK_LUSERS = "E";
char *MSG_MOTD = "MOTD";
char *TOK_MOTD = "F";
char *MSG_MODE = "MODE";
char *TOK_MODE = "G";
char *MSG_KICK = "KICK";
char *TOK_KICK = "H";
char *MSG_SERVICE = "SERVICE";
char *TOK_SERVICE = "I";
char *MSG_USERHOST = "USERHOST";
char *TOK_USERHOST = "J";
char *MSG_ISON = "ISON";
char *TOK_ISON = "K";
char *MSG_REHASH = "REHASH";
char *TOK_REHASH = "O";
char *MSG_RESTART = "RESTART";
char *TOK_RESTART = "P";
char *MSG_CLOSE = "CLOSE";
char *TOK_CLOSE = "Q";
char *MSG_DIE = "DIE";
char *TOK_DIE = "R";
char *MSG_HASH = "HASH";
char *TOK_HASH = "S";
char *MSG_DNS = "DNS";
char *TOK_DNS = "T";
char *MSG_SILENCE = "SILENCE";
char *TOK_SILENCE = "U";
char *MSG_AKILL = "AKILL";
char *TOK_AKILL = "V";
char *MSG_KLINE = "KLINE";
char *TOK_KLINE = "W";
char *MSG_UNKLINE = "UNKLINE";
char *TOK_UNKLINE = "X";
char *MSG_RAKILL = "RAKILL";
char *TOK_RAKILL = "Y";
char *MSG_GNOTICE = "GNOTICE";
char *TOK_GNOTICE = "Z";
char *MSG_GOPER = "GOPER";
char *TOK_GOPER = "[";
char *MSG_GLOBOPS = "GLOBOPS";
char *TOK_GLOBOPS = "]";
char *MSG_LOCOPS = "LOCOPS";
char *TOK_LOCOPS = "^";
char *MSG_PROTOCTL = "PROTOCTL";
char *TOK_PROTOCTL = "_";
char *MSG_WATCH = "WATCH";
char *TOK_WATCH = "`";
char *MSG_TRACE = "TRACE";
char *TOK_TRACE = "b";
char *MSG_SQLINE = "SQLINE";
char *TOK_SQLINE = "c";
char *MSG_UNSQLINE = "UNSQLINE";
char *TOK_UNSQLINE = "d";
char *MSG_SVSNICK = "SVSNICK";
char *TOK_SVSNICK = "e";
char *MSG_SVSNOOP = "SVSNOOP";
char *TOK_SVSNOOP = "f";
char *MSG_IDENTIFY = "IDENTIFY";
char *TOK_IDENTIFY = "g";
char *MSG_SVSKILL = "SVSKILL";
char *TOK_SVSKILL = "h";
char *MSG_NICKSERV = "NICKSERV";
char *MSG_NS = "NS";
char *TOK_NICKSERV = "i";
char *MSG_CHANSERV = "CHANSERV";
char *MSG_CS = "CS";
char *TOK_CHANSERV = "j";
char *MSG_OPERSERV = "OPERSERV";
char *MSG_OS = "OS";
char *TOK_OPERSERV = "k";
char *MSG_MEMOSERV = "MEMOSERV";
char *MSG_MS = "MS";
char *TOK_MEMOSERV = "l";
char *MSG_SERVICES = "SERVICES";
char *TOK_SERVICES = "m";
char *MSG_SVSMODE = "SVSMODE";
char *TOK_SVSMODE = "n";
char *MSG_SAMODE = "SAMODE";
char *TOK_SAMODE = "o";
char *MSG_CHATOPS = "CHATOPS";
char *TOK_CHATOPS = "p";
char *MSG_ZLINE = "ZLINE";
char *TOK_ZLINE = "q";
char *MSG_UNZLINE = "UNZLINE";
char *TOK_UNZLINE = "r";
char *MSG_HELPSERV = "HELPSERV";
char *MSG_HS = "HS";
char *TOK_HELPSERV = "s";
char *MSG_RULES = "RULES";
char *TOK_RULES = "t";
char *MSG_MAP = "MAP";
char *TOK_MAP = "u";
char *MSG_SVS2MODE = "SVS2MODE";
char *TOK_SVS2MODE = "v";
char *MSG_DALINFO = "DALINFO";
char *TOK_DALINFO = "w";
char *MSG_ADMINCHAT = "ADCHAT";
char *TOK_ADMINCHAT = "x";
char *MSG_MKPASSWD = "MKPASSWD";
char *TOK_MKPASSWD = "y";
char *MSG_ADDLINE = "ADDLINE";
char *TOK_ADDLINE = "z";
char *MSG_GLINE = "GLINE";
char *TOK_GLINE = "}";
char *MSG_GZLINE = "GZLINE";
char *TOK_GZLINE = "{";
char *MSG_SJOIN = "SJOIN";
char *TOK_SJOIN = "~";
char *MSG_SETHOST = "SETHOST";
char *TOK_SETHOST = "AA";
char *MSG_NACHAT = "NACHAT";
char *TOK_NACHAT = "AC";
char *MSG_SETIDENT = "SETIDENT";
char *TOK_SETIDENT = "AD";
char *MSG_SETNAME = "SETNAME";
char *TOK_SETNAME = "AE";
char *MSG_LAG = "LAG";
char *TOK_LAG = "AF";
char *MSG_SDESC = "SDESC";
char *TOK_SDESC = "AG";
char *MSG_STATSERV = "STATSERV";
char *TOK_STATSERV = "AH";
char *MSG_KNOCK = "KNOCK";
char *TOK_KNOCK = "AI";
char *MSG_CREDITS = "CREDITS";
char *TOK_CREDITS = "AJ";
char *MSG_LICENSE = "LICENSE";
char *TOK_LICENSE = "AK";
char *MSG_CHGHOST = "CHGHOST";
char *TOK_CHGHOST = "AL";
char *MSG_RPING = "RPING";
char *TOK_RPING = "AM";
char *MSG_RPONG = "RPONG";
char *TOK_RPONG = "AN";
char *MSG_NETINFO = "NETINFO";
char *TOK_NETINFO = "AO";
char *MSG_SENDUMODE = "SENDUMODE";
char *TOK_SENDUMODE = "AP";
char *MSG_ADDMOTD = "ADDMOTD";
char *TOK_ADDMOTD = "AQ";
char *MSG_ADDOMOTD = "ADDOMOTD";
char *TOK_ADDOMOTD = "AR";
char *MSG_SVSMOTD = "SVSMOTD";
char *TOK_SVSMOTD = "AS";
char *MSG_SMO = "SMO";
char *TOK_SMO = "AU";
char *MSG_OPERMOTD = "OPERMOTD";
char *TOK_OPERMOTD = "AV";
char *MSG_TSCTL = "TSCTL";
char *TOK_TSCTL = "AW";
char *MSG_SVSJOIN = "SVSJOIN";
char *TOK_SVSJOIN = "AX";
char *MSG_SAJOIN = "SAJOIN";
char *TOK_SAJOIN = "AY";
char *MSG_SVSPART = "SVSPART";
char *TOK_SVSPART = "AX";
char *MSG_SAPART = "SAPART";
char *TOK_SAPART = "AY";
char *MSG_CHGIDENT = "CHGIDENT";
char *TOK_CHGIDENT = "AZ";
char *MSG_SWHOIS = "SWHOIS";
char *TOK_SWHOIS = "BA";
char *MSG_SVSO = "SVSO";
char *TOK_SVSO = "BB";
char *MSG_SVSFLINE = "SVSFLINE";
char *TOK_SVSFLINE = "BC";
char *MSG_TKL = "TKL";
char *TOK_TKL = "BD";
char *MSG_VHOST = "VHOST";
char *TOK_VHOST = "BE";
char *MSG_BOTMOTD = "BOTMOTD";
char *TOK_BOTMOTD = "BF";
char *MSG_REMGLINE = "REMGLINE";	/* remove g-line */
char *TOK_REMGLINE = "BG";
char *MSG_REMGZLINE = "REMGZLINE";	/* remove global z-line */
char *TOK_REMGZLINE = "BP";
char *MSG_HTM = "HTM";
char *TOK_HTM = "BH";
char *MSG_UMODE2 = "UMODE2";
char *TOK_UMODE2 = "|";
char *MSG_DCCDENY = "DCCDENY";
char *TOK_DCCDENY = "BI";
char *MSG_UNDCCDENY = "UNDCCDENY";
char *TOK_UNDCCDENY = "BJ";
char *MSG_CHGNAME = "CHGNAME";
char *MSG_SVSNAME = "SVSNAME";
char *TOK_CHGNAME = "BK";
char *MSG_SHUN = "SHUN";
char *TOK_SHUN = "BL";
char *MSG_NEWJOIN = "NEWJOIN";	/* For CR Java Chat */
char *MSG_POST = "POST";
char *TOK_POST = "BN";
char *MSG_INFOSERV = "INFOSERV";
char *MSG_IS = "IS";
char *TOK_INFOSERV = "BO";
char *MSG_BOTSERV = "BOTSERV";
char *TOK_BOTSERV = "BS";

/* Umodes */
#define UMODE_FAILOP		0x00100000	/* Shows some global messages */
#define UMODE_SERVNOTICE	0x00200000	/* server notices such as kill */
#define UMODE_KILLS			0x00400000	/* Show server-kills... */
#define UMODE_FLOOD			0x00800000	/* Receive flood warnings */
#define UMODE_JUNK			0x01000000	/* can junk */
#define UMODE_EYES			0x02000000	/* Mode to see server stuff */
#define UMODE_WHOIS			0x04000000	/* gets notice on /whois */
#define UMODE_SECURE		0x08000000	/* User is a secure connect */
#define UMODE_VICTIM		0x10000000	/* Intentional Victim */
#define UMODE_HIDEOPER		0x20000000	/* Hide oper mode */
#define UMODE_SETHOST		0x40000000	/* used sethost */
#define UMODE_STRIPBADWORDS 0x80000000	/* */

/* Cmodes */
#define CMODE_NOKICKS		0x02000000
#define CMODE_MODREG		0x04000000
#define CMODE_STRIPBADWORDS	0x08000000
#define CMODE_NOCTCP		0x10000000
#define CMODE_AUDITORIUM	0x20000000
#define CMODE_ONLYSECURE	0x40000000
#define CMODE_NONICKCHANGE	0x80000000

static void m_server( char *origin, char **argv, int argc, int srv );
static void m_umode2( char *origin, char **argv, int argc, int srv );
static void m_svsmode( char *origin, char **argv, int argc, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );
static void m_sjoin( char *origin, char **argv, int argc, int srv );
static void m_smo( char *origin, char **argv, int argc, int srv );
static void m_swhois( char *origin, char **argv, int argc, int srv );
static void m_tkl( char *origin, char **argv, int argc, int srv );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN,
	/* Protocol options negotiated at link by this IRCd */
	PROTOCOL_TOKEN,
	/* Features supported by this IRCd */
	FEATURE_UMODECLOAK,
	/* Max host length */
	128,
	/* Max password length */
	32,
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
	"+oSq",
	/* Default channel mode for NeoStats service bots */
	"+o",
};

irc_cmd cmd_list[] = 
{
	/*Message	Token	Function	usage */
	{&MSG_SERVER, &TOK_SERVER, m_server, 0},
	{&MSG_UMODE2, &TOK_UMODE2, m_umode2, 0},
	{&MSG_SVSMODE, &TOK_SVSMODE, m_svsmode, 0},
	{&MSG_SVS2MODE, &TOK_SVS2MODE, m_svsmode, 0},
	{&MSG_NICK, &TOK_NICK, m_nick, 0},
	{&MSG_SJOIN, &TOK_SJOIN, m_sjoin, 0},
	{&MSG_SWHOIS, &TOK_SWHOIS, m_swhois, 0},
	{&MSG_SMO, &TOK_SMO, m_smo, 0},
	{&MSG_TKL, &TOK_TKL, m_tkl, 0},
	IRC_CMD_END()
};

mode_init chan_umodes[] = 
{
	{'h', CUMODE_HALFOP, 0, '%'},
	{'a', CUMODE_CHANPROT, 0, '*'},
	{'q', CUMODE_CHANOWNER, 0, '~'},
	MODE_INIT_END()
};

mode_init chan_modes[] = 
{
	{'r', CMODE_RGSTR, 0, 0},
	{'R', CMODE_RGSTRONLY, 0, 0},
	{'c', CMODE_NOCOLOR, 0, 0},
	{'O', CMODE_OPERONLY, 0, 0},
	{'A', CMODE_ADMONLY, 0, 0},
	{'L', CMODE_LINK, MODEPARAM, 0},
	{'Q', CMODE_NOKICKS, 0, 0},
	{'S', CMODE_STRIP, 0, 0},
	{'e', CMODE_EXCEPT, MODEPARAM, 0},
	{'K', CMODE_NOKNOCK, 0, 0},
	{'V', CMODE_NOINVITE, 0, 0},
	{'f', CMODE_FLOODLIMIT, MODEPARAM, 0},
	{'M', CMODE_MODREG, 0, 0},
	{'G', CMODE_STRIPBADWORDS, 0, 0},
	{'C', CMODE_NOCTCP, 0, 0},
	{'u', CMODE_AUDITORIUM, 0, 0},
	{'z', CMODE_ONLYSECURE, 0, 0},
	{'N', CMODE_NONICKCHANGE, 0, 0},
	MODE_INIT_END()
};

mode_init user_umodes[] = 
{
	{'S', UMODE_SERVICES, 0, 0},
	{'N', UMODE_NETADMIN, 0, 0},
	{'a', UMODE_SADMIN, 0, 0},
	{'A', UMODE_ADMIN, 0, 0},
	{'C', UMODE_COADMIN, 0, 0},
	{'O', UMODE_LOCOP, 0, 0},
	{'r', UMODE_REGNICK, 0, 0},
	{'w', UMODE_WALLOP, 0, 0},
	{'g', UMODE_FAILOP, 0, 0},
	{'h', UMODE_HELPOP, 0, 0},
	{'s', UMODE_SERVNOTICE, 0, 0},
	{'q', UMODE_KIX, 0, 0},
	{'B', UMODE_BOT, 0, 0},
 	{'d', UMODE_DEAF, 0, 0},
	{'k', UMODE_KILLS, 0, 0},
	{'e', UMODE_EYES, 0, 0},
	{'F', UMODE_FCLIENT, 0, 0},
	{'c', UMODE_CLIENT, 0, 0},
	{'f', UMODE_FLOOD, 0, 0},
	{'j', UMODE_JUNK, 0, 0},
	{'G', UMODE_STRIPBADWORDS, 0, 0},
	{'t', UMODE_SETHOST, 0, 0},
	{'x', UMODE_HIDE, 0, 0},
	/*{'b', UMODE_CHATOP, 0, 0},*/
	{'W', UMODE_WHOIS, 0, 0},
	{'z', UMODE_SECURE, 0, 0},
	{'v', UMODE_VICTIM, 0, 0},	
	MODE_INIT_END()
};

void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
/* PROTOCTL NOQUIT TOKEN NICKv2 SJOIN SJOIN2 UMODE2 VL SJ3 NS SJB64 */
	send_cmd( "%s TOKEN NICKv2 VHP SJOIN SJOIN2 SJ3 UMODE2", MSGTOK( PROTOCTL ) );
	send_cmd( "%s %s", MSGTOK( PASS ), pass );
	send_cmd( "%s %s %d :U0-*-%d %s", MSGTOK( SERVER ), name, 1, numeric, infoline );
}

void send_sjoin( const char *source, const char *target, const char *chan, const unsigned long ts )
{
	send_cmd( ":%s %s %lu %s + :%s", source, MSGTOK( SJOIN ), ts, chan, target );
}

void send_nick( const char *nick, const unsigned long ts, const char *newmode, const char *ident, const char *host, const char *server, const char *realname )
{
	send_cmd( "%s %s 1 %lu %s %s %s 0 %s * :%s", MSGTOK( NICK ), nick, ts, ident, host, server, newmode, realname );
}

void send_smo( const char *source, const char *umodetarget, const char *msg )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( SMO ), umodetarget, msg );
}

void send_swhois( const char *source, const char *target, const char *swhois )
{
	send_cmd( "%s %s :%s", MSGTOK( SWHOIS ), target, swhois );
}

/* akill is gone in the latest Unreals, so we set Glines instead */
void send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts )
{
	send_cmd( ":%s %s + G %s %s %s %lu %lu :%s", source, MSGTOK( TKL ), ident, host, setby, ( ts + length ), ts, reason );
}

void send_rakill( const char *source, const char *host, const char *ident )
{
	send_cmd( ":%s %s - G %s %s %s", source, MSGTOK( TKL ), ident, host, source );
}

void send_svstime( const char *source, const unsigned long ts )
{
	send_cmd( ":%s %s SVSTIME %lu", source, MSGTOK( TSCTL ), ts );
}

/** m_server
 *
 *  process SERVER command
 *  RX:
 *    SERVER irc.foonet.com 1 :U2305-FinWXOoZE-1 FooNet Server
 *  Format:
 *    SERVER servername hopcount numeric :U<protocol>-flags-numeric serverdesc
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = servername
 *    argv[1] = hopcount
 *    argv[2] = numeric
 *    argv[3] = serverinfo
 *  on old protocols, serverinfo is argv[2], and numeric is left out
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

static void m_server( char *origin, char **argv, int argc, int srv )
{
	char *s = argv[argc-1];
	if( *origin== 0 )
	{
		/* server desc from uplink includes extra info so we need to 
		   strip protocol, flags and numeric. We can use the first
		   space to do this*/
		while( *s != ' ' )
			s++;
		/* Strip the now leading space */
		s++;
	}
	if( argc > 3 )
	{
		do_server( argv[0], origin, argv[1], argv[2], s, srv );
	}
	else
	{
		do_server( argv[0], origin, argv[1], NULL, s, srv );
	}
}

/** m_svsmode
 *
 *  process SVSMODE command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] - username to change mode for
 *    argv[1] - modes to change
 *    argv[2] - Service Stamp( if mode == d )
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

static void m_svsmode( char *origin, char **argv, int argc, int srv )
{
	if( argv[0][0] == '#' )
	{
		do_svsmode_channel( origin, argv, argc );
	}
	else
	{
		do_svsmode_user( argv[0], argv[1], argv[2] );
	}
}

/** m_umode2
 *
 *  process UMODE2 command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] - modes to change
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

static void m_umode2( char *origin, char **argv, int argc, int srv )
{
	do_mode_user( origin, argv[0] );
}

/** m_nick
 *
 *  process NICK command
 *  RX:
 *    NICK Mark 1 1089324634 mark 127.0.0.1 irc.foonet.com 0 +iowghaAxN F72CBABD.ABE021B4.D9E4BB78.IP fwAAAQ== :Mark
 *    NICK Mark 1 1089324634 mark 127.0.0.1 irc.foonet.com 0 +iowghaAxN F72CBABD.ABE021B4.D9E4BB78.IP :Mark
 *  Format:
 *    NICK nick hop TS user host uplink servicestamp umode vhost [base64 IP] :realname
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] = nickname
 *  if from new client
 *    argv[1] = nick password
 *  if from server:
 *    argv[1] = hopcount
 *    argv[2] = timestamp
 *    argv[3] = username
 *    argv[4] = hostname
 *    argv[5] = servername
 *  if NICK version 1:
 *    argv[6] = servicestamp
 *    argv[7] = info
 *  if NICK version 2:
 *    argv[6] = servicestamp
 *    argv[7] = umodes
 *    argv[8] = virthost, * if none
 *    argv[9] = info
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( !srv )
	{
		if( ircd_srv.protocol & PROTOCOL_NICKv2 )
		{
			do_nick( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], 
				NULL, argv[6], argv[7], argv[8], argv[9], NULL, NULL );
		}
		else
		{
			do_nick( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], 
				NULL, argv[6], NULL, NULL, argv[9], NULL, NULL );
		}
	}
	else
	{
		do_nickchange( origin, argv[0], NULL );
	}
}

/** m_sjoin
 *
 *  process SJOIN command
 *  RX:
 *    SJOIN 1073861298 #services + <none> :Mark
 *  Format:
 *    SJOIN creationtime chname modebuf parabuf :member list
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] = channel timestamp
 *      char *argv[], pvar[MAXMODEPARAMS][MODEBUFLEN + 3];
 *    argv[1] = channel name
 *      "ts chname :"
 *  if( argc == 3 ) 
 *    argv[2] = nick names + modes - all in one parameter
 *      "ts chname modebuf :"
 *      "ts chname :"@/"""name"	OPT_SJ3
 *  if( argc == 4 )
 *    argv[2] = channel modes
 *    argv[3] = nick names + modes - all in one parameter
 *      "ts chname modebuf parabuf :"
 *  if( argc > 4 )
 *    argv[2] = channel modes
 *    argv[3 to argc - 2] = mode parameters
 *    argv[argc - 1] = nick names + modes
 *      "ts parabuf :parv[parc - 1]"	OPT_SJOIN | OPT_SJ3 
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

static void m_sjoin( char *origin, char **argv, int argc, int srv )
{
	do_sjoin( argv[0], argv[1], ( ( argc >= 4 ) ? argv[2] : "" ), origin, argv, argc );
}

/** m_swhois
 *
 *  process SWHOIS command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] = nickname
 *    argv[1] = new swhois
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

static void m_swhois( char *origin, char **argv, int argc, int srv )
{
	do_swhois( argv[0], argv[1] );
}

/** m_smo
 *
 *  process SMO command
 *  RX:
 *    :irc.foonet.com SMO o :(\1link\1) Link irc.foonet.com -> stats.neostats.net[@127.0.0.1.2722] established
 *    :irc.foonet.com SMO o :\1(sync)\1 Possible negative TS split at link stats.neostats.net (1128112841 - 1128112842 = -1)
 *    :irc.foonet.com SMO o :\1(sync)\1 Link stats.neostats.net -> irc.foonet.com is now synced [secs: 2 recv: 0.825 sent: 0.657]
 *  Format:
 *    :origin SMO ? :message
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

static void m_smo( char *origin, char **argv, int argc, int srv )
{
	/* TODO */
}

/** m_tkl
 *
 *  process TKL command
 *  RX:
 *    TODO
 *  Format:
 *    :server BD + G * mask setter 1074811259 1074206459 :reason
 *    :server BD + Z * mask setter 0 1070062390 :reason
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0]  +|- 
 *    argv[1]  G   
 *    argv[2]  user 
 *    argv[3]  host 
 *    argv[4]  setby 
 *    argv[5]  expire_at 
 *    argv[6]  set_at 
 *    argv[7]  reason 
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

static void m_tkl( char *origin, char **argv, int argc, int srv )
{
	do_tkl( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7] );
}
