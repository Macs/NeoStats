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
** $Id: ircsend.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "ircprotocol.h"
/* Declare MSGDEF */
#define DECLARE_MSGDEF
#include "protocol.h"
#include "services.h"
#include "dcc.h"
#include "users.h"
#include "channels.h"
#include "modes.h"
#include "base64.h"
#include "ircsend.h"
#include "dl.h"

extern void *protocol_module_handle;

typedef struct protocol_sym
{
	void **handler;
	void *defaulthandler;
	char *sym;
	char **msgptr;
	char *msgsym;
	char **tokptr;
	char *toksym;
	unsigned int required;
	unsigned int feature;
} protocol_sym;

static void( *irc_send_privmsg )( const char *source, const char *to, const char *buf );
static void( *irc_send_notice )( const char *source, const char *to, const char *buf );
static void( *irc_send_globops )( const char *source, const char *buf );
static void( *irc_send_wallops )( const char *source, const char *buf );
static void( *irc_send_numeric )( const char *source, const int numeric, const char *target, const char *buf );
static void( *irc_send_quit )( const char *source, const char *quitmsg );
static void( *irc_send_umode )( const char *source, const char *target, const char *mode );
static void( *irc_send_join )( const char *source, const char *chan, const char *key, const unsigned long ts );
static void( *irc_send_sjoin )( const char *source, const char *who, const char *chan, const unsigned long ts );
static void( *irc_send_part )( const char *source, const char *chan, const char *reason );
static void( *irc_send_nickchange )( const char *oldnick, const char *newnick, const unsigned long ts );
static void( *irc_send_cmode )( const char *sourceserver, const char *sourceuser, const char *chan, const char *mode, const char *args, unsigned long ts );
static void( *irc_send_kill )( const char *source, const char *target, const char *reason );
static void( *irc_send_kick )( const char *source, const char *chan, const char *target, const char *reason );
static void( *irc_send_invite )( const char *source, const char *to, const char *chan );
static void( *irc_send_topic )( const char *source, const char *channel, const char *topic );
static void( *irc_send_svskill )( const char *source, const char *target, const char *reason );
static void( *irc_send_svsmode )( const char *source, const char *target, const char *modes );
static void( *irc_send_svshost )( const char *source, const char *who, const char *vhost );
static void( *irc_send_svsjoin )( const char *source, const char *target, const char *chan );
static void( *irc_send_svspart )( const char *source, const char *target, const char *chan );
static void( *irc_send_svsnick )( const char *source, const char *target, const char *newnick, const unsigned long ts );
static void( *irc_send_swhois )( const char *source, const char *target, const char *swhois );
static void( *irc_send_smo )( const char *source, const char *umodetarget, const char *msg );
static void( *irc_send_akill )( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, unsigned long ts );
static void( *irc_send_rakill )( const char *source, const char *host, const char *ident );
static void( *irc_send_sqline )( const char *source, const char *mask, const char *reason );
static void( *irc_send_unsqline )( const char *source, const char *mask );
static void( *irc_send_sgline )( const char *source, const char *mask, const char *reason );
static void( *irc_send_unsgline )( const char *source, const char *mask );
static void( *irc_send_gline )( const char *source, const char *mask, const char *reason );
static void( *irc_send_remgline )( const char *source, const char *mask );
static void( *irc_send_zline )( const char *source, const char *mask, const char *reason );
static void( *irc_send_unzline )( const char *source, const char *mask );
static void( *irc_send_kline )( const char *source, const char *mask, const char *reason );
static void( *irc_send_unkline )( const char *source, const char *mask );
static void( *irc_send_ping )( const char *source, const char *reply, const char *to );
static void( *irc_send_pong )( const char *reply, const char *data);
static void( *irc_send_server )( const char *source, const char *name, const int numeric, const char *infoline );
static void( *irc_send_squit )( const char *server, const char *quitmsg );
static void( *irc_send_nick )( const char *nick, const unsigned long ts, const char *newmode, const char *ident, const char *host, const char *server, const char *realname );
static void( *irc_send_server_connect )( const char *name, const int numeric, const char *infoline, const char *pass, time_t tsboot, time_t tslink );
static void( *irc_send_netinfo )( const char *source, const char *maxglobalcnt, const unsigned long ts, const int prot, const char *cloak, const char *netname );
static void( *irc_send_snetinfo )( const char *source, const char *maxglobalcnt, const unsigned long ts, const int prot, const char *cloak, const char *netname );
static void( *irc_send_svinfo )( const int tscurrent, const int tsmin, const unsigned long tsnow );
static void( *irc_send_eob )( const char *server );
static void( *irc_send_vctrl )( const int uprot, const int nicklen, const int modex, const int gc, const char *netname );
static void( *irc_send_burst )( int b );
static void( *irc_send_svstime )( const char *source, const unsigned long ts );
static void( *irc_send_setname )( const char *nick, const char *realname );
static void( *irc_send_sethost )( const char *nick, const char *host );
static void( *irc_send_setident )( const char *nick, const char *ident );
static void( *irc_send_chgname )( const char *source, const char *nick, const char *realname );
static void( *irc_send_chghost )( const char *source, const char *nick, const char *host );
static void( *irc_send_chgident )( const char *source, const char *nick, const char *ident );
static void( *irc_send_stats )( const char *source, const char type, const char *target );
static void( *irc_send_version )( const char *source, const char *target );
static void( *irc_send_cloakhost )( char *host );

static void _send_numeric( const char *source, const int numeric, const char *target, const char *buf );
static void _send_privmsg( const char *source, const char *target, const char *buf );
static void _send_notice( const char *source, const char *target, const char *buf );
static void _send_wallops( const char *source, const char *buf );
static void _send_globops( const char *source, const char *buf );
static void _send_nickchange( const char *oldnick, const char *newnick, const unsigned long ts );
static void _send_umode( const char *source, const char *target, const char *mode );
static void _send_cmode( const char *sourceserver, const char *sourceuser, const char *chan, const char *mode, const char *args, const unsigned long ts );
static void _send_join( const char *source, const char *chan, const char *key, const unsigned long ts );
static void _send_part( const char *source, const char *chan, const char *reason );
static void _send_kick( const char *source, const char *chan, const char *target, const char *reason );
static void _send_invite( const char *source, const char *target, const char *chan );
static void _send_topic( const char *source, const char *channel, const char *topic );
static void _send_quit( const char *source, const char *quitmsg );
static void _send_ping( const char *source, const char *reply, const char *target );
static void _send_pong( const char *reply, const char *data);
static void _send_server( const char *source, const char *name, const int numeric, const char *infoline );
static void _send_squit( const char *server, const char *quitmsg );
static void _send_netinfo( const char *source, const char *maxglobalcnt, const unsigned long ts, const int prot, const char *cloak, const char *netname );
static void _send_snetinfo( const char *source, const char *maxglobalcnt, const unsigned long ts, const int prot, const char *cloak, const char *netname );
static void _send_svinfo( const int tscurrent, const int tsmin, const unsigned long tsnow );
static void _send_eob( const char *server );
static void _send_kill( const char *source, const char *target, const char *reason );
static void _send_setname( const char *nick, const char *realname );
static void _send_sethost( const char *nick, const char *host );
static void _send_setident( const char *nick, const char *ident );
static void _send_chgname( const char *source, const char *nick, const char *realname );
static void _send_chghost( const char *source, const char *nick, const char *host );
static void _send_chgident( const char *source, const char *nick, const char *ident );
static void _send_svsnick( const char *source, const char *target, const char *newnick, const unsigned long ts );
static void _send_svsjoin( const char *source, const char *target, const char *chan );
static void _send_svspart( const char *source, const char *target, const char *chan );
static void _send_svsmode( const char *source, const char *target, const char *modes );
static void _send_svskill( const char *source, const char *target, const char *reason );

