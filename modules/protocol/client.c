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
** $Id: client.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "protocol.h"
#include "bots.h"
#include "users.h"

/* Messages/Tokens */
char *MSG_PRIVATE = "PRIVMSG";	/* PRIV */
char *MSG_WHOIS = "WHOIS";	/* WHOI */
char *MSG_WHOWAS = "WHOWAS";	/* WHOW */
char *MSG_USER = "USER";	/* USER */
char *MSG_NICK = "NICK";	/* NICK */
char *MSG_LIST = "LIST";	/* LIST */
char *MSG_TOPIC = "TOPIC";	/* TOPI */
char *MSG_INVITE = "INVITE";	/* INVI */
char *MSG_VERSION = "VERSION";	/* VERS */
char *MSG_QUIT = "QUIT";	/* QUIT */
char *MSG_KILL = "KILL";	/* KILL */
char *MSG_INFO = "INFO";	/* INFO */
char *MSG_LINKS = "LINKS";	/* LINK */
char *MSG_SUMMON = "SUMMON";	/* SUMM */
char *MSG_USERS = "USERS";	/* USER -> USRS */
char *MSG_HELP = "HELP";	/* HELP */
char *MSG_HELPOP = "HELPOP";	/* HELP */
char *MSG_ERROR = "ERROR";	/* ERRO */
char *MSG_AWAY = "AWAY";	/* AWAY */
char *MSG_CONNECT = "CONNECT";	/* CONN */
char *MSG_PING = "PING";	/* PING */
char *MSG_PONG = "PONG";	/* PONG */
char *MSG_OPER = "OPER";	/* OPER */
char *MSG_PASS = "PASS";	/* PASS */
char *MSG_WALLOPS = "WALLOPS";	/* WALL */
char *MSG_TIME = "TIME";	/* TIME */
char *MSG_NAMES = "NAMES";	/* NAME */
char *MSG_ADMIN = "ADMIN";	/* ADMI */
char *MSG_NOTICE = "NOTICE";	/* NOTI */
char *MSG_JOIN = "JOIN";	/* JOIN */
char *MSG_PART = "PART";	/* PART */
char *MSG_LUSERS = "LUSERS";	/* LUSE */
char *MSG_MOTD = "MOTD";	/* MOTD */
char *MSG_MODE = "MODE";	/* MODE */
char *MSG_KICK = "KICK";	/* KICK */
char *MSG_SERVICE = "SERVICE";	/* SERV -> SRVI */
char *MSG_USERHOST = "USERHOST";	/* USER -> USRH */
char *MSG_ISON = "ISON";	/* ISON */
char *MSG_REHASH = "REHASH";	/* REHA */
char *MSG_RESTART = "RESTART";	/* REST */
char *MSG_CLOSE = "CLOSE";	/* CLOS */
char *MSG_DIE = "DIE";	/* DIE */
char *MSG_HASH = "HASH";	/* HASH */
char *MSG_DNS = "DNS";	/* DNS  -> DNSS */
char *MSG_SILENCE = "SILENCE";	/* SILE */
char *MSG_AKILL = "AKILL";	/* AKILL */
char *MSG_KLINE = "KLINE";	/* KLINE */
char *MSG_UNKLINE = "UNKLINE";	/* UNKLINE */
char *MSG_RAKILL = "RAKILL";	/* RAKILL */
char *MSG_GNOTICE = "GNOTICE";	/* GNOTICE */
char *MSG_GOPER = "GOPER";	/* GOPER */
char *MSG_GLOBOPS = "GLOBOPS";	/* GLOBOPS */
char *MSG_LOCOPS = "LOCOPS";	/* LOCOPS */
char *MSG_PROTOCTL = "PROTOCTL";	/* PROTOCTL */
char *MSG_WATCH = "WATCH";	/* WATCH */
char *MSG_TRACE = "TRACE";	/* TRAC */
char *MSG_SQLINE = "SQLINE";	/* SQLINE */
char *MSG_UNSQLINE = "UNSQLINE";	/* UNSQLINE */
char *MSG_IDENTIFY = "IDENTIFY";	/* IDENTIFY */
char *MSG_NICKSERV = "NICKSERV";	/* NICKSERV */
char *MSG_NS = "NS";
char *MSG_CHANSERV = "CHANSERV";	/* CHANSERV */
char *MSG_CS = "CS";
char *MSG_OPERSERV = "OPERSERV";	/* OPERSERV */
char *MSG_OS = "OS";
char *MSG_MEMOSERV = "MEMOSERV";	/* MEMOSERV */
char *MSG_MS = "MS";
char *MSG_SERVICES = "SERVICES";	/* SERVICES */
char *MSG_SAMODE = "SAMODE";	/* SAMODE */
char *MSG_CHATOPS = "CHATOPS";	/* CHATOPS */
char *MSG_ZLINE = "ZLINE";	/* ZLINE */
char *MSG_UNZLINE = "UNZLINE";	/* UNZLINE */
char *MSG_HELPSERV = "HELPSERV";	/* HELPSERV */
char *MSG_HS = "HS";
char *MSG_RULES = "RULES";	/* RULES */
char *MSG_MAP = "MAP";	/* MAP */
char *MSG_DALINFO = "DALINFO";	/* dalinfo */
char *MSG_ADMINCHAT = "ADCHAT";	/* Admin chat */
char *MSG_MKPASSWD = "MKPASSWD";	/* MKPASSWD */
char *MSG_ADDLINE = "ADDLINE";	/* ADDLINE */
char *MSG_GLINE = "GLINE";	/* The awesome g-line */
char *MSG_SJOIN = "SJOIN";
char *MSG_SETHOST = "SETHOST";	/* sethost */
char *MSG_NACHAT = "NACHAT";	/* netadmin chat */
char *MSG_SETIDENT = "SETIDENT";
char *MSG_SETNAME = "SETNAME";	/* set GECOS */
char *MSG_LAG = "LAG";	/* Lag detect */
char *MSG_STATSERV = "STATSERV";	/* alias */
char *MSG_KNOCK = "KNOCK";
char *MSG_CREDITS = "CREDITS";
char *MSG_LICENSE = "LICENSE";
char *MSG_CHGHOST = "CHGHOST";
char *MSG_RPING = "RPING";
char *MSG_RPONG = "RPONG";
char *MSG_NETINFO = "NETINFO";
char *MSG_SENDUMODE = "SENDUMODE";
char *MSG_ADDMOTD = "ADDMOTD";
char *MSG_ADDOMOTD = "ADDOMOTD";
char *MSG_SMO = "SMO";
char *MSG_OPERMOTD = "OPERMOTD";
char *MSG_TSCTL = "TSCTL";
char *MSG_SAJOIN = "SAJOIN";
char *MSG_SAPART = "SAPART";
char *MSG_CHGIDENT = "CHGIDENT";
char *MSG_SWHOIS = "SWHOIS";
char *MSG_SVSO = "SVSO";
char *MSG_TKL = "TKL";
char *MSG_VHOST = "VHOST";
char *MSG_BOTMOTD = "BOTMOTD";
char *MSG_REMGLINE = "REMGLINE";	/* remove g-line */
char *MSG_UMODE2 = "UMODE2";
char *MSG_DCCDENY = "DCCDENY";
char *MSG_UNDCCDENY = "UNDCCDENY";
char *MSG_CHGNAME = "CHGNAME";
char *MSG_SHUN = "SHUN";
char *MSG_NEWJOIN = "NEWJOIN";	/* For CR Java Chat */
char *MSG_POST = "POST";
char *MSG_INFOSERV = "INFOSERV";
char *MSG_IS = "IS";
char *MSG_BOTSERV = "BOTSERV";
char *MSG_CYCLE = "CYCLE";
char *MSG_MODULE = "MODULE";
char *MSG_SENDSNO = "SENDSNO";

