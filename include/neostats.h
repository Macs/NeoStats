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
** $Id: neostats.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _NEOSTATS_H_
#define _NEOSTATS_H_

#ifdef WIN32
/* Disable some warnings on MSVC 2005 */
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1
#include "configwin32.h"
#include <winsock2.h>
#else /* WIN32 */
#include "config.h"
#endif /* WIN32 */

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif /* HAVE_STDDEF_H */
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif /* HAVE_STDIO_H */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif /* HAVE_SYS_SOCKET_H */
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif /* HAVE_NETDB_H */
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif /* HAVE_NETINET_IN_H */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif /* HAVE_SYS_TIME_H */
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif /* HAVE_SYS_RESOURCE_H */
#ifdef HAVE_TIME_H
#include <time.h>
#endif /* HAVE_TIME_H */
#ifdef HAVE_STRING_H
#include <string.h>
#endif  /* HAVE_STRING_H */
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif /* HAVE_STDARG_H */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif /* HAVE_CTYPE_H */
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */
#ifdef HAVE_SETJMP_H
#include <setjmp.h>
#endif /* HAVE_SETJMP_H */
#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif /* HAVE_ASSERT_H */
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif /* HAVE_ARPA_INET_H */
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif /* HAVE_SYS_IOCTL_H */

#ifdef WIN32
typedef SOCKET OS_SOCKET;
#else /* WIN32 */
typedef int OS_SOCKET;
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif /* SOCKET_ERROR */
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif /* INVALID_SOCKET */
#endif /* WIN32 */

/* These macros handle DLL imports and exports for win32 module support */
#ifdef WIN32
#ifdef NEOSTATSCORE
#define EXPORTFUNC __declspec(dllexport)
#define EXPORTVAR __declspec(dllexport)
#define MODULEFUNC 
#define MODULEVAR 
#else /* NEOSTATSCORE */
#define EXPORTVAR __declspec(dllimport)
#define EXPORTFUNC __declspec(dllimport)
#define MODULEFUNC __declspec(dllexport)
#define MODULEVAR __declspec(dllexport)
#endif /* NEOSTATSCORE */
#else /* WIN32 */
#define MODULEFUNC 
#define MODULEVAR 
#define EXPORTVAR
#define EXPORTFUNC
#include "version.h"
#endif /* WIN32 */

#ifdef WIN32
#define MODULECONFIG "modconfigwin32.h"
#else /* WIN32 */
#define MODULECONFIG "modconfig.h"
#endif /* WIN32 */


/* No language support under win32 */
#ifndef WIN32
/* when db stuff is working, change this back */
/* #ifdef HAVE_DB_H */
#if 0
#define USEGETTEXT
#else /* HAVE_DB_H */
/* so our defines for _(x) are not active */
#undef USEGETTEXT
#endif /* HAVE_DB_H */
#endif /* WIN32 */

#ifdef USEGETTEXT
char *LANGgettext( const char *string, int mylang );
/* our own defines for language support */
/* this one is for standard language support */
#define _( x ) LANGgettext( ( x ), me.lang )
/* this one is for custom langs based on chan/user struct */
#define __( x, y ) LANGgettext( ( x ), ( y )->lang )
#else /* USEGETTEXT */
#define _( x ) ( x )
#define __( x, y ) ( x )
#endif /* USEGETTEXT */

/* If we're not using GNU C, elide __attribute__ */
#ifndef __GNUC__
#define __attribute__( x )  /* NOTHING */
#endif /* __GNUC__ */

/* va_copy handling*/
#ifndef HAVE_VA_COPY
#if HAVE___VA_COPY 
#define va_copy( dest, src ) __va_copy( ( dest ), ( src ) ) 
#else /* HAVE___VA_COPY */
#define va_copy( dest, src ) memcpy( &( dest ), &( src ), sizeof( dest ) ) 
#endif /* HAVE___VA_COPY */ 
#endif /* HAVE_VA_COPY */

#include "pcre.h"
#include "adns.h"
#include "list.h"
#include "hash.h"
#include "support.h"
#include "events.h"
#include "numeric.h"

#define ARRAYLEN( a ) ( sizeof( a ) / sizeof( *( a ) ) )

#define PROTOCOL_NOQUIT		0x00000001	/* NOQUIT */
#define PROTOCOL_TOKEN		0x00000002	/* TOKEN */
#define PROTOCOL_SJOIN		0x00000004	/* SJOIN */
#define PROTOCOL_NICKv2		0x00000008	/* NICKv2 */
#define PROTOCOL_SJOIN2		0x00000010	/* SJOIN2 */
#define PROTOCOL_UMODE2		0x00000020	/* UMODE2 */
#define PROTOCOL_NS			0x00000040	/* NS */
#define PROTOCOL_ZIP		0x00000080	/* ZIP - not actually supported by NeoStats */
#define PROTOCOL_VL			0x00000100	/* VL */
#define PROTOCOL_SJ3		0x00000200	/* SJ3 */
#define PROTOCOL_VHP		0x00000400	/* Send hostnames in NICKv2 even if not sethosted */
#define PROTOCOL_SJB64		0x00000800  /* */
#define PROTOCOL_CLIENT		0x00001000  /* CLIENT */
#define PROTOCOL_B64SERVER	0x00002000  /* Server names use Base 64 */
#define PROTOCOL_B64NICK	0x00004000  /* Nick names use Base 64 */
#define PROTOCOL_UNKLN		0x00008000  /* Have UNKLINE support */
#define PROTOCOL_NICKIP		0x00010000  /* NICK passes IP address */
#define PROTOCOL_KICKPART	0x00020000  /* KICK also generates PART */
#define PROTOCOL_P10		0x00040000  /* Protocol is IRCu P10 based */
#define PROTOCOL_EOB		0x00080000  /* Protocol supports End Of Burst Info */
#define PROTOCOL_CLIENTMODE	0x80000000  /* Client mode */

#define FEATURE_SWHOIS		0x00000001	/* SWHOIS */
#define FEATURE_SVSTIME		0x00000002	/* SVSTIME */
#define FEATURE_SVSHOST		0x00000004	/* SVSHOST */
#define FEATURE_SVSJOIN		0x00000008	/* SVSJOIN */
#define FEATURE_SVSMODE		0x00000010	/* SVSMODE */
#define FEATURE_SVSPART		0x00000020	/* SVSPART */
#define FEATURE_SVSNICK		0x00000040	/* SVSNICK */
#define FEATURE_SVSKILL		0x00000080	/* SVSKILL */
#define FEATURE_UMODECLOAK  0x00000100	/* auto cloak host with umode */
#define FEATURE_USERSMODES	0x00000200	/* User Smode field */
#define FEATURE_SMO			0x00000400	/* SMO */

/* cumodes are channel modes which affect a user */
#define CUMODE_CHANOP		0x00000001
#define CUMODE_VOICE		0x00000002
#define CUMODE_HALFOP		0x00000004
#define CUMODE_CHANOWNER	0x00000008
/* Following are mutually exclusive in current IRCd support so share bits.
 * If this changes, all these must change.
 */
#define CUMODE_CHANPROT		0x00000010
#define CUMODE_CHANADMIN	0x00000010

/* Channel modes available on all IRCds */
#define CMODE_PRIVATE		0x00000020
#define CMODE_SECRET		0x00000040
#define CMODE_MODERATED		0x00000080
#define CMODE_TOPICLIMIT	0x00000100
#define CMODE_BAN			0x00000200
#define CMODE_INVITEONLY	0x00000400
#define CMODE_NOPRIVMSGS	0x00000800
#define CMODE_KEY			0x00001000
#define CMODE_LIMIT			0x00002000

/* Channel modes available on most IRCds */
#define CMODE_EXCEPT		0x00004000
#define CMODE_RGSTR			0x00008000
#define CMODE_RGSTRONLY		0x00010000
#define CMODE_LINK			0x00020000
#define CMODE_NOCOLOR		0x00040000
#define CMODE_OPERONLY		0x00080000
#define CMODE_ADMONLY		0x00100000
#define CMODE_STRIP			0x00200000
#define CMODE_NOKNOCK		0x00400000
#define CMODE_NOINVITE		0x00800000
#define CMODE_FLOODLIMIT	0x01000000

/* Other channel modes available on IRCds cannot be easily supported so 
 * should be defined locally beginning at 0x02000000
 */

/* Cmode macros */
#define is_hidden_chan( x ) ( ( x ) && ( ( x )->modes & ( CMODE_PRIVATE | CMODE_SECRET | CMODE_ADMONLY | CMODE_OPERONLY ) ) )
#define is_pub_chan( x )  ( ( x ) && !( ( x )->modes & ( CMODE_PRIVATE | CMODE_SECRET | CMODE_RGSTRONLY | CMODE_ADMONLY | CMODE_OPERONLY | CMODE_INVITEONLY | CMODE_KEY ) ) )
#define is_priv_chan( x ) ( ( x ) && ( ( x )->modes & ( CMODE_PRIVATE | CMODE_SECRET | CMODE_RGSTRONLY | CMODE_ADMONLY | CMODE_OPERONLY | CMODE_INVITEONLY | CMODE_KEY ) ) )

