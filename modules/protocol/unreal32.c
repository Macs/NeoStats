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
** $Id: unreal32.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "protocol.h"

/* Messages/Tokens */
char *MSG_PRIVATE = "PRIVMSG";/* PRIV */
char *TOK_PRIVATE = "!";/* 33 */
char *MSG_WHOIS = "WHOIS";/* WHOI */
char *TOK_WHOIS = "#";/* 35 */
char *MSG_WHOWAS = "WHOWAS";/* WHOW */
char *TOK_WHOWAS = "$";/* 36 */
char *MSG_USER = "USER";/* USER */
char *TOK_USER = "%";/* 37 */
char *MSG_NICK = "NICK";/* NICK */
char *TOK_NICK = "&";/* 38 */
char *MSG_SERVER = "SERVER";/* SERV */
char *TOK_SERVER = "'";/* 39 */
char *MSG_LIST = "LIST";/* LIST */
char *TOK_LIST = "(";/* 40 */
char *MSG_TOPIC = "TOPIC";/* TOPI */
char *TOK_TOPIC = ")";/* 41 */
char *MSG_INVITE = "INVITE";/* INVI */
char *TOK_INVITE = "*";/* 42 */
char *MSG_VERSION = "VERSION";/* VERS */
char *TOK_VERSION = "+";/* 43 */
char *MSG_QUIT = "QUIT";/* QUIT */
char *TOK_QUIT = ",";/* 44 */
char *MSG_SQUIT = "SQUIT";/* SQUI */
char *TOK_SQUIT = "-";/* 45 */
char *MSG_KILL = "KILL";/* KILL */
char *TOK_KILL = ".";/* 46 */
char *MSG_INFO = "INFO";/* INFO */
char *TOK_INFO = "/";/* 47 */
char *MSG_LINKS = "LINKS";/* LINK */
char *TOK_LINKS = "0";/* 48 */
char *MSG_SUMMON = "SUMMON";/* SUMM */
char *TOK_SUMMON = "1";/* 49 */
char *MSG_STATS = "STATS";/* STAT */
char *TOK_STATS = "2";/* 50 */
char *MSG_USERS = "USERS";/* USER -> USRS */
char *TOK_USERS = "3";/* 51 */
char *MSG_HELP = "HELP";/* HELP */
char *MSG_HELPOP = "HELPOP";/* HELP */
char *TOK_HELP = "4";/* 52 */
char *MSG_ERROR = "ERROR";/* ERRO */
char *TOK_ERROR = "5";/* 53 */
char *MSG_AWAY = "AWAY";/* AWAY */
char *TOK_AWAY = "6";/* 54 */
char *MSG_CONNECT = "CONNECT";/* CONN */
char *TOK_CONNECT = "7";/* 55 */
char *MSG_PING = "PING";/* PING */
char *TOK_PING = "8";/* 56 */
char *MSG_PONG = "PONG";/* PONG */
char *TOK_PONG = "9";/* 57 */
char *MSG_OPER = "OPER";/* OPER */
char *TOK_OPER = ";";/* 59 */
char *MSG_PASS = "PASS";/* PASS */
char *TOK_PASS = "<";/* 60 */
char *MSG_WALLOPS = "WALLOPS";/* WALL */
char *TOK_WALLOPS = "=";/* 61 */
char *MSG_TIME = "TIME";/* TIME */
char *TOK_TIME = ">";/* 62 */
char *MSG_NAMES = "NAMES";/* NAME */
char *TOK_NAMES = "?";/* 63 */
char *MSG_ADMIN = "ADMIN";/* ADMI */
char *TOK_ADMIN = "@";/* 64 */
char *MSG_NOTICE = "NOTICE";/* NOTI */
char *TOK_NOTICE = "B";/* 66 */
char *MSG_JOIN = "JOIN";/* JOIN */
char *TOK_JOIN = "C";/* 67 */
char *MSG_PART = "PART";/* PART */
char *TOK_PART = "D";/* 68 */
char *MSG_LUSERS = "LUSERS";/* LUSE */
char *TOK_LUSERS = "E";/* 69 */
char *MSG_MOTD = "MOTD";/* MOTD */
char *TOK_MOTD = "F";/* 70 */
char *MSG_MODE = "MODE";/* MODE */
char *TOK_MODE = "G";/* 71 */
char *MSG_KICK = "KICK";/* KICK */
char *TOK_KICK = "H";/* 72 */
char *MSG_SERVICE = "SERVICE";/* SERV -> SRVI */
char *TOK_SERVICE = "I";/* 73 */
char *MSG_USERHOST = "USERHOST";/* USER -> USRH */
char *TOK_USERHOST = "J";/* 74 */
char *MSG_ISON = "ISON";/* ISON */
char *TOK_ISON = "K";/* 75 */
char *MSG_REHASH = "REHASH";/* REHA */
char *TOK_REHASH = "O";/* 79 */
char *MSG_RESTART = "RESTART";/* REST */
char *TOK_RESTART = "P";/* 80 */
char *MSG_CLOSE = "CLOSE";/* CLOS */
char *TOK_CLOSE = "Q";/* 81 */
char *MSG_DIE = "DIE";/* DIE */
char *TOK_DIE = "R";/* 82 */
char *MSG_HASH = "HASH";/* HASH */
char *TOK_HASH = "S";/* 83 */
char *MSG_DNS = "DNS";/* DNS -> DNSS */
char *TOK_DNS = "T";/* 84 */
char *MSG_SILENCE = "SILENCE";/* SILE */
char *TOK_SILENCE = "U";/* 85 */
char *MSG_AKILL = "AKILL";/* AKILL */
char *TOK_AKILL = "V";/* 86 */
char *MSG_KLINE = "KLINE";/* KLINE */
char *TOK_KLINE = "W";/* 87 */
char *MSG_UNKLINE = "UNKLINE";/* UNKLINE */
char *TOK_UNKLINE = "X";/* 88 */
char *MSG_RAKILL = "RAKILL";/* RAKILL */
char *TOK_RAKILL = "Y";/* 89 */
char *MSG_GNOTICE = "GNOTICE";/* GNOTICE */
char *TOK_GNOTICE = "Z";/* 90 */
char *MSG_GOPER = "GOPER";/* GOPER */
char *TOK_GOPER = "[";/* 91 */
char *MSG_GLOBOPS = "GLOBOPS";/* GLOBOPS */
char *TOK_GLOBOPS = "]";/* 93 */
char *MSG_LOCOPS = "LOCOPS";/* LOCOPS */
char *TOK_LOCOPS = "^";/* 94 */
char *MSG_PROTOCTL = "PROTOCTL";/* PROTOCTL */
char *TOK_PROTOCTL = "_";/* 95 */
char *MSG_WATCH = "WATCH";/* WATCH */
char *TOK_WATCH = "`";/* 96 */
char *MSG_TRACE = "TRACE";/* TRAC */
char *TOK_TRACE = "b";/* 97 */
char *MSG_SQLINE = "SQLINE";/* SQLINE */
char *TOK_SQLINE = "c";/* 98 */
char *MSG_UNSQLINE = "UNSQLINE";/* UNSQLINE */
char *TOK_UNSQLINE = "d";/* 99 */
char *MSG_SVSNICK = "SVSNICK";/* SVSNICK */
char *TOK_SVSNICK = "e";/* 100 */
char *MSG_SVSNOOP = "SVSNOOP";/* SVSNOOP */
char *TOK_SVSNOOP = "f";/* 101 */
char *MSG_IDENTIFY = "IDENTIFY";/* IDENTIFY */
char *TOK_IDENTIFY = "g";/* 102 */
char *MSG_SVSKILL = "SVSKILL";/* SVSKILL */
char *TOK_SVSKILL = "h";/* 103 */
char *MSG_NICKSERV = "NICKSERV";/* NICKSERV */
char *MSG_NS = "NS";
char *TOK_NICKSERV = "i";/* 104 */
char *MSG_CHANSERV = "CHANSERV";/* CHANSERV */
char *MSG_CS = "CS";
char *TOK_CHANSERV = "j";/* 105 */
char *MSG_OPERSERV = "OPERSERV";/* OPERSERV */
char *MSG_OS = "OS";
char *TOK_OPERSERV = "k";/* 106 */
char *MSG_MEMOSERV = "MEMOSERV";/* MEMOSERV */
char *MSG_MS = "MS";
char *TOK_MEMOSERV = "l";/* 107 */
char *MSG_SERVICES = "SERVICES";/* SERVICES */
char *TOK_SERVICES = "m";/* 108 */
char *MSG_SVSMODE = "SVSMODE";/* SVSMODE */
char *TOK_SVSMODE = "n";/* 109 */
char *MSG_SAMODE = "SAMODE";/* SAMODE */
char *TOK_SAMODE = "o";/* 110 */
char *MSG_CHATOPS = "CHATOPS";/* CHATOPS */
char *TOK_CHATOPS = "p";/* 111 */
char *MSG_ZLINE = "ZLINE";/* ZLINE */
char *TOK_ZLINE = "q";/* 112 */
char *MSG_UNZLINE = "UNZLINE";/* UNZLINE */
char *TOK_UNZLINE = "r";/* 113 */
char *MSG_HELPSERV = "HELPSERV";/* HELPSERV */
char *MSG_HS = "HS";
char *TOK_HELPSERV = "s";/* 114 */
char *MSG_RULES = "RULES";/* RULES */
char *TOK_RULES = "t";/* 115 */
char *MSG_MAP = "MAP";/* MAP */
char *TOK_MAP = "u";/* 117 */
char *MSG_SVS2MODE = "SVS2MODE";/* SVS2MODE */
char *TOK_SVS2MODE = "v";/* 118 */
char *MSG_DALINFO = "DALINFO";/* dalinfo */
char *TOK_DALINFO = "w";/* 119 */
char *MSG_ADMINCHAT = "ADCHAT";/* Admin chat */
char *TOK_ADMINCHAT = "x";/* 120 */
char *MSG_MKPASSWD = "MKPASSWD";/* MKPASSWD */
char *TOK_MKPASSWD = "y";/* 121 */
char *MSG_ADDLINE = "ADDLINE";/* ADDLINE */
char *TOK_ADDLINE = "z";/* 122 */
char *MSG_GLINE = "GLINE";/* The awesome g-line */
char *TOK_GLINE = "}";/* 125 */
char *MSG_SJOIN = "SJOIN";
char *TOK_SJOIN = "~";
char *MSG_SETHOST = "SETHOST";/* sethost */
char *TOK_SETHOST = "AA";/* 127 4ever !;) */
char *MSG_NACHAT = "NACHAT";/* netadmin chat */
char *TOK_NACHAT = "AC";/* *beep* */
char *MSG_SETIDENT = "SETIDENT";
char *TOK_SETIDENT = "AD";
char *MSG_SETNAME = "SETNAME";/* set GECOS */
char *TOK_SETNAME = "AE";/* its almost unreeaaall... */
char *MSG_LAG = "LAG";/* Lag detect */
char *TOK_LAG = "AF";/* a or ? */
char *MSG_STATSERV = "STATSERV";/* alias */
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
char *TOK_SVSJOIN = "BX";
char *MSG_SAJOIN = "SAJOIN";
char *TOK_SAJOIN = "AX";
char *MSG_SVSPART = "SVSPART";
char *TOK_SVSPART = "BT";
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
char *MSG_REMGLINE = "REMGLINE";/* remove g-line */
char *TOK_REMGLINE = "BG";
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
char *MSG_NEWJOIN = "NEWJOIN";/* For CR Java Chat */
char *MSG_POST = "POST";
char *TOK_POST = "BN";
char *MSG_INFOSERV = "INFOSERV";
char *MSG_IS = "IS";
char *TOK_INFOSERV = "BO";
char *MSG_BOTSERV = "BOTSERV";
char *TOK_BOTSERV = "BS";
char *MSG_CYCLE = "CYCLE";
char *TOK_CYCLE = "BP";
char *MSG_MODULE = "MODULE";
char *TOK_MODULE = "BQ";
char *MSG_SENDSNO = "SENDSNO";
char *TOK_SENDSNO = "Ss";
char *MSG_EOS = "EOS";
char *TOK_EOS = "ES";

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