#if 0 /* Work in progress */
static void _send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts );
static void _send_rakill( const char *source, const char *host, const char *ident );
static void _send_sqline( const char *source, const char *mask, const char *reason );
static void _send_unsqline( const char *source, const char *mask );
static void _send_sgline( const char *source, const char *mask, const char *reason );
static void _send_unsgline( const char *source, const char *mask );
static void _send_gline( const char *source, const char *mask, const char *reason );
static void _send_remgline( const char *source, const char *mask );
static void _send_zline( const char *source, const char *mask, const char *reason );
static void _send_unzline( const char *source, const char *mask );
static void _send_kline( const char *source, const char *mask, const char *reason );
static void _send_unkline( const char *source, const char *mask );
#endif /* 0 Work in progress */
static void _send_stats( const char *source, const char type, const char *target );
static void _send_version( const char *source, const char *target );

static char ircd_buf[BUFSIZE];

static protocol_sym protocol_sym_table[] = 
{
	{( void * )&irc_send_privmsg, _send_privmsg, "send_privmsg", &MSG_PRIVATE, "MSG_PRIVATE", &TOK_PRIVATE, "TOK_PRIVATE", 1, 0},
	{( void * )&irc_send_notice, _send_notice, "send_notice", &MSG_NOTICE, "MSG_NOTICE", &TOK_NOTICE, "TOK_NOTICE", 1, 0},
	{( void * )&irc_send_globops, _send_globops, "send_globops", &MSG_GLOBOPS, "MSG_GLOBOPS", &TOK_GLOBOPS, "TOK_GLOBOPS", 0, 0},
	{( void * )&irc_send_wallops, _send_wallops, "send_wallops", &MSG_WALLOPS, "MSG_WALLOPS", &TOK_WALLOPS, "TOK_WALLOPS", 0, 0},
	{( void * )&irc_send_numeric, _send_numeric, "send_numeric", NULL, NULL, NULL, NULL, 0, 0},
	{( void * )&irc_send_umode, _send_umode, "send_umode", &MSG_MODE, "MSG_MODE", &TOK_MODE, "TOK_MODE", 1, 0},
	{( void * )&irc_send_join, _send_join, "send_join", &MSG_JOIN, "MSG_JOIN", &TOK_JOIN, "TOK_JOIN", 1, 0},
	{( void * )&irc_send_sjoin, NULL, "send_sjoin", NULL, NULL, NULL, NULL, 0, 0},
	{( void * )&irc_send_part, _send_part, "send_part", &MSG_PART, "MSG_PART", &TOK_PART, "TOK_PART", 1, 0},
	{( void * )&irc_send_nickchange, _send_nickchange, "send_nickchange", &MSG_NICK, "MSG_NICK", &TOK_NICK, "TOK_NICK", 1, 0},
	{( void * )&irc_send_cmode, _send_cmode, "send_cmode", &MSG_MODE, "MSG_MODE", &TOK_MODE, "TOK_MODE", 1, 0},
	{( void * )&irc_send_quit, _send_quit, "send_quit", &MSG_QUIT, "MSG_QUIT", &TOK_QUIT, "TOK_QUIT", 1, 0},
	{( void * )&irc_send_kill, _send_kill, "send_kill", &MSG_KILL, "MSG_KILL", &TOK_KILL, "TOK_KILL", 0, 0},
	{( void * )&irc_send_kick, _send_kick, "send_kick", &MSG_KICK, "MSG_KICK", &TOK_KICK, "TOK_KICK", 0, 0},
	{( void * )&irc_send_invite, _send_invite, "send_invite", &MSG_INVITE, "MSG_INVITE", &TOK_INVITE, "TOK_INVITE", 0, 0},
	{( void * )&irc_send_topic, _send_topic, "send_topic", &MSG_TOPIC, "MSG_TOPIC", &TOK_TOPIC, "TOK_TOPIC", 0, 0},
	{( void * )&irc_send_svskill, _send_svskill, "send_svskill", &MSG_SVSKILL, "MSG_SVSKILL", &TOK_SVSKILL, "TOK_SVSKILL", 0, FEATURE_SVSKILL},
	{( void * )&irc_send_svsmode, _send_svsmode, "send_svsmode", &MSG_SVSMODE, "MSG_SVSMODE", &TOK_SVSMODE, "TOK_SVSMODE", 0, FEATURE_SVSMODE},
	{( void * )&irc_send_svshost, NULL, "send_svshost", &MSG_SVSHOST, "MSG_SVSHOST", &TOK_SVSHOST, "TOK_SVSHOST", 0, FEATURE_SVSHOST},
	{( void * )&irc_send_svsjoin, _send_svsjoin, "send_svsjoin", &MSG_SVSJOIN, "MSG_SVSJOIN", &TOK_SVSJOIN, "TOK_SVSJOIN", 0, FEATURE_SVSJOIN},
	{( void * )&irc_send_svspart, _send_svspart, "send_svspart", &MSG_SVSPART, "MSG_SVSPART", &TOK_SVSPART, "TOK_SVSPART", 0, FEATURE_SVSPART},
	{( void * )&irc_send_svsnick, _send_svsnick, "send_svsnick", &MSG_SVSNICK, "MSG_SVSNICK", &TOK_SVSNICK, "TOK_SVSNICK", 0, FEATURE_SVSNICK},
	{( void * )&irc_send_swhois, NULL, "send_swhois", NULL, NULL, NULL, NULL, 0, FEATURE_SWHOIS},
	{( void * )&irc_send_smo, NULL, "send_smo", NULL, NULL, NULL, NULL, 0, FEATURE_SMO},
	{( void * )&irc_send_svstime, NULL, "send_svstime", NULL, NULL, NULL, NULL, 0, FEATURE_SVSTIME},
	{( void * )&irc_send_akill, NULL, "send_akill", &MSG_AKILL, "MSG_AKILL", &TOK_AKILL, "TOK_AKILL", 0, 0},
	{( void * )&irc_send_rakill, NULL, "send_rakill", &MSG_RAKILL, "MSG_RAKILL", &TOK_RAKILL, "TOK_RAKILL", 0, 0},
	{( void * )&irc_send_sqline, NULL, "send_sqline", &MSG_UNSQLINE, "MSG_UNSQLINE", &TOK_UNSQLINE, "TOK_UNSQLINE", 0, 0},
	{( void * )&irc_send_unsqline, NULL, "send_unsqline", &MSG_SQLINE, "MSG_SQLINE", &TOK_SQLINE, "TOK_SQLINE", 0, 0},
	{( void * )&irc_send_zline, NULL, "send_zline", &MSG_ZLINE, "MSG_ZLINE", &TOK_ZLINE, "TOK_ZLINE", 0, 0},
	{( void * )&irc_send_unzline, NULL, "send_unzline", &MSG_UNZLINE, "MSG_UNZLINE", &TOK_UNZLINE, "TOK_UNZLINE", 0, 0},
	{( void * )&irc_send_kline, NULL, "send_kline", &MSG_KLINE, "MSG_KLINE", &TOK_KLINE, "TOK_KLINE", 0, 0},
	{( void * )&irc_send_unkline, NULL, "send_unkline", &MSG_UNKLINE, "MSG_UNKLINE", &TOK_UNKLINE, "TOK_UNKLINE", 0, 0},
	{( void * )&irc_send_gline, NULL, "send_gline", &MSG_GLINE, "MSG_GLINE", &TOK_GLINE, "TOK_GLINE", 0, 0},
	{( void * )&irc_send_remgline, NULL, "send_remgline", &MSG_REMGLINE, "MSG_REMGLINE", &TOK_REMGLINE, "TOK_REMGLINE", 0, 0},
	{( void * )&irc_send_ping, _send_ping, "send_ping", &MSG_PING, "MSG_PING", &TOK_PING, "TOK_PING", 0, 0},
	{( void * )&irc_send_pong, _send_pong, "send_pong", &MSG_PONG, "MSG_PONG", &TOK_PONG, "TOK_PONG", 0, 0},
	{( void * )&irc_send_server, _send_server, "send_server", &MSG_SERVER, "MSG_SERVER", &TOK_SERVER, "TOK_SERVER", 0, 0},
	{( void * )&irc_send_squit, _send_squit, "send_squit", &MSG_SQUIT, "MSG_SQUIT", &TOK_SQUIT, "TOK_SQUIT", 0, 0},
	{( void * )&irc_send_netinfo, _send_netinfo, "send_netinfo", &MSG_NETINFO, "MSG_NETINFO", &TOK_NETINFO, "TOK_NETINFO", 0, 0},
	{( void * )&irc_send_snetinfo, _send_snetinfo, "send_snetinfo", &MSG_SNETINFO, "MSG_SNETINFO", &TOK_SNETINFO, "TOK_SNETINFO", 0, 0},
	{( void * )&irc_send_nick, NULL, "send_nick", NULL, NULL, NULL, NULL, 1, 0},
	{( void * )&irc_send_server_connect, NULL, "send_server_connect", NULL, NULL, NULL, NULL, 1, 0},
	{( void * )&irc_send_svinfo, _send_svinfo, "send_svinfo", &MSG_SVINFO, "MSG_SVINFO", &TOK_SVINFO, "TOK_SVINFO", 0, 0},
	{( void * )&irc_send_eob, _send_eob, "send_eob", &MSG_EOB, "MSG_EOB", &TOK_EOB, "TOK_EOB", 0, 0},
	{( void * )&irc_send_vctrl, NULL, "send_vctrl", NULL, NULL, NULL, NULL, 0, 0},
	{( void * )&irc_send_burst, NULL, "send_burst", NULL, NULL, NULL, NULL,  0, 0},
	{( void * )&irc_send_setname, _send_setname, "send_setname", &MSG_SETNAME, "MSG_SETNAME", &TOK_SETNAME, "TOK_SETNAME", 0, 0},
	{( void * )&irc_send_sethost, _send_sethost, "send_sethost", &MSG_SETHOST, "MSG_SETHOST", &TOK_SETHOST, "TOK_SETHOST", 0, 0},
	{( void * )&irc_send_setident, _send_setident, "send_setident", &MSG_SETIDENT, "MSG_SETIDENT", &TOK_SETIDENT, "TOK_SETIDENT", 0, 0},
	{( void * )&irc_send_chgname, _send_chgname, "send_chgname", &MSG_CHGNAME, "MSG_CHGNAME", &TOK_CHGNAME , "TOK_CHGNAME", 0, 0},
	{( void * )&irc_send_chghost, _send_chghost, "send_chghost", &MSG_CHGHOST, "MSG_CHGHOST", &TOK_CHGHOST, "TOK_CHGHOST", 0, FEATURE_SVSHOST},
	{( void * )&irc_send_chgident, _send_chgident, "send_chgident", &MSG_CHGIDENT, "MSG_CHGIDENT", &TOK_CHGIDENT, "TOK_CHGIDENT", 0, 0},
	{( void * )&irc_send_cloakhost, NULL, "cloakhost", NULL, NULL, NULL, NULL, 0, 0},
	{( void * )&irc_send_stats, _send_stats, "send_stats", &MSG_STATS, "MSG_STATS", &TOK_STATS, "TOK_STATS", 0, 0 },
	{( void * )&irc_send_version, _send_version, "send_version", &MSG_VERSION, "MSG_VERSION", &TOK_VERSION, "TOK_VERSION", 0, 0 },
	{( void * )NULL, NULL ,NULL, &MSG_WHOIS, "MSG_WHOIS", &TOK_WHOIS, "TOK_WHOIS", 0, 0 },
	{( void * )NULL, NULL ,NULL, &MSG_MOTD, "MSG_MOTD", &TOK_MOTD, "TOK_MOTD", 0, 0 },
	{( void * )NULL, NULL ,NULL, &MSG_ADMIN, "MSG_ADMIN", &TOK_ADMIN, "TOK_ADMIN", 0, 0 },
	{( void * )NULL, NULL ,NULL, &MSG_CREDITS, "MSG_CREDITS", &TOK_CREDITS, "TOK_CREDITS", 0, 0 },
	{( void * )NULL, NULL ,NULL, &MSG_AWAY, "MSG_AWAY", &TOK_AWAY, "TOK_AWAY", 0, 0 },
	{( void * )NULL, NULL ,NULL, &MSG_PROTOCTL, "MSG_PROTOCTL", &TOK_PROTOCTL, "TOK_PROTOCTL", 0, 0},
	{( void * )NULL, NULL ,NULL, &MSG_CAPAB, "MSG_CAPAB", &TOK_CAPAB, "TOK_CAPAB", 0, 0},
	{( void * )NULL, NULL ,NULL, &MSG_PASS, "MSG_PASS", &TOK_PASS, "TOK_PASS", 0, 0},
	{( void * )NULL, NULL ,NULL, &MSG_TOPIC, "MSG_TOPIC", &TOK_TOPIC, "TOK_TOPIC", 0, 0},
	{( void * )NULL, NULL ,NULL, &MSG_CHATOPS, "MSG_CHATOPS", &TOK_CHATOPS, "TOK_CHATOPS", 0, 0},
	{( void * )NULL, NULL ,NULL, &MSG_ERROR, "MSG_ERROR", &TOK_ERROR, "TOK_ERROR", 0, 0},
	{( void * )NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0},
};