/* User modes available on all IRCds */
#define UMODE_INVISIBLE		0x00000001	/* makes user invisible */
#define UMODE_OPER			0x00000002	/* Operator */
#define UMODE_WALLOP		0x00000004	/* send wallops to them */

/* User modes available on most IRCds */
#define UMODE_LOCOP			0x00000008	/* Local operator -- SRB */
#define UMODE_REGNICK		0x00000010	/* umode +r - registered nick */
#define UMODE_DEAF          0x00000020	/* Dont see chan msgs */
#define UMODE_HIDE          0x00000040	/* Hide from Nukes */
#define UMODE_BOT			0x00000080	/* User is a bot */

#define UMODE_RBOT			UMODE_BOT	/* Registered Bot */
#define UMODE_SBOT			UMODE_BOT	/* Server Bot */

#define UMODE_SADMIN		0x00000100	/* Services Admin */
#define UMODE_ADMIN			0x00000200	/* Admin */
#define UMODE_SERVICES		0x00000400	/* services */
#define UMODE_NETADMIN		0x00000800	/* Network Admin */
#define UMODE_COADMIN		0x00001000	/* Co Admin */
#define UMODE_TECHADMIN     0x00002000  /* Technical Administrator */
#define UMODE_CLIENT		0x00004000	/* Show client information */
#define UMODE_FCLIENT		0x00008000	/* receive client on far connects.. */
#define UMODE_KIX			0x00010000	/* protected oper, only ulines can kick */
#define UMODE_HELPOP		0x00020000	/* Help system operator */
#define UMODE_RGSTRONLY		0x00040000	/* only registered nicks may PM */

/* Other user modes available on IRCds cannot be easily supported so 
 * should be defined locally beginning at 0x00080000
 */

/* Smodes */
#define SMODE_SSL			0x00000001	/* ssl client */
#define SMODE_COADMIN		0x00000002	/* co admin on a server */
#define SMODE_ADMIN			0x00000004	/* server admin */
#define SMODE_COTECHADMIN	0x00000008	/* co-tech admin */
#define SMODE_TECHADMIN		0x00000010	/* tech administrator */
#define SMODE_CONETADMIN	0x00000020	/* Co-Network Admin */
#define SMODE_NETADMIN		0x00000040	/* Network Admin */
#define SMODE_GUESTADMIN	0x00000080	/* Guest Admin */

EXPORTVAR extern unsigned int ircd_supported_umodes;
EXPORTVAR extern unsigned int ircd_supported_smodes;
EXPORTVAR extern unsigned int ircd_supported_cmodes;
EXPORTVAR extern unsigned int ircd_supported_cumodes;
#define HaveUmodeRegNick() ( ircd_supported_umodes & UMODE_REGNICK )
#define HaveUmodeDeaf() ( ircd_supported_umodes & UMODE_DEAF )

/* Umode macros */
#define IsOper(x) ( ( x ) && ( ( x->user->Umode & ( UMODE_OPER | UMODE_LOCOP ) ) ) )
#define IsBot(x) ( ( x ) && ( x->user->Umode & UMODE_BOT ) )
#define IsServerOperMode( mode ) ( mode & ( UMODE_ADMIN | UMODE_COADMIN | UMODE_OPER | UMODE_LOCOP ) )
#define IsServerOperSMode( mode ) ( mode & ( UMODE_ADMIN | UMODE_COADMIN | UMODE_OPER | UMODE_LOCOP ) )

EXPORTFUNC unsigned int UmodeCharToMask( unsigned char mode );
EXPORTFUNC const char *GetUmodeDesc( unsigned int mask );
EXPORTFUNC unsigned int SmodeCharToMask( unsigned char mode );
EXPORTFUNC const char *GetSmodeDesc( unsigned int mask );
EXPORTFUNC unsigned int UmodeStringToMask( const char *UmodeString );
EXPORTFUNC char *UmodeMaskToString( unsigned int mask );
EXPORTFUNC unsigned char UmodeMaskToChar( unsigned int mask );
EXPORTFUNC unsigned int SmodeStringToMask( const char *UmodeString );
EXPORTFUNC char *SmodeMaskToString( unsigned int mask );
EXPORTFUNC unsigned char SmodeMaskToChar( unsigned int mask );
EXPORTFUNC unsigned int CmodeStringToMask( const char *UmodeString );
EXPORTFUNC char *CmodeMaskToString( unsigned int mask );
EXPORTFUNC char *CmodeMaskToPrefixString( unsigned int mask );
EXPORTFUNC unsigned int CmodeCharToMask( unsigned char mode );
EXPORTFUNC unsigned char CmodeMaskToChar( unsigned int mask );
EXPORTFUNC unsigned int CmodeCharToFlags( unsigned char mode );
EXPORTFUNC unsigned int CmodePrefixToMask( unsigned char prefix );
EXPORTFUNC unsigned char CmodePrefixToChar( unsigned char prefix );
EXPORTFUNC unsigned char CmodeMaskToPrefix( unsigned int mask );
EXPORTFUNC unsigned char CmodeCharToPrefix( unsigned char mode );

#ifdef NEOSTATS_REVISION
#define NEOSTATS_VERSION NEO_VERSION " (" NEOSTATS_REVISION ") " NS_HOST
#else /* NEOSTATS_REVISION */
#define NEOSTATS_VERSION NEO_VERSION " " NS_HOST
#endif /* NEOSTATS_REVISION */
#define CORE_MODULE_VERSION NEOSTATS_VERSION

#define TS5

#ifdef TS5
#define TS_CURRENT	5	/* current TS protocol version */
#else /* TS5 */
#define TS_CURRENT	3	/* current TS protocol version */
#endif /* TS5 */

#define TS5_ONLY

#ifdef TS5_ONLY
#define TS_MIN          5
#else /* TS5_ONLY */
#define TS_MIN          3       /* minimum supported TS protocol version */
#endif /* TS5_ONLY */

#define TS_ONE_MINUTE	( 60 )
#define TS_ONE_HOUR		( TS_ONE_MINUTE * 60 )	/* 3600 */
#define TS_ONE_DAY		( TS_ONE_HOUR * 24 )	/* 86400 */
#define TS_ONE_WEEK		( TS_ONE_DAY * 7 )		/* 604800 */

#define MOD_PATH		"modules"

#define BASE64SERVERSIZE	2
#define BASE64NICKSIZE		5

#define BUFSIZE			512

/* this is like a recvq setting. Going over this, we are getting flooded */
#define LINEBUFSIZE		2048 

/* this is the max data we read from a sock at once */
#define	READBUFSIZE		4096

#define MAXHOST			(128 + 1)
#define MAXPASS			(32 + 1)
#define MAXNICK			(32 + 1)
#define MAXUSER			(15 + 1)
#define MAXREALNAME		(50 + 1)
#define MAXCHANLEN		(50 + 1)
#define MAXTOPICLEN		(307 + 1)
#define CLOAKKEYLEN		(40 + 1)

/* the max number of calls we will print out in a backtrace */
#define MAXBACKTRACESIZE		30

#define HOSTIPLEN		( 15 + 1 )	/* Size of IP address in dotted quad */
/* Size of nick!user@host mask */
#define	USERHOSTLEN		(MAXNICK + MAXHOST + MAXUSER + 5)

#define MODESIZE		53
#define PARAMSIZE		MAXNICK+MAXUSER+MAXHOST+10
#define MAXINFO			MAXREALNAME
#define B64SIZE			16

#define KEYLEN			(32 + 1)

/* MAXCHANLENLIST
 * the max length a string can be that holds channel lists 
 */
#define MAXCHANLENLIST	1024 

/* MAXPATH 
 * used to determine buffer sizes for file system operations
 */
#ifndef MAXPATH
#define MAXPATH			1024
#endif /* MAXPATH */

/* TIMEBUFSIZE
 * used to determine buffer sizes for time formatting buffers
 */
#define TIMEBUFSIZE		80

/* STR_TIME_T_SIZE
 * size of a time_t converted to a string. 
 */
#define STR_TIME_T_SIZE	24

/* MAX_MOD_NAME
   ModuleInfo will allow any length since it is merely a char *
   functions displaying ModuleInfo contents will display the full string.
   NeoStats core will truncate to this length for use internally. 
*/
#define MAX_MOD_NAME	32

/* Buffer size for reason string */
#define MAXREASON		128

/* Buffer size for version string */
#define VERSIONSIZE		128

/* Maximum number of modules that can be loaded */
#define NUM_MODULES		40

#define IsNeoStatsSynched()		me.synched

/* Unified return values and error system */

/* NeoStats general success failure return type */
#define NS_SUCCESS			 1
#define NS_FAILURE			-1

/* NeoStats boolean return type */
#define NS_TRUE			1
#define NS_FALSE		0

/* Specific errors beyond SUCCESS/FAILURE so that functions can handle errors 
 * Treat as unsigned with top bit set to give us a clear distinction from 
 * other values and use a typedef ENUM so that we can indicate return type */
typedef enum NS_ERR {
	NS_ERR_NICK_IN_USE		= 0x8000001,
	NS_ERR_OUT_OF_MEMORY	= 0x8000002,
	NS_ERR_VERSION			= 0x8000003,
	NS_ERR_SYNTAX_ERROR		= 0x8000004,
	NS_ERR_NEED_MORE_PARAMS	= 0x8000005,
	NS_ERR_NO_PERMISSION	= 0x8000006,
	NS_ERR_UNKNOWN_COMMAND	= 0x8000007,
	NS_ERR_UNKNOWN_OPTION	= 0x8000008,
	NS_ERR_PARAM_OUT_OF_RANGE= 0x8000009,
}NS_ERR ;