/* Umodes */
#define UMODE_FAILOP		0x00200000
#define UMODE_SERVNOTICE	0x00400000
#define UMODE_NOCTCP		0x00800000
#define UMODE_WEBTV			0x01000000
#define UMODE_WHOIS			0x02000000
#define UMODE_SECURE		0x04000000
#define UMODE_VICTIM		0x08000000
#define UMODE_HIDEOPER		0x10000000
#define UMODE_SETHOST		0x20000000
#define UMODE_STRIPBADWORDS	0x40000000
#define UMODE_HIDEWHOIS		0x80000000

/* Cmodes */
#define CMODE_NOKICKS		0x02000000
#define CMODE_MODREG		0x04000000
#define CMODE_STRIPBADWORDS	0x08000000
#define CMODE_NOCTCP		0x10000000
#define CMODE_AUDITORIUM	0x20000000
#define CMODE_ONLYSECURE	0x40000000
#define CMODE_NONICKCHANGE	0x80000000

static void m_private( char *origin, char **av, int ac, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );
static void m_topic( char *origin, char **argv, int argc, int srv );
static void m_kick( char *origin, char **argv, int argc, int srv );
static void m_join( char *origin, char **argv, int argc, int srv );
static void m_part( char *origin, char **argv, int argc, int srv );
static void m_rpl_welcome( char *origin, char **argv, int argc, int srv );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_CLIENTMODE,
	/* Protocol options negotiated at link by this IRCd */
	0,
	/* Features supported by this IRCd */
	0,
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