/** @brief InitIrcdSymbols
 *
 *  Map protocol module pointers to core pointers and check for minimum 
 *  requirements
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitIrcdSymbols( void )
{
	void **protocol_handler = NULL;
	protocol_sym *pprotocol_sym;

	/* Build up supported message and function table */
	pprotocol_sym = protocol_sym_table;
	while( pprotocol_sym->msgptr != NULL || pprotocol_sym->handler != NULL )
	{
		char **ptr;

		dlog( DEBUG7, "InitIrcdSymbols: start" );
		/* Find MSG_cmd */
		if( pprotocol_sym->msgptr )
		{
			ptr = ns_dlsym( protocol_module_handle, pprotocol_sym->msgsym );
			if( ptr )
				*pprotocol_sym->msgptr = *ptr;
			dlog( DEBUG7, "InitIrcdSymbols: %s: %s", pprotocol_sym->msgsym, ptr ? *ptr : "NONE" );
		}
		/* Find TOK_cmd */
		if( pprotocol_sym->tokptr )
		{			
			ptr = ns_dlsym( protocol_module_handle, pprotocol_sym->toksym );
			if( ptr )
				*pprotocol_sym->tokptr = *ptr;
			dlog( DEBUG7, "InitIrcdSymbols: %s: %s", pprotocol_sym->toksym, ptr ? *ptr : "NONE" );
		}
		/* If we have a message or token, apply default handler */
		if( ( pprotocol_sym->msgptr || pprotocol_sym->tokptr ) && pprotocol_sym->handler )
		{
			*pprotocol_sym->handler = pprotocol_sym->defaulthandler;
			dlog( DEBUG7, "InitIrcdSymbols: default handler for: %s %p", pprotocol_sym->sym ? pprotocol_sym->sym : "NONE", pprotocol_sym->defaulthandler );
		}
		/* Check for IRCd override handler */
		if( pprotocol_sym->sym )
		{
			protocol_handler = ns_dlsym( protocol_module_handle, pprotocol_sym->sym );
			dlog( DEBUG7, "InitIrcdSymbols: override handler for: %s %p", pprotocol_sym->sym ? pprotocol_sym->sym : "NONE", protocol_handler );
		}
		/* If we need a handler */
		if( pprotocol_sym->handler )
		{
			/* Apply IRCd override handler */
			if( protocol_handler )
			{
				*pprotocol_sym->handler = protocol_handler;
				dlog( DEBUG7, "InitIrcdSymbols: apply override handler for: %s %p", pprotocol_sym->sym ? pprotocol_sym->sym : "NONE", protocol_handler );
			}
			/* Try to apply handler if no IRCd override and we have not already set the default */
			if( *pprotocol_sym->handler == NULL ) 
			{
				*pprotocol_sym->handler = pprotocol_sym->defaulthandler;
				dlog( DEBUG7, "InitIrcdSymbols: apply default handler for: %s %p", pprotocol_sym->sym ? pprotocol_sym->sym : "NONE", pprotocol_sym->defaulthandler );
			}
			/* If no default or IRCd handler but we require the function, quit with error */
			if( pprotocol_sym->required && ( *pprotocol_sym->handler == NULL ) ) 
			{
				nlog( LOG_CRITICAL, "Unable to find %s in selected IRCd module", pprotocol_sym->sym );
				return NS_FAILURE;	
			}
			/* If handler implies a feature, apply it */
			if( *pprotocol_sym->handler ) 
				ircd_srv.features |= pprotocol_sym->feature;
		}
		dlog( DEBUG7, "InitIrcdSymbols: end" );
		/* Next... */
		pprotocol_sym ++;
	}
	return NS_SUCCESS;
}