/* General flags for for clients (users and servers) and channels */
#define NS_FLAG_EXCLUDED	0x00000001 /* matches a exclusion */

/* Flags for clients (users and servers) */
#define CLIENT_FLAG_EXCLUDED	NS_FLAG_EXCLUDED /* client is excluded */
#define CLIENT_FLAG_ME			0x00000002 /* client is a NeoStats one */
#define CLIENT_FLAG_SYNCHED		0x00000004 /* client is synched */
#define CLIENT_FLAG_SETHOST		0x00000008 /* client is sethosted */
#define CLIENT_FLAG_DCC			0x00000010 /* client is connected via DCC */
#define CLIENT_FLAG_ZOMBIE		0x00000020 /* client is zombie */
#define CLIENT_FLAG_GOTVERSION	0x00000040 /* got version reply */
#define NS_FLAGS_NETJOIN		0x00000080 /* client is on a net join */

#define CHANNEL_FLAG_EXCLUDED	NS_FLAG_EXCLUDED /* channel is excluded */
#define CHANNEL_FLAG_ME			0x00000002 /* channel is services channel */

#define IsServicesChannel( x ) ( ( x )->flags & CHANNEL_FLAG_ME )

#define IsNetSplit( x ) ( ( x )->flags & NS_FLAGS_NETJOIN )
#define ClearNetSplit( x ) ( ( x )->flags &= ~NS_FLAGS_NETJOIN )

/* NeoStats levels */
#define NS_ULEVEL_ROOT		200
#define NS_ULEVEL_ADMIN		185
#define NS_ULEVEL_OPER		50
#define NS_ULEVEL_LOCOPER	40
#define NS_ULEVEL_REG		10

/* transfer stuff */
typedef enum NS_TRANSFER {
	NS_FILE = 0,
	NS_MEMORY,
} NS_TRANSFER;

#define SEGV_LOCATION_BUFSIZE	255
#define SET_SEGV_LOCATION() ircsnprintf( segv_location, SEGV_LOCATION_BUFSIZE, "%s %d %s", __FILE__, __LINE__, __PRETTY_FUNCTION__ ); 
#define SET_SEGV_LOCATION_EXTRA( debug_text ) ircsnprintf( segv_location, SEGV_LOCATION_BUFSIZE, "%s %d %s %s", __FILE__, __LINE__, __PRETTY_FUNCTION__, (debug_text) ); 
#define CLEAR_SEGV_LOCATION() segv_location[0]='\0';

EXPORTVAR extern char segv_location[SEGV_LOCATION_BUFSIZE];

/* this is the dns structure */
extern adns_state nsads;

/* version info */
EXPORTVAR extern const char version_date[];
EXPORTVAR extern const char version_time[];

/* Forward references for cyclic structs */
typedef struct Module Module;
typedef struct Bot Bot;

/* to avoid warnings for Sock */
struct Sock;

/** @brief Server structure
 *  Client extension structure for server specifics
 */
typedef struct Server {
	unsigned int users;
	unsigned int awaycount;
	int hops;
	int numeric;
	time_t ping;
	time_t uptime;
} Server;

/** @brief User structure
 *  Client extension structure for user specifics
 */
typedef struct User {
	char hostname[MAXHOST];
	char username[MAXUSER];
	char vhost[MAXHOST];
	char awaymsg[MAXHOST];
	char swhois[MAXHOST];
	char userhostmask[USERHOSTLEN];
	char uservhostmask[USERHOSTLEN];
	unsigned int flood;
	int is_away;
	time_t tslastmsg;
	time_t tslastnick;
	time_t tslastaway;
	time_t servicestamp;
	char modes[MODESIZE];
	unsigned int Umode;
	char smodes[MODESIZE];
	unsigned int Smode;
	int ulevel;
	list_t *chans;
	Bot *bot;
} User;

/** @brief Client structure
 *  
 */
typedef struct Client {
	User *user;
	Server *server;
	char name[MAXNICK];
	char name64[B64SIZE];
	char uplinkname[MAXHOST];
	struct Client *uplink;
	char info[MAXREALNAME];
	char version[MAXHOST];
	unsigned int flags;
	time_t tsconnect;
	struct in_addr ip;
	char hostip[HOSTIPLEN];
	int lang;
	void *modptr[NUM_MODULES];
	void *modvalue[NUM_MODULES];
	OS_SOCKET fd;
	int port;
	struct Sock *sock;
} Client; 


/** @brief me structure
 *  structure containing information about the neostats core
 */
typedef struct tme {
	char name[MAXHOST];
	int numeric; 
	char protocol[MAXHOST];
	char rootnick[MAXNICK];
	char dbm[MAXHOST];
	char uplink[MAXHOST];
	char infoline[MAXHOST];
	char netname[MAXPASS];
	char local[MAXHOST];
	char servicehost[MAXHOST];
	int port;
	int lang;
	time_t ts_boot;
	unsigned int usercount;
	unsigned int awaycount;
	unsigned int channelcount;
	unsigned int servercount;
	unsigned int maxsocks;
	unsigned int cursocks;
	unsigned int want_nickip:1;
	char servicescmode[MODESIZE];
	unsigned int servicescmodemask;
	char servicesumode[MODESIZE];
	unsigned int servicesumodemask;
	char serviceschan[MAXCHANLEN];
	unsigned int synched:1;
	Client *s;
	struct Sock *servsock;
	int requests;
	long SendM;
	long SendBytes;
	long RcveM;
	long RcveBytes;
	time_t lastmsg;
	time_t now;
	char strnow[STR_TIME_T_SIZE];
	char version[VERSIONSIZE];
	int dobind;
	struct sockaddr_in lsa;
	struct sockaddr_in srvip;
	time_t tslastping;
	time_t ulag;
	unsigned int versionscan;
} tme;

EXPORTVAR extern tme me;

#define NSGetChannelCount() me.channelcount
#define NSGetServerCount() me.servercount
#define NSGetUserCount() me.usercount
#define NSGetAwayCount() me.awaycount

/** @brief Bans structure
 *  
 */
typedef struct Ban {
	char type[8];
	char user[MAXUSER];
	char host[MAXHOST];
	char mask[MAXHOST];
	char reason[BUFSIZE];
	char setby[MAXHOST];
	time_t tsset;
	time_t tsexpires;
} Ban;


/** @brief ModeParams structure
 *  
 */
typedef struct ModeParams {
	unsigned int mask;
	char param[PARAMSIZE];
} ModeParams;

/** @brief ChannelMember structure
 *  
 */
typedef struct ChannelMember {
	Client *u;
	time_t tsjoin;
	unsigned int modes;
} ChannelMember;

/** @brief Channel structure
 *  
 */
typedef struct Channel {
	char name[MAXCHANLEN];
	char name64[B64SIZE];
	unsigned int users;
	unsigned int neousers;
	unsigned int persistentusers;
	int lang;
	unsigned int modes;
	list_t *members;
	char topic[BUFSIZE];
	char topicowner[MAXHOST];	/* because a "server" can be a topic owner */
	time_t topictime;
	int  limit;
	char key[KEYLEN];
	list_t *modeparams;
	time_t creationtime;
	unsigned int flags;
	void *modptr[NUM_MODULES];
	void *modvalue[NUM_MODULES];
} Channel;

typedef struct bot_cmd bot_cmd;

typedef struct CmdParams {
	Client *source;		/* pointer to client triggering command */
	Client *target;		/* pointer to client command acts on */
	Bot *bot;			/* pointer to associated bot where appropriate */
	char *param;		/* command parameter */
	char *cmd;			/* command */
	bot_cmd *cmd_ptr;	/* pointer to associated command structure */
	Channel *channel;	/* pointer to channel struct where appropriate */
	char **av;			/* command parameter list */
	int ac;				/* count of command parameter list */
	int eventid;		/* the event that triggered this */
} CmdParams; 

/* Comand list handling */

/** @brief bot_cmd_handler type
 *  defines handler function definition
 */
/* SET_REASON is passed to SET command callback funtions 
 * so the callback knows why it triggered
 */
typedef enum SET_REASON {
	SET_LOAD = 0,
	SET_LIST,
	SET_CHANGE,
	SET_VALIDATE,
} SET_REASON;

typedef int (*bot_cmd_handler) ( const CmdParams* cmdparams );
typedef int (*bot_set_handler) ( const CmdParams* cmdparams, SET_REASON reason );

/* Command will only respond to privmsg. !command in channel is ignored.
 * Use of this flag is discouraged.
 */
#define CMD_FLAG_PRIVMSGONLY	0x00000001
/* Command will only respond to !command in channel. privmsg is ignored.
 * Use of this flag is discouraged.
 */
#define CMD_FLAG_CHANONLY		0x00000002

/** @brief bot_cmd structure
 *  defines command lists for bots
 */
