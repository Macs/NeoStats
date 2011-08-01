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
** $Id: ultimate2.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "protocol.h"
#include "services.h"

/* Messages/Tokens */
char *MSG_PRIVATE = "PRIVMSG";	/* PRIV */
/* char *TOK_PRIVATE = "!"; */	/* 33 */
char *MSG_WHO = "WHO";	/* WHO -> WHOC */
/* char *TOK_WHO = "\""; */	/* 34 */
char *MSG_WHOIS = "WHOIS";	/* WHOI */
/* char *TOK_WHOIS = "#"; */	/* 35 */
char *MSG_WHOWAS = "WHOWAS";	/* WHOW */
/* char *TOK_WHOWAS = "$"; */	/* 36 */
char *MSG_USER = "USER";	/* USER */
/* char *TOK_USER = "%"; */	/* 37 */
char *MSG_NICK = "NICK";	/* NICK */
char *TOK_NICK = "&";	/* 38 */
char *MSG_SERVER = "SERVER";	/* SERV */
/* char *TOK_SERVER = "'"; */	/* 39 */
char *MSG_LIST = "LIST";	/* LIST */
/* char *TOK_LIST = "("; */	/* 40 */
char *MSG_TOPIC = "TOPIC";	/* TOPI */
char *TOK_TOPIC = ")";	/* 41 */
char *MSG_INVITE = "INVITE";	/* INVI */
/* char *TOK_INVITE = "*"; */	/* 42 */
char *MSG_VERSION = "VERSION";	/* VERS */
/* char *TOK_VERSION = "+"; */	/* 43 */
char *MSG_QUIT = "QUIT";	/* QUIT */
/* char *TOK_QUIT = ","; */	/* 44 */
char *MSG_SQUIT = "SQUIT";	/* SQUI */
/* char *TOK_SQUIT = "-"; */	/* 45 */
char *MSG_KILL = "KILL";	/* KILL */
/* char *TOK_KILL = "."; */	/* 46 */
char *MSG_INFO = "INFO";	/* INFO */
/* char *TOK_INFO = "/"; */	/* 47 */
char *MSG_LINKS = "LINKS";	/* LINK */
/* char *TOK_LINKS = "0"; */	/* 48 */
char *MSG_WATCH = "WATCH";	/* WATCH */
/* char *TOK_WATCH = "1"; */	/* 49 */
char *MSG_STATS = "STATS";	/* STAT */
/* char *TOK_STATS = "2"; */	/* 50 */
char *MSG_HELP = "HELP";	/* HELP */
char *MSG_HELPOP = "HELPOP";	/* HELP */
/* char *TOK_HELP = "4"; */	/* 52 */
char *MSG_ERROR = "ERROR";	/* ERRO */
/* char *TOK_ERROR = "5"; */	/* 53 */
char *MSG_AWAY = "AWAY";	/* AWAY */
char *TOK_AWAY = "6";	/* 54 */
char *MSG_CONNECT = "CONNECT";	/* CONN */
/* char *TOK_CONNECT = "7"; */	/* 55 */
char *MSG_PING = "PING";	/* PING */
/* char *TOK_PING = "8"; */	/* 56 */
char *MSG_PONG = "PONG";	/* PONG */
/* char *TOK_PONG = "9"; */	/* 57 */
char *MSG_OPER = "OPER";	/* OPER */
/* char *TOK_OPER = "\""; */	/* 59 */
char *MSG_PASS = "PASS";	/* PASS */
/* char *TOK_PASS = "<"; */	/* 60 */
char *MSG_WALLOPS = "WALLOPS";	/* WALL */
/* char *TOK_WALLOPS = "="; */	/* 61 */
char *MSG_TIME = "TIME";	/* TIME */
/* char *TOK_TIME = ">"; */	/* 62 */
char *MSG_NAMES = "NAMES";	/* NAME */
/* char *TOK_NAMES = "?"; */	/* 63 */
char *MSG_ADMIN = "ADMIN";	/* ADMI */
/* char *TOK_ADMIN = "@"; */	/* 64 */
char *MSG_NOTICE = "NOTICE";	/* NOTI */
/* char *TOK_NOTICE = "B"; */	/* 66 */
char *MSG_JOIN = "JOIN";	/* JOIN */
char *TOK_JOIN = "C";	/* 67 */
char *MSG_PART = "PART";	/* PART */
/* char *TOK_PART = "D"; */	/* 68 */
char *MSG_LUSERS = "LUSERS";	/* LUSE */
/* char *TOK_LUSERS = "E"; */	/* 69 */
char *MSG_MOTD = "MOTD";	/* MOTD */
/* char *TOK_MOTD = "F"; */	/* 70 */
char *MSG_MODE = "MODE";	/* MODE */
char *TOK_MODE = "G";	/* 71 */
char *MSG_KICK = "KICK";	/* KICK */
/* char *TOK_KICK = "H"; */	/* 72 */
char *MSG_SERVICE = "SERVICE";	/* SERV -> SRVI */
/* char *TOK_SERVICE = "I"; */	/* 73 */
char *MSG_USERHOST = "USERHOST";	/* USER -> USRH */
/* char *TOK_USERHOST = "J"; */	/* 74 */
char *MSG_ISON = "ISON";	/* ISON */
/* char *TOK_ISON = "K"; */	/* 75 */
char *MSG_SQUERY = "SQUERY";	/* SQUE */
/* char *TOK_SQUERY = "L"; */	/* 76 */
char *MSG_SERVLIST = "SERVLIST";	/* SERV -> SLIS */
/* char *TOK_SERVLIST = "M"; */	/* 77 */
char *MSG_SERVSET = "SERVSET";	/* SERV -> SSET */
/* char *TOK_SERVSET = "N"; */	/* 78 */
char *MSG_REHASH = "REHASH";	/* REHA */
/* char *TOK_REHASH = "O"; */	/* 79 */
char *MSG_RESTART = "RESTART";	/* REST */
/* char *TOK_RESTART = "P"; */	/* 80 */
char *MSG_CLOSE = "CLOSE";	/* CLOS */
/* char *TOK_CLOSE = "Q"; */	/* 81 */
char *MSG_DIE = "DIE";	/* DIE */
/* char *TOK_DIE = "R"; */	/* 82 */
char *MSG_HASH = "HASH";	/* HASH */
/* char *TOK_HASH = "S"; */	/* 83 */
char *MSG_DNS = "DNS";	/* DNS -> DNSS */
/* char *TOK_DNS = "T"; */	/* 84 */
char *MSG_SILENCE = "SILENCE";	/* SILE */
/* char *TOK_SILENCE = "U"; */	/* 85 */
char *MSG_AKILL = "AKILL";	/* AKILL */
/* char *TOK_AKILL = "V"; */	/* 86 */
char *MSG_KLINE = "KLINE";	/* KLINE */
/* char *TOK_KLINE = "W"; */	/* 87 */
char *MSG_UNKLINE = "UNKLINE";	/* UNKLINE */
/* char *TOK_UNKLINE = "X"; */	/* 88 */
char *MSG_RAKILL = "RAKILL";	/* RAKILL */
/* char *TOK_RAKILL = "Y"; */	/* 89 */
char *MSG_GNOTICE = "GNOTICE";	/* GNOTICE */
/* char *TOK_GNOTICE = "Z"; */	/* 90 */
char *MSG_GOPER = "GOPER";	/* GOPER */
/* char *TOK_GOPER = "["; */	/* 91 */
char *MSG_GLOBOPS = "GLOBOPS";	/* GLOBOPS */
/* char *TOK_GLOBOPS = "]"; */	/* 93 */
char *MSG_LOCOPS = "LOCOPS";	/* LOCOPS */
/* char *TOK_LOCOPS = "^"; */	/* 94 */
char *MSG_PROTOCTL = "PROTOCTL";	/* PROTOCTL */
/* char *TOK_PROTOCTL = "_"; */	/* 95 */
char *MSG_TRACE = "TRACE";	/* TRAC */
/* char *TOK_TRACE = "b"; */	/* 98 */
char *MSG_SQLINE = "SQLINE";	/* SQLINE */
/* char *TOK_SQLINE = "c"; */	/* 99 */
char *MSG_UNSQLINE = "UNSQLINE";	/* UNSQLINE */
/* char *TOK_UNSQLINE = "d"; */	/* 100 */
char *MSG_SVSNICK = "SVSNICK";	/* SVSNICK */
/* char *TOK_SVSNICK = "e"; */	/* 101 */
char *MSG_SVSNOOP = "SVSNOOP";	/* SVSNOOP */
/* char *TOK_SVSNOOP = "f"; */	/* 101 */
char *MSG_IDENTIFY = "IDENTIFY";	/* IDENTIFY */
/* char *TOK_IDENTIFY = "g"; */	/* 103 */
char *MSG_SVSKILL = "SVSKILL";	/* SVSKILL */
/* char *TOK_SVSKILL = "h"; */	/* 104 */
char *MSG_NICKSERV = "NICKSERV";	/* NICKSERV */
char *MSG_NS = "NS";
/* char *TOK_NICKSERV = "i"; */	/* 105 */
char *MSG_CHANSERV = "CHANSERV";	/* CHANSERV */
char *MSG_CS = "CS";
/* char *TOK_CHANSERV = "j"; */	/* 106 */
char *MSG_OPERSERV = "OPERSERV";	/* OPERSERV */
char *MSG_OS = "OS";
/* char *TOK_OPERSERV = "k"; */	/* 107 */
char *MSG_MEMOSERV = "MEMOSERV";	/* MEMOSERV */
char *MSG_MS = "MS";
/* char *TOK_MEMOSERV = "l"; */	/* 108 */
char *MSG_SERVICES = "SERVICES";	/* SERVICES */
/* char *TOK_SERVICES = "m"; */	/* 109 */
char *MSG_SVSMODE = "SVSMODE";	/* SVSMODE */
/* char *TOK_SVSMODE = "n"; */	/* 110 */
char *MSG_SAMODE = "SAMODE";	/* SAMODE */
/* char *TOK_SAMODE = "o"; */	/* 111 */
char *MSG_CHATOPS = "CHATOPS";	/* CHATOPS */
/* char *TOK_CHATOPS = "p"; */	/* 112 */
char *MSG_HELPSERV = "HELPSERV";	/* HELPSERV */
/* char *TOK_HELPSERV = "r"; */	/* 114 */
char *MSG_ZLINE = "ZLINE";	/* ZLINE */
/* char *TOK_ZLINE = "s"; */	/* 115 */
char *MSG_UNZLINE = "UNZLINE";	/* UNZLINE */
/* char *TOK_UNZLINE = "t"; */	/* 116 */
char *MSG_NETINFO = "NETINFO";	/* NETINFO */
/* char *TOK_NETINFO = "u"; */	/* 117 */
char *MSG_RULES = "RULES";	/* RULES */
/* char *TOK_RULES = "v"; */	/* 118 */
char *MSG_MAP = "MAP";	/* MAP */
/* char *TOK_MAP = "w"; */	/* 119 */
char *MSG_NETG = "NETG";	/* NETG */
/* char *TOK_NETG = "x"; */	/* 120 */
char *MSG_ADCHAT = "ADCHAT";	/* Adchat */
/* char *TOK_ADCHAT = "y"; */	/* 121 */
char *MSG_MAKEPASS = "MAKEPASS";	/* MAKEPASS */
/* char *TOK_MAKEPASS = "z"; */	/* 122 */
char *MSG_ADDHUB = "ADDHUB";	/* ADDHUB */
/* char *TOK_ADDHUB = "{"; */	/* 123 */
char *MSG_DELHUB = "DELHUB";	/* DELHUB */
/* char *TOK_DELHUB = "|"; */	/* 124 */
char *MSG_ADDCNLINE = "ADDCNLINE";	/* ADDCNLINE */
/* char *TOK_ADDCNLINE = "}"; */	/* 125 */
char *MSG_DELCNLINE = "DELCNLINE";	/* DELCNLINE */
/* char *TOK_DELCNLINE = "~"; */	/* 126 */
char *MSG_ADDOPER = "ADDOPER";	/* ADDOPER */
/* char *TOK_ADDOPER = ""; */	/* 127 */
char *MSG_DELOPER = "DELOPER";	/* DELOPER */
/* char *TOK_DELOPER = "!!"; */	/* 33 + 33 */
char *MSG_ADDQLINE = "ADDQLINE";	/* ADDQLINE */
/* char *TOK_ADDQLINE = "!\""; */	/* 33 + 34 */
char *MSG_DELQLINE = "DELQLINE";	/* DELQLINE */
/* char *TOK_DELQLINE = "!#"; */	/* 33 + 35 */
char *MSG_GSOP = "GSOP";	/* GSOP */
/* char *TOK_GSOP = "!$"; */	/* 33 + 36 */
char *MSG_ISOPER = "ISOPER";	/* ISOPER */
/* char *TOK_ISOPER = "!%"; */	/* 33 + 37 */
char *MSG_ADG = "ADG";	/* ADG */
/* char *TOK_ADG = "!&"; */	/* 33 + 38 */
char *MSG_NMON = "NMON";	/* NMON */
/* char *TOK_NMON = "!'"; */	/* 33 + 39 */
char *MSG_DALINFO = "DALINFO";	/* DALnet Credits */
/* char *TOK_DALINFO = "!("; */	/* 33 + 40 */
char *MSG_CREDITS = "CREDITS";	/* UltimateIRCd Credits and "Thanks To" */
/* char *TOK_CREDITS = "!)"; */	/* 33 + 41 */
char *MSG_OPERMOTD = "OPERMOTD";	/* OPERMOTD */
/* char *TOK_OPERMOTD = "!*"; */	/* 33 + 42 */
char *MSG_REMREHASH = "REMREHASH";	/* Remote Rehash */
/* char *TOK_REMREHASH = "!+"; */	/* 33 + 43 */
char *MSG_MONITOR = "MONITOR";	/* MONITOR */
/* char *TOK_MONITOR = "!,"; */	/* 33 + 44 */
char *MSG_GLINE = "GLINE";	/* The awesome g-line */
/* char *TOK_GLINE = "!-"; */	/* 33 + 45 */
char *MSG_REMGLINE = "REMGLINE";	/* remove g-line */
/* char *TOK_REMGLINE = "!."; */	/* 33 + 46 */
char *MSG_STATSERV = "STATSERV";	/* StatServ */
/* char *TOK_STATSERV = "!/"; */	/* 33 + 47 */
char *MSG_RULESERV = "RULESERV";	/* RuleServ */
/* char *TOK_RULESERV = "!0"; */	/* 33 + 48 */
char *MSG_SNETINFO = "SNETINFO";	/* SNetInfo */
/* char *TOK_SNETINFO = "!1"; */	/* 33 + 49 */
char *MSG_TSCTL = "TSCTL";	/* TSCTL */
/* char *TOK_TSCTL = "!3"; */	/* 33 + 51 */
char *MSG_SVSJOIN = "SVSJOIN";	/* SVSJOIN */
/* char *TOK_SVSJOIN = "!4"; */	/* 33 + 52 */
char *MSG_SAJOIN = "SAJOIN";	/* SAJOIN */
/* char *TOK_SAJOIN = "!5"; */	/* 33 + 53 */
char *MSG_SDESC = "SDESC";	/* SDESC */
/* char *TOK_SDESC = "!6"; */	/* 33 + 54 */
char *MSG_UNREALINFO = "UNREALINFO";	/* Unreal Info */
/* char *TOK_UNREALINFO = "!7"; */	/* 33 + 55 */
char *MSG_SETHOST = "SETHOST";	/* sethost */
/* char *TOK_SETHOST = "!8"; */	/* 33 + 56 */
char *MSG_SETIDENT = "SETIDENT";	/* set ident */
/* char *TOK_SETIDENT = "!9"; */	/* 33 + 57 */
char *MSG_SETNAME = "SETNAME";	/* set Realname */
/* char *TOK_SETNAME = "! "; */	/* 33 + 59 */
char *MSG_CHGHOST = "CHGHOST";	/* Changehost */
/* char *TOK_CHGHOST = "!<"; */	/* 33 + 60 */
char *MSG_CHGIDENT = "CHGIDENT";	/* Change Ident */
/* char *TOK_CHGIDENT = "!="; */	/* 33 + 61 */
char *MSG_RANDQUOTE = "RANDQUOTE";	/* Random Quote */
/* char *TOK_RANDQUOTE = "!>"; */	/* 33 + 62 */
char *MSG_ADDQUOTE = "ADDQUOTE";	/* Add Quote */
/* char *TOK_ADDQUOTE = "!?"; */	/* 33 + 63 */
char *MSG_ADDGQUOTE = "ADDGQUOTE";	/* Add Global Quote */
/* char *TOK_ADDGQUOTE = "!@"; */	/* 33 + 64 */
char *MSG_ADDULINE = "ADDULINE";	/* Adds an U Line to ircd.conf file */
/* char *TOK_ADDULINE = "!B"; */	/* 33 + 66 */
char *MSG_DELULINE = "DELULINE";	/* Removes an U line from the ircd.conf */
/* char *TOK_DELULINE = "!C"; */	/* 33 + 67 */
char *MSG_KNOCK = "KNOCK";	/* Knock Knock - Who's there? */
/* char *TOK_KNOCK = "!D"; */	/* 33 + 68 */
char *MSG_SETTINGS = "SETTINGS";	/* Settings */
/* char *TOK_SETTINGS = "!E"; */	/* 33 + 69 */
char *MSG_IRCOPS = "IRCOPS";	/* Shows Online IRCOps */
/* char *TOK_IRCOPS = "!F"; */	/* 33 + 70 */
char *MSG_SVSPART = "SVSPART";	/* SVSPART */
/* char *TOK_SVSPART = "!G"; */	/* 33 + 71 */
char *MSG_SAPART = "SAPART";	/* SAPART */
/* char *TOK_SAPART = "!H"; */	/* 33 + 72 */
char *MSG_VCTRL = "VCTRL";	/* VCTRL */
/* char *TOK_VCTRL = "!I"; */	/* 33 + 73 */
char *MSG_GCLIENT = "GCLIENT";	/* GLIENT */
/* char *TOK_GCLIENT = "!J"; */	/* 33 + 74 */
char *MSG_CHANNEL = "CHANNEL";	/* CHANNEL */
/* char *TOK_CHANNEL = "!K"; */	/* 33 + 75 */
char *MSG_UPTIME = "UPTIME";	/* UPTIME */
/* char *TOK_UPTIME = "!L"; */	/* 33 + 76 */
char *MSG_FAILOPS = "FAILOPS";	/* FAILOPS */
/* char *TOK_FAILOPS = "!M"; */	/* 33 + 77 */

