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
** $Id: ircrecv.c 3295 2008-02-24 02:51:03Z Fish $
*/

#include "neostats.h"
#include "protocol.h"
#include "ircsend.h"
#include "ircrecv.h"
#include "main.h"
#include "bots.h"
#include "modules.h"
#include "nsevents.h"
#include "modes.h"
#include "users.h"
#include "servers.h"
#include "channels.h"
#include "services.h"
#include "bans.h"
#include "dns.h"
#include "base64.h"

#define MOTD_FILENAME	"neostats.motd"
#define ADMIN_FILENAME	"neostats.admin"

typedef struct ProtocolEntry {
	char *token;
	unsigned int flag;
} ProtocolEntry;

extern ProtocolInfo *protocol_info;

extern irc_cmd *cmd_list;

static ProtocolEntry protocol_list[] =
{
	{"TOKEN",	PROTOCOL_TOKEN},
	{"CLIENT",	PROTOCOL_CLIENT},
	{"UNKLN",	PROTOCOL_UNKLN},
	{"NOQUIT",	PROTOCOL_NOQUIT},
	{"NICKIP",	PROTOCOL_NICKIP},
	{"NICKv2",	PROTOCOL_NICKv2},
	{"SJ3",		PROTOCOL_SJ3},	
	{NULL, 0}
};

irc_cmd intrinsic_cmd_list[] = 
{
	{&MSG_PRIVATE, &TOK_PRIVATE, _m_private, 0},
	{&MSG_NOTICE, &TOK_NOTICE, _m_notice, 0},
	{&MSG_STATS, &TOK_STATS, _m_stats, 0},
	{&MSG_VERSION, &TOK_VERSION, _m_version, 0},
	{&MSG_MOTD, &TOK_MOTD, _m_motd, 0},
	{&MSG_ADMIN, &TOK_ADMIN, _m_admin, 0},
	{&MSG_CREDITS, &TOK_CREDITS, _m_credits, 0},
	{&MSG_INFO, &TOK_INFO, _m_info, 0},
	{&MSG_SQUIT, &TOK_SQUIT, _m_squit, 0},
	{&MSG_AWAY, &TOK_AWAY, _m_away, 0},
	{&MSG_QUIT, &TOK_QUIT, _m_quit, 0},
	{&MSG_MODE, &TOK_MODE, _m_mode, 0},
	{&MSG_SVSJOIN, &TOK_SVSJOIN, _m_svsjoin, 0},
	{&MSG_SVSPART, &TOK_SVSPART, _m_svspart, 0},
	{&MSG_KILL, &TOK_KILL, _m_kill, 0},
	{&MSG_PING, &TOK_PING, _m_ping, 0},
	{&MSG_PONG, &TOK_PONG, _m_pong, 0},
	{&MSG_JOIN, &TOK_JOIN, _m_join, 0},
	{&MSG_PART, &TOK_PART, _m_part, 0},
	{&MSG_KICK, &TOK_KICK, _m_kick, 0},
	{&MSG_GLOBOPS, &TOK_GLOBOPS, _m_globops, 0},
	{&MSG_WALLOPS, &TOK_WALLOPS, _m_wallops, 0},
	{&MSG_SVINFO, &TOK_SVINFO, _m_svinfo, 0},
	{&MSG_NETINFO, &TOK_NETINFO, _m_netinfo, 0},
	{&MSG_SNETINFO, &TOK_SNETINFO, _m_snetinfo, 0},
	{&MSG_EOB, &TOK_EOB, _m_eob, 0},
	{&MSG_PROTOCTL, &TOK_PROTOCTL, _m_protoctl, 0},
	{&MSG_CAPAB, &TOK_CAPAB, _m_capab, 0},
	{&MSG_PASS, &TOK_PASS, _m_pass, 0},
	{&MSG_TOPIC, &TOK_TOPIC, _m_topic, 0},
	{&MSG_SVSNICK, &TOK_SVSNICK, _m_svsnick, 0},
	{&MSG_SETNAME, &TOK_SETNAME, _m_setname, 0},
	{&MSG_SETHOST, &TOK_SETHOST, _m_sethost, 0},
	{&MSG_SETIDENT, &TOK_SETIDENT, _m_setident, 0},
	{&MSG_CHGHOST, &TOK_CHGHOST, _m_chghost, 0},
	{&MSG_CHGIDENT, &TOK_CHGIDENT, _m_chgident, 0},
	{&MSG_CHGNAME, &TOK_CHGNAME, _m_chgname, 0},
	{&MSG_CHATOPS, &TOK_CHATOPS, _m_chatops, 0},
	{&MSG_SQLINE, &TOK_SQLINE, _m_sqline, 0},
	{&MSG_UNSQLINE, &TOK_UNSQLINE, _m_unsqline, 0},
	{&MSG_ZLINE, &TOK_ZLINE, _m_zline, 0},
	{&MSG_UNZLINE, &TOK_UNZLINE, _m_unzline, 0},
	{&MSG_AKILL, &TOK_AKILL, _m_akill, 0},
	{&MSG_RAKILL, &TOK_RAKILL, _m_rakill, 0},
	{&MSG_KLINE, &TOK_KLINE, _m_kline, 0},
	{&MSG_UNKLINE, &TOK_UNKLINE, _m_unkline, 0},
	{&MSG_GLINE, &TOK_GLINE, _m_gline, 0},
	{&MSG_REMGLINE, &TOK_REMGLINE, _m_remgline, 0},
	{&MSG_ERROR, &TOK_ERROR, _m_error, 0},
	{&MSG_WHOIS, &TOK_WHOIS, _m_whois, 0},
	{0, 0, 0, 0},
};

/*  _m_command functions are highly generic support functions for 
 *  use in protocol modules. If a protocol differs at all from 
 *  the RFC, they must provide their own local version of this 
 *  function. These	are purely to avoid protocol module bloat for
 *  the more common forms of these commands and allow protocol module
 *  coders to concentrate on the areas that need it.
 */