struct bot_cmd
{
	const char		*cmd;		/* command string */
	bot_cmd_handler	handler;	/* handler */
	int				minparams;	/* min num params */
	int				ulevel;		/* min user level */
	const char		**helptext;	/* pointer to help text */
	int				flags;		/* command flags */
	void			*moddata;	/* pointer for module use */
	Module			*modptr;	/* NeoStats internal use only */
};

#define NS_CMD_END() { NULL, NULL, 0, 0, NULL, 0, NULL, NULL }

/** @brief flags for bots
 *  flags to influence how bots are managed
 *  e.g. restrict to opers
 */

/* Restrict module bot to only respond to oper requests
 * when ONLY_OPERS is set in the config file 
 * E.g. StatServ
 */
#define BOT_FLAG_ONLY_OPERS		0x00000001
/* Restrict module bot to only respond to oper requests
 * regardless of ONLY_OPERS setting in the config file
 * E.g. Connectserv
 */
#define BOT_FLAG_RESTRICT_OPERS	0x00000002
/* Stop bot listening to channel chatter when they do not need to
 * E.g. Connectserv
 */
#define BOT_FLAG_DEAF	0x00000004
/* Mark bot as a root bot that will manage commands and settings 
 * for the module. Limited to one per module.
 * E.g. Connectserv
 */
#define BOT_FLAG_ROOT	0x00000008
/* Temp while flag is deprecated across modules */
#define BOT_FLAG_SERVICEBOT	BOT_FLAG_ROOT
/* Mark bot as persistent even when no users are left in a channel
 * If not set, and there are no bots with this flag set, when all
 * users leave a channel, the bot will automatically leave aswell.
 * You should watch the NEWCHAN event to join channels when they
 * are created. 
 */
#define BOT_FLAG_PERSIST	0x00000010
/* Bot becomes CTCP master for version requests and replies
 * E.g. SecureServ issuing CTCP VERSION rather than NeoStats
 */
#define BOT_FLAG_CTCPVERSIONMASTER	0x00000020

/* This defines a "NULL" string for the purpose of BotInfo structures that 
 * want to inherit the main host used by NeoStats and still make the info
 * readable
 */
#define BOT_COMMON_HOST	""

/* SET Comand handling */

typedef enum SET_TYPE {
	SET_TYPE_NONE = -1,	/* ON or OFF */
	SET_TYPE_BOOLEAN = 0,	/* ON or OFF */
	SET_TYPE_INT,		/* valid integer */
	SET_TYPE_STRING,	/* single string */
	SET_TYPE_MSG,		/* multiple strings to be treated as a message and stored in one field */
	SET_TYPE_NICK,		/* valid nick */
	SET_TYPE_USER,		/* valid user */
	SET_TYPE_HOST,		/* valid host name */
	SET_TYPE_REALNAME,	/* valid realname */
	SET_TYPE_CHANNEL,	/* valid channel */
	SET_TYPE_IPV4,		/* valid IPv4 dotted quad */
	SET_TYPE_CUSTOM,	/* handled by module */
}SET_TYPE;

/** @brief bot_setting structure
 *  defines SET list for bots
 */
typedef struct bot_setting {
	char			*option;	/* option string */
	void			*varptr;	/* pointer to var */
	SET_TYPE		type;		/* type of var */
	int				min;		/* min value */
	int				max;		/* max value */
	int				ulevel;		/* min user level */
	const char		*desc;		/* description of setting for messages e.g. seconds, days*/
	const char		**helptext;	/* pointer to help text */
	bot_set_handler	handler;	/* handler for custom/post-set processing */
	void			*defaultval;/* default value for setting */
}bot_setting;

#define NS_SETTING_END() { NULL, NULL, SET_TYPE_NONE, 0, 0, 0, NULL, NULL, NULL, NULL }

/** @brief Message function types
 * 
 */
typedef int (*timer_handler) ( void * );

/* Event system flags */
#define	EVENT_FLAG_DISABLED			0x00000001	/* Event is disabled */
#define	EVENT_FLAG_IGNORE_SYNCH		0x00000002	/* Event triggers even if not synched */
#define	EVENT_FLAG_USE_EXCLUDE		0x00000004  /* Event observes global exclusions */
#define	EVENT_FLAG_EXCLUDE_ME		0x00000008	/* Event excludes neostats bots and servers */
#define	EVENT_FLAG_EXCLUDE_MODME	0x00000010	/* Event excludes module bots */
#define EVENT_FLAG_PERLCALL			0x00000020	/* Event is for a perl Module/Extension */

#ifdef USE_PERL
/** @brief Forward declaration of perl events 
 */
struct PerlEvent;
#endif /* USE_PERL */

/** @brief Event function types
 * 
 */
typedef int (*event_handler) ( const CmdParams *cmdparams );

/** @brief ModuleEvent functions structure
 * 
 */

typedef struct ModuleEvent {
	Event event;
	event_handler handler;
	unsigned int flags;
#ifdef USE_PERL
	struct PerlEvent *pe;
#endif /* USE_PERL */
}ModuleEvent;

#define NS_EVENT_END() { EVENT_NULL, NULL, 0 }

typedef int ModuleProtocol;
typedef int ModuleFeatures;
typedef int ModuleFlags;

#define MODULE_FLAG_NONE			0x00000000
#define MODULE_FLAG_AUTH			0x00000001
#define MODULE_FLAG_LOCAL_EXCLUDES	0x00000002
#define MODULE_FLAG_CTCP_VERSION	0x00000004

typedef enum MOD_TYPE {
	/* standard C Modules */
	MOD_TYPE_STANDARD = 1,
	/* Perl Modules */
	MOD_TYPE_PERL
} MOD_TYPE;

/** @brief Module Info structure
 *	This describes the module to the NeoStats core and provides information
 *  to end users when modules are queried.
 *  The presence of this structure is required but some fields are optional.
 */
typedef struct ModuleInfo {
	/* REQUIRED: 
	 * name of module e.g. StatServ */
	const char *name;
	/* REQUIRED: 
	 * one line brief description of module */
	const char *description;
	/* OPTIONAL: 
	 * pointer to a NULL terminated list with copyright information
	 * NeoStats will automatically provide a CREDITS command to output this
	 * use NULL for none */
	const char **copyright;
	/* OPTIONAL: 
	 * pointer to a NULL terminated list with extended description
	 * NeoStats will automatically provide an ABOUT command to output this
	 * use NULL for none */
	const char **about_text;
	/* REQUIRED: 
	 * version of neostats used to build module
	 * must be NEOSTATS_VERSION */
	const char *neostats_version;
	/* REQUIRED: 
	 * string containing version of module */
	const char *version;
	/* REQUIRED: string containing build date of module 
	 * should be __DATE__ */
	const char *build_date;
	/* REQUIRED: string containing build time of module 
	 * should be __TIME__ */
	const char *build_time;
	/* OPTIONAL: 
	 * Module control flags, 
	 * use 0 if not needed */
	const ModuleFlags flags;
	/* OPTIONAL: 
	 * Protocol flags for required protocol specfic features e.g. NICKIP
	 * use 0 if not needed */
	const ModuleProtocol protocol;
	/* OPTIONAL: 
	 * Protocol flags for required protocol specfic features e.g. SETHOST
	 * use 0 if not needed */
	const ModuleFeatures features;
}ModuleInfo;

typedef int (*mod_auth) ( const Client *u );

/* Module type macros */
#define IS_STANDARD_MOD( mod ) ( ( mod )->type == MOD_TYPE_STANDARD )
#ifdef USE_PERL	
#define IS_PERL_MOD( mod ) ( ( mod )->type == MOD_TYPE_PERL )
#else /* USE_PERL */
#define IS_PERL_MOD( mod ) ( 0 )
#endif /* USE_PERL */

#ifdef USE_PERL	

/* forward decleration (in perlmod.h) for perl module info
 * we don't include any perl includes here because it screws up
 * some of the existing system defines (like readdir) */
struct PerlModInfo;

/* to save some chars while typing */
#define PMI PerlInterpreter

#endif /* USE_PERL */

#define MODULE_STATUS_SYNCHED	0x00000001
#define MODULE_STATUS_INSYNCH	0x00000002
#define MODULE_STATUS_ERROR		0x00000004
#define MODULE_STATUS_ROOTBOT	0x00000008

/** @brief Module structure
 * 
 */
struct Module {
	/** type of module  */
	MOD_TYPE type;
	/** Pointer to info structure */
	ModuleInfo *info;
	/** Pointer to event list */
	ModuleEvent **event_list;
	/** Optional module supplied auth callback for modules to authorise
	 *  module commands not handled by core auth modules (e.g. SecureServ helpers) */
	mod_auth authcb;
	/** Auth callback for auth modules */
	mod_auth userauth;
	/** Exclude list */
	list_t *exclude_list;
	/** Exclude command list */
	bot_cmd *exclude_cmd_list;
	/** Dynamic library handle */
	void *handle;
	/** index */
	unsigned int modnum;
	/** status flag for synch, error, etc */
	unsigned int status;
	/** moddata flags */
	unsigned int userdatacnt;
	unsigned int serverdatacnt;
	unsigned int channeldatacnt;
#ifdef USE_PERL
	struct PerlModInfo *pm;
#endif /* USE_PERL */
};