char *MSG_RPING = "RPING";	/* RPING */
/* char *TOK_RPING = "!P"; */	/* 33 + 80 */
char *MSG_RPONG = "RPONG";	/* RPONG */
/* char *TOK_RPONG = "!Q"; */	/* 33 + 81 */
char *MSG_UPING = "UPING";	/* UPING */
/* char *TOK_UPING = "!R"; */	/* 33 + 82 */
char *MSG_COPYRIGHT = "COPYRIGHT";	/* Copyright */
/* char *TOK_COPYRIGHT = "!S"; */	/* 33 + 83 */
char *MSG_BOTSERV = "BOTSERV";	/* BOTSERV */
char *MSG_BS = "BS";
/* char *TOK_BOTSERV = "!T"; */	/* 33 + 84 */
char *MSG_ROOTSERV = "ROOTSERV";	/* ROOTSERV */
char *MSG_RS = "RS";
/* char *TOK_ROOTSERV = "!U"; */	/* 33 + 85 */
char *MSG_SVINFO = "SVINFO";
char *MSG_CAPAB = "CAPAB";
char *MSG_BURST = "BURST";
char *MSG_SJOIN = "SJOIN";
char *MSG_CLIENT = "CLIENT";
char *MSG_SMODE = "SMODE";

/* Umodes */
#define UMODE_FAILOP	 	0x00020000	/* Shows some global messages */
#define UMODE_SERVICESOPER	0x00040000	/* Services Oper */
#define UMODE_ALTADMIN	 	0x00080000	/* Admin */
#define UMODE_SERVNOTICE 	0x00100000	/* server notices such as kill */
#define UMODE_KILLS	 		0x00200000	/* Show server-kills... */
#define UMODE_FLOOD	 		0x00400000	/* Receive flood warnings */
#define UMODE_CHATOP	 	0x00800000	/* can receive chatops */
#define UMODE_SUPER			0x01000000	/* Oper Is Protected from Kick's and Kill's */
#define UMODE_NGLOBAL 		0x02000000	/* See Network Globals */
#define UMODE_WHOIS 		0x04000000	/* Lets Opers see when people do a /WhoIs on them */
#define UMODE_NETINFO 		0x08000000	/* Server link, Delink Notces etc. */
#define UMODE_MAGICK 		0x10000000	/* Allows Opers To See +s and +p Channels */
#define UMODE_IRCADMIN 		0x20000000	/* Marks the client as an IRC Administrator */
#define UMODE_WATCHER		0x40000000	/* Recive Monitor Globals */
#define UMODE_NETMON		0x80000000	/* Marks the client as an Network Monitor */