/** @brief _m_glopops
 *
 *  process GLOBOPS command
 *  RX:
 *    :Mark GLOBOPS :test globops
 *  Format:
 *    :origin GLOBOPS :message
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = message
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_globops( char *origin, char **argv, int argc, int srv )
{
	do_globops( origin, argv[0] );	
}

/** @brief _m_wallops
 *
 *  process WALLOPS command
 *  RX:
 *    :Mark WALLOPS :test wallops
 *  Format:
 *    :origin WALLOPS :message
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = message
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_wallops( char *origin, char **argv, int argc, int srv )
{
	do_wallops( origin, argv[0] );	
}

/** @brief _m_chatops
 *
 *  process CHATOPS command
 *  RX:
 *    :Mark CHATOPS :test chatops
 *  Format:
 *    :origin CHATOPS :message
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = message
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_chatops( char *origin, char **argv, int argc, int srv )
{
	do_chatops( origin, argv[0] );	
}

/** @brief _m_error
 *
 *  process ERROR command
 *  RX:
 *    :Mark ERROR :message
 *  Format:
 *    :origin ERROR :message
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = message
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_error( char *origin, char **argv, int argc, int srv )
{
	nlog (LOG_ERROR, "IRCD reported error: %s", argv[0] );
	do_exit (NS_EXIT_ERROR, argv[0] );
}

/** @brief _m_whois
 *
 *  process WHOIS command
 *  RX:
 *    :Mark WHOIS neostats :neostats
 *  Format:
 *    :origin WHOIS server :target
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = 
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_whois( char *origin, char **argv, int argc, int srv )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		do_whois( base64_to_nick( origin ), argv[0], argv[1] );
	else
		do_whois( origin, argv[0], argv[1] );
}

/** @brief _m_ignorecommand
 *
 *  silently drop command
 *  RX:
 *    N/A
 *  Format:
 *    :origin COMMAND parameters :parameters
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_ignorecommand( char *origin, char **argv, int argc, int srv )
{
}

/** @brief _m_pass
 *
 *  process PASS command
 *  RX:
 *    PASS :password
 *  Format:
 *    PASS :password
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = password
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_pass( char *origin, char **argv, int argc, int srv )
{
	
}

/** @brief _m_protoctl
 *
 *  process PROTOCTL command
 *  RX:
 *    PROTOCTL NOQUIT TOKEN 
 *  Format:
 *    PROTOCTL <token list>
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0 - argc] = tokens
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_protoctl( char *origin, char **argv, int argc, int srv )
{
	do_protocol( origin, argv, argc );
}

/** @brief _m_version
 *
 *  process VERSION command
 *  RX:
 *    :Mark VERSION :stats.neostats.net
 *  Format:
 *    :origin VERSION :servername
 *  P10:
 *    ABAAB V :Bj
 *    originnum V :servernum
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = servername
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_version( char *origin, char **argv, int argc, int srv )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )		
		do_version( base64_to_name( origin ), base64_to_server( argv[0] ) );
	else
		do_version( origin, argv[0] );
}

/** @brief _m_motd
 *
 *  process MOTD command
 *  RX:
 *    :Mark MOTD :stats.neostats.net
 *  Format:
 *    :origin MOTD :servername
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = servername
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_motd( char *origin, char **argv, int argc, int srv )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		do_motd( base64_to_nick( origin ), base64_to_server( argv[0] ) );
	else
		do_motd( origin, argv[0] );
}

/** @brief _m_admin
 *
 *  process ADMIN command
 *  RX:
 *    :Mark ADMIN :servername
 *  Format:
 *    :origin ADMIN :stats.neostats.net
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = servername
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_admin( char *origin, char **argv, int argc, int srv )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		do_admin( base64_to_nick( origin ), base64_to_server( argv[0] ) );
	else
		do_admin( origin, argv[0] );
}

/** @brief _m_credits
 *
 *  process CREDITS command
 *  RX:
 *    :Mark CREDITS u :stats.neostats.net
 *  Format:
 *    :origin CREDITS :servername
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = servername
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_credits( char *origin, char **argv, int argc, int srv )
{
	do_credits( origin, argv[0] );
}

/** @brief _m_info
 *
 *  process INFO command
 *  RX:
 *    :Mark INFO u :stats.neostats.net
 *  Format:
 *    :origin INFO :servername
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = servername
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_info( char *origin, char **argv, int argc, int srv )
{
	/* For now */
	do_credits( origin, argv[0] );
}