char *numeric001 = "001";
irc_cmd cmd_list[] = 
{
	/*Message	Token	Function	usage */
	{&MSG_PRIVATE, 0, m_private, 0},
	{&MSG_NOTICE, 0, _m_notice, 0},
	{&numeric001, 0, m_rpl_welcome, 0},
	{&MSG_NICK, 0, m_nick, 0},
	{&MSG_TOPIC, 0, m_topic, 0},
	{&MSG_KICK, 0, m_kick, 0},
	{&MSG_JOIN, 0, m_join, 0},
	{&MSG_PART, 0, m_part, 0},
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
	{'R', UMODE_RGSTRONLY, 0, 0},
 	{'T', UMODE_NOCTCP, 0, 0},
	{'V', UMODE_WEBTV, 0, 0},
	{'p', UMODE_HIDEWHOIS, 0, 0},
	{'H', UMODE_HIDEOPER, 0, 0},
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
	send_cmd( "%s %s", MSG_PASS, pass );
	send_cmd( "%s %s", MSG_NICK, "NeoStats" );
	send_cmd( "%s %s %d %d :%s", MSG_USER, "user", ( int ) me.now, ( int ) me.now, "real name" );
}

void send_cmode( const char *sourceserver, const char *sourceuser, const char *chan, const char *mode, const char *args, const unsigned long ts )
{
	send_cmd( ":%s %s %s %s %s %lu", sourceuser, MSG_MODE, chan, mode, args, ts );
}

void send_nick( const char *nick, const unsigned long ts, const char *newmode, const char *ident, const char *host, const char *server, const char *realname )
{
	send_cmd( "%s %s", MSG_NICK, nick );
}

void send_swhois( const char *source, const char *target, const char *swhois )
{
	send_cmd( "%s %s :%s", MSG_SWHOIS, target, swhois );
}

/* akill is gone in the latest Unreals, so we set Glines instead */
void send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts )
{
	send_cmd( ":%s %s + G %s %s %s %lu %lu :%s", source, MSG_TKL, ident, host, setby, ( ts + length ), ts, reason );
}

void send_rakill( const char *source, const char *host, const char *ident )
{
	send_cmd( ":%s %s - G %s %s %s", source, MSG_TKL, ident, host, source );
}

static void m_nick( char *origin, char **argv, int argc, int srv )
{
	do_nickchange( origin, argv[0], NULL );
}

/* m_topic
 *  argv[0] = channel name
 *  argv[1] = topic text
 */
/* TOPIC #channel :topic */
static void m_topic( char *origin, char **argv, int argc, int srv )
{
	do_topic( argv[0], NULL, NULL, argv[1] );
}

/* m_kick
 *	argv[0] = channel
 *	argv[1] = client to kick
 *	argv[2] = kick comment
 */
static void m_kick( char *origin, char **argv, int argc, int srv )
{
	do_kick( origin, argv[0], argv[1], argv[2] );
}

/* m_join
 *	argv[0] = channel
 *	argv[1] = channel password( key )
 */
static void m_join( char *origin, char **argv, int argc, int srv )
{
	do_join( origin, argv[0], argv[1] );
}

/* m_part
 *	argv[0] = channel
 *	argv[1] = comment
 */
static void m_part( char *origin, char **argv, int argc, int srv )
{
	do_part( origin, argv[0], argv[1] );
}

/** @brief process privmsg
 *
 * 
 *
 * @return none
 */

static void m_private( char *origin, char **av, int ac, int srv )
{
	char *p;
	char nick[MAXNICK];
	
	strlcpy( nick, origin, MAXNICK );
	p = strchr( nick, '!' );
	*p = 0;
	AddFakeUser( origin );
	_m_private( nick, av, ac, srv );
	DelFakeUser( origin );
}

static void m_rpl_welcome( char *origin, char **argv, int argc, int srv )
{
	send_cmd( "%s mark mark", MSG_OPER );
	do_synch_neostats();
}