static void m_server( char *origin, char **argv, int argc, int srv );
static void m_svsmode( char *origin, char **argv, int argc, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );
static void m_vctrl( char *origin, char **argv, int argc, int srv );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN,
	/* Protocol options negotiated at link by this IRCd */
	PROTOCOL_TOKEN,
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
	{&MSG_SERVER,    0,		m_server,	0},
	{&MSG_SVSMODE,   0,   m_svsmode,   0},
	{&MSG_NICK,      &TOK_NICK,      m_nick,      0},
	{&MSG_VCTRL,     0,     m_vctrl,     0},
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
	{'S', UMODE_SERVICES, 0, 0},
	{'P', UMODE_SADMIN, 0, 0},
	{'T', UMODE_TECHADMIN, 0, 0},
	{'N', UMODE_NETADMIN, 0, 0},
	{'a', UMODE_SERVICESOPER, 0, 0},
	{'Z', UMODE_IRCADMIN, 0, 0},
	{'z', UMODE_ADMIN, 0, 0},
	{'i', UMODE_ALTADMIN, 0, 0},
	{'p', UMODE_SUPER, 0, 0},
	{'O', UMODE_LOCOP, 0, 0},
	{'r', UMODE_REGNICK, 0, 0},
	{'w', UMODE_WALLOP, 0, 0},
	{'g', UMODE_FAILOP, 0, 0},
	{'h', UMODE_HELPOP, 0, 0},
	{'s', UMODE_SERVNOTICE, 0, 0},
	{'k', UMODE_KILLS, 0, 0},
	{'B', UMODE_RBOT, 0, 0},
	{'b', UMODE_SBOT, 0, 0},
	{'c', UMODE_CLIENT, 0, 0},
	{'f', UMODE_FLOOD, 0, 0},
	{'x', UMODE_HIDE, 0, 0},
	{'W', UMODE_WATCHER, 0, 0},
	MODE_INIT_END()
};