/* Set module status */
#define SetModuleSynched( m ) ( ( m )->status |= MODULE_STATUS_SYNCHED )
#define SetModuleInSynch( m ) ( ( m )->status |= MODULE_STATUS_INSYNCH )
#define SetModuleError( m ) ( ( m )->status |= MODULE_STATUS_ERROR )
#define SetModuleRootBot( m ) ( ( m )->status |= MODULE_STATUS_ROOTBOT )
/* Test module status */
#define IsModuleSynched( m ) ( ( m )->status & MODULE_STATUS_SYNCHED )
#define IsModuleInSynch( m ) ( ( m )->status & MODULE_STATUS_INSYNCH )
#define IsModuleError( m ) ( ( m )->status & MODULE_STATUS_ERROR )
#define IsModuleRootBot( m ) ( ( m )->status & MODULE_STATUS_ROOTBOT )

#define ModuleSynched()	( GET_CUR_MODULE()->status & MODULE_STATUS_SYNCHED )

/* Simple stack to manage run level replacing segv_module used in 
 * previous versions. This makes it easier to determine where we are 
 * running and avoids the need for modules to manage this or the core to
 * have to set/reset when a module calls a core function which triggers
 * other modules to run (e.g. AddBot)
 */
/* Run level stack */
EXPORTVAR extern Module *RunModule[10]; 
/* Run level stack index */
EXPORTVAR extern int RunLevel;
/* Macros to manage run level stack */
/* Set current run level */
#define SET_RUN_LEVEL( moduleptr ) { if( RunLevel < 10 ) { RunLevel++; RunModule[RunLevel] = moduleptr; } }
/* Reset run level */
#define RESET_RUN_LEVEL() { if( RunLevel > 0 ) { RunLevel--; } }
/* Get current run level module pointer */
#define GET_CUR_MODULE() RunModule[RunLevel]
/* Get current run level module index */
#define GET_CUR_MODULE_INDEX() RunModule[RunLevel]->modnum
/* Get current run level module name */
#define GET_CUR_MODNAME() RunModule[RunLevel]->info->name
/* Get current run level module version */
#define GET_CUR_MODVERSION() RunModule[RunLevel]->info->version

/** @brief Socket function types
 * 
 */

/* socket interface type */
typedef enum SOCK_TYPE {
	/* */
	SOCK_STANDARD = 1,
	/* */
	SOCK_BUFFERED,
	/* */
	SOCK_LINEMODE,
	/* */
	SOCK_LISTEN,
	/* */
	SOCK_NATIVE,
}SOCK_TYPE;

typedef int (*sockcb)(int, void *data);
typedef int (*sockfunccb)(void *, void *, int);

/** @brief Module socket list structure
 * 
 */
typedef struct Sock {
	/** Owner module ptr */
	Module *moduleptr;
	/** Socket number */
	OS_SOCKET sock_no;
	/** Socket name */
	char name[MAX_MOD_NAME];
	/** socket interface (poll or standard) type */
	int socktype;
	/** data */
	void *data;
	/* if socktype = SOCK_STANDARD, function calls */
	/** Socket read function */
	/** rmsgs */
	long rmsgs;
	/** rbytes */
	long rbytes;
	/** smsgs */
	long smsgs;
	/** sbytes */
	long sbytes;
	union {
		struct bufferevent *buffered;
		struct event *event;
	} event;
	union {
		struct linemode {
			char *readbuf;
			size_t readbufsize;
			sockfunccb funccb;
			sockcb errcb;
			size_t recvq;
		} linemode;
		struct listenmode {
			int port;
			sockcb funccb;
		} listenmode;
		struct standmode {
			sockfunccb readfunc;
			sockcb writefunc;
		} standmode;
	} sfunc;		
} Sock;

typedef enum TIMER_TYPE {
	/* Called at the specified interval */	
	TIMER_TYPE_INTERVAL,
	/* Called at the beginning of the day (midnight) */
	TIMER_TYPE_DAILY,
	/* Called at the beginning of the week */	
	TIMER_TYPE_WEEKLY,
	/* Called at the beginning of the month */	
	TIMER_TYPE_MONTHLY,
	/* One shot countdown which is removed after trigger */	
	TIMER_TYPE_COUNTDOWN,
} TIMER_TYPE;

/** @brief Module Timer structure
 * 
 */
typedef struct Timer {
	/** Owner module ptr */
	Module *moduleptr;
	/** Timer type */
	TIMER_TYPE type;
	/** Timer name */
	char name[MAX_MOD_NAME];
	/** Timer interval */
	time_t interval;
	/** Time last run */
	time_t lastrun;
	/** Timer handler */
	timer_handler handler;
	/** Pointer to user suplied context */
	void *userptr;
	/** Next Run at time */
	time_t nextrun;
} Timer;

/** @brief BotInfo structure
 * 
 */
typedef struct BotInfo {		
	/* REQUIRED: nick */
	char nick[MAXNICK];
	/* OPTIONAL: altnick, use "" if not needed */
	char altnick[MAXNICK];
	/* REQUIRED: user */
	char user[MAXUSER];
	/* REQUIRED: host */
	char host[MAXHOST];
	/* REQUIRED: realname */
	char realname[MAXREALNAME];
	/* OPTIONAL: flags */
	unsigned int flags;
	/* OPTIONAL: bot command list pointer */
	bot_cmd *bot_cmd_list;
	/* OPTIONAL: bot command setting pointer */
	bot_setting *bot_setting_list;
} BotInfo;

/** @brief Bot structure
 * 
 */

struct Bot
{
	/** Owner module ptr */
	Module *moduleptr;
	/** Nick */
	char name[MAXNICK];
	/* bot flags */
	unsigned int flags;
	/* hash for command list */
	hash_t *botcmds;
	/* hash for settings */
	hash_t *botsettings;
	/* hash for bot_info settings */
	bot_setting *bot_info_settings;
	/* min ulevel for settings */
	int set_ulevel;
	/* Link back to user struct associated with this bot*/
	Client *u;
	/* link back to BotInfo struct so if we have a module that needs to free it, it can be referenced */
	BotInfo *botinfo;
	/* pointer for module use */
	void *moddata;
};

/* current state */
typedef enum MQS_STATE {
	MQS_DISCONNECTED,
	MQS_CONNECTING,
	MQS_SENTAUTH,
	MQS_OK,
} MQS_STATE;

/* connection strategy */
typedef enum MQS_CONNECT {
        MQ_CONNECT_ERROR,
        MQ_CONNECT_NO,
        MQ_CONNECT_DEMAND,
        MQ_CONNECT_YES,
} MQS_CONNECT;


/* this is the NeoNet details */
typedef struct updateserver {
	MQS_STATE state;
	OS_SOCKET sock;
	MQS_CONNECT connect;
	char username[MAXUSER];
	char password [MAXUSER];
	char hostname[MAXHOST];
	int port;
	Sock *Sockinfo;
	/* max of 8 groups so far */
	char groups[8][MAXUSER];
	int nogroups;
}updateserver;

extern updateserver mqs;

/* MQ Server update sending functions */
typedef enum MQ_MSG_TYPE {
	UPDATE_SSREPORT=1,
	UPDATE_OPSBREPORT,
} MQ_MSG_TYPE;


/* load configuration associated with this bot_setting list */
EXPORTFUNC void ModuleConfig( bot_setting *bot_settings );

/* Add a new timer callback to NeoStats */
EXPORTFUNC int AddTimer( TIMER_TYPE type, timer_handler handler, const char *name, int interval, void *userptr );
/* Delete a timer callback from NeoStats */
EXPORTFUNC int DelTimer( const char *timer_name );
/* Change timer callback interval counter */
EXPORTFUNC int SetTimerInterval( const char *timer_name, int interval );
/* Find timer from name */
EXPORTFUNC Timer *FindTimer( const char *timer_name );

EXPORTFUNC Sock *AddSock( SOCK_TYPE type, const char *sock_name, int socknum, sockfunccb readfunc, sockcb writefunc, short what, void *data, struct timeval *tv);
EXPORTFUNC int UpdateSock( Sock *sock, short what, short reset, struct timeval *tv );
EXPORTFUNC int DelSock( Sock *sock );
EXPORTFUNC Sock *FindSock( const char *sock_name );
EXPORTFUNC OS_SOCKET sock_connect( int socktype, struct in_addr ip, int port );
EXPORTFUNC Sock *add_listen_sock( const char *sock_name, const int port, int type, sockcb acceptcb, void *data );
EXPORTFUNC Sock *add_linemode_socket( const char *sock_name, OS_SOCKET socknum, sockfunccb readcb, sockcb errcb, void *arg );
EXPORTFUNC int send_to_sock( Sock *sock, const char *buf, size_t buflen );

/* Add a new bot to NeoStats */
EXPORTFUNC Bot *AddBot( BotInfo *botinfo );
/* Find bot from name */
EXPORTFUNC Bot *FindBot( const char *bot_name );

/* main.c */
EXPORTFUNC void fatal_error( char *file, int line, const char *func, char *error_text ) __attribute__( ( noreturn ) );
#define FATAL_ERROR( error_text ) fatal_error(__FILE__, __LINE__, __PRETTY_FUNCTION__, ( error_text ) ); 

