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
** $Id: protocol.h 3294 2008-02-24 02:45:41Z Fish $
*/
#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define NICKPARAM	0x00000001
#define MODEPARAM	0x00000002
#define MULTIPARAM	0x00000004

#define MSGTOK( a ) ( ( ircd_srv.protocol & PROTOCOL_TOKEN )? TOK_##a : MSG_##a )

typedef void( *ircd_cmd_handler )( char *origin, char **argv, int argc, int srv );

typedef struct irc_cmd {
	char **name;
	char **token;
	ircd_cmd_handler handler;
	unsigned int usage;
}irc_cmd;

#define IRC_CMD_END() { NULL, NULL, NULL, 0 }

typedef struct mode_init {
	unsigned char mode;
	unsigned int mask;
	unsigned int flags;
	unsigned char sjoin;
} mode_init;

#define MODE_INIT_END() {0, 0, 0, 0}

typedef struct ircd_server {
	int burst;
	int uprot;
	int modex;
	int nicklen;
	int gc;
	char cloak[CLOAKKEYLEN];
	int maxglobalcnt;
	int tsendsync;
	unsigned int protocol;
	unsigned int features;
} ircd_server;

typedef struct ProtocolInfo {
	/* Minimum protocols that are required * e.g. NOQUIT */
	const unsigned int required;
	/* Optional protocols that are negotiated during connection that the
	 * protocol module supports but will work when not available e.g. SJOIN */
	const unsigned int options;
	/* Features provided by this protocol module e.g. USERSMODES support. */
	const unsigned int features;
	/* Max host length */
	const unsigned int maxhost;
	/* Max password length */
	const unsigned int maxpass;
	/* Max nick length */
	const unsigned int maxnick;
	/* Max user length */
	const unsigned int maxuser;
	/* Max real name length */
	const unsigned int maxrealname;
	/* Max channel name length */
	const unsigned int maxchannelname;
	/* Max topic length */
	const unsigned int maxtopic;
	/* Default operator modes for NeoStats service bots */
	char *services_umode;
	/* Default channel mode for NeoStats service bots */
	char *services_cmode;
} ProtocolInfo;