void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
	send_cmd( "%s %s", MSG_PASS, pass );
	send_cmd( "%s %s %d :%s", MSG_SERVER, name, numeric, infoline );
	send_cmd( "%s TOKEN CLIENT", MSG_PROTOCTL );
}

void send_cmode( const char *sourceserver, const char *sourceuser, const char *chan, const char *mode, const char *args, const unsigned long ts )
{
	send_cmd( ":%s %s %s %s %s %lu", sourceuser, MSGTOK( MODE ), chan, mode, args, ts );
}

void send_nick( const char *nick, const unsigned long ts, const char *newmode, const char *ident, const char *host, const char *server, const char *realname )
{
	send_cmd( "%s %s 1 %lu %s %s %s 0 :%s", MSGTOK( NICK ), nick, ts, ident, host, server, realname );
	send_cmd( ":%s %s %s :%s", nick, MSGTOK( MODE ), nick, newmode );
}

void send_vctrl( const int uprot, const int nicklen, const int modex, const int gc, const char *netname )
{
	send_cmd( "%s %d %d %d %d 0 0 0 0 0 0 0 0 0 0 :%s", MSG_VCTRL, uprot, nicklen, modex, gc, netname );
}

void send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts )
{
	send_cmd( ":%s %s %s@%s %lu %lu %s :%s", source, MSG_GLINE, ident, host, ( ts + length ), ts, setby, reason );
}

