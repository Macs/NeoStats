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
** $Id: ircup10.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "protocol.h"
#include "base64.h"
#include "numerics.h"

/* Messages/Tokens */

char *MSG_PRIVATE = "PRIVMSG";
char *TOK_PRIVATE = "P";
char *MSG_WHO = "WHO";
char *TOK_WHO = "H";
char *MSG_WHOIS = "WHOIS";
char *TOK_WHOIS = "W";
char *MSG_WHOWAS = "WHOWAS";
char *TOK_WHOWAS = "X";
char *MSG_USER = "USER";
char *TOK_USER = "USER";
char *MSG_NICK = "NICK";
char *TOK_NICK = "N";
char *MSG_SERVER = "SERVER";
char *TOK_SERVER = "S";
char *MSG_LIST = "LIST";
char *TOK_LIST = "LIST";
char *MSG_TOPIC = "TOPIC";
char *TOK_TOPIC = "T";
char *MSG_INVITE = "INVITE";
char *TOK_INVITE = "I";
char *MSG_VERSION = "VERSION";
char *TOK_VERSION = "V";
char *MSG_QUIT = "QUIT";
char *TOK_QUIT = "Q";
char *MSG_SQUIT = "SQUIT";
char *TOK_SQUIT = "SQ";
char *MSG_KILL = "KILL";
char *TOK_KILL = "D";
char *MSG_INFO = "INFO";
char *TOK_INFO = "F";
char *MSG_LINKS = "LINKS";
char *TOK_LINKS = "LI";
char *MSG_STATS = "STATS";
char *TOK_STATS = "R";
char *MSG_HELP = "HELP";
char *TOK_HELP = "HELP";
char *MSG_ERROR = "ERROR";
char *TOK_ERROR = "Y";
char *MSG_AWAY = "AWAY";
char *TOK_AWAY = "A";
char *MSG_CONNECT = "CONNECT";
char *TOK_CONNECT = "CO";
char *MSG_MAP = "MAP";
char *TOK_MAP = "MAP";
char *MSG_PING = "PING";
char *TOK_PING = "G";
char *MSG_PONG = "PONG";
char *TOK_PONG = "Z";
char *MSG_OPER = "OPER";
char *TOK_OPER = "OPER";
char *MSG_PASS = "PASS";
char *TOK_PASS = "PA";
char *MSG_WALLOPS = "WALLOPS";
char *TOK_WALLOPS = "WA";
char *MSG_WALLUSERS = "WALLUSERS";
char *TOK_WALLUSERS = "WU";
char *MSG_DESYNCH = "DESYNCH";
char *TOK_DESYNCH = "DS";
char *MSG_TIME = "TIME";
char *TOK_TIME = "TI";
char *MSG_SETTIME = "SETTIME";
char *TOK_SETTIME = "SE";
char *MSG_RPING = "RPING";
char *TOK_RPING = "RI";
char *MSG_RPONG = "RPONG";
char *TOK_RPONG = "RO";
char *MSG_NAMES = "NAMES";
char *TOK_NAMES = "E";
char *MSG_ADMIN = "ADMIN";
char *TOK_ADMIN = "AD";
char *MSG_TRACE = "TRACE";
char *TOK_TRACE = "TR";
char *MSG_NOTICE = "NOTICE";
char *TOK_NOTICE = "O";
char *MSG_WALLCHOPS = "WALLCHOPS";
char *TOK_WALLCHOPS = "WC";
char *MSG_WALLVOICES = "WALLVOICES";
char *TOK_WALLVOICES = "WV";
char *MSG_CPRIVMSG = "CPRIVMSG";
char *TOK_CPRIVMSG = "CP";
char *MSG_CNOTICE = "CNOTICE";
char *TOK_CNOTICE = "CN";
char *MSG_JOIN = "JOIN";
char *TOK_JOIN = "J";
char *MSG_PART = "PART";
char *TOK_PART = "L";
char *MSG_LUSERS = "LUSERS";
char *TOK_LUSERS = "LU";
char *MSG_MOTD = "MOTD";
char *TOK_MOTD = "MO";
char *MSG_MODE = "MODE";
char *TOK_MODE = "M";
char *MSG_KICK = "KICK";
char *TOK_KICK = "K";
char *MSG_USERHOST = "USERHOST";
char *TOK_USERHOST = "USERHOST";
char *MSG_USERIP = "USERIP";
char *TOK_USERIP = "USERIP";
char *MSG_ISON = "ISON";
char *TOK_ISON = "ISON";
char *MSG_SQUERY = "SQUERY";
char *TOK_SQUERY = "SQUERY";
char *MSG_SERVLIST = "SERVLIST";
char *TOK_SERVLIST = "SERVSET";
char *MSG_SERVSET = "SERVSET";
char *TOK_SERVSET = "SERVSET";
char *MSG_REHASH = "REHASH";
char *TOK_REHASH = "REHASH";
char *MSG_RESTART = "RESTART";
char *TOK_RESTART = "RESTART";
char *MSG_CLOSE = "CLOSE";
char *TOK_CLOSE = "CLOSE";
char *MSG_DIE = "DIE";
char *TOK_DIE = "DIE";
char *MSG_HASH = "HASH";
char *TOK_HASH = "HASH";
char *MSG_DNS = "DNS";
char *TOK_DNS = "DNS";
char *MSG_SILENCE = "SILENCE";
char *TOK_SILENCE = "U";
char *MSG_GLINE = "GLINE";
char *TOK_GLINE = "GL";
char *MSG_BURST = "BURST";
char *TOK_BURST = "B";
char *MSG_UPING = "UPING";
char *TOK_UPING = "UP";
char *MSG_CREATE = "CREATE";
char *TOK_CREATE = "C";
char *MSG_DESTRUCT = "DESTRUCT";
char *TOK_DESTRUCT = "DE";
char *MSG_END_OF_BURST = "END_OF_BURST";
char *TOK_END_OF_BURST = "EB";
char *MSG_END_OF_BURST_ACK = "EOB_ACK";
char *TOK_END_OF_BURST_ACK = "EA";
char *MSG_PROTO = "PROTO";
char *TOK_PROTO = "PROTO";
char *MSG_JUPE = "JUPE";
char *TOK_JUPE = "JU";
char *MSG_OPMODE = "OPMODE";
char *TOK_OPMODE = "OM";
char *MSG_CLEARMODE = "CLEARMODE";
char *TOK_CLEARMODE = "CM";
char *MSG_ACCOUNT = "ACCOUNT";
char *TOK_ACCOUNT = "AC";
char *MSG_ASLL = "ASLL";
char *TOK_ASLL = "LL";
char *MSG_POST = "POST";
char *TOK_POST = "POST";
char *MSG_SET = "SET";
char *TOK_SET = "SET";
char *MSG_RESET = "RESET";
char *TOK_RESET = "RESET";
char *MSG_GET = "GET";
char *TOK_GET = "GET";
char *MSG_PRIVS = "PRIVS";
char *TOK_PRIVS = "PRIVS";

 /* User modes: */