static void m_server( char *origin, char **argv, int argc, int srv );
static void m_umode2( char *origin, char **argv, int argc, int srv );
static void m_svsmode( char *origin, char **argv, int argc, int srv );
static void m_nick( char *origin, char **argv, int argc, int srv );
static void m_eos( char *origin, char **argv, int argc, int srv );
static void m_sjoin( char *origin, char **argv, int argc, int srv );
static void m_smo( char *origin, char **argv, int argc, int srv );
static void m_swhois( char *origin, char **argv, int argc, int srv );
static void m_tkl( char *origin, char **argv, int argc, int srv );
static void m_svskill( char *origin, char **argv, int argc, int srv);

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_SJOIN,
	/* Protocol options negotiated at link by this IRCd */
	PROTOCOL_TOKEN | PROTOCOL_NICKIP | PROTOCOL_NICKv2 | PROTOCOL_SJ3,
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
	"+OwoSq",
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
	{&MSG_EOS, &TOK_EOS, m_eos, 0},
	{&MSG_TKL, &TOK_TKL, m_tkl, 0},
	{&MSG_SVSKILL, &TOK_SVSKILL, m_svskill, 0},
	IRC_CMD_END()
};

mode_init chan_umodes[] = 
{
	{'h', CUMODE_HALFOP, 0, '%'},
	{'a', CUMODE_CHANPROT, 0, '~'},
	{'q', CUMODE_CHANOWNER, 0, '*'},
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

static const char Base64[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char Pad64 = '=';

int b64_decode( char const *src, unsigned char *target, int targsize )
{
	int tarindex, state, ch;
	char *pos;

	state = 0;
	tarindex = 0;

	while( ( ch = *src++ ) != '\0' )
	{
		if( isspace( ch ) )	/* Skip whitespace anywhere. */
			continue;

		if( ch == Pad64 )
			break;

		pos = strchr( Base64, ch );
		if( pos == 0 ) 		/* A non-base64 character. */
			return( -1 );

		switch( state )
		{
		case 0:
			if( target )
			{
				if( tarindex >= targsize )
					return( -1 );
				target[tarindex] = ( unsigned char )( pos - Base64 ) << 2;
			}
			state = 1;
			break;
		case 1:
			if( target )
			{
				if( tarindex + 1 >= targsize )
					return( -1 );
				target[tarindex]   |= ( pos - Base64 ) >> 4;
				target[tarindex+1]  = ( unsigned char )( ( pos - Base64 ) & 0x0f )
							<< 4 ;
			}
			tarindex++;
			state = 2;
			break;
		case 2:
			if( target )
			{
				if( tarindex + 1 >= targsize )
					return( -1 );
				target[tarindex]   |= ( pos - Base64 ) >> 2;
				target[tarindex+1]  = ( unsigned char )( ( pos - Base64 ) & 0x03 )
							<< 6;
			}
			tarindex++;
			state = 3;
			break;
		case 3:
			if( target )
			{
				if( tarindex >= targsize )
					return( -1 );
				target[tarindex] |= ( pos - Base64 );
			}
			tarindex++;
			state = 0;
			break;
		default:
			abort();
		}
	}

	/*
	 * We are done decoding Base-64 chars.  Let's see if we ended
	 * on a byte boundary, and/or with erroneous trailing characters.
	 */

	if( ch == Pad64 )
	{		/* We got a pad char. */
		ch = *src++;		/* Skip it, get next. */
		switch( state )
		{
		case 0:		/* Invalid = in first position */
		case 1:		/* Invalid = in second position */
			return( -1 );

		case 2:		/* Valid, means one byte of info */
			/* Skip any number of spaces. */
			for( ( void )NULL; ch != '\0'; ch = *src++ )
				if( !isspace( ch ) )
					break;
			/* Make sure there is another trailing = sign. */
			if( ch != Pad64 )
				return( -1 );
			ch = *src++;		/* Skip the = */
			/* Fall through to "single trailing =" case. */
			/* FALLTHROUGH */

		case 3:		/* Valid, means two bytes of info */
			/*
			 * We know this char is an =.  Is there anything but
			 * whitespace after it?
			 */
			for( ( void )NULL; ch != '\0'; ch = *src++ )
				if( !isspace( ch ) )
					return( -1 );

			/*
			 * Now make sure for cases 2 and 3 that the "extra"
			 * bits that slopped past the last full byte were
			 * zeros.  If we don't check them, they become a
			 * subliminal channel.
			 */
			if( target && target[tarindex] != 0 )
				return( -1 );
		}
	}
	else
	{
		/*
		 * We ended by seeing the end of the string.  Make sure we
		 * have no partial bytes lying around.
		 */
		if( state != 0 )
			return( -1 );
	}

	return( tarindex );
}

int decode_ip( const char *buf )
{
	size_t len = strlen( buf );
	char targ[25];
	struct in_addr ia;

	b64_decode( buf, (unsigned char *)targ, 25 );
	ia = *( struct in_addr * )targ;
	if( len == 8 )  /* IPv4 */
		return ia.s_addr;
	return 0;
}

void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink )
{
/* PROTOCTL NOQUIT TOKEN NICKv2 SJOIN SJOIN2 UMODE2 VL SJ3 NS SJB64 TKLEXT NICKIP CHANMODES=be,kfL,l,psmntirRcOAQKVGCuzNSMT */
	send_cmd( "%s TOKEN NICKv2 SJOIN SJOIN2 UMODE2 VL SJ3 NICKIP VHP", MSGTOK( PROTOCTL ) );
	send_cmd( "%s %s", MSGTOK( PASS ), pass );
	send_cmd( "%s %s %d :U0-*-%d %s", MSGTOK( SERVER ), name, 1, numeric, infoline );
}

void send_sjoin( const char *source, const char *target, const char *chan, const unsigned long ts )
{
	send_cmd( ":%s %s %lu %s + :%s", source, MSGTOK( SJOIN ), ts, chan, target );
}

void send_cmode( const char *server, const char *user, const char *chan, const char *mode, const char *args, unsigned long ts )
{
	send_cmd( ":%s %s %s %s %s", user, MSGTOK( MODE ), chan, mode, args );
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
 *  if NICKIP:
 *    argv[9] = ip
 *    argv[10] = info
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
			if( ircd_srv.protocol & PROTOCOL_NICKIP )
			{
				char ip[25];

				ircsnprintf( ip, 25, "%d", ntohl( decode_ip( argv[9] ) ) );
				do_nick( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], 
					ip, argv[6], argv[7], argv[8], argv[10], NULL, NULL );
			}
			else
			{
				do_nick( argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], 
					NULL, argv[6], argv[7], argv[8], argv[9], NULL, NULL );
			}
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

/** m_eos
 *
 *  process EOS command
 *  RX:
 *    :irc.foonet.com EOS
 *  Format:
 *    :origin EOS
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

static void m_eos( char *origin, char **argv, int argc, int srv )
{
	do_eos( origin );
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

/** m_svskill
 *
 *  process SVSKILL command
 *  RX:
 *    DEBUG1 NeoStats - origin: NickServ 
 *    DEBUG1 NeoStats - cmd   : h 
 *    DEBUG1 NeoStats - args  : Sean :NickServ (GHOST command used by Eliot) 
 *  Format:
 *    :origin SMO ? :message
 *
 *  @param origin source of kill (user/server)
 *  @param argv list of message parameters
 *    argv[0] - Killer
 *    argv[1] - Comment
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

static void m_svskill( char *origin, char **argv, int argc, int srv )
{
	do_kill(origin, argv[0], argv[1]);
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