void send_rakill( const char *source, const char *host, const char *ident )
{
	/* ultimate2 needs an oper to remove */
	send_cmd( ":%s %s :%s@%s", ns_botptr->name, MSG_REMGLINE, host, ident );
}

void send_burst( int b )
{
	if( b == 0 ) {
		send_cmd( "BURST 0" );
	} else {
		send_cmd( "BURST" );
	}
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
		do_svsmode_user( argv[0], argv[1], argv[2] );
	}
}

static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( !srv )
		do_nick( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], 
			NULL, NULL, NULL, NULL, argv[7], NULL, NULL );
	else
		do_nickchange( origin, argv[0], NULL );
}

/*
 *  argv[0] = ultimate protocol
 *  argv[1] = nickname length
 *  argv[2] = Global Connect Notices
 *  argv[3] = Reserved for future extentions
 *  argv[4] = Reserved for future extentions
 *  argv[5] = Reserved for future extentions
 *  argv[6] = Reserved for future extentions
 *  argv[7] = Reserved for future extentions
 *  argv[8] = Reserved for future extentions
 *  argv[9] = Reserved for future extentions
 *  argv[10] = Reserved for future extentions
 *  argv[11] = Reserved for future extentions
 *  argv[12] = Reserved for future extentions
 *  argv[13] = Reserved for future extentions
 *  argv[14] = ircnet
 */
static void m_vctrl( char *origin, char **argv, int argc, int srv )
{
	do_vctrl( argv[0], argv[1], argv[2], argv[3], argv[14] );
}