static void _send_numeric( const char *source, const int numeric, const char *target, const char *buf )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %d %s :%s", me.s->name64, numeric, nick_to_base64( target ), buf );
	else
		send_cmd( ":%s %d %s %s", source, numeric, target, buf );
}

static void _send_privmsg( const char *source, const char *target, const char *buf )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %s %s :%s", nick_to_base64( source ), TOK_PRIVATE, ( target[0] == '#' ) ? target : nick_to_base64( target ), buf );
	else
		send_cmd( ":%s %s %s :%s", source, MSGTOK( PRIVATE ), target, buf );
}

static void _send_notice( const char *source, const char *target, const char *buf )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %s %s :%s", nick_to_base64( source ), TOK_NOTICE, ( target[0] == '#' ) ? target : nick_to_base64( target ), buf );
	else
		send_cmd( ":%s %s %s :%s", source, MSGTOK( NOTICE ), target, buf );
}

static void _send_wallops( const char *source, const char *buf )
{
	send_cmd( ":%s %s :%s", source, MSGTOK( WALLOPS ), buf );
}

static void _send_globops( const char *source, const char *buf )
{
	send_cmd( ":%s %s :%s", source, MSGTOK( GLOBOPS ), buf );
}

static void _send_join( const char *source, const char *chan, const char *key, const unsigned long ts )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %s %s %lu", nick_to_base64( source ), TOK_JOIN, chan, ts );
	else
		send_cmd( ":%s %s %s", source, MSGTOK( JOIN ), chan );
}

static void _send_part( const char *source, const char *chan, const char *reason )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %s %s :%s", nick_to_base64( source ), TOK_PART, chan, reason );
	else
		send_cmd( ":%s %s %s :%s", source, MSGTOK( PART ), chan, reason );
}

static void _send_kick( const char *source, const char *chan, const char *target, const char *reason )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %s %s %s :%s", nick_to_base64( source ), TOK_KICK, chan, nick_to_base64( target ), ( reason ? reason : "No reason given" ) );
	else
		send_cmd( ":%s %s %s %s :%s", source, MSGTOK( KICK ), chan, target, ( reason ? reason : "No reason given" ) );
}

static void _send_invite( const char *source, const char *target, const char *chan )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %s %s %s", nick_to_base64( source ), TOK_INVITE, target, chan );
	else
		send_cmd( ":%s %s %s %s", source, MSGTOK( INVITE ), target, chan );
}

static void _send_topic( const char *source, const char *channel, const char *topic )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( TOPIC ), channel, topic );
}


static void _send_nickchange( const char *oldnick, const char *newnick, const unsigned long ts )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %s %s %lu", nick_to_base64( oldnick ), TOK_NICK, newnick, ts );
	else
		send_cmd( ":%s %s %s %lu", oldnick, MSGTOK( NICK ), newnick, ts );
}

static void _send_umode( const char *source, const char *target, const char *mode )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %s %s :%s", nick_to_base64( source ), TOK_MODE, target, mode );
	else
		send_cmd( ":%s %s %s :%s", source, MSGTOK( MODE ), target, mode );
}

/* R: ABAAH M #c3 +tn */
static void _send_cmode( const char *sourceserver, const char *sourceuser, const char *chan, const char *mode, const char *args, const unsigned long ts )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %s %s %s %s %lu", me.s->name64, TOK_MODE, chan, mode, args, ts );
	else
		/* TS of 0 forces the ircd to set the mode */
		send_cmd( ":%s %s %s %s %s 0", sourceserver, MSGTOK( MODE ), chan, mode, args );
}

static void _send_quit( const char *source, const char *quitmsg )
{
	send_cmd( ":%s %s :%s", source, MSGTOK( QUIT ), quitmsg );
}

static void _send_ping( const char *source, const char *reply, const char *target )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %s %s :%s", me.s->name64, TOK_PING, reply, target );
	else
		send_cmd( ":%s %s %s :%s", source, MSGTOK( PING ), reply, target );
}

static void _send_pong( const char *reply, const char *data )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %s %s :%s", me.s->name64, TOK_PONG, me.s->name64, reply );
	else	
		send_cmd( "%s %s", MSGTOK( PONG ), reply );
}

static void _send_server( const char *source, const char *name, const int numeric, const char *infoline )
{
	send_cmd( ":%s %s %s %d :%s", source, MSGTOK( SERVER ), name, numeric, infoline );
}

static void _send_squit( const char *server, const char *quitmsg )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %s %s 0 :%s", me.s->name64, TOK_SQUIT, server, quitmsg );
	else	
		send_cmd( "%s %s :%s", MSGTOK( SQUIT ), server, quitmsg );
}

static void _send_netinfo( const char *source, const char *maxglobalcnt, const unsigned long ts, const int prot, const char *cloak, const char *netname )
{
	send_cmd( ":%s %s 0 %lu %d %s 0 0 0 :%s", source, MSGTOK( NETINFO ), ts, prot, cloak, netname );
}

static void _send_snetinfo( const char *source, const char *maxglobalcnt, const unsigned long ts, const int prot, const char *cloak, const char *netname )
{
	send_cmd( ":%s %s 0 %lu %d %s 0 0 0 :%s", source, MSGTOK( SNETINFO ), ts, prot, cloak, netname );
}

static void _send_svinfo( const int tscurrent, const int tsmin, const unsigned long tsnow )
{
	send_cmd( "%s %d %d 0 :%lu", MSGTOK( SVINFO ), tscurrent, tsmin, tsnow );
}

static void _send_eob( const char *server )
{
	send_cmd( ":%s %s", server, MSGTOK( EOB ) );
}

static void _send_kill( const char *source, const char *target, const char *reason )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %s %s :%s", me.s->name64, TOK_KILL, nick_to_base64( target ), reason );
	else	
		send_cmd( ":%s %s %s :%s", source, MSGTOK(KILL), target, reason );
}

static void _send_setname( const char *nick, const char *realname )
{
	send_cmd( ":%s %s :%s", nick, MSGTOK( SETNAME ), realname );
}

static void _send_sethost( const char *nick, const char *host )
{
	send_cmd( ":%s %s :%s", nick, MSGTOK( SETHOST ), host );
}