/** @brief _m_stats
 *
 *  process STATS command
 *  RX:
 *    :Mark STATS u :stats.neostats.net
 *  Format:
 *    :origin STATS u :servername
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = stats type
 *	  argv[1] = servername
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_stats( char *origin, char **argv, int argc, int srv )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		do_stats( base64_to_name( origin ), argv[0] );
	else
		do_stats( origin, argv[0] );
}

/** @brief _m_ping
 *
 *  process PING command
 *  RX:
 *    PING :irc.foonet.com
 *  P10:
 *    AB G !1076065765.431368 stats.mark.net 1076065765.431368
 *  Format:
 *    TODO
 *  
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = origin
 *	  argv[1] = destination
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_ping( char *origin, char **argv, int argc, int srv )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		do_ping( base64_to_server( origin ), argv[1]);
	else
		do_ping( argv[0], argc > 1 ? argv[1] : NULL );
}

/** @brief _m_pong
 *
 *  process PONG command
 *  RX:
 *    irc.foonet.com PONG irc.foonet.com :stats.neostats.net
 *  P10: 
 *    AB Z AB :stats.neostats.net
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] = origin
 *    argv[1] = destination
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_pong( char *origin, char **argv, int argc, int srv )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		do_pong( base64_to_server( origin ), argv[1] );
	else
		do_pong( argv[0], argv[1] );
}

/** @brief _m_quit
 *
 *  process QUIT command
 *  RX:
 *    :Mark QUIT :Quit: Client exited
 *  Format:
 *    :origin QUIT :comment
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] = comment
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_quit( char *origin, char **argv, int argc, int srv )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		do_quit( base64_to_nick( origin ), argv[0] );
	else
		do_quit( origin, argv[0] );
}

/** @brief m_join
 *
 *  process JOIN command
 *  RX:
 *    TODO
 *  Format:
 *    :origin JOIN #channel key
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = channel
 *	  argv[1] = channel password( key )
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_join( char *origin, char **argv, int argc, int srv )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		do_join( base64_to_nick( origin ), argv[0], argv[1] );
	else
		do_join( origin, argv[0], argv[1] );
}

/** @brief m_part
 *
 *  process PART command
 *  RX:
 *    :Mark PART #test :leaving this place
 *  Format:
 *    :origin PART #channel :reason
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = channel
 *	  argv[1] = comment
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_part( char *origin, char **argv, int argc, int srv )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		do_part( base64_to_nick( origin ), argv[0], argv[1] );
	else
		do_part( origin, argv[0], argv[1] );
}

/** @brief _m_kick
 *
 *  process KICK command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = channel
 *	  argv[1] = client to kick
 *	  argv[2] = kick comment
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_kick( char *origin, char **argv, int argc, int srv )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		do_kick( base64_to_name( origin ), argv[0], base64_to_nick( argv[1] ), argv[2] );
	else
		do_kick( origin, argv[0], argv[1], argv[2] );
}

/** @brief _m_topic
 *
 *  process TOPIC command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *  origin TOPIC #channel owner TS :topic
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] = topic text
 *  For servers using TS:
 *    argv[0] = channel name
 *    argv[1] = topic nickname
 *    argv[2] = topic time
 *    argv[3] = topic text
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_topic( char *origin, char **argv, int argc, int srv )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		do_topic( argv[0], base64_to_name( origin ), NULL, argv[argc-1] );
	else
		do_topic( argv[0], argv[1], argv[2], argv[3] );
}

/** @brief _m_away
 *
 *  process AWAY command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = away message
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_away( char *origin, char **argv, int argc, int srv )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
	{
		char *buf;

		if( argc > 0 )
		{
			buf = joinbuf( argv, argc, 0 );
			do_away( base64_to_nick( origin ), buf );
			ns_free( buf );
		}
		else
		{
			do_away( base64_to_nick( origin ), NULL );
		}
	}
	else
		do_away( origin, ( argc > 0 ) ? argv[0] : NULL );
}

/** @brief _m_kill
 *
 *  process KILL command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = kill victim(s) - comma separated list
 *	  argv[1] = kill path
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_kill( char *origin, char **argv, int argc, int srv )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		do_kill( base64_to_name( origin ), base64_to_nick( argv[0] ), argv[1] );
	else
		do_kill( origin, argv[0], argv[1] );
}

/** @brief _m_squit
 *
 *  process SQUIT command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *  P10: R: AB SQ mark.local.org 0 :Ping timeout 
 *  P10: R: ABAAV SQ york.gose.org 1076280461 :relink
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = server name
 *	  argv[argc-1] = comment
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_squit( char *origin, char **argv, int argc, int srv )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
	{
		const char *b64argv0;

		b64argv0 = base64_to_server( argv[0] );
		if( b64argv0 != NULL )
			do_squit( b64argv0, argv[2] );
		else
			do_squit( argv[0], argv[2] );
	}
	else
		do_squit( argv[0], argv[argc-1] );
}

/** @brief _m_netinfo
 *
 *  process NETINFO command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] = max global count
 *    argv[1] = time of end sync
 *    argv[2] = protocol
 *    argv[3] = cloak
 *    argv[4] = free( ** )
 *    argv[5] = free( ** )
 *    argv[6] = free( ** )
 *    argv[7] = ircnet
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_netinfo( char *origin, char **argv, int argc, int srv )
{
	do_netinfo( argv[0], argv[1], argv[2], argv[3], argv[7] );
}

/** @brief _m_snetinfo
 *
 *  process NETINFO command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] = max global count
 *    argv[1] = time of end sync
 *    argv[2] = protocol
 *    argv[3] = cloak
 *    argv[4] = free( ** )
 *    argv[5] = free( ** )
 *    argv[6] = free( ** )
 *    argv[7] = ircnet
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_snetinfo( char *origin, char **argv, int argc, int srv )
{
	do_snetinfo( argv[0], argv[1], argv[2], argv[3], argv[7] );
}

/** @brief _m_mode
 *
 *  process MODE command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	 argv[0] - channel
 *  m_umode
 *   argv[0] - username to change mode for
 *   argv[1] - modes to change
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */
/*  MODE
 *  :nick MODE nick :+modestring 
 *  :servername MODE #channel +modes parameter list TS 
 */

void _m_mode( char *origin, char **argv, int argc, int srv )
{
	if( argv[0][0] == '#' )
		do_mode_channel( origin, argv, argc );
	else
		do_mode_user( argv[0], argv[1] );
}

/** @brief _m_svsnick
 *
 *  process SVSNICK command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] = old nickname
 *    argv[1] = new nickname
 *    argv[2] = timestamp
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_svsnick( char *origin, char **argv, int argc, int srv )
{
	do_svsnick( argv[0], argv[1], ( argc > 2 ) ? argv[2] : NULL );
}

/** @brief _m_setname
 *
 *  process SETNAME command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = name
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_setname( char *origin, char **argv, int argc, int srv )
{
	do_setname( origin, argv[0] );
}

/** @brief _m_chgname
 *
 *  process CHGNAME command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = nick
 *	  argv[1] = name
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_chgname( char *origin, char **argv, int argc, int srv )
{
	do_setname( argv[0], argv[1] );
}

/** @brief _m_sethost
 *
 *  process SETHOST command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = host
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_sethost( char *origin, char **argv, int argc, int srv )
{
	do_sethost( origin, argv[0] );
}

/** @brief _m_chghost
 *
 *  process CHGHOST command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = nick
 *	  argv[1] = host
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_chghost( char *origin, char **argv, int argc, int srv )
{
	do_chghost( argv[0], argv[1] );
}

/** @brief _m_setident
 *
 *  process SETIDENT command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = ident
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_setident( char *origin, char **argv, int argc, int srv )
{
	do_setident( origin, argv[0] );
}

/** @brief _m_chgident
 *
 *  process CHGIDENT command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = nick
 *	  argv[1] = ident
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_chgident( char *origin, char **argv, int argc, int srv )
{
	do_setident( argv[0], argv[1] );
}

/** @brief _m_svsjoin
 *
 *  process SVSJOIN command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = nick
 *	  argv[1] = channel
 *	  argv[2] = key
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_svsjoin( char *origin, char **argv, int argc, int srv )
{
	do_join( argv[0], argv[1], argv[2] );
}

/** @brief _m_svspart
 *
 *  process SVSPART command
 *  RX:
 *    TODO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0] = nick
 *	  argv[1] = channel
 *	  argv[2] = reason
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_svspart( char *origin, char **argv, int argc, int srv )
{
	do_part( argv[0], argv[1], argv[2] );
}

/** @brief _m_svinfo
 *
 *  process SVINFO command
 *  RX:
 *    SVINFO
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *	  argv[0]
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_svinfo( char *origin, char **argv, int argc, int srv )
{
	do_svinfo();
}

/** @brief _m_eob
 *
 *  process EOB command
 *  RX:
 *    EOB
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_eob( char *origin, char **argv, int argc, int srv )
{
	Client *s;
	if (ircd_srv.protocol & PROTOCOL_EOB) {
		/* IRCd server supports per server EOB messages */
		s = FindClient(origin);
		if (!IsSynched(s)) {
			/* need to run through this server, and its clients + uplinked servers and clear the Netjoin */
			SyncServer(s->name);
		}
	}
	if (!IsNeoStatsSynched()) {
		irc_eob( me.name );
		do_synch_neostats();
	}
}