/* nsmemory.c */
EXPORTFUNC void *ns_malloc( size_t size );
EXPORTFUNC void *ns_calloc( size_t size );
EXPORTFUNC void *ns_realloc( void *ptr, size_t size );
EXPORTFUNC void _ns_free( void **ptr );
#define ns_free( ptr ) _ns_free( ( void **) &( ptr ) );

/* misc.c */
EXPORTFUNC unsigned hrand( const unsigned upperbound, const unsigned lowerbound );
EXPORTFUNC void strip( char *line );
EXPORTFUNC char *sstrdup( const char *s );
EXPORTFUNC char *ns_strlwr( char *s );
EXPORTFUNC void AddStringToList( char ***List, char S[], int *C );
EXPORTFUNC void strip_mirc_codes( char *text );
EXPORTFUNC void clean_string(char *text, size_t len);
EXPORTFUNC char *sctime( time_t t );
EXPORTFUNC char *sftime( time_t t );
EXPORTFUNC char *make_safe_filename( char *name );
EXPORTFUNC char *joinbuf( char **av, int ac, int from );
EXPORTFUNC unsigned int split_buf( char *buf, char ***argv );
EXPORTFUNC unsigned int ircsplitbuf( char *buf, char ***argv, int colon_special );

/*  For use by modules to report command information channel which 
 *  takes account of neostats reporting options
 */
EXPORTFUNC void CommandReport( const Bot *botptr, const char *fmt, ... );

/* IRC interface for modules 
 *  Modules use these functions to perform actions on IRC
 *  They use a similar naming convention to the same actions as IRC commands 
 *  issued by users from an IRC client.
 */

/*  Messaging functions to send messages to users and channels
 */