static void _send_setident( const char *nick, const char *ident )
{
	send_cmd( ":%s %s :%s", nick, MSGTOK( SETIDENT ), ident );
}

static void _send_chgname( const char *source, const char *nick, const char *realname )
{
	send_cmd( ":%s %s %s %s", source, MSGTOK( CHGNAME ), nick, realname );
}

static void _send_chghost( const char *source, const char *nick, const char *host )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( CHGHOST ), nick, host );
}

static void _send_chgident( const char *source, const char *nick, const char *ident )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( CHGIDENT ), nick, ident );
}

static void _send_svsnick( const char *source, const char *target, const char *newnick, const unsigned long ts )
{
	send_cmd( "%s %s %s :%lu", MSGTOK( SVSNICK ), target, newnick, ts );
}

static void _send_svsjoin( const char *source, const char *target, const char *chan )
{
	send_cmd( "%s %s %s", MSGTOK( SVSJOIN ), target, chan);
}

static void _send_svspart( const char *source, const char *target, const char *chan )
{
	send_cmd( "%s %s %s", MSGTOK( SVSPART ), target, chan );
}

static void _send_svsmode( const char *source, const char *target, const char *modes )
{
	send_cmd( ":%s %s %s %s", source, MSGTOK( SVSMODE ), target, modes );
}

static void _send_svskill( const char *source, const char *target, const char *reason )
{
	send_cmd( "%s %s %s :%s", source, MSGTOK( SVSKILL ), target, reason );
}

#if 0 /* Work in progress */
static void _send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts )
{
}

static void _send_rakill( const char *source, const char *host, const char *ident )
{
}

static void _send_sqline( const char *source, const char *mask, const char *reason )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( SQLINE ), mask, reason );
}

static void _send_unsqline( const char *source, const char *mask )
{
	send_cmd( ":%s %s %s", source, MSGTOK( UNSQLINE ), mask );
}

static void _send_sgline( const char *source, const char *mask, const char *reason )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( SGLINE ), mask, reason );
}

static void _send_unsgline( const char *source, const char *mask )
{
	send_cmd( ":%s %s %s", source, MSGTOK( UNSGLINE ), mask );
}

static void _send_gline( const char *source, const char *mask, const char *reason )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( GLINE ), mask, reason );
}

static void _send_remgline( const char *source, const char *mask )
{
	send_cmd( ":%s %s %s", source, MSGTOK( REMGLINE ), mask );
}

static void _send_zline( const char *source, const char *mask, const char *reason )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( ZLINE ), mask, reason );
}

static void _send_unzline( const char *source, const char *mask )
{
	send_cmd( ":%s %s %s", source, MSGTOK( UNZLINE ), mask );
}

static void _send_kline( const char *source, const char *mask, const char *reason )
{
	send_cmd( ":%s %s %s :%s", source, MSGTOK( KLINE ), mask, reason );
}

static void _send_unkline( const char *source, const char *mask )
{
	send_cmd( ":%s %s %s", source, MSGTOK( UNKLINE ), mask );
}
#endif /* 0 Work in progress */

static void _send_stats( const char *source, const char type, const char *target )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %s %c :%s", nick_to_base64( source ), TOK_STATS, type, server_to_base64( target ) );
	else	
		send_cmd(":%s %s %c %s", source, MSGTOK( STATS ), type, target );
}

static void _send_version( const char *source, const char *target )
{
	if( ircd_srv.protocol & PROTOCOL_P10 )
		send_cmd( "%s %s %s", server_to_base64( source ), TOK_VERSION, server_to_base64( target ) );		
	else	
		send_cmd( ":%s %s %s", source, MSGTOK( VERSION ), target );
}

/** @brief send_cmd
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
void send_cmd( const char *fmt, ... )
{
	static char buf[BUFSIZE];
	va_list ap;
	size_t buflen;
	
	va_start( ap, fmt );
	ircvsnprintf( buf, BUFSIZE, fmt, ap );
	va_end( ap );

	dlog( DEBUGTX, "%s", buf );
	if( strnlen( buf, BUFSIZE ) < BUFSIZE - 2 )
	{
		strlcat( buf, "\r\n", BUFSIZE );
	}
	else
	{
		buf[BUFSIZE - 1] = 0;
		buf[BUFSIZE - 2] = '\r';
		buf[BUFSIZE - 3] = '\n';
	}
	buflen = strnlen( buf, BUFSIZE );
	send_to_sock( me.servsock, buf, buflen );
}

/** @brief unsupported_cmd
 *
 *  report attempts to use a feature not supported by the loaded protocol
 *
 *  @param none
 *
 *  @return none
 */

static void unsupported_cmd( const char *cmd )
{
	irc_chanalert( ns_botptr, _( "Warning, %s tried to %s which is not supported" ), GET_CUR_MODNAME(), cmd );
	nlog( LOG_NOTICE, "Warning, %s tried to %s, which is not supported", GET_CUR_MODNAME(), cmd );
}