/** @brief _m_sqline
 *
 *  process SQLINE command
 *  RX:
 *    SQLINE
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] - mask
 *    argv[1] - reason
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_sqline( char *origin, char **argv, int argc, int srv )
{
	dlog( DEBUG1, "_m_sqline: WORK IN PROGRESS" );
}

/** @brief _m_unsqline
 *
 *  process UNSQLINE command
 *  RX:
 *    UNSQLINE
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] - mask
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_unsqline( char *origin, char **argv, int argc, int srv )
{
	dlog( DEBUG1, "_m_unsqline: WORK IN PROGRESS" );
}

/** @brief _m_zline
 *
 *  process ZLINE command
 *  RX:
 *    ZLINE
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] - mask
 *    argv[1] - reason
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_zline( char *origin, char **argv, int argc, int srv )
{
	dlog( DEBUG1, "_m_zline: WORK IN PROGRESS" );
}

/** @brief _m_unzline
 *
 *  process UNZLINE command
 *  RX:
 *    UNZLINE
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] - mask
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_unzline( char *origin, char **argv, int argc, int srv )
{
	dlog( DEBUG1, "_m_unzline: WORK IN PROGRESS" );
}

/** @brief _m_akill
 *
 *  process AKILL command
 *  RX:
 *    AKILL
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_akill( char *origin, char **argv, int argc, int srv )
{
	dlog( DEBUG1, "_m_akill: WORK IN PROGRESS" );
}

/** @brief _m_rakill
 *
 *  process RAKILL command
 *  RX:
 *    RAKILL
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_rakill( char *origin, char **argv, int argc, int srv )
{
	dlog( DEBUG1, "_m_rakill: WORK IN PROGRESS" );
}

/** @brief _m_kline
 *
 *  process KLINE command
 *  RX:
 *    KLINE
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] - mask
 *    argv[1] - reason
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_kline( char *origin, char **argv, int argc, int srv )
{
	dlog( DEBUG1, "_m_kline: WORK IN PROGRESS" );
}

/** @brief _m_unkline
 *
 *  process UNKLINE command
 *  RX:
 *    UNKLINE
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] - mask
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_unkline( char *origin, char **argv, int argc, int srv )
{
	dlog( DEBUG1, "_m_unkline: WORK IN PROGRESS" );
}

/** @brief _m_gline
 *
 *  process GLINE command
 *  RX:
 *    GLINE
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] - mask
 *    argv[1] - reason
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_gline( char *origin, char **argv, int argc, int srv )
{
	dlog( DEBUG1, "_m_gline: WORK IN PROGRESS" );
}

/** @brief _m_remgline
 *
 *  process REMGLINE command
 *  RX:
 *    REMGLINE
 *  Format:
 *    TODO
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *    argv[0] - mask
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_remgline( char *origin, char **argv, int argc, int srv )
{
	dlog( DEBUG1, "_m_remgline: WORK IN PROGRESS" );
}

/** @brief _m_notice
 *
 *  process NOTICE command
 *  RX:
 *    :Mark NOTICE NeoStats :this is a notice
 *  Format:
 *    :origin NOTICE target :message
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_notice( char *origin, char **argv, int argc, int srv )
{
	SET_SEGV_LOCATION();
	if( argv[0] == NULL )
	{
		dlog( DEBUG1, "_m_notice: dropping notice from %s to NULL: %s", origin, argv[argc-1] );
		return;
	}
	dlog( DEBUG1, "_m_notice: from %s, to %s : %s", origin, argv[0], argv[argc-1] );
	/* who to */
	if( argv[0][0] == '#' )
	{
		bot_chan_notice( origin, argv, argc );
		return;
	}
#if 0
	if( ircstrcasecmp( argv[0], "AUTH" ) == 0 )
	{
		dlog( DEBUG1, "_m_notice: dropping server notice from %s, to %s : %s", origin, argv[0], argv[argc-1] );
		return;
	}
#endif
	bot_notice( origin, argv, argc );
}

/** @brief _m_private
 *
 *  process PRIVATE command
 *  RX:
 *    :Mark PRIVATE NeoStats :this is a private message
 *  Format:
 *    :origin PRIVATE target :message
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_private( char *origin, char **argv, int argc, int cmdptr )
{
	char target[64];

	SET_SEGV_LOCATION();
	if( argv[0] == NULL )
	{
		dlog( DEBUG1, "_m_private: dropping privmsg from %s to NULL: %s", origin, argv[argc-1] );
		return;
	}
	dlog( DEBUG1, "_m_private: from %s, to %s : %s", origin, argv[0], argv[argc-1] );
	/* who to */
	if( argv[0][0] == '#' )
	{
		bot_chan_private( origin, argv, argc );
		return;
	}
	if( strstr( argv[0], "!" ) != NULL )
	{
		strlcpy( target, argv[0], 64 );
		argv[0] = strtok( target, "!" );
	}
	else if( strstr( argv[0], "@" ) != NULL )
	{
		strlcpy( target, argv[0], 64 );
		argv[0] = strtok( target, "@" );
	}
	bot_private( origin, argv, argc );
}

/** @brief do_globops
 *
 * 
 *
 *  @param origin
 *  @param message
 *
 *  @return none
 */

void do_globops( const char *origin, const char *message )
{
	Client *c;
	CmdParams * cmdparams;

	dlog( DEBUG1, "GLOBOPS: %s %s", origin, message );
	c = FindClient( origin );
	if( c != NULL )
	{
		cmdparams = ( CmdParams* )ns_calloc( sizeof( CmdParams ) );
		cmdparams->source = c;
		cmdparams->param = ( char * )message;
		SendAllModuleEvent( EVENT_GLOBOPS, cmdparams );
		ns_free( cmdparams );
	}
}

/** @brief do_wallops
 *
 * 
 *
 *  @param origin
 *  @param message
 *
 *  @return none
 */