#ifndef NEOSTATSCORE
MODULEVAR extern irc_cmd cmd_list[];
MODULEVAR extern mode_init chan_umodes[];
MODULEVAR extern mode_init chan_modes[];
MODULEVAR extern mode_init user_umodes[];
MODULEVAR extern mode_init user_smodes[];
MODULEVAR extern ProtocolInfo protocol_info;
#endif /* NEOSTATSCORE */
#ifdef NEOSTATSCORE
#ifdef DECLARE_MSGDEF
#define MSGDEF( msg ) char *msg;
#else
#define MSGDEF( msg ) extern char *msg;
#endif /* MSGDEF */
#else /* NEOSTATSCORE */
#define MSGDEF( msg ) MODULEVAR extern char *msg;
#endif /* NEOSTATSCORE */
MSGDEF( MSG_PRIVATE );
MSGDEF( TOK_PRIVATE );
MSGDEF( MSG_NOTICE );
MSGDEF( TOK_NOTICE );
MSGDEF( MSG_WALLOPS );
MSGDEF( TOK_WALLOPS );
MSGDEF( MSG_GLOBOPS );
MSGDEF( TOK_GLOBOPS );
MSGDEF( MSG_STATS );
MSGDEF( TOK_STATS );
MSGDEF( MSG_VERSION );
MSGDEF( TOK_VERSION );
MSGDEF( MSG_MOTD );
MSGDEF( TOK_MOTD );
MSGDEF( MSG_ADMIN );
MSGDEF( TOK_ADMIN );
MSGDEF( MSG_CREDITS );
MSGDEF( TOK_CREDITS );
MSGDEF( MSG_INFO );
MSGDEF( TOK_INFO );
MSGDEF( MSG_NICK );
MSGDEF( TOK_NICK );
MSGDEF( MSG_MODE );
MSGDEF( TOK_MODE );
MSGDEF( MSG_AWAY );
MSGDEF( TOK_AWAY );
MSGDEF( MSG_QUIT );
MSGDEF( TOK_QUIT );
MSGDEF( MSG_JOIN );
MSGDEF( TOK_JOIN );
MSGDEF( MSG_PART );
MSGDEF( TOK_PART );
MSGDEF( MSG_KICK );
MSGDEF( TOK_KICK );
MSGDEF( MSG_INVITE );
MSGDEF( TOK_INVITE );
MSGDEF( MSG_TOPIC );
MSGDEF( TOK_TOPIC );
MSGDEF( MSG_PING );
MSGDEF( TOK_PING );
MSGDEF( MSG_PONG );
MSGDEF( TOK_PONG );
MSGDEF( MSG_SERVER );
MSGDEF( TOK_SERVER );
MSGDEF( MSG_SQUIT );
MSGDEF( TOK_SQUIT );
MSGDEF( MSG_NETINFO );
MSGDEF( TOK_NETINFO );
MSGDEF( MSG_SNETINFO );
MSGDEF( TOK_SNETINFO );
MSGDEF( MSG_SVINFO );
MSGDEF( TOK_SVINFO );
MSGDEF( MSG_EOB );
MSGDEF( TOK_EOB );
MSGDEF( MSG_PROTOCTL );
MSGDEF( TOK_PROTOCTL );
MSGDEF( MSG_CAPAB );
MSGDEF( TOK_CAPAB );
MSGDEF( MSG_PASS );
MSGDEF( TOK_PASS );
MSGDEF( MSG_CHATOPS );
MSGDEF( TOK_CHATOPS );
MSGDEF( MSG_ERROR );
MSGDEF( TOK_ERROR );
MSGDEF( MSG_KILL );
MSGDEF( TOK_KILL );
MSGDEF( MSG_SETNAME );
MSGDEF( TOK_SETNAME );
MSGDEF( MSG_CHGNAME );
MSGDEF( TOK_CHGNAME );
MSGDEF( MSG_SETHOST );
MSGDEF( TOK_SETHOST );
MSGDEF( MSG_CHGHOST );
MSGDEF( TOK_CHGHOST );
MSGDEF( MSG_SETIDENT );
MSGDEF( TOK_SETIDENT );
MSGDEF( MSG_CHGIDENT );
MSGDEF( TOK_CHGIDENT );
MSGDEF( MSG_SVSNICK );
MSGDEF( TOK_SVSNICK );
MSGDEF( MSG_SVSJOIN );
MSGDEF( TOK_SVSJOIN );
MSGDEF( MSG_SVSPART );
MSGDEF( TOK_SVSPART );
MSGDEF( MSG_SVSMODE );
MSGDEF( TOK_SVSMODE );
MSGDEF( MSG_SVSKILL );
MSGDEF( TOK_SVSKILL );
MSGDEF( MSG_SVSHOST );
MSGDEF( TOK_SVSHOST );
MSGDEF( MSG_SQLINE );
MSGDEF( TOK_SQLINE );
MSGDEF( MSG_UNSQLINE );
MSGDEF( TOK_UNSQLINE );
MSGDEF( MSG_SGLINE );
MSGDEF( TOK_SGLINE );
MSGDEF( MSG_UNSGLINE );
MSGDEF( TOK_UNSGLINE );
MSGDEF( MSG_ZLINE );
MSGDEF( TOK_ZLINE );
MSGDEF( MSG_UNZLINE );
MSGDEF( TOK_UNZLINE );
MSGDEF( MSG_AKILL );
MSGDEF( TOK_AKILL );
MSGDEF( MSG_RAKILL );
MSGDEF( TOK_RAKILL );
MSGDEF( MSG_KLINE );
MSGDEF( TOK_KLINE );
MSGDEF( MSG_UNKLINE );
MSGDEF( TOK_UNKLINE );
MSGDEF( MSG_GLINE );
MSGDEF( TOK_GLINE );
MSGDEF( MSG_REMGLINE );
MSGDEF( TOK_REMGLINE );
MSGDEF( MSG_WHOIS );
MSGDEF( TOK_WHOIS );

EXPORTVAR extern char *numeric219;
EXPORTVAR extern char *numeric242;
EXPORTVAR extern char *numeric351;

EXPORTVAR extern ircd_server ircd_srv;

EXPORTFUNC void process_ircd_cmd( int cmdptr, const char *cmd, char *origin, char **av, int ac );