/** @brief irc_connect
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_connect( const char *name, const int numeric, const char *infoline, const char *pass, const time_t tsboot, const time_t tslink )
{
	irc_send_server_connect( name, numeric, infoline, pass, tsboot, tslink );
	return NS_SUCCESS;
}

/** @brief irc_prefmsg_list 
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_prefmsg_list( const Bot *botptr, const Client * target, const char **text )
{
	if( IsMe( target ) )
	{
		nlog( LOG_NOTICE, "Dropping irc_prefmsg_list from bot (%s) to bot (%s)", botptr->u->name, target->name );
		return NS_SUCCESS;
	}
	while( *text != '\0' )
	{
		if( **text )
		{
			irc_prefmsg( botptr, target, ( char *)*text );
		}
		else
		{
			irc_prefmsg( botptr, target, " " );
		}
		text++;
	}
	return NS_SUCCESS;
}

/** @brief irc_privmsg_list
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_privmsg_list( const Bot *botptr, const Client * target, const char **text )
{
	if( IsMe( target ) )
	{
		nlog( LOG_NOTICE, "Dropping irc_privmsg_list from bot (%s) to bot (%s)", botptr->u->name, target->name );
		return NS_SUCCESS;
	}
	while( *text != '\0' )
	{
		if( **text )
		{
			irc_privmsg( botptr, target, ( char *)*text );
		}
		else
		{
			irc_privmsg( botptr, target, " " );
		}
		text++;
	}
	return NS_SUCCESS;
}

/** @brief irc_chanalert
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_chanalert( const Bot *botptr, const char *fmt, ... )
{
	va_list ap;

	if( !IsNeoStatsSynched() )
		return NS_SUCCESS;
	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	irc_send_privmsg( botptr ? botptr->name : ns_botptr->u->name, me.serviceschan, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_prefmsg
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_prefmsg( const Bot *botptr, const Client *target, const char *fmt, ... )
{
	va_list ap;

	if( IsMe( target ) )
	{
		nlog( LOG_NOTICE, "Dropping irc_prefmsg from bot (%s) to bot (%s)", botptr->u->name, target->name );
		return NS_SUCCESS;
	}
	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	if( target->flags & CLIENT_FLAG_DCC )
	{
		dcc_send_msg( target, ircd_buf );
	}
	else if( nsconfig.want_privmsg )
	{
		irc_send_privmsg( botptr->u->name, target->name, ircd_buf );
	}
	else
	{
		irc_send_notice( botptr ? botptr->u->name : ns_botptr->u->name, target->name, ircd_buf );
	}
	return NS_SUCCESS;
}

/** @brief irc_privmsg
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_privmsg( const Bot *botptr, const Client *target, const char *fmt, ... )
{
	va_list ap;

	if( IsMe( target ) )
	{
		nlog( LOG_NOTICE, "Dropping privmsg from bot (%s) to bot (%s)", botptr->u->name, target->name );
		return NS_SUCCESS;
	}
	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	irc_send_privmsg( botptr->u->name, target->name, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_notice
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_notice( const Bot *botptr, const Client *target, const char *fmt, ... )
{
	va_list ap;

	if( IsMe( target ) )
	{
		nlog( LOG_NOTICE, "Dropping notice from bot (%s) to bot (%s)", botptr->u->name, target->name );
		return NS_SUCCESS;
	}
	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	irc_send_notice( botptr->u->name, target->name, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_chanprivmsg
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_chanprivmsg( const Bot *botptr, const char *chan, const char *fmt, ... )
{
	va_list ap;

	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	irc_send_privmsg( botptr->u->name, chan, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_channotice
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_channotice( const Bot *botptr, const char *chan, const char *fmt, ... )
{
	va_list ap;

	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	irc_send_notice( botptr->u->name, chan, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_globops
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_globops( const Bot *botptr, const char *fmt, ... )
{
	va_list ap;

	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	if( IsNeoStatsSynched() )
	{
		if( irc_send_globops == NULL )
		{
			unsupported_cmd( "GLOBOPS" );
			nlog( LOG_NOTICE, "Dropping unhandled globops: %s", ircd_buf );
			return NS_FAILURE;
		}
		irc_send_globops( ( botptr?botptr->u->name:me.name ), ircd_buf );
	}
	else
	{
		nlog( LOG_NOTICE, "globops before sync: %s", ircd_buf );
	}
	return NS_SUCCESS;
}

/** @brief irc_wallops
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_wallops( const Bot *botptr, const char *fmt, ... )
{
	va_list ap;

	if( irc_send_wallops == NULL )
	{
		unsupported_cmd( "WALLOPS" );
		nlog( LOG_NOTICE, "Dropping unhandled wallops: %s", ircd_buf );
		return NS_FAILURE;
	}
	va_start( ap, fmt );
	ircvsnprintf( ircd_buf, BUFSIZE, fmt, ap );
	va_end( ap );
	irc_send_wallops( ( botptr?botptr->name:me.name ), ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_numeric
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_numeric( const int numeric, const char *target, const char *data, ... )
{
	va_list ap;

	if( irc_send_numeric == NULL )
	{
		unsupported_cmd( "NUMERIC" );
		return NS_FAILURE;
	}
	va_start( ap, data );
	ircvsnprintf( ircd_buf, BUFSIZE, data, ap );
	va_end( ap );
	irc_send_numeric( me.name, numeric, target, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_nick
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_nick( const char *nick, const char *user, const char *host, const char *realname, const char *modes )
{
	irc_send_nick( nick, ( unsigned long )me.now, modes, user, host, me.name, realname );
	return NS_SUCCESS;
}

/** @brief irc_cloakhost
 *
 *  Create a hidden hostmask for the bot 
 *  Support is currently just via UMODE auto cloaking
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_cloakhost( const Bot *botptr )
{
	if( ircd_srv.features&FEATURE_UMODECLOAK )
	{
		irc_umode( botptr, botptr->name, UMODE_HIDE );
		return NS_SUCCESS;	
	}
	if( irc_send_cloakhost )
	{
		irc_send_cloakhost( botptr->u->user->vhost );
		return NS_SUCCESS;	
	}
	return NS_FAILURE;	
}

/** @brief irc_umode
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_umode( const Bot *botptr, const char *target, unsigned int mode )
{
	char *newmode;
	
	newmode = UmodeMaskToString( mode );
	irc_send_umode( botptr->u->name, target, newmode );
	UserMode( target, newmode );
	return NS_SUCCESS;
}

/** @brief irc_join
 *
 *  @param none
 *
 *  @return none
 */

int irc_join( const Bot *botptr, const char *chan, const char *mode )
{
	time_t ts;
	Channel *c;

	c = FindChannel( chan );
	ts = ( c == NULL ) ? me.now : c->creationtime;
	/* Use sjoin if available */
	if( ( ircd_srv.protocol & PROTOCOL_SJOIN ) && irc_send_sjoin ) 
	{
		if( mode == NULL ) 
		{
			irc_send_sjoin( me.name, botptr->u->name, chan, ( unsigned long )ts );
			JoinChannel( botptr->u->name, chan );
		} 
		else 
		{
			unsigned int i;
			int prefixlen = 0;
			char prefix[MODESIZE] = "";
			for( i = 1; i < strlen( mode ); i++ )
			{
				if( CmodeCharToPrefix( mode[i] ) )
				{
					prefix[ prefixlen ] = CmodeCharToPrefix( mode[i] );
					prefixlen++;
				}
			}
			prefix[ prefixlen ] = 0;
			ircsnprintf( ircd_buf, BUFSIZE, "%s%s", prefix, botptr->u->name );
			irc_send_sjoin( me.name, ircd_buf, chan, ( unsigned long )ts );
			JoinChannel( botptr->u->name, chan );
			ChanUserMode( chan, botptr->u->name, 1, CmodeStringToMask( mode ) );
		}
	}
	else
	{
		/* sjoin not available so use normal join */	
		irc_send_join( botptr->u->name, chan, NULL, ( unsigned long )me.now );
		JoinChannel( botptr->u->name, chan );
		if( mode )
		{
			irc_chanusermode( botptr, chan, mode, botptr->u->name );
		}
	}
	/* Increment number of persistent users if needed */
	if( botptr->flags & BOT_FLAG_PERSIST )
	{
		if( c == NULL )
			c = FindChannel( chan );
		c->persistentusers ++;
	}
	return NS_SUCCESS;
}

/** @brief irc_part
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_part( const Bot *botptr, const char *chan, const char *quitmsg )
{
	Channel *c;

	if( irc_send_part == NULL )
	{
		unsupported_cmd( "PART" );
		return NS_FAILURE;
	}
	c = FindChannel( chan );
	/* Decrement number of persistent users if needed 
	 * Must be BEFORE we part the channel in order to trigger
	 * empty channel processing for other bots
	 */
	if( botptr->flags & BOT_FLAG_PERSIST )
	{
		c->persistentusers --;
	}
	irc_send_part( botptr->u->name, chan, quitmsg ? quitmsg : "" );
	PartChannel( botptr->u, ( char *) chan, quitmsg );
	return NS_SUCCESS;
}

/** @brief irc_nickchange
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_nickchange( const Bot *botptr, const char *newnick )
{
	if( botptr == NULL )
	{
		nlog( LOG_WARNING, "Unknown bot tried to change nick to %s", newnick );
		return NS_FAILURE;
	}
	/* Check newnick is not in use */
	if( FindUser( newnick ) )
	{
		nlog( LOG_WARNING, "Bot %s tried to change nick to one that already exists %s", botptr->name, newnick );
		return NS_FAILURE;
	}
	irc_send_nickchange( botptr->name, newnick, ( unsigned long )me.now );
	UserNickChange( botptr->name, newnick, NULL );
	return NS_SUCCESS;
}