EXPORTFUNC int irc_prefmsg( const Bot *botptr, const Client *target, const char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_prefmsg_list( const Bot *botptr, const Client *target, const char **text );
EXPORTFUNC int irc_privmsg( const Bot *botptr, const Client *target, const char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_privmsg_list( const Bot *botptr, const Client *target, const char **text );
EXPORTFUNC int irc_notice( const Bot *botptr, const Client *target, const char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_chanprivmsg( const Bot *botptr, const char *chan, const char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_channotice( const Bot *botptr, const char *chan, const char *fmt, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */

EXPORTFUNC int irc_dccmsgall( const char *fmt, ...) __attribute__((format(printf,1,2))); /* 1=format 2=params */


/*  Specialised messaging functions for global messages, services channel 
 *  alerts and numeric responses
 */
EXPORTFUNC int irc_chanalert( const Bot *botptr, const char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
EXPORTFUNC int irc_globops( const Bot *botptr, const char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
EXPORTFUNC int irc_wallops( const Bot *botptr, const char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
EXPORTFUNC int irc_numeric( const int numeric, const char *target, const char *data, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_smo( const char *source, const char *umodetarget, const char *msg );

/*  General irc actions for join/part channels etc
 */
EXPORTFUNC int irc_nickchange( const Bot *botptr, const char *newnick );
EXPORTFUNC int irc_quit( const Bot *botptr, const char *quitmsg );
EXPORTFUNC int irc_join( const Bot *botptr, const char *chan, const char *chanmodes );
EXPORTFUNC int irc_part( const Bot *botptr, const char *chan, const char *quitmsg );
EXPORTFUNC int irc_kick( const Bot *botptr, const char *chan, const char *target, const char *reason );
EXPORTFUNC int irc_invite( const Bot *botptr, const Client *target, const char *chan );
EXPORTFUNC int irc_topic( const Bot *botptr, const Channel *channel, const char *topic );

EXPORTFUNC int irc_cloakhost( const Bot *botptr );
EXPORTFUNC int irc_setname( const Bot *botptr, const char *realname );

/*  Mode functions
 */
EXPORTFUNC int irc_umode( const Bot *botptr, const char *target, unsigned int mode );
EXPORTFUNC int irc_cmode( const Bot *botptr, const char *chan, const char *mode, const char *args );
EXPORTFUNC int irc_chanusermode( const Bot *botptr, const char *chan, const char *mode, const char *target );

/*  Oper functions
 *  Require an opered bot to operate
 */
EXPORTFUNC int irc_kill( const Bot *botptr, const char *target, const char *reason, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_akill( const Bot *botptr, const char *host, const char *ident, const unsigned long length, const char *reason, ...)  __attribute__((format(printf,5,6))); /* 5=format 6=params */
EXPORTFUNC int irc_rakill( const Bot *botptr, const char *host, const char *ident );
EXPORTFUNC int irc_sqline( const Bot *botptr, const char *mask, const char *reason, ...)  __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_unsqline( const Bot *botptr, const char *mask );
EXPORTFUNC int irc_sgline( const Bot *botptr, const char *mask, const char *reason, ...)  __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_unsgline( const Bot *botptr, const char *mask );
EXPORTFUNC int irc_gline( const Bot *botptr, const char *mask, const char *reason, ...)  __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_remgline( const Bot *botptr, const char *mask );
EXPORTFUNC int irc_zline( const Bot *botptr, const char *mask, const char *reason, ...)  __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_unzline( const Bot *botptr, const char *mask );
EXPORTFUNC int irc_kline( const Bot *botptr, const char *mask, const char *reason, ...)  __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_unkline( const Bot *botptr, const char *mask );
EXPORTFUNC int irc_swhois( const char *target, const char *swhois );
EXPORTFUNC int irc_sethost( const Bot *botptr, const char *host );
EXPORTFUNC int irc_setident( const Bot *botptr, const char *ident );

/* Other */
EXPORTFUNC int irc_stats( const char *source, const char type, const char *target );

int irc_ping( const char *source, const char *reply, const char *to );
int irc_pong( const char *reply, const char *data );

/*  SVS functions 
 *  these operate from the server rather than a bot 
 */
EXPORTFUNC int irc_svsnick( const Bot *botptr, const Client *target, const char *newnick );
EXPORTFUNC int irc_svsjoin( const Bot *botptr, const Client *target, const char *chan );
EXPORTFUNC int irc_svspart( const Bot *botptr, const Client *target, const char *chan );
EXPORTFUNC int irc_svshost( const Bot *botptr, Client *target, const char *vhost );
EXPORTFUNC int irc_svsmode( const Bot *botptr, const Client *target, const char *modes );
EXPORTFUNC int irc_svskill( const Bot *botptr, const Client *target, const char *reason, ...) __attribute__((format(printf,3,4))); /* 3=format 4=params */
EXPORTFUNC int irc_svstime( const Bot *botptr, const Client *target, const time_t ts );

/*  CTCP functions to correctly format CTCP requests and replies
 */
EXPORTFUNC int irc_ctcp_version_req( const Bot *botptr, const Client *target );
EXPORTFUNC int irc_ctcp_version_rpl( const Bot *botptr, const Client *target, const char *version );
EXPORTFUNC int irc_ctcp_ping_req( const Bot *botptr, const Client *target );

EXPORTFUNC int irc_ctcp_finger_req( const Bot *botptr, const Client *target );

EXPORTFUNC int irc_ctcp_action_req( const Bot *botptr, const Client *target, const char *action );
EXPORTFUNC int irc_ctcp_action_req_channel( const Bot *botptr, const Channel *channel, const char *action );

EXPORTFUNC int irc_ctcp_time_req( const Bot *botptr, const Client *target );

EXPORTFUNC int irc_ctcp_unhandled_req( const Bot *botptr, const Client *target, const char *ctcp_command );
EXPORTFUNC int irc_ctcp_unhandled_rpl( const Bot *botptr, const Client *target, const char *ctcp_command, const char *ctcp_parameters );


/* bots.c */
EXPORTFUNC int GenerateBotNick( char *nickbuf, size_t stublen, int alphacount, int numcount);

/* users.c */
EXPORTFUNC Client *FindUser( const char *nick );
EXPORTFUNC int UserLevel( Client *u );

/* server.c */
EXPORTFUNC Client *FindServer( const char *name );

EXPORTFUNC Client *FindClient( const char *name );

/* chans.c */
EXPORTFUNC Channel *FindChannel( const char *chan );
EXPORTFUNC int test_cmode( const Channel *c, unsigned int mode );
EXPORTFUNC int IsChannelMember( const Channel *c, const Client *u );
EXPORTFUNC int test_cumode( const Channel *c, const Client *u, unsigned int mode );
EXPORTFUNC Channel *GetRandomChannel( void );
EXPORTFUNC Client *GetRandomChannelMember( const Channel *c, int uge );
EXPORTFUNC char *GetRandomChannelKey( int length );

#define IsChanOp( chan, nick ) test_cumode( chan, nick, CUMODE_CHANOP )
#define IsChanHalfOp( chan, nick ) test_cumode( chan, nick, CUMODE_HALFOP )
#define IsChanVoice( chan, nick ) test_cumode( chan, nick, CUMODE_VOICE )
#define IsChanOwner( chan, nick ) test_cumode( chan, nick, CUMODE_CHANOWNER )
#define IsChanProt( chan, nick ) test_cumode( chan, nick, CUMODE_CHANPROT )
#define IsChanAdmin( chan, nick ) test_cumode( chan, nick, CUMODE_CHANADMIN )

EXPORTVAR extern unsigned char UmodeChRegNick;

/* dns.c */
EXPORTFUNC int dns_lookup( char *str, adns_rrtype type, void (*callback) ( void *data, adns_answer *a), void *data );

/* services.c */
EXPORTFUNC int add_services_cmd_list( bot_cmd *bot_cmd_list );
EXPORTFUNC void del_services_cmd_list( const bot_cmd *bot_cmd_list );
EXPORTFUNC int add_services_set_list( bot_setting *bot_setting_list );
EXPORTFUNC void del_services_set_list( const bot_setting *bot_setting_list );
EXPORTFUNC int add_bot_cmd_list( Bot *bot_ptr, bot_cmd *bot_cmd_list );
EXPORTFUNC void del_bot_cmd_list( const Bot *bot_ptr, const bot_cmd *bot_cmd_list );
EXPORTFUNC int add_bot_setting_list( Bot *bot_ptr, bot_setting *bot_setting_list );
EXPORTFUNC void del_bot_setting_list( const Bot *bot_ptr, const bot_setting *bot_setting_list );

EXPORTFUNC Client *FindValidUser( const Bot *botptr, const Client *u, const char *target_nick );

/* transfer.c stuff */
typedef void (transfer_callback) ( void *data, int returncode, char *body, int bodysize );
EXPORTFUNC int new_transfer( char *url, char *params, NS_TRANSFER savetofileormemory, char *filename, void *data, transfer_callback *callback );

/* Is the client excluded */
#define IsExcluded( x ) ( ( x ) && ( ( x )->flags & NS_FLAG_EXCLUDED ) )

/* Is the client a NeoStats one? */
#define IsMe( x ) ( ( x ) && ( ( x )->flags & CLIENT_FLAG_ME ) )

/* Is the client synched? */
#define IsSynched( x ) ( ( x ) && ( ( x )->flags & CLIENT_FLAG_SYNCHED ) )

/* Mark server as synched */
#define SetServerSynched( x ) ( ( ( x )->flags |= CLIENT_FLAG_SYNCHED ) )

/* Mark Server as Unsynced */
#define SetSynching(x) ( ( ( x )->flags &= ~CLIENT_FLAG_SYNCHED ) )

/* Has NeoStats issued a SETHOST for this user? */
#define IsUserSetHosted( x )  ( ( x ) && ( ( x )->flags & CLIENT_FLAG_SETHOST ) )

/* Is the client marked away? */
#define IsAway( x ) ( ( x ) && ( x->user->is_away ) )

EXPORTFUNC int ValidateString (const char *string);
EXPORTFUNC int ValidateNick( const char *nick );
EXPORTFUNC int ValidateNickWild( const char *nick );
EXPORTFUNC int ValidateUser( const char *username );
EXPORTFUNC int ValidateUserWild( const char *username );
EXPORTFUNC int ValidateHost( const char *hostname );
EXPORTFUNC int ValidateHostWild( const char *hostname );
EXPORTFUNC int ValidateUserHost( const char *userhost );
EXPORTFUNC int ValidateUserHostWild( const char *userhost );
EXPORTFUNC int ValidateURL( const char *url );
EXPORTFUNC int ValidateChannel( const char *channel_name );
EXPORTFUNC int ValidateChannelWild( const char *channel_name );
EXPORTFUNC int ValidateChannelKey( const char *key );
EXPORTFUNC int IsJustWildcard( const char *mask, int ishostmask );

/* DBA */
#define CONFIG_TABLE_NAME	"config"

/* Row fetch handler type */
typedef int (*DBRowHandler) ( void *data, int size );

/* Row fetch handler type version 2 (returns key as well) */
typedef int (*DBRowHandler2) ( char *key, void *data, int size );

/* DB API */
EXPORTFUNC int DBAOpenDatabase( void );
EXPORTFUNC int DBACloseDatabase( void );
EXPORTFUNC int DBAOpenTable( const char *table );
EXPORTFUNC int DBACloseTable( const char *table );
EXPORTFUNC int DBAStore( const char *table, const char *key, void *data, int size );
EXPORTFUNC int DBAFetch( const char *table, const char *key, void *data, int size );
EXPORTFUNC int DBADelete( const char *table, const char * key );
EXPORTFUNC int DBAFetchRows( const char *table, DBRowHandler handler );
EXPORTFUNC int DBAFetchRows2( const char *table, DBRowHandler2 handler );
/* DB API Macros to wrap common types */
#define DBAStoreBool( table, key, data ) DBAStore( table, key, ( void * )data, sizeof ( int ) )
#define DBAStoreInt( table, key, data ) DBAStore( table, key, ( void * )data, sizeof ( int ) )
#define DBAStoreStr( table, key, data, size ) DBAStore( table, key, ( void * )data, size)
#define DBAFetchBool( table, key, data ) DBAFetch( table, key, ( void * )data, sizeof ( int ) )
#define DBAFetchInt( table, key, data ) DBAFetch( table, key, ( void * )data, sizeof ( int ) )
#define DBAFetchStr( table, key, data, size ) DBAFetch( table, key, ( void * )data, size)
/* DB API Macros to wrap common config types */
#define DBAStoreConfigBool( key, data ) DBAStoreBool(CONFIG_TABLE_NAME, key, data)
#define DBAStoreConfigInt( key, data ) DBAStoreInt(CONFIG_TABLE_NAME, key, data)
#define DBAStoreConfigStr( key, data, size ) DBAStoreStr(CONFIG_TABLE_NAME, key, data, size)
#define DBAFetchConfigBool( key, data )	DBAFetchBool(CONFIG_TABLE_NAME, key, data)
#define DBAFetchConfigInt( key, data ) DBAFetchInt(CONFIG_TABLE_NAME, key, data)
#define DBAFetchConfigStr( key, data , size) DBAFetchStr(CONFIG_TABLE_NAME, key, data, size)

/* log.c API export */
/* define the log levels */

typedef enum NS_LOG_LEVEL {
	LOG_CRITICAL=1,	/* critical crash type notices */
	LOG_ERROR,		/* something is majorly wrong */
	LOG_WARNING,	/* Hey, you should know about this type messages */
	LOG_NOTICE,		/* did you know messages */
	LOG_NORMAL,		/* our normal logging level? */
	LOG_INFO,		/* lots of info about what we are doing */
	LOG_LEVELMAX = LOG_INFO,
} NS_LOG_LEVEL;

/* define debug levels */

typedef enum NS_DEBUG_LEVEL {
	DEBUGRX = 1,
	DEBUGTX,
	DEBUG1,
	DEBUG2,
	DEBUG3,
	DEBUG4,
	DEBUG5,
	DEBUG6,
	DEBUG7,
	DEBUG8,
	DEBUG9,
	DEBUG10,
	DEBUGMAX = DEBUG10,
} NS_DEBUG_LEVEL;

/* this is for the neostats assert replacement. */
/* Version 2.4 and later of GCC define a magical variable _PRETTY_FUNCTION__'
   which contains the name of the function currently being defined.
   This is broken in G++ before version 2.6.
   C9x has a similar variable called __func__, but prefer the GCC one since
   it demangles C++ function names.  */

#define __NASSERT_FUNCTION    __PRETTY_FUNCTION__
/* Not all compilers provide __STRING so define it here if it is unknown */
#ifndef __STRING
#define __STRING(x) #x
#endif /* __STRING */ 


#ifndef __ASSERT_VOID_CAST
#define __ASSERT_VOID_CAST ( void)
#endif /* __ASSERT_VOID_CAST */
extern void nassert_fail( const char *expr, const char *file, const int line, const char *infunk );

#ifndef NDEBUG
#define nassert(expr) \
  (__ASSERT_VOID_CAST ((expr) ? 0 :                                           \
	(nassert_fail((__STRING((expr))), __FILE__, __LINE__, __NASSERT_FUNCTION), 0)))
#else /* NDEBUG */
#define nassert(expr) (__ASSERT_VOID_CAST (0))
#endif /* NDEBUG */
EXPORTFUNC void CaptureBackTrace (const char *file, const int line, const char *func);
EXPORTFUNC void nlog( NS_LOG_LEVEL level, const char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */
EXPORTFUNC void dlog( NS_DEBUG_LEVEL level, const char *fmt, ...) __attribute__((format(printf,2,3))); /* 2=format 3=params */

/* List walk handler type */
typedef int (*ChannelListHandler) ( Channel *c, void *v );
EXPORTFUNC int ProcessChannelList( const ChannelListHandler handler, void *v );
/* List walk handler type */
typedef int (*ChannelMemberListHandler) ( Channel *c, ChannelMember *m, void *v );
EXPORTFUNC int ProcessChannelMembers( Channel *c, const ChannelMemberListHandler handler, void *v );
/* List walk handler type */
typedef int (*UserListHandler) ( Client *u, void *v );
EXPORTFUNC int ProcessUserList( const UserListHandler handler, void *v );
/* List walk handler type */
typedef int (*ServerListHandler) ( Client *s, void *v );
EXPORTFUNC int ProcessServerList( const ServerListHandler handler, void *v );
/* List walk handler type */
typedef void( *ServerMapHandler )( const Client *s, int isroot, int depth, void *v );
EXPORTFUNC void ProcessServerMap( const ServerMapHandler handler, int useexclusions, void *v );
/* List walk handler type */
typedef int (*ModuleListHandler) ( Module *module_ptr, void *v );
EXPORTFUNC int ProcessModuleList( const ModuleListHandler handler, void *v );
/* List walk handler type */
typedef int (*BanListHandler) ( Ban *ban, void *v );
EXPORTFUNC int ProcessBanList( const BanListHandler handler, void *v );

EXPORTFUNC int HaveFeature( int mask );

EXPORTFUNC void AddEvent( ModuleEvent *eventptr );
EXPORTFUNC void AddEventList( ModuleEvent *eventlistptr );
EXPORTFUNC void DeleteEvent( Event event );
EXPORTFUNC void DeleteEventList( const ModuleEvent *eventlistptr );
EXPORTFUNC void SetAllEventFlags( unsigned int flag, unsigned int enable );
EXPORTFUNC void SetEventFlags( Event event, unsigned int flag, unsigned int enable );
EXPORTFUNC void EnableEvent( Event event );
EXPORTFUNC void DisableEvent( Event event );

/* String functions */
/* vs[n]printf replacements */
EXPORTFUNC int ircvsprintf( char *buf, const char *fmt, va_list args );
EXPORTFUNC int ircvsnprintf( char *buf, size_t size, const char *fmt, va_list args );
/* s[n]printf replacements */
EXPORTFUNC int ircsprintf( char *buf, const char *fmt, ...) __attribute__((format(printf,2,3)) ); /* 2=format 3=params */
EXPORTFUNC int ircsnprintf( char *buf, size_t size, const char *fmt, ...) __attribute__((format(printf,3,4)) ); /* 3=format 4=params */
/* str[n]casecmp replacements */
EXPORTFUNC int ircstrcasecmp( const char *s1, const char *s2 );
EXPORTFUNC int ircstrncasecmp( const char *s1, const char *s2, size_t size );

EXPORTFUNC int match( const char *mask, const char *name );

/* 
 *  Portability wrapper functions
 */

/* File system functions */
EXPORTFUNC int os_mkdir( const char *filename, mode_t mode );
EXPORTFUNC int os_create_dir( const char *dirname );
EXPORTFUNC FILE *os_fopen( const char *filename, const char *filemode );
EXPORTFUNC int os_fclose( FILE *handle );
EXPORTFUNC int os_fseek( FILE *handle, long offset, int origin );
EXPORTFUNC long os_ftell( FILE *handle );
EXPORTFUNC int os_fprintf( FILE *handle, const char *fmt, ...) __attribute__((format(printf,2,3)) ); /* 2=format 3=params */
EXPORTFUNC int os_fputs( const char *string, FILE *handle );
EXPORTFUNC size_t os_fread( void *buffer, size_t size, size_t count, FILE *handle );
EXPORTFUNC char *os_fgets( char *string, int n, FILE *handle );
EXPORTFUNC size_t os_fwrite( const void *buffer, size_t size, size_t count, FILE *handle );
EXPORTFUNC int os_fflush( FILE *handle );
EXPORTFUNC int os_rename( const char *oldname, const char *newname );
EXPORTFUNC int os_stat( const char *path, struct stat *buffer );
EXPORTFUNC int os_access( const char *path, int mode );
EXPORTFUNC int os_write( int fd, const void *buffer, unsigned int count );
EXPORTFUNC int os_close( int fd );
EXPORTFUNC int os_mkstemp( char *ftemplate );
EXPORTFUNC int os_write_temp_file( char *ftemplate, const void *buffer, unsigned int count );
EXPORTFUNC int os_file_get_size( const char *filename );
EXPORTFUNC int os_chmod( const char *filename, int pmode );
EXPORTVAR extern int os_file_errno;

EXPORTFUNC char *os_strerror( void );
EXPORTFUNC size_t os_strftime( char *strDest, size_t maxsize, const char *format, const struct tm *timeptr );
EXPORTFUNC struct tm* os_localtime( const time_t *timer );
/* Socket functions */
EXPORTFUNC int os_sock_close( OS_SOCKET sock );
EXPORTFUNC int os_sock_write( OS_SOCKET s, const char *buf, int len );
EXPORTFUNC int os_sock_sendto( OS_SOCKET s, const char *buf, int len, int flags, const struct sockaddr* to, int tolen );
EXPORTFUNC int os_sock_read( OS_SOCKET s, char *buf, int len );
EXPORTFUNC int os_sock_recvfrom( OS_SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen );
EXPORTFUNC int os_sock_set_nonblocking( OS_SOCKET s );
EXPORTFUNC int os_sock_connect( OS_SOCKET s, const struct sockaddr* name, int namelen );
EXPORTFUNC OS_SOCKET os_sock_socket( int socket_family, int socket_type, int protocol );
EXPORTFUNC int os_sock_bind( OS_SOCKET s, const struct sockaddr* name, int namelen );
EXPORTFUNC int os_sock_listen( OS_SOCKET s, int backlog );
EXPORTFUNC int os_sock_setsockopt( OS_SOCKET s, int level, int optname, const char *optval, int optlen );
EXPORTFUNC int os_sock_ioctl( OS_SOCKET s, int cmd, void *argp );
EXPORTVAR extern int os_sock_errno;
EXPORTFUNC char *os_sock_getlasterrorstring( void );
EXPORTFUNC char *os_sock_strerror( int sockerrno );
EXPORTFUNC int os_sock_select( int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout );

/* Memory functions */
EXPORTFUNC void *os_memset( void *dest, int c, size_t count );
EXPORTFUNC void *os_memcpy( void *dest, const void *src, size_t count );
EXPORTFUNC void *os_malloc( size_t size );
EXPORTFUNC void os_free( void *ptr );

#ifdef WIN32
#define OS_SOCK_EMSGSIZE                WSAEMSGSIZE
#define OS_SOCK_EAGAIN                  WSAEPROCLIM
#define OS_SOCK_ENOBUFS					WSAENOBUFS 
#define OS_SOCK_EWOULDBLOCK				WSAEWOULDBLOCK
#define OS_SOCK_EINPROGRESS				WSAEINPROGRESS
#define OS_SOCK_EINTR					WSAEINTR
#else /* WIN32 */
#define OS_SOCK_EMSGSIZE                EMSGSIZE
#define OS_SOCK_EAGAIN                  EAGAIN
#define OS_SOCK_ENOBUFS					ENOBUFS 
#define OS_SOCK_EWOULDBLOCK				EWOULDBLOCK
#define OS_SOCK_EINPROGRESS				EINPROGRESS
#define OS_SOCK_EINTR					EINTR
#endif /* WIN32 */

/* 
 * Module interface 
 */
/* Module basic interface */
MODULEVAR extern ModuleInfo module_info;   
MODULEFUNC int ModInit( void );
MODULEFUNC int ModSynch( void );
MODULEFUNC int ModFini( void );
/* Module event interface */
MODULEVAR extern ModuleEvent module_events[];  
/* Module auth interface */
MODULEFUNC int ModAuthUser( const Client *u );
/* Module exclude interface */
EXPORTFUNC int ModIsServerExcluded( const Client *s );
EXPORTFUNC int ModIsUserExcluded( const Client *u );
EXPORTFUNC int ModIsChannelExcluded( const Channel *c );

/* Module data pointer interface */
/* Module data pointer interface channel */
EXPORTFUNC void *AllocChannelModPtr( Channel *c, size_t size );
EXPORTFUNC void FreeChannelModPtr( Channel *c );
EXPORTFUNC void *GetChannelModPtr( const Channel *c );
/* Module data pointer interface user */
EXPORTFUNC void *AllocUserModPtr( Client *u, size_t size );
EXPORTFUNC void FreeUserModPtr( Client *u );
EXPORTFUNC void *GetUserModPtr( const Client *u );
/* Module data pointer interface server */
EXPORTFUNC void *AllocServerModPtr( Client *s, size_t size );
EXPORTFUNC void FreeServerModPtr( Client *s );
EXPORTFUNC void *GetServerModPtr( const Client *s );
/* Module data pointer interface bot */
EXPORTFUNC void *AllocBotModPtr( Bot *pBot, size_t size );
EXPORTFUNC void FreeBotModPtr( Bot *pBot );
EXPORTFUNC void *GetBotModPtr( const Bot *pBot );
/* Module data value interface */
/* Module data value interface channel */
EXPORTFUNC void ClearChannelModValue( Channel *c );
EXPORTFUNC void SetChannelModValue( Channel *c, void *data );
EXPORTFUNC void *GetChannelModValue( const Channel *c );
/* Module data value interface user */
EXPORTFUNC void ClearUserModValue( Client *u );
EXPORTFUNC void SetUserModValue( Client *u, void *data );
EXPORTFUNC void *GetUserModValue( const Client *u );
/* Module data value interface server */
EXPORTFUNC void ClearServerModValue( Client *s );
EXPORTFUNC void SetServerModValue( Client *s, void *data );
EXPORTFUNC void *GetServerModValue( const Client *s );
/* Module data value interface bot */
EXPORTFUNC void ClearBotModValue( Bot *pBot );
EXPORTFUNC void SetBotModValue( Bot *pBot, void *data );
EXPORTFUNC void *GetBotModValue( const Bot *pBot );


#ifdef BUILDINGMOD
/* these defines collide with modules that include neostats.h */
#undef PACKAGE
#undef VERSION
#undef DEBUG
#endif

#endif /* _NEOSTATS_H_ */