/* IRCD protocol module API */
EXPORTFUNC void _m_private( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_notice( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_pass( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_protoctl( char *origin, char **argv, int argc, int srv );
#define _m_capab _m_protoctl
EXPORTFUNC void _m_version( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_motd( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_admin( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_credits( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_info( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_stats( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_ping( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_pong( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_quit( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_topic( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_join( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_part( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_kick( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_away( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_kill( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_squit( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_netinfo( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_snetinfo( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_mode( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_svsnick( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_setname( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_sethost( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_chghost( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_chgname( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_chgident( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_setident( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_svsjoin( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_svspart( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_globops( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_wallops( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_chatops( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_svinfo( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_eob( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_sqline( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_unsqline( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_zline( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_unzline( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_akill( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_rakill( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_kline( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_unkline( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_gline( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_remgline( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_error( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_whois( char *origin, char **argv, int argc, int srv );
EXPORTFUNC void _m_ignorecommand( char *origin, char **argv, int argc, int srv );

EXPORTFUNC void do_synch_neostats( void );
EXPORTFUNC void do_motd( const char *nick, const char *remoteserver );
EXPORTFUNC void do_admin( const char *nick, const char *remoteserver );
EXPORTFUNC void do_credits( const char *nick, const char *remoteserver );
EXPORTFUNC void do_stats( const char *nick, const char *what );
EXPORTFUNC void do_ping( const char *origin, const char *destination );
EXPORTFUNC void do_pong( const char *origin, const char *destination );
EXPORTFUNC void do_version( const char *nick, const char *remoteserver );
EXPORTFUNC void do_protocol( const char *origin, char **argv, int argc );
#define do_capab do_protocol
EXPORTFUNC void do_sjoin( const char *tstime, const char *channame, const char *modes, const char *sjoinnick, char **argv, int argc );
EXPORTFUNC void do_netinfo( const char *maxglobalcnt, const char *tsendsync, const char *prot, const char *cloak, const char *netname );
EXPORTFUNC void do_snetinfo( const char *maxglobalcnt, const char *tsendsync, const char *prot, const char *cloak, const char *netname );
EXPORTFUNC void do_join( const char *nick, const char *chanlist, const char *keys );
EXPORTFUNC void do_part( const char *nick, const char *chan, const char *reason );
EXPORTFUNC void do_nick( const char *nick, const char *hopcount, const char *TS, 
		const char *user, const char *host, const char *server, 
		const char *ip, const char *servicestamp, const char *modes, 
		const char *vhost, const char *realname, const char *numeric, 
		const char *smodes );
EXPORTFUNC void do_client( const char *nick, const char *hopcount, const char *TS, 
		const char *modes, const char *smodes, 
		const char *user, const char *host, const char *vhost, 
		const char *server, const char *servicestamp, 
		const char *ip, const char *realname );
EXPORTFUNC void do_quit( const char *target, const char *quitmsg );
EXPORTFUNC void do_kill( const char *source, const char *target, const char *killmsg );
EXPORTFUNC void do_squit( const char *name, const char *reason );
EXPORTFUNC void do_kick( const char *kickby, const char *chan, const char *kicked, const char *kickreason );
EXPORTFUNC void do_svinfo( void );
EXPORTFUNC void do_vctrl( const char *uprot, const char *nicklen, const char *modex, const char *gc, const char *netname );
EXPORTFUNC void do_smode( const char *target, const char *modes );
EXPORTFUNC void do_mode_user( const char *target, const char *modes );
EXPORTFUNC void do_mode_channel( char *origin, char **argv, int argc );
EXPORTFUNC void do_svsmode_user( const char *target, const char *modes, const char *ts );
EXPORTFUNC void do_svsnick( const char *oldnick, const char *newnick, const char *ts );
/* These are the same for now but we might need to be different in the 
 * future so use macros
 */
#define do_svsmode_channel do_mode_channel
EXPORTFUNC void do_away( const char *target, const char *reason );
EXPORTFUNC void do_vhost( const char *target, const char *vhost );
EXPORTFUNC void do_nickchange( const char *oldnick, const char *newnick, const char *ts );
EXPORTFUNC void do_topic( const char *chan, const char *owner, const char *ts, const char *topic );
EXPORTFUNC void do_server( const char *name, const char *uplink, const char *hops, const char *numeric, const char *infoline, int srv );
EXPORTFUNC void do_burst( char *origin, char **argv, int argc );
EXPORTFUNC void do_swhois( const char *who, const char *swhois );
EXPORTFUNC void do_tkl( const char *add, const char *type, const char *user, const char *host, const char *setby, const char *tsexpire, const char *tsset, const char *reason );
EXPORTFUNC void do_eos( const char *name );
EXPORTFUNC void do_setname( const char *nick, const char *realname );
EXPORTFUNC void do_sethost( const char *nick, const char *host );
EXPORTFUNC void do_setident( const char *nick, const char *ident );
EXPORTFUNC void do_chghost( const char *nick, const char *host );
EXPORTFUNC void do_chgident( const char *nick, const char *ident );
EXPORTFUNC void do_chgname( const char *nick, const char *realname );

EXPORTFUNC void do_globops( const char *origin, const char *message );
EXPORTFUNC void do_wallops( const char *origin, const char *message );
EXPORTFUNC void do_chatops( const char *origin, const char *message );
EXPORTFUNC void do_whois( const char *origin, const char *server, const char *target );

/* Defined in ircd specific files */
MODULEFUNC void send_privmsg( const char *source, const char *target, const char *buf );
MODULEFUNC void send_notice( const char *source, const char *target, const char *buf );
MODULEFUNC void send_globops( const char *source, const char *buf );
MODULEFUNC void send_wallops( const char *source, const char *buf );
MODULEFUNC void send_numeric( const char *source, const int numeric, const char *target, const char *buf );
MODULEFUNC void send_umode( const char *source, const char *target, const char *mode );
MODULEFUNC void send_join( const char *source, const char *chan, const char *key, const unsigned long ts );
MODULEFUNC void send_sjoin( const char *source, const char *who, const char *chan, const unsigned long ts );
MODULEFUNC void send_part( const char *source, const char *chan, const char *reason );
MODULEFUNC void send_nickchange( const char *oldnick, const char *newnick, const unsigned long ts );
MODULEFUNC void send_cmode( const char *sourceserver, const char *sourceuser, const char *chan, const char *mode, const char *args, const unsigned long ts );
MODULEFUNC void send_quit( const char *source, const char *quitmsg );
MODULEFUNC void send_kill( const char *source, const char *target, const char *reason );
MODULEFUNC void send_kick( const char *source, const char *chan, const char *target, const char *reason );
MODULEFUNC void send_invite( const char *source, const char *target, const char *chan );
MODULEFUNC void send_topic( const char *source, const char *channel, const char *topic );
MODULEFUNC void send_svskill( const char *source, const char *target, const char *reason );
MODULEFUNC void send_svsmode( const char *source, const char *target, const char *modes );
MODULEFUNC void send_svshost( const char *source, const char *target, const char *vhost );
MODULEFUNC void send_svsjoin( const char *source, const char *target, const char *chan );
MODULEFUNC void send_svspart( const char *source, const char *target, const char *chan );
MODULEFUNC void send_svsnick( const char *source, const char *target, const char *newnick, const unsigned long ts );
MODULEFUNC void send_swhois( const char *source, const char *target, const char *swhois );
MODULEFUNC void send_smo( const char *source, const char *umodetarget, const char *msg );
MODULEFUNC void send_akill( const char *source, const char *host, const char *ident, const char *setby, const unsigned long length, const char *reason, const unsigned long ts );
MODULEFUNC void send_rakill( const char *source, const char *host, const char *ident );
MODULEFUNC void send_sqline( const char *source, const char *mask, const char *reason );
MODULEFUNC void send_unsqline( const char *source, const char *mask );
MODULEFUNC void send_sgline( const char *source, const char *mask, const char *reason );
MODULEFUNC void send_unsgline( const char *source, const char *mask );
MODULEFUNC void send_gline( const char *source, const char *mask, const char *reason );
MODULEFUNC void send_remgline( const char *source, const char *mask );
MODULEFUNC void send_zline( const char *source, const char *mask, const char *reason );
MODULEFUNC void send_unzline( const char *source, const char *mask );
MODULEFUNC void send_kline( const char *source, const char *mask, const char *reason );
MODULEFUNC void send_unkline( const char *source, const char *mask );
MODULEFUNC void send_ping( const char *source, const char *reply, const char *target );
MODULEFUNC void send_pong( const char *reply, const char *data );
MODULEFUNC void send_server( const char *source, const char *name, const int numeric, const char *infoline );
MODULEFUNC void send_squit( const char *server, const char *quitmsg );
MODULEFUNC void send_nick( const char *nick, const unsigned long ts, const char *newmode, const char *ident, const char *host, const char *server, const char *realname );
MODULEFUNC void send_server_connect( const char *name, const int numeric, const char *infoline, const char *pass, const unsigned long tsboot, const unsigned long tslink );
MODULEFUNC void send_netinfo( const char *source, const char *maxglobalcnt, const unsigned long ts, const int prot, const char *cloak, const char *netname );
MODULEFUNC void send_snetinfo( const char *source, const char *maxglobalcnt, const unsigned long ts, const int prot, const char *cloak, const char *netname );
MODULEFUNC void send_svinfo( const int tscurrent, const int tsmin, const unsigned long tsnow );
MODULEFUNC void send_eob( const char *server );
MODULEFUNC void send_vctrl( const int uprot, const int nicklen, const int modex, const int gc, const char *netname );
MODULEFUNC void send_burst( int b );
MODULEFUNC void send_svstime( const char *source, const unsigned long ts );
MODULEFUNC void send_setname( const char *nick, const char *realname );
MODULEFUNC void send_sethost( const char *nick, const char *host );
MODULEFUNC void send_setident( const char *nick, const char *ident );
MODULEFUNC void send_chgname( const char *source, const char *nick, const char *realname );
MODULEFUNC void send_chghost( const char *source, const char *nick, const char *host );
MODULEFUNC void send_chgident( const char *source, const char *nick, const char *ident );
MODULEFUNC void send_stats( const char *source, const char type, const char *target );
MODULEFUNC void send_version( const char *source, const char *target );

MODULEFUNC void cloakhost( const char *host );

EXPORTFUNC void send_cmd( const char *fmt, ... )__attribute__( ( format( printf, 1, 2 ) ) ); /* 2=format 3=params */

#endif /* _PROTOCOL_H_ */