#define UMODE_SERVNOTICE        0x00800000	/* See server notices */
#define UMODE_DEBUG             0x01000000	/* See hack notices */

/* I really hate including .c files but this is a temporary measure
 * while core/IRCu interaction is improved.
 */
#include "ircup10base.c"

static void m_nick( char *origin, char **argv, int argc, int srv );

ProtocolInfo protocol_info = 
{
	/* Protocol options required by this IRCd */
	PROTOCOL_TOKEN|PROTOCOL_NOQUIT|PROTOCOL_P10|PROTOCOL_B64SERVER|PROTOCOL_B64NICK|PROTOCOL_NICKIP|PROTOCOL_KICKPART,
	/* Protocol options negotiated at link by this IRCd */
	0,
	/* Features supported by this IRCd */
	FEATURE_SVSTIME,
	/* Max host length */
	63 ,
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
	250,
	/* Default operator modes for NeoStats service bots */
	"+o",
	/* Default channel mode for NeoStats service bots */
	"+o",
};

/* this is the command list and associated functions to run */
irc_cmd cmd_list[] = 
{
	/* Command Token Function usage */
	{&MSG_PRIVATE, &TOK_PRIVATE, m_private, 0},
	{&MSG_CPRIVMSG, &TOK_CPRIVMSG, m_private, 0},
	{&MSG_NOTICE, &TOK_NOTICE, m_notice, 0},
	{&MSG_CNOTICE, &TOK_CNOTICE, m_notice, 0},
	{&MSG_SERVER, &TOK_SERVER, m_server, 0},
	{&MSG_MODE, &TOK_MODE, m_mode, 0},
	{&MSG_NICK, &TOK_NICK, m_nick, 0},
	{&MSG_CREATE, &TOK_CREATE, m_create, 0},
	{&MSG_BURST, &TOK_BURST, m_burst, 0},
	{&MSG_END_OF_BURST, &TOK_END_OF_BURST, m_end_of_burst, 0},
	{&MSG_END_OF_BURST_ACK, &TOK_END_OF_BURST_ACK, _m_ignorecommand, 0},
	{&MSG_WALLOPS, &TOK_WALLOPS, m_wallops, 0},
	{&MSG_WALLUSERS, &TOK_WALLUSERS, m_wallusers, 0},
	IRC_CMD_END()
};