/** @brief irc_setname
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_setname( const Bot *botptr, const char *realname )
{
	if( irc_send_setname == NULL )
	{
		unsupported_cmd( "SETNAME" );
		return NS_FAILURE;
	}
	irc_send_setname( botptr->name, realname );
	strlcpy( botptr->u->info, ( char *)realname, MAXHOST );
	return NS_SUCCESS;
}

/** @brief irc_sethost
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_sethost( const Bot *botptr, const char *host )
{
	if( irc_send_sethost == NULL )
	{
		unsupported_cmd( "SETHOST" );
		return NS_FAILURE;
	}
	irc_send_sethost( botptr->name, host );
	strlcpy( botptr->u->user->hostname, ( char *)host, MAXHOST );
	return NS_SUCCESS;
}
 
/** @brief irc_setident
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_setident( const Bot *botptr, const char *ident )
{
	if( irc_send_setident == NULL )
	{
		unsupported_cmd( "SETIDENT" );
		return NS_FAILURE;
	}
	irc_send_setident( botptr->name, ident );
	strlcpy( botptr->u->user->username, ( char *)ident, MAXHOST );
	return NS_SUCCESS;
}

/** @brief irc_cmode
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_cmode( const Bot *botptr, const char *chan, const char *mode, const char *args )
{
	char **av;
	unsigned int ac;
	Channel *c;
	
	c = FindChannel(chan);
	if (c) {
		irc_send_cmode( me.name, botptr->u->name, chan, mode, args, c->creationtime );
	} else
		irc_send_cmode( me.name, botptr->u->name, chan, mode, args, (unsigned long)me.now);
	ircsnprintf( ircd_buf, BUFSIZE, "%s %s %s", chan, mode, args );
	ac = split_buf( ircd_buf, &av );
	ChanMode( me.name, av, ac );
	ns_free( av );
	return NS_SUCCESS;
}

/** @brief irc_chanusermode
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_chanusermode( const Bot *botptr, const char *chan, const char *mode, const char *target )
{
	Channel *c;
	if( ( ircd_srv.protocol & PROTOCOL_B64NICK ) )
	{
		irc_send_cmode( me.name, botptr->u->name, chan, mode, nick_to_base64( target ), ( unsigned long )me.now );
	}
	else
	{
		c = FindChannel(chan);
		if (c) 
			irc_send_cmode( me.name, botptr->u->name, chan, mode, target, ( unsigned long )c->creationtime );
		else
			irc_send_cmode( me.name, botptr->u->name, chan, mode, target, ( unsigned long )me.now );
	}
	ChanUserMode( chan, target, 1, CmodeStringToMask( mode ) );
	return NS_SUCCESS;
}

/** @brief irc_quit
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_quit( const Bot *botptr, const char *quitmsg )
{
	if( botptr == NULL )
	{
		return NS_FAILURE;
	}
	irc_send_quit( botptr->u->name, quitmsg );
	do_quit( botptr->u->name, quitmsg );
	return NS_SUCCESS;
}

/** @brief irc_kill
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_kill( const Bot *botptr, const char *target, const char *reason, ... )
{
	va_list ap;

	if( irc_send_kill == NULL )
	{
		unsupported_cmd( "KILL" );
		return NS_FAILURE;
	}
	va_start( ap, reason );
	ircvsnprintf( ircd_buf, BUFSIZE, reason, ap );
	va_end( ap );
	irc_send_kill( botptr->u->name, target, ircd_buf );
	do_quit( target, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_kick
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_kick( const Bot *botptr, const char *chan, const char *target, const char *reason )
{
	if( irc_send_kick == NULL )
	{
		unsupported_cmd( "KICK" );
		return NS_FAILURE;
	}
	irc_send_kick( botptr->u->name, chan, target, reason );
	PartChannel( FindUser( target ), ( char *) chan, reason[0] != '\0' ?( char *)reason : NULL );
	return NS_SUCCESS;
}

/** @brief irc_invite
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_invite( const Bot *botptr, const Client *target, const char *chan ) 
{
	if( irc_send_invite == NULL ) 
	{
		unsupported_cmd( "INVITE" );
		return NS_FAILURE;
	}
	if( IsChannelMember( FindChannel( chan ), target ) )
	{
		dlog( DEBUGTX, "irc_invite: %s already on channel %s, skipping invite", target->name, chan );
		return NS_SUCCESS;
	}
	irc_send_invite( botptr->u->name, target->name, chan );
	return NS_SUCCESS;
}

/** @brief irc_topic
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_topic( const Bot *botptr, const Channel *channel, const char *topic )
{
	if( irc_send_topic == NULL )
	{
		unsupported_cmd( "TOPIC" );
		return NS_FAILURE;
	}
	irc_send_topic( botptr->u->name, channel->name, topic );
	do_topic( botptr->u->name, channel->name, me.strnow, topic );
	return NS_SUCCESS;
}

/** @brief irc_svstime
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_svstime( const Bot *botptr, const Client *target, const time_t ts )
{
	if( irc_send_svstime == NULL )
	{
		unsupported_cmd( "SVSTIME" );
		return NS_FAILURE;
	}
	irc_send_svstime( me.name, ( unsigned long )ts );
	nlog( LOG_NOTICE, "irc_svstime: synching server times to %lu", ts );
	return NS_SUCCESS;
}

/** @brief irc_svskill
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_svskill( const Bot *botptr, const Client *target, const char *reason, ... )
{
	va_list ap;

	va_start( ap, reason );
	ircvsnprintf( ircd_buf, BUFSIZE, reason, ap );
	va_end( ap );
	if( irc_send_svskill )
	{
		irc_send_svskill( me.name, target->name, ircd_buf );
	}
	else if( irc_send_kill )
	{
		irc_send_kill( me.name, target->name, ircd_buf );
		do_quit( target->name, ircd_buf );
	}
	else
	{
		unsupported_cmd( "SVSKILL" );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief irc_svsmode
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_svsmode( const Bot *botptr, const Client *target, const char *modes )
{
	if( irc_send_svsmode == NULL )
	{
		unsupported_cmd( "SVSMODE" );
		return NS_FAILURE;
	}
	irc_send_svsmode( me.name, target->name, modes );
	UserMode( target->name, modes );
	return NS_SUCCESS;
}

/** @brief irc_svshost
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_svshost( const Bot *botptr, Client *target, const char *vhost )
{
	if( irc_send_svshost )
	{
		irc_send_svshost( me.name, target->name, vhost );
	}
	else if( irc_send_chghost )
	{
		irc_send_chghost( me.name, target->name, vhost );
	}
	else
	{
		unsupported_cmd( "SVSHOST" );
		return NS_FAILURE;
	}
	strlcpy( target->user->vhost, vhost, MAXHOST );
	target->flags |= CLIENT_FLAG_SETHOST;
	return NS_SUCCESS;
}

/** @brief irc_svsjoin
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_svsjoin( const Bot *botptr, const Client *target, const char *chan )
{
	if( irc_send_svsjoin == NULL )
	{
		unsupported_cmd( "SVSJOIN" );
		return irc_invite( botptr, target, chan );
	}
	irc_send_svsjoin( me.name, target->name, chan );
	return NS_SUCCESS;
}

/** @brief irc_svspart
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_svspart( const Bot *botptr, const Client *target, const char *chan )
{
	if( irc_send_svspart == NULL )
	{
		unsupported_cmd( "SVSPART" );
		return NS_FAILURE;
	}
	irc_send_svspart( me.name, target->name, chan );
	return NS_SUCCESS;
}

/** @brief irc_swhois
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_swhois( const char *target, const char *swhois )
{
	if( irc_send_swhois == NULL )
	{
		unsupported_cmd( "SWHOIS" );
		return NS_FAILURE;
	}
	irc_send_swhois( me.name, target, swhois );
	return NS_SUCCESS;
}

/** @brief irc_svsnick
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_svsnick( const Bot *botptr, const Client *target, const char *newnick )
{
	if( irc_send_svsnick == NULL )
	{
		unsupported_cmd( "SVSNICK" );
		return NS_FAILURE;
	}
	irc_send_svsnick( me.name, target->name, newnick, ( unsigned long )me.now );
	return NS_SUCCESS;
}

/** @brief irc_smo
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_smo( const char *source, const char *umodetarget, const char *msg )
{
	if( irc_send_smo == NULL )
	{
		unsupported_cmd( "SMO" );
		return NS_FAILURE;
	}
	irc_send_smo( source, umodetarget, msg );
	return NS_SUCCESS;
}

/** @brief irc_akill
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_akill( const Bot *botptr, const char *host, const char *ident, const unsigned long length, const char *reason, ... )
{
	va_list ap;

	if( irc_send_akill == NULL )
	{
		unsupported_cmd( "AKILL" );
		return NS_FAILURE;
	}
	va_start( ap, reason );
	ircvsnprintf( ircd_buf, BUFSIZE, reason, ap );
	va_end( ap );
	irc_send_akill( me.name, host, ident, botptr->name, length, ircd_buf, ( unsigned long )me.now );
	return NS_SUCCESS;
}

/** @brief irc_rakill
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_rakill( const Bot *botptr, const char *host, const char *ident )
{
	if( irc_send_rakill == NULL )
	{
		unsupported_cmd( "RAKILL" );
		return NS_FAILURE;
	}
	irc_send_rakill( me.name, host, ident );
	return NS_SUCCESS;
}

/** @brief irc_sqline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_sqline( const Bot *botptr, const char *mask, const char *reason, ...)
{
	va_list ap;

	if( irc_send_sqline == NULL )
	{
		unsupported_cmd( "SQLINE" );
		return NS_FAILURE;
	}
	va_start( ap, reason );
	ircvsnprintf( ircd_buf, BUFSIZE, reason, ap );
	va_end( ap );
	irc_send_sqline( me.name, mask, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_unsqline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_unsqline( const Bot *botptr, const char *mask )
{
	if( irc_send_unsqline == NULL )
	{
		unsupported_cmd( "UNSQLINE" );
		return NS_FAILURE;
	}
	irc_send_unsqline( me.name, mask );
	return NS_SUCCESS;
}

/** @brief irc_sgline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_sgline( const Bot *botptr, const char *mask, const char *reason, ...)
{
	va_list ap;

	if( irc_send_sgline == NULL )
	{
		unsupported_cmd( "SGLINE" );
		return NS_FAILURE;
	}
	va_start( ap, reason );
	ircvsnprintf( ircd_buf, BUFSIZE, reason, ap );
	va_end( ap );
	irc_send_sgline( me.name, mask, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_unsgline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_unsgline( const Bot *botptr, const char *mask )
{
	if( irc_send_unsgline == NULL )
	{
		unsupported_cmd( "UNSGLINE" );
		return NS_FAILURE;
	}
	irc_send_unsgline( me.name, mask );
	return NS_SUCCESS;
}

/** @brief irc_gline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_gline( const Bot *botptr, const char *mask, const char *reason, ...)
{
	va_list ap;

	if( irc_send_gline == NULL )
	{
		unsupported_cmd( "GLINE" );
		return NS_FAILURE;
	}
	va_start( ap, reason );
	ircvsnprintf( ircd_buf, BUFSIZE, reason, ap );
	va_end( ap );
	irc_send_gline( me.name, mask, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_remgline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_remgline( const Bot *botptr, const char *mask )
{
	if( irc_send_remgline == NULL )
	{
		unsupported_cmd( "REMGLINE" );
		return NS_FAILURE;
	}
	irc_send_remgline( me.name, mask );
	return NS_SUCCESS;
}

/** @brief irc_zline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_zline( const Bot *botptr, const char *mask, const char *reason, ...)
{
	va_list ap;

	if( irc_send_sqline == NULL )
	{
		unsupported_cmd( "ZLINE" );
		return NS_FAILURE;
	}
	va_start( ap, reason );
	ircvsnprintf( ircd_buf, BUFSIZE, reason, ap );
	va_end( ap );
	irc_send_zline( me.name, mask, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_unzline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_unzline( const Bot *botptr, const char *mask )
{
	if( irc_send_unsqline == NULL )
	{
		unsupported_cmd( "UNZLINE" );
		return NS_FAILURE;
	}
	irc_send_unzline( me.name, mask );
	return NS_SUCCESS;
}

/** @brief irc_kline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_kline( const Bot *botptr, const char *mask, const char *reason, ...)
{
	va_list ap;

	if( irc_send_sqline == NULL )
	{
		unsupported_cmd( "KLINE" );
		return NS_FAILURE;
	}
	va_start( ap, reason );
	ircvsnprintf( ircd_buf, BUFSIZE, reason, ap );
	va_end( ap );
	irc_send_kline( me.name, mask, ircd_buf );
	return NS_SUCCESS;
}

/** @brief irc_unkline
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_unkline( const Bot *botptr, const char *mask )
{
	if( irc_send_unsqline == NULL )
	{
		unsupported_cmd( "UNKLINE" );
		return NS_FAILURE;
	}
	irc_send_unkline( me.name, mask );
	return NS_SUCCESS;
}

/** @brief irc_ping
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_ping( const char *source, const char *reply, const char *to )
{
	if( irc_send_ping == NULL )
	{
		unsupported_cmd( "PING" );
		return NS_FAILURE;
	}
	irc_send_ping( source, reply, to );
	return NS_SUCCESS;
}

/** @brief irc_pong
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_pong( const char *reply, const char *data )
{
	if( irc_send_pong == NULL )
	{
		unsupported_cmd( "PONG" );
		return NS_FAILURE;
	}
	irc_send_pong( reply, data );
	return NS_SUCCESS;
}

/** @brief irc_server
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_server( const char *name, const int numeric, const char *infoline )
{
	if( irc_send_server == NULL )
	{
		unsupported_cmd( "SERVER" );
		return NS_FAILURE;
	}
	irc_send_server( me.name, name, numeric, infoline );
	return NS_SUCCESS;
}

/** @brief irc_stats
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_stats( const char *source, const char type, const char *target )
{
	if( irc_send_stats == NULL )
	{
		unsupported_cmd( "STATS" );
		return NS_FAILURE;
	}
	irc_send_stats( source, type, target );
	return NS_SUCCESS;
}

/** @brief irc_version
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_version( const char *source, const char *target )
{
	if( irc_send_version == NULL )
	{
		unsupported_cmd( "VERSION" );
		return NS_FAILURE;
	}
	irc_send_version( source, target );
	return NS_SUCCESS;
}

/** @brief irc_squit
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int irc_squit( const char *server, const char *quitmsg )
{
	if( irc_send_squit == NULL )
	{
		unsupported_cmd( "SQUIT" );
		return NS_FAILURE;
	}
	irc_send_squit( server, quitmsg );
	return NS_SUCCESS;
}

int irc_eob( const char *server )
{
	if( irc_send_eob )
		irc_send_eob( server );
	return NS_SUCCESS;
}

int irc_netinfo( const char *source, const char *maxglobalcnt, const unsigned long ts, const int prot, const char *cloak, const char *netname )
{
	if( irc_send_netinfo )
		irc_send_netinfo( source, maxglobalcnt, ts, prot, cloak, netname );
	return NS_SUCCESS;
}

int irc_snetinfo( const char *source, const char *maxglobalcnt, const unsigned long ts, const int prot, const char *cloak, const char *netname )
{
	if( irc_send_snetinfo )
		irc_send_snetinfo( source, maxglobalcnt, ts, prot, cloak, netname );
	return NS_SUCCESS;
}

int irc_vctrl( const int uprot, const int nicklen, const int modex, const int gc, const char *netname )
{
	if( irc_send_vctrl )
		irc_send_vctrl( uprot, nicklen, modex, gc, netname );
	return NS_SUCCESS;
}

int irc_burst( int b )
{
	if( irc_send_burst )
		irc_send_burst( b );
	return NS_SUCCESS;
}

int irc_svinfo( const int tscurrent, const int tsmin, const unsigned long tsnow )
{
	if( irc_send_svinfo )
		irc_send_svinfo( tscurrent, tsmin, tsnow );
	return NS_SUCCESS;
}