void do_wallops( const char *origin, const char *message )
{
	Client *c;
	CmdParams * cmdparams;

	dlog( DEBUG1, "WALLOPS: %s %s", origin, message );
	c = FindClient( origin );
	if( c != NULL )
	{
		cmdparams = ( CmdParams* )ns_calloc( sizeof( CmdParams ) );
		cmdparams->source = c;
		cmdparams->param = ( char * )message;
		SendAllModuleEvent( EVENT_WALLOPS, cmdparams );
		ns_free( cmdparams );
	}
}

/** @brief do_chatops
 *
 * 
 *
 *  @param origin
 *  @param message
 *
 *  @return none
 */

void do_chatops( const char *origin, const char *message )
{
	Client *c;
	CmdParams * cmdparams;

	dlog( DEBUG1, "CHATOPS: %s %s", origin, message );
	c = FindClient( origin );
	if( c != NULL )
	{
		cmdparams = ( CmdParams* )ns_calloc( sizeof( CmdParams ) );
		cmdparams->source = c;
		cmdparams->param = ( char * )message;
		SendAllModuleEvent( EVENT_CHATOPS, cmdparams );
		ns_free( cmdparams );
	}
}

/** @brief do_synch_neostats
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_synch_neostats( void )
{
	init_services_bot();
	irc_globops( NULL, "Link with Network \2Complete!\2" );
}

/** @brief do_ping
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_ping( const char *origin, const char *destination )
{
	irc_pong( origin, destination );
	if( ircd_srv.burst )
	{
		irc_ping( me.name, origin, origin );
	}
}

/** @brief do_pong
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_pong( const char *origin, const char *destination )
{
	Client *s;
	CmdParams * cmdparams;

	s = FindServer( origin );
	if( s != NULL )
	{
		if (me.tslastping == 0)
			me.tslastping = me.now;
		s->server->ping = me.now - me.tslastping;
		if( me.ulag > 1 )
			s->server->ping -= me.ulag;
		if( IsMe( s ) )
			me.ulag = me.s->server->ping;
		cmdparams = ( CmdParams* )ns_calloc( sizeof( CmdParams ) );
		cmdparams->source = s;
		SendAllModuleEvent( EVENT_PONG, cmdparams );
		ns_free( cmdparams );
		return;
	}
	nlog( LOG_NOTICE, "Received PONG from unknown server: %s", origin );
}

/** @brief Display NeoStats version info
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_version( const char *nick, const char *remoteserver )
{
	SET_SEGV_LOCATION();
	irc_numeric( RPL_VERSION, nick, "%s :%s %s %s", me.version, me.name, ns_module_info.build_date, ns_module_info.build_time );
	AllModuleVersions( nick, remoteserver );
}

/** @brief Display our MOTD Message of the Day from the external neostats.motd file 
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_motd( const char *nick, const char *remoteserver )
{
	FILE *fp;
	char buf[BUFSIZE];

	SET_SEGV_LOCATION();
	fp = fopen( MOTD_FILENAME, "rt" );
	if( fp == NULL )
	{
		irc_numeric( ERR_NOMOTD, nick, ":- MOTD file Missing" );
		return;
	}
	irc_numeric( RPL_MOTDSTART, nick, ":- %s Message of the Day -", me.name );
	irc_numeric( RPL_MOTD, nick, ":- %s. Copyright (c) 1999 - 2008 The NeoStats Group", me.version );
	irc_numeric( RPL_MOTD, nick, ":-" );
	while( fgets( buf, sizeof( buf ), fp ) != NULL )
	{
		buf[strnlen( buf, BUFSIZE ) - 1] = 0;
		irc_numeric( RPL_MOTD, nick, ":- %s", buf );
	}
	fclose( fp );
	irc_numeric( RPL_ENDOFMOTD, nick, ":End of /MOTD command." );
}

/** @brief Display the ADMIN Message from the external stats.admin file
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_admin( const char *nick, const char *remoteserver )
{
	FILE *fp;
	char buf[BUFSIZE];
	SET_SEGV_LOCATION();

	fp = fopen( ADMIN_FILENAME, "rt" );
	if( fp == NULL )
	{
		irc_numeric( ERR_NOADMININFO, nick, "%s :No administrative info available", me.name );
		return;
	}
	irc_numeric( RPL_ADMINME, nick, ":%s :Administrative info", me.name );
	irc_numeric( RPL_ADMINLOC1, nick, ":%s.  Copyright (c) 1999 - 2008 The NeoStats Group", me.version );
	while( fgets( buf, sizeof( buf ), fp ) != NULL )
	{
		buf[strnlen( buf, BUFSIZE ) - 1] = 0;
		irc_numeric( RPL_ADMINLOC2, nick, ":- %s", buf );
	}
	fclose( fp );
	irc_numeric( RPL_ADMINLOC2, nick, ":End of /ADMIN command." );
}

/** @brief 
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_credits( const char *nick, const char *remoteserver )
{
	SET_SEGV_LOCATION();
	irc_numeric( RPL_INFO, nick, ":- NeoStats %s Credits ", me.version );
	irc_numeric( RPL_INFO, nick, ":- Now Maintained by Fish (fish@dynam.ac) and Mark (mark@ctcp.net) and DNB (dnb@majestic-liasons.com)" );
	irc_numeric( RPL_INFO, nick, ":- Previous Authors: Shmad (shmad@neostats.net) and ^Enigma^ (enigma@neostats.net)" );
	irc_numeric( RPL_INFO, nick, ":- For Support, you can find us at" );
	irc_numeric( RPL_INFO, nick, ":- irc.irc-chat.net #NeoStats" );
	irc_numeric( RPL_INFO, nick, ":- Thanks to:" );
	irc_numeric( RPL_INFO, nick, ":- Enigma for being part of the dev team" );
	irc_numeric( RPL_INFO, nick, ":- Stskeeps for writing the best IRCD ever!" );
	irc_numeric( RPL_INFO, nick, ":- chrisv@b0rked.dhs.org for the Code for Dynamically Loading Modules (Hurrican IRCD)" );
	irc_numeric( RPL_INFO, nick, ":- monkeyIRCD for the Module Segv Catching code" );
	irc_numeric( RPL_INFO, nick, ":- the Users of Global-irc.net and Dreaming.org for being our Guinea Pigs!" );
	irc_numeric( RPL_INFO, nick, ":- Andy For Ideas" );
	irc_numeric( RPL_INFO, nick, ":- HeadBang for BetaTesting, and Ideas, And Hassling us for Beta Copies" );
	irc_numeric( RPL_INFO, nick, ":- sre and Jacob for development systems and access" );
	irc_numeric( RPL_INFO, nick, ":- Error51 for Translating our FAQ and README files" );
	irc_numeric( RPL_INFO, nick, ":- users and opers of irc.irc-chat.net/org for putting up with our constant coding crashes!" );
	irc_numeric( RPL_INFO, nick, ":- Eggy for proving to use our code still had bugs when we thought it didn't( and all the bug reports! )" );
	irc_numeric( RPL_INFO, nick, ":- Hwy - Helping us even though he also has a similar project, and providing solaris porting tips : )" );
	irc_numeric( RPL_INFO, nick, ":- M - Updating lots of Doco and code and providing lots of great feedback" );
	irc_numeric( RPL_INFO, nick, ":- J Michael Jones - Giving us Patches to support QuantumIRCd" );
	irc_numeric( RPL_INFO, nick, ":- Blud - Giving us patches for Mystic IRCd" );
	irc_numeric( RPL_INFO, nick, ":- herrohr - Giving us patches for Liquid IRCd support" );
	irc_numeric( RPL_INFO, nick, ":- OvErRiTe - Giving us patches for Viagra IRCd support" );
	irc_numeric( RPL_INFO, nick, ":- Reed Loden - Contributions to IRCu support" );
	irc_numeric( RPL_INFO, nick, ":- Adam Rutter (Shmad) - Developer from the 1.0 days to 2.0 Days");
	irc_numeric( RPL_INFO, nick, ":- DeadNotBuried - early testing of 3.0, providing patches and feedback and his NeoStats modules" );
	irc_numeric( RPL_INFO, nick, ":- Rothgar - Lots of testing in the 3.0 series");
	irc_numeric( RPL_INFO, nick, ":- The users of irc-chat.net for putting up with our testing");
	irc_numeric( RPL_INFO, nick, ":- The Authors of all the code that has become part of NeoStats");
	irc_numeric( RPL_ENDOFINFO, nick, ":End of /CREDITS." );
}

/** @brief 
 *
 * 
 *
 *  @param none
 *
 *  @return none
 */