mode_init chan_umodes[] = 
{
	MODE_INIT_END()
};

mode_init chan_modes[] = 
{
	{'r', CMODE_RGSTRONLY, 0, 0},
	MODE_INIT_END()
};

mode_init user_umodes[] = 
{
	{'O', UMODE_LOCOP, 0, 0},
	{'g', UMODE_DEBUG, 0, 0},
	{'w', UMODE_WALLOP, 0, 0},
	{'s', UMODE_SERVNOTICE, 0, 0},
	{'d', UMODE_DEAF, 0, 0},
	{'k', UMODE_SERVICES, 0, 0},
	{'r', UMODE_REGNICK, 0, 0},
	{'x', UMODE_HIDE, 0, 0},
	MODE_INIT_END()
};

/*
1 <nickname>
2 <hops>
3 <TS>
4 <userid>
5 <host>
6 [<+modes>]
7+ [<mode parameters>]
-3 <base64 IP>
-2 <numeric>
-1 <fullname>
*/
/* R: AB N Mark 1 1076011621 a xxx.xxx.xxx.xxx DAqO4N ABAAB :M */
/* R: AB N TheEggMan 1 1076104492 ~eggy 64.XX.XXX.XXX +oiwg BAFtnj ABAAA :eggy */
/* R: ABAAH N m2 1076077934 */
static void m_nick( char *origin, char **argv, int argc, int srv )
{
	if( argc > 2 ) {
		char IPAddress[32];
		const char *modes;
		const char *modeptr;
		const char *account = NULL;
		int param;

		modes = ( argv[5][0] == '+' ) ? argv[5]: NULL;
		if( modes ) {
			param = 6;
			for( modeptr = modes; *modeptr; ++modeptr ) {
				switch( *modeptr ) {
				case 'r':
					account = argv[param++];
					break;
				default:
					break;
				} /* switch( *modeptr ) */
			} /* for( ) */
		} /* if( modes ) */

		ircsnprintf( IPAddress, 32, "%du", base64toIP( argv[argc-3]) );
		
		/*       nick,    hopcount, TS,     user,    host, */       
		do_nick( argv[0], argv[1], argv[2], argv[3], argv[4], 
			/* server, ip, servicestamp, modes, */
			base64_to_server( origin ), IPAddress, NULL, modes,
			/* vhost, realname, numeric, smodes */ 
			NULL, argv[argc-1], argv[argc-2], NULL );
	} else {
		do_nickchange( base64_to_nick( origin ), argv[0], argv[1] );
	}
}