void do_stats( const char *nick, const char *what )
{
	Client *u;

	SET_SEGV_LOCATION();
	u = FindUser( nick );
	if( u == NULL )
	{
		nlog( LOG_WARNING, "do_stats: message from unknown user %s", nick );
		return;
	}
	switch( *what )
	{
		case 'u':	/* uptime */
			{
				time_t uptime = me.now - me.ts_boot;
				irc_numeric( RPL_STATSUPTIME, u->name, "Server up %ld days, %ld:%02ld:%02ld", ( uptime / TS_ONE_DAY ), ( uptime / TS_ONE_HOUR ) % 24, ( uptime / TS_ONE_MINUTE ) % TS_ONE_MINUTE, uptime % 60 );
			}
			break;
		case 'c':	/* Connections */
			irc_numeric( RPL_STATSNLINE, u->name, "N *@%s * * %d 50", me.uplink, me.port );
			irc_numeric( RPL_STATSCLINE, u->name, "C *@%s * * %d 50", me.uplink, me.port );
			break;
		case 'o':	/* Operators */
			break;
		case 'l':	/* Port Lists */
			{
				time_t tmp, tmp2;
				tmp = me.now - me.lastmsg;
				tmp2 = me.now - me.ts_boot;
				irc_numeric( RPL_STATSLINKINFO, u->name, "l SendQ SendM SendBytes RcveM RcveBytes Open_Since CPU :IDLE" );
				irc_numeric( RPL_STATSLLINE, u->name, "%s 0 %d %d %d %d %d 0 :%d", me.uplink, ( int )me.SendM, ( int )me.SendBytes, ( int )me.RcveM, ( int )me.RcveBytes, ( int )tmp2, ( int )tmp );
			}
			break;
		case 'Z':	/*  */
			if( UserLevel( u ) >= NS_ULEVEL_ADMIN )
			{
				do_dns_stats_Z( u );
			}
			break;
		case 'M':	/*  */
			{
				irc_cmd* ircd_cmd_ptr;
				ircd_cmd_ptr = cmd_list;
				while( ircd_cmd_ptr->name != NULL )
				{
					if( ircd_cmd_ptr->usage > 0 )
					{
						irc_numeric( RPL_STATSCOMMANDS, u->name, "Command %s Usage %d", *ircd_cmd_ptr->name, ircd_cmd_ptr->usage );
					}
					ircd_cmd_ptr ++;
				}
			}
			break;
		default:
			break;
	}
	irc_numeric( RPL_ENDOFSTATS, u->name, "%s :End of /STATS report", what );
	irc_chanalert( ns_botptr, "%s requested STATS %s", u->name, what );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_protocol( const char *origin, char **argv, int argc )
{
	ProtocolEntry *protocol_ptr;
	int i;

	for( i = 0; i < argc; ++i )
	{
		protocol_ptr = protocol_list;
		while( protocol_ptr->token != NULL )
		{
			if( ircstrcasecmp( protocol_ptr->token, argv[i] ) == 0 )
			{
				if( protocol_info->options & protocol_ptr->flag )
				{
					ircd_srv.protocol |= protocol_ptr->flag;
					break;
				}
			}
			++protocol_ptr;
		}
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

/* SJOIN <TS> #<channel> <modes> :[@][+]<nick_1> ...  [@][+]<nick_n> */

void do_sjoin( const char *tstime, const char *channame, const char *modes, const char *sjoinnick, char **argv, int argc )
{
	char nick[MAXNICK];
	char *nicklist;
	unsigned int mask = 0;
	Channel *c;
	char **param;
	unsigned int paramcnt = 0;
	unsigned int paramidx = 0;

	if( modes && *modes == '#' )
	{
		JoinChannel( sjoinnick, modes );
		return;
	}
	paramcnt = split_buf( argv[argc-1], &param );		   
	while( paramcnt > paramidx )
	{
		nicklist = param[paramidx];
		if( ircd_srv.protocol & PROTOCOL_SJ3 )
		{
			/* Unreal passes +b( & ) and +e( " ) via SJ3 so skip them for now */	
			if( *nicklist == '&' || *nicklist == '"' )
			{
				dlog( DEBUG1, "Skipping %s", nicklist );
				paramidx++;
				continue;
			}
		}
		mask = 0;
		while( CmodePrefixToMask( *nicklist ) != -1 )
		{
			mask |= CmodePrefixToMask( *nicklist );
			nicklist ++;
		}
		strlcpy( nick, nicklist, MAXNICK );
		JoinChannel( nick, channame ); 
		ChanUserMode( channame, nick, 1, mask );
		paramidx++;
	}
	c = FindChannel( channame );
	if (c)
	{
		/* update the TS time */
		c->creationtime = atoi( tstime );
		ChanModeHandler( c, modes, 3, argv, argc );
	}
	ns_free( param );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_netinfo( const char *maxglobalcnt, const char *tsendsync, const char *prot, const char *cloak, const char *netname )
{
	ircd_srv.maxglobalcnt = atoi( maxglobalcnt );
	ircd_srv.tsendsync = atoi( tsendsync );
	ircd_srv.uprot = atoi( prot );
	strlcpy( ircd_srv.cloak, cloak, CLOAKKEYLEN );
	strlcpy( me.netname, netname, MAXPASS );
	irc_netinfo( me.name, maxglobalcnt, ( unsigned long )me.now, ircd_srv.uprot, ircd_srv.cloak, me.netname );
	do_synch_neostats();
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_snetinfo( const char *maxglobalcnt, const char *tsendsync, const char *prot, const char *cloak, const char *netname )
{
	ircd_srv.uprot = atoi( prot );
	strlcpy( ircd_srv.cloak, cloak, CLOAKKEYLEN );
	strlcpy( me.netname, netname, MAXPASS );
	irc_snetinfo( me.name, maxglobalcnt, ( unsigned long )me.now, ircd_srv.uprot, ircd_srv.cloak, me.netname );
	do_synch_neostats();
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_join( const char *nick, const char *chanlist, const char *keys )
{
	char *s, *t;
	t = ( char *)chanlist;
	while( *( s = t ) != '\0' )
	{
		t = s + strcspn( s, "," );
		if( *t != '\0' )
			*t++ = 0;
		JoinChannel( nick, s );
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_part( const char *nick, const char *chan, const char *reason )
{
	PartChannel( FindUser( nick ), chan, reason );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_nick( const char *nick, const char *hopcount, const char *TS, 
		 const char *user, const char *host, const char *server, 
		 const char *ip, const char *servicestamp, const char *modes, 
		 const char *vhost, const char *realname, const char *numeric, 
		 const char *smodes )
{
	if( nick == NULL )
	{
		nlog( LOG_CRITICAL, "do_nick: trying to add user with NULL nickname" );
		return;
	}
	AddUser( nick, user, host, realname, server, ip, TS, numeric );
	if( modes != NULL )
		UserMode( nick, modes );
	if( vhost != NULL )
		SetUserVhost( nick, vhost );
	if( smodes != NULL )
		UserSMode( nick, smodes );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_client( const char *nick, const char *hopcount, const char *TS, 
		const char *modes, const char *smodes, 
		const char *user, const char *host, const char *vhost, 
		const char *server, const char *servicestamp, 
		const char *ip, const char *realname )
{
	do_nick( nick, hopcount, TS, user, host, server, ip, servicestamp, 
		modes, vhost, realname, NULL, smodes );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_kill( const char *source, const char *nick, const char *reason )
{
	KillUser( source, nick, reason );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_quit( const char *nick, const char *quitmsg )
{
	QuitUser( nick, quitmsg );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_squit( const char *name, const char *reason )
{
	DelServer( name, reason );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_kick( const char *kickby, const char *chan, const char *kicked, const char *kickreason )
{
	KickChannel( kickby, chan, kicked, kickreason );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_svinfo( void )
{
	irc_svinfo( TS_CURRENT, TS_MIN, ( unsigned long )me.now );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_vctrl( const char *uprot, const char *nicklen, const char *modex, const char *gc, const char *netname )
{
	ircd_srv.uprot = atoi( uprot );
	ircd_srv.nicklen = atoi( nicklen );
	ircd_srv.modex = atoi( modex );
	ircd_srv.gc = atoi( gc );
	strlcpy( me.netname, netname, MAXPASS );
	irc_vctrl( ircd_srv.uprot, ircd_srv.nicklen, ircd_srv.modex, ircd_srv.gc, me.netname );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_smode( const char *targetnick, const char *modes )
{
	UserSMode( targetnick, modes );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_mode_user( const char *targetnick, const char *modes )
{
	UserMode( targetnick, modes );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_svsmode_user( const char *targetnick, const char *modes, const char *ts )
{
	char modebuf[MODESIZE];
	
	if( ts != NULL && isdigit( *ts ) )
	{
		const char *pModes;	
		char *pNewModes;	

		SetUserServicesTS( targetnick, ts );
		/* If only setting TS, we do not need further mode processing */
		if( ircstrcasecmp( modes, "+d" ) == 0 )
		{
			dlog( DEBUG3, "dropping modes since this is a services TS %s", modes );
			return;
		}
		/* We need to strip the d from the mode string */
		pNewModes = modebuf;
		pModes = modes;
		while( *pModes != '\0' )
		{
			if( *pModes != 'd' )
			{
				*pNewModes = *pModes;
			}
			pModes++;
			pNewModes++;			
		}
		/* NULL terminate */
		*pNewModes = 0;
		UserMode( targetnick, modebuf );
	}
	else
	{
		UserMode( targetnick, modes );
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_mode_channel( char *origin, char **argv, int argc )
{
	ChanMode( origin, argv, argc );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_away( const char *nick, const char *reason )
{
	UserAway( nick, reason );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_vhost( const char *nick, const char *vhost )
{
	SetUserVhost( nick, vhost );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_nickchange( const char *oldnick, const char *newnick, const char *ts )
{
	UserNickChange( oldnick, newnick, ts );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_topic( const char *chan, const char *owner, const char *ts, const char *topic )
{
	ChannelTopic( chan, owner, ts, topic );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_server( const char *name, const char *uplink, const char *hops, const char *numeric, const char *infoline, int srv )
{
	if( srv == 0 )
	{
		if( uplink == NULL || *uplink == 0 )
			AddServer( name, me.name, hops, numeric, infoline );
		else
			AddServer( name, uplink, hops, numeric, infoline );
	}
	else
	{
		AddServer( name, uplink, hops, numeric, infoline );
	}
	irc_version( me.name, name );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_burst( char *origin, char **argv, int argc )
{
	if( argc > 0 )
	{
		if( ircd_srv.burst == 1 )
		{
			irc_burst( 0 );
			ircd_srv.burst = 0;
			do_synch_neostats();
		}
	}
	else
	{
		ircd_srv.burst = 1;
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_swhois( const char *who, const char *swhois )
{
	Client *u;
	u = FindUser( who );
	if( u != NULL )
		strlcpy( u->user->swhois, swhois, MAXHOST );
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_tkl( const char *add, const char *type, const char *user, const char *host, const char *setby, const char *tsexpire, const char *tsset, const char *reason )
{
	static char mask[MAXHOST];

	ircsnprintf( mask, MAXHOST, "%s@%s", user, host );
	if( add[0] == '+' )
	{
		AddBan( type, user, host, mask, reason, setby, tsset, tsexpire );
	}
	else
	{
		/*TEMPDelBan( type, user, host, mask, reason, setby, tsset, tsexpire );*/
		DelBan( mask );
	}
}

/** @brief 
 *
 *  
 *
 *  @param 
 *
 *  @return none
 */

void do_eos( const char *name )
{
	Client *s;

	s = FindServer( name );
	if( s == NULL )
	{
		nlog( LOG_WARNING, "do_eos: server %s not found", name );
		return;
	}
	SyncServer( s->name );
	dlog( DEBUG1, "do_eos: server %s is now synched", name );
}

/** @brief do_svsnick
 *
 *  SVSNICK handler
 *
 *  @param oldnick of user to change
 *  @param newnick to change to
 *  @param ts of change
 *
 *  @return none
 */

void do_svsnick( const char *oldnick, const char *newnick, const char *ts )
{
	UserNickChange( oldnick, newnick, ts );
}

/** @brief do_sethost
 *
 *  SETHOST handler
 *
 *  @param nick of user to change
 *  @param host to change to
 *
 *  @return none
 */

void do_sethost( const char *nick, const char *host )
{
	Client *u;

	u = FindUser( nick );
	if( u == NULL )
	{
		nlog( LOG_WARNING, "do_sethost: user %s not found", nick );
		return;
	}
	dlog( DEBUG1, "do_sethost: setting host of user %s to %s", nick, host );
	strlcpy( u->user->hostname, ( char *)host, MAXHOST );
}

/** @brief do_setident
 *
 *  SETIDENT handler
 *
 *  @param nick of user to change
 *  @param ident to change to
 *
 *  @return none
 */

void do_setident( const char *nick, const char *ident )
{
	Client *u;

	u = FindUser( nick );
	if( u == NULL )
	{
		nlog( LOG_WARNING, "do_setident: user %s not found", nick );
		return;
	}
	dlog( DEBUG1, "do_setident: setting ident of user %s to %s", nick, ident );
	strlcpy( u->user->username, ident, MAXHOST );
}

/** @brief do_setname
 *
 *  SETNAME handler
 *
 *  @param nick of user to change
 *  @param realname to change to
 *
 *  @return none
 */

void do_setname( const char *nick, const char *realname )
{
	Client *u;

	u = FindUser( nick );
	if( u == NULL )
	{
		nlog( LOG_WARNING, "do_setname: user %s not found", nick );
		return;
	}
	dlog( DEBUG1, "do_setname: setting realname of user %s to %s", nick, realname );
	strlcpy( u->info, ( char *)realname, MAXHOST );
}

/** @brief do_chghost
 *
 *  CHGHOST handler
 *
 *  @param nick of user to change
 *  @param host to change to
 *
 *  @return none
 */

void do_chghost( const char *nick, const char *host )
{
	Client *u;

	u = FindUser( nick );
	if( u == NULL )
	{
		nlog( LOG_WARNING, "do_chghost: user %s not found", nick );
		return;
	}
	dlog( DEBUG1, "do_chghost: setting host of user %s to %s", nick, host );
	strlcpy( u->user->hostname, host, MAXHOST );
}

/** @brief do_chgident
 *
 *  CHGIDENT handler
 *
 *  @param nick of user to change
 *  @param ident to change to
 *
 *  @return none
 */

void do_chgident( const char *nick, const char *ident )
{
	Client *u;

	u = FindUser( nick );
	if( u == NULL )
	{
		nlog( LOG_WARNING, "do_chgident: user %s not found", nick );
		return;
	}
	dlog( DEBUG1, "do_chgident: setting ident of user %s to %s", nick, ident );
	strlcpy( u->user->username, ident, MAXHOST );
}

/** @brief do_chgname
 *
 *  CHGNAME handler
 *
 *  @param nick of user to change
 *  @param realname to change to
 *
 *  @return none
 */

void do_chgname( const char *nick, const char *realname )
{
	Client *u;

	u = FindUser( nick );
	if( u == NULL )
	{
		nlog( LOG_WARNING, "do_chgname: user %s not found", nick );
		return;
	}
	dlog( DEBUG1, "do_chgname: setting realname of user %s to %s", nick, realname );
	strlcpy( u->info, realname, MAXHOST );
}

/** @brief do_whois
 *
 *  WHOIS handler
 *
 *  @param origin nick requesting whois
 *  @param server to query
 *  @param target nick to whois
 *
 *  @return none
 */

void do_whois( const char *origin, const char *server, const char *target )
{
	Client *u, *t;

	dlog( DEBUG4, "do_whois: %s %s %s", origin, server, target );
	u = FindUser( origin );
	if( u == NULL )
	{
		nlog( LOG_WARNING, "do_whois: origin %s not found", origin );
		return;
	}
	t = FindUser( target );
	if( t == NULL )
	{
		irc_numeric( ERR_NOSUCHNICK, origin, "%s :No such nick/channel", target );
	}
	else
	{
		irc_numeric( RPL_WHOISUSER, origin, "%s %s %s * :%s", t->name, t->user->username, t->user->vhost, t->info );
		if( t->user->bot == NULL )
		{
			irc_numeric( RPL_WHOISSERVER, origin, "%s %s :%s", t->name, t->uplink->name, t->uplink->info );
		}
		else if( t->user->bot->flags & BOT_FLAG_ROOT )
		{
			irc_numeric( RPL_WHOISOPERATOR, origin, "%s :is an IRC operator", t->name );
		}
	}
	irc_numeric( RPL_ENDOFWHOIS, origin, "%s :End of /WHOIS list", target );
}
