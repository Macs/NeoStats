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
** $Id: hostserv.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "hostserv.h"
#include "namedvars.h"

#define PAGESIZE	20

/** vhost entry struct */
typedef struct vhostentry
{
	char nick[MAXNICK];
	char host[MAXHOST];
	char vhost[MAXHOST];
	char passwd[MAXPASS];
	char added[MAXNICK];
	time_t tslastused;
} vhostentry;

/** ban entry struct */
typedef struct banentry
{
	char host[MAXHOST];
	char who[MAXNICK];
	char reason[MAXREASON];
} banentry;

/** config struct */
static struct hs_cfg
{
	char vhostdom[MAXHOST];
	int expire;
	int regnick;
	int operhosts;
	int verbose;
	int addlevel;
} hs_cfg;

nv_struct nv_hostserv[] = {
	{ "nick", NV_STR, offsetof(vhostentry, nick), NV_FLG_RO, -1, MAXNICK},
	{ "host", NV_STR, offsetof(vhostentry, host), 0, -1, MAXHOST},
	{ "vhost", NV_STR, offsetof(vhostentry, vhost), 0, -1, MAXHOST},
	{ "tslastused", NV_INT, offsetof(vhostentry, tslastused), NV_FLG_RO, -1, -1},
	{ "passwd", NV_STR, offsetof(vhostentry, passwd), 0, -1, MAXPASS},
	{ "added", NV_STR, offsetof(vhostentry, added), NV_FLG_RO, -1, MAXNICK}, 
	NV_STRUCT_END()
};

nv_struct nv_hostservban[] = {
	{"host", NV_STR, offsetof(banentry, host), 0, -1, MAXHOST},
	{"who", NV_STR, offsetof(banentry, who), 0, -1, MAXNICK},
	{"reason", NV_STR, offsetof(banentry, reason), 0, -1, MAXREASON},
	NV_STRUCT_END()
};

/** prototypes */
static int hs_event_signon( const CmdParams *cmdparams );
static int hs_event_umode( const CmdParams *cmdparams );

static int hs_cmd_bans( const CmdParams *cmdparams );
static int hs_cmd_login( const CmdParams *cmdparams );
static int hs_cmd_chpass( const CmdParams *cmdparams );
static int hs_cmd_add( const CmdParams *cmdparams );
static int hs_cmd_list( const CmdParams *cmdparams );
static int hs_cmd_listwild( const CmdParams *cmdparams );
static int hs_cmd_view( const CmdParams *cmdparams );
static int hs_cmd_del( const CmdParams *cmdparams );
static int hs_check_vhost( const CmdParams *cmdparams, const vhostentry *vhe);

static int hs_set_regnick_cb( const CmdParams* cmdparams, SET_REASON reason );
static int hs_set_expire_cb( const CmdParams* cmdparams, SET_REASON reason );

/** vhost list */
static list_t *vhost_list;
/** bans list */
static hash_t *banhash;

/** Bot pointer */
static Bot *hs_bot;

/** Copyright info */
static const char *hs_copyright[] =
{
	"Copyright (c) 1999-2008, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo module_info =
{
	"HostServ",
	"Network virtual host service",
	hs_copyright,
	hs_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	MODULE_FLAG_LOCAL_EXCLUDES,
	0,
	FEATURE_SVSHOST,
};

/** Bot command table */
static bot_cmd hs_commands[] =
{
	{"ADD",		hs_cmd_add,		4,	NS_ULEVEL_LOCOPER,	hs_help_add,		0,	NULL,	NULL},
	{"DEL",		hs_cmd_del,		1,	NS_ULEVEL_LOCOPER,	hs_help_del,		0,	NULL,	NULL},
	{"LIST",	hs_cmd_list,		0,	NS_ULEVEL_LOCOPER,	hs_help_list,		0,	NULL,	NULL},
	{"LISTWILD",	hs_cmd_listwild,	2,	NS_ULEVEL_LOCOPER,	hs_help_listwild,	0,	NULL,	NULL},
	{"BANS",	hs_cmd_bans,		1,	NS_ULEVEL_ADMIN,	hs_help_bans,		0,	NULL,	NULL},
	{"VIEW",	hs_cmd_view,		1,	NS_ULEVEL_OPER,		hs_help_view,		0,	NULL,	NULL},
	{"LOGIN",	hs_cmd_login,		2,	0,			hs_help_login,		0,	NULL,	NULL},
	{"CHPASS",	hs_cmd_chpass,		3,	0,			hs_help_chpass,		0,	NULL,	NULL},
	NS_CMD_END()
};

/** Bot setting table */
static bot_setting hs_settings[] =
{
	{"EXPIRE",	&hs_cfg.expire,		SET_TYPE_INT,		0, 99, 		NS_ULEVEL_ADMIN, "days",hs_help_set_expire,	hs_set_expire_cb,	( void* )TS_ONE_MINUTE	},
	{"HIDDENHOST",	&hs_cfg.regnick,	SET_TYPE_BOOLEAN,	0, 0, 		NS_ULEVEL_ADMIN, NULL,	hs_help_set_hiddenhost, hs_set_regnick_cb,	( void* )0	},
	{"HOSTNAME",	hs_cfg.vhostdom,	SET_TYPE_STRING,	0, MAXHOST,	NS_ULEVEL_ADMIN, NULL,	hs_help_set_hostname,	NULL,			( void* )""	},
	{"OPERHOSTS",	&hs_cfg.operhosts,	SET_TYPE_BOOLEAN,	0, 0, 		NS_ULEVEL_ADMIN, NULL,	hs_help_set_operhosts,	NULL,			( void* )0	},
	{"VERBOSE",	&hs_cfg.verbose,	SET_TYPE_BOOLEAN,	0, 0, 		NS_ULEVEL_ADMIN, NULL,	hs_help_set_verbose,	NULL,			( void* )1	},
	{"ADDLEVEL",	&hs_cfg.addlevel,	SET_TYPE_INT,		0, 200, 		NS_ULEVEL_ADMIN, NULL,	hs_help_set_addlevel,	NULL,			( void* )NS_ULEVEL_LOCOPER },
	NS_SETTING_END()
};

/** BotInfo */
static BotInfo hs_botinfo = 
{
	"HostServ", 
	"HostServ1", 
	"HS", 
	BOT_COMMON_HOST, 
	"Network virtual host service",
	BOT_FLAG_ROOT|BOT_FLAG_DEAF, 
	hs_commands, 
	hs_settings,
};

/** Module Events */
ModuleEvent module_events[] =
{
	{EVENT_SIGNON,	hs_event_signon,	EVENT_FLAG_EXCLUDE_ME | EVENT_FLAG_USE_EXCLUDE},
	{EVENT_UMODE,	hs_event_umode,		EVENT_FLAG_EXCLUDE_ME | EVENT_FLAG_USE_EXCLUDE}, 
	NS_EVENT_END()
};

/** @brief findnick
 *
 *  list sorting helper
 *
 *  @param key1
 *  @param key2
 *
 *  @return results of strcmp
 */

static int findnick( const void *key1, const void *key2 )
{
	const vhostentry *vhost = key1;
	return( ircstrcasecmp( vhost->nick, ( char * )key2 ) );
}

/** @brief DelVhost
 *
 *  Delete a vhost entry
 *
 *  @param pointer to vhost entry to delete
 *
 *  @return none
 */

static void DelVhost( vhostentry *vhost ) 
{
	DBADelete( "vhosts", vhost->nick );
	ns_free( vhost );
}

/** @brief ExpireOldHosts
 *
 *  Timer function to expire old hosts
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

int ExpireOldHosts( void *userptr )
{
	lnode_t *hn, *hn2;
	vhostentry *vhe;

	SET_SEGV_LOCATION();
	hn = list_first( vhost_list );
	while( hn != NULL )
	{
		hn2 = list_next( vhost_list, hn );
		vhe = lnode_get( hn );
		if( vhe->tslastused < ( me.now -( hs_cfg.expire * TS_ONE_DAY ) ) )
		{
			nlog( LOG_NOTICE, "Expiring old vhost: %s for %s", vhe->vhost, vhe->nick );
			DelVhost( vhe );
			list_delete_destroy_node( vhost_list, hn );
		}
		hn = hn2;
	}
	return NS_SUCCESS;
}

/** @brief LoadVhost
 *
 *  Table load handler
 *
 *  @param data pointer to table row data
 *  @param size of loaded data
 *
 *  @return NS_TRUE to abort load or NS_FALSE to continue loading
 */

static int LoadVhost( void *data, int size )
{
	vhostentry *vhe;

	if( size != sizeof( vhostentry ) )
		return NS_FALSE;
	vhe = ns_calloc( sizeof( vhostentry ) );
	os_memcpy( vhe, data, sizeof( vhostentry ) );
	lnode_create_append( vhost_list, vhe );
	return NS_FALSE;
}

/** @brief LoadVhosts
 *
 *  Load vhosts
 *
 *  @param none
 *
 *  @return none
 */

static void LoadVhosts( void )
{
	DBAFetchRows( "vhosts", LoadVhost );
	list_sort( vhost_list, findnick );
}

/** @brief SaveVhost
 *
 *  Save a vhost entry
 *
 *  @param pointer to vhost entry to save
 *
 *  @return none
 */

static void SaveVhost( vhostentry *vhe ) 
{
	vhe->tslastused = me.now;
	DBAStore( "vhosts", vhe->nick, ( void * )vhe, sizeof( vhostentry ) );
	list_sort( vhost_list, findnick );
}

/** @brief SaveBan
 *
 *  Save banned vhost
 *
 *  @param pointer to ban to save
 *
 *  @return none
 */

static void SaveBan( banentry *ban )
{
	DBAStore( "bans", ban->host, ( void * )ban, sizeof( banentry ) );
}

/** @brief LoadBan
 *
 *  Load banned vhost
 *
 *  @param data pointer to table row data
 *  @param size of loaded data
 *
 *  @return NS_TRUE to abort load or NS_FALSE to continue loading
 */

static int LoadBan( void *data, int size )
{
	banentry *ban;

	if( size != sizeof( banentry ) )
		return NS_FALSE;
	ban = ns_calloc( sizeof( banentry ) );
	os_memcpy( ban, data, sizeof( banentry ) );
	hnode_create_insert( banhash, ban, ban->host );
	return NS_FALSE;
}

/** @brief LoadBans
 *
 *  Load banned vhosts
 *
 *  @param none
 *
 *  @return none
 */

static void LoadBans( void )
{
	DBAFetchRows( "bans", LoadBan );
}

/** @brief hs_set_regnick_cb
 *
 *  SET REGNICK callback
 *
 *  @param cmdparams
 *  @param reason
 *
 *  @return NS_SUCCESS 
 */

static int hs_set_regnick_cb( const CmdParams* cmdparams, SET_REASON reason )
{
	if( reason == SET_LOAD || reason == SET_CHANGE )
	{
		if( hs_cfg.regnick )
		{
			EnableEvent( EVENT_UMODE );
		}
		else
		{
			DisableEvent( EVENT_UMODE );
		}
	}
	return NS_SUCCESS;
}

/** @brief hs_set_expire_cb
 *
 *  SET EXPIRE callback
 *
 *  @param cmdparams
 *  @param reason
 *
 *  @return NS_SUCCESS 
 */

static int hs_set_expire_cb( const CmdParams* cmdparams, SET_REASON reason )
{
	if( reason == SET_CHANGE )
	{
		if( hs_cfg.expire )
		{
			AddTimer( TIMER_TYPE_INTERVAL, ExpireOldHosts, "ExpireOldHosts", 7200, NULL );
		}
		else
		{
			DelTimer( "ExpireOldHosts" );
		}
	}
	return NS_SUCCESS;
}

/** @brief hs_event_signon
 *
 *  Event handler for signon to automatically set a user's host
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_event_signon( const CmdParams *cmdparams )
{
	vhostentry *vhe;

	SET_SEGV_LOCATION();
	/* Check HostName Against Data Contained in vhosts.data */
	vhe = lnode_find( vhost_list, cmdparams->source->name, findnick );
	if( vhe )
	{
		dlog( DEBUG1, "Checking %s against %s", vhe->host, cmdparams->source->user->hostname );
		if( match( vhe->host, cmdparams->source->user->hostname ) )
		{
			irc_svshost( hs_bot, cmdparams->source, vhe->vhost );
			irc_prefmsg( hs_bot, cmdparams->source, 
				"Automatically setting your hidden host to %s", vhe->vhost );
			SaveVhost( vhe );
		}
	}
	return NS_SUCCESS;
}

/** @brief hs_event_umode
 *
 *  Event handler for umode to set a user's host on regnick
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_event_umode( const CmdParams *cmdparams ) 
{
	static char vhost[MAXHOST];
	int add = 0;
	char *modes;

	SET_SEGV_LOCATION();
	if( IsOper( cmdparams->source ) && hs_cfg.operhosts == 0 ) 
		return NS_SUCCESS;
	/* first, find if its a regnick mode */
	modes = cmdparams->param;
	while( *modes != '\0' )
	{
		switch( *modes )
		{
			case '+':
				add = 1;
				break;
			case '-':
				add = 0;
				break;
			default:
				if( *modes == UmodeChRegNick )
				{
					if( add )
					{
						if( IsUserSetHosted( cmdparams->source ) )
						{
							dlog( DEBUG2, "not setting hidden host on %s since they already have a vhost set", cmdparams->source->name );
							return NS_FAILURE;
						}
						dlog( DEBUG2, "Regnick Mode on %s", cmdparams->source->name );
						ircsnprintf( vhost, MAXHOST, "%s.%s", cmdparams->source->name, hs_cfg.vhostdom );
						irc_svshost( hs_bot, cmdparams->source, vhost );
						irc_prefmsg( hs_bot, cmdparams->source, "Setting your host to %s", vhost );
						if( hs_cfg.verbose )
						{
							irc_chanalert( hs_bot, "\2VHOST\2 registered nick %s now using vhost %s", 
								cmdparams->source->name, vhost );
						}

					}
				}
				break;
		}
		modes++;
	}
	return NS_SUCCESS;
}

/* XXX Still more work todo */
int hs_nv_check(nv_item *item, nv_write_action action) {
	vhostentry *vhe, *vhe1;
	if (action == NV_ACTION_DEL) {
		vhe = ( vhostentry * ) lnode_get( item->node.lnode);
		CommandReport( hs_bot, "Removed vhost %s for %s",
			vhe->vhost, vhe->nick );
		DelVhost( vhe );
		list_delete_destroy_node( vhost_list, item->node.lnode );
		return NS_SUCCESS;
	} else if (action == NV_ACTION_ADD) {
		vhe = ns_malloc(sizeof(vhostentry));
		strlcpy(vhe->nick, item->fields[nv_get_field_item(item, "nick")]->values.v_char, MAXNICK);
		strlcpy(vhe->host, item->fields[nv_get_field_item(item, "host")]->values.v_char, MAXHOST);
		strlcpy(vhe->vhost, item->fields[nv_get_field_item(item, "vhost")]->values.v_char, MAXHOST);
		strlcpy(vhe->passwd, item->fields[nv_get_field_item(item, "passwd")]->values.v_char, MAXPASS);
		strlcpy(vhe->added, item->fields[nv_get_field_item(item, "added")]->values.v_char, MAXNICK);
		if (hs_check_vhost(NULL, vhe) == NS_FAILURE) {
			ns_free(vhe);
			return NS_FAILURE;
		}
		if( list_find( vhost_list, vhe->nick, findnick ) )
		{
			CommandReport( hs_bot, "%s already has a vhost entry", vhe->nick);
			ns_free(vhe);
			return NS_FAILURE;
		}	
		lnode_create_prepend( vhost_list, vhe );
		SaveVhost( vhe );
	} else if (action == NV_ACTION_MOD) {
		vhe = lnode_get(item->node.lnode);	
		/* copy the vhost entry incase we have to fall back */
		vhe1 = ns_malloc(sizeof(vhostentry));
		os_memcpy(vhe1, vhe, sizeof(vhostentry));
		strlcpy(vhe1->nick, item->fields[nv_get_field_item(item, "nick")]->values.v_char, MAXNICK);
		strlcpy(vhe1->host, item->fields[nv_get_field_item(item, "host")]->values.v_char, MAXHOST);
		strlcpy(vhe1->vhost, item->fields[nv_get_field_item(item, "vhost")]->values.v_char, MAXHOST);
		strlcpy(vhe1->passwd, item->fields[nv_get_field_item(item, "passwd")]->values.v_char, MAXPASS);
		strlcpy(vhe1->added, item->fields[nv_get_field_item(item, "added")]->values.v_char, MAXNICK);
		vhe1->tslastused = item->fields[nv_get_field_item(item, "tslastused")]->values.v_int;
		if (hs_check_vhost(NULL, vhe1) == NS_FAILURE) {
			ns_free(vhe1);
			return NS_FAILURE;
		}
		/* if its here, its ok. Remove the old entry and insert the new one */
		DelVhost( vhe );
		list_delete_destroy_node( vhost_list, item->node.lnode );
		lnode_create_append( vhost_list, vhe1 );
		SaveVhost( vhe1 );
		return NS_SUCCESS;
	}		
	return NS_SUCCESS;
}

/** @brief ModInit
 *
 *  Init handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds else NS_FAILURE
 */

int ModInit( void )
{
	SET_SEGV_LOCATION();
	vhost_list = nv_list_create( LISTCOUNT_T_MAX, "HostServ", nv_hostserv, NV_FLAGS_NONE, hs_nv_check);
	if( !vhost_list )
	{
		nlog( LOG_CRITICAL, "Unable to create vhost list" );
		return NS_FAILURE;
	}
	/* XXX TODO: RO for now */
	banhash = nv_hash_create( HASHCOUNT_T_MAX, 0, 0, "HostServ-Bans", nv_hostservban, NV_FLAGS_RO, NULL);
	if( !banhash )
	{
		nlog( LOG_CRITICAL, "Unable to create ban hash" );
		return NS_FAILURE;
	}
	ModuleConfig( hs_settings );
	LoadBans();
	LoadVhosts();
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *  Introduce bot onto network
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds else NS_FAILURE
 */

int ModSynch( void )
{
	SET_SEGV_LOCATION();
	hs_bot = AddBot( &hs_botinfo );
	if( !hs_bot )
		return NS_FAILURE;
	if( hs_cfg.expire )
		AddTimer( TIMER_TYPE_INTERVAL, ExpireOldHosts, "ExpireOldHosts", 7200, NULL );
	if( !HaveUmodeRegNick() ) 
		DisableEvent( EVENT_UMODE );
	return NS_SUCCESS;
}

/** @brief ModFini
 *
 *  Fini handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModFini( void )
{
	banentry *ban;
	hnode_t *hn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, banhash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL )
	{
		ban = ( ( banentry * )hnode_get( hn ) );
		hash_scan_delete_destroy_node( banhash, hn );
		ns_free( ban );
	}
	hash_destroy( banhash );
	list_destroy_auto( vhost_list );
	return NS_SUCCESS;
}

/** @brief hs_cmd_bans_list
 *
 *  Command handler for BANS LIST
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_bans_list( const CmdParams *cmdparams )
{
	banentry *ban;
	hnode_t *hn;
	hscan_t hs;
	int index = 1;

	SET_SEGV_LOCATION();
	if( hash_count( banhash ) == 0 )
	{
		irc_prefmsg( hs_bot, cmdparams->source, "No bans are defined." );
		return NS_SUCCESS;
	}
	hash_scan_begin( &hs, banhash );
	irc_prefmsg( hs_bot, cmdparams->source, "Banned vhosts" );
	while( ( hn = hash_scan_next( &hs ) ) != NULL )
	{
		ban = ( ( banentry * )hnode_get( hn ) );
		irc_prefmsg( hs_bot, cmdparams->source, "%d - %s added by %s for %s", index, ban->host, ban->who, ban->reason );
		index++;
	}
	irc_prefmsg( hs_bot, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief hs_cmd_bans_add
 *
 *  Command handler for BANS ADD
 *
 *  @param cmdparams
 *    cmdparams->av[1] = ban host mask
 *    cmdparams->av[2 - cmdparams->ac-1] = reason
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_bans_add( const CmdParams *cmdparams )
{
	banentry *ban;
	char *buf;

	SET_SEGV_LOCATION();
	if( cmdparams->ac < 3 )
		return NS_ERR_NEED_MORE_PARAMS;
	if( hash_lookup( banhash, cmdparams->av[1] ) != NULL )
	{
		irc_prefmsg( hs_bot, cmdparams->source, 
			"%s already exists in the banned vhost list", cmdparams->av[1] );
		return NS_SUCCESS;
	}
	ban = ns_calloc( sizeof( banentry ) );
	strlcpy( ban->host, cmdparams->av[1], MAXHOST );
	strlcpy( ban->who, cmdparams->source->name, MAXNICK );
	buf = joinbuf( cmdparams->av, cmdparams->ac, 2 );
	strlcpy( ban->reason, buf, MAXREASON );
	ns_free( buf );

	hnode_create_insert( banhash, ban, ban->host );
	irc_prefmsg( hs_bot, cmdparams->source, 
		"%s added to the banned vhosts list", cmdparams->av[1] );
	CommandReport( hs_bot, "%s added %s to the banned vhosts list",
		  cmdparams->source->name, cmdparams->av[1] );
	SaveBan( ban );
	return NS_SUCCESS;
}

/** @brief hs_cmd_bans_del
 *
 *  Command handler for BANS DEL
 *
 *  @param cmdparams
 *    cmdparams->av[1] = ban host mask
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_bans_del( const CmdParams *cmdparams )
{
	banentry *ban;
	hnode_t *hn;
	hscan_t hs;

	SET_SEGV_LOCATION();
	if( cmdparams->ac < 2 )
		return NS_ERR_NEED_MORE_PARAMS;
	hash_scan_begin( &hs, banhash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL )
	{
		ban = ( banentry * )hnode_get( hn );
		if( ircstrcasecmp( ban->host, cmdparams->av[1] ) == 0 )
		{
			irc_prefmsg( hs_bot, cmdparams->source, 
				"Deleted %s from the banned vhost list", cmdparams->av[1] );
			CommandReport( hs_bot, "%s deleted %s from the banned vhost list",
				cmdparams->source->name, cmdparams->av[1] );
			hash_scan_delete_destroy_node( banhash, hn );
			DBADelete( "bans", ban->host );
			ns_free( ban );
			return NS_SUCCESS;
		}
	}
	irc_prefmsg( hs_bot, cmdparams->source, "No entry for %s", cmdparams->av[1] );
	return NS_SUCCESS;
}

/** @brief hs_cmd_bans
 *
 *  Command handler for BANS
 *
 *  @param cmdparams
 *    cmdparams->av[0] = sub command
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_bans( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	if( ircstrcasecmp( cmdparams->av[0], "LIST" ) == 0 )
		return hs_cmd_bans_list( cmdparams );
	if( ircstrcasecmp( cmdparams->av[0], "ADD" ) == 0 )
		return hs_cmd_bans_add( cmdparams );
	if( ircstrcasecmp( cmdparams->av[0], "DEL" ) == 0 )
		return hs_cmd_bans_del( cmdparams );
	return NS_ERR_SYNTAX_ERROR;
}

/** @brief FindBan
 *
 *  Find ban in list of bans
 *
 *  @param mask to find
 *
 *  @return pointer to ban else NULL if not found
 */

static banentry *FindBan( const char *mask )
{
	hnode_t *hn;
	hscan_t hs;
	banentry *ban;

	hash_scan_begin( &hs, banhash );
	while( ( hn = hash_scan_next( &hs ) ) != NULL )
	{
		ban = ( banentry * ) hnode_get( hn );
		if( match( ban->host, mask ) )
		{
			return ban;
		}
	}
	return NULL;
}

/** @brief hs_cmd_chpass
 *
 *  Command handler for CHPASS
 *
 *  @param cmdparams
 *    cmdparams->av[0] = login
 *    cmdparams->av[1] = old password
 *    cmdparams->av[2] = new password
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_chpass( const CmdParams *cmdparams )
{
	vhostentry *vhe;

	SET_SEGV_LOCATION();
	vhe = lnode_find( vhost_list, cmdparams->av[0], findnick );
	if( !vhe )
	{
		irc_prefmsg( hs_bot, cmdparams->source, "No vhost for that user." );
		irc_chanalert( hs_bot, "%s tried to change the password for %s, but there is no user with that name",
			cmdparams->source->name, cmdparams->av[0] );
		nlog( LOG_WARNING, "%s tried to change the password for %s, but there is no user with that name",
			cmdparams->source->name, cmdparams->av[0] );
		return NS_SUCCESS;
	}
	if( ( match( vhe->host, cmdparams->source->user->hostname ) )
		||( UserLevel( cmdparams->source ) >= 100 ) )
	{
		if( ircstrcasecmp( vhe->passwd, cmdparams->av[1] ) == 0 )
		{
			strlcpy( vhe->passwd, cmdparams->av[2], MAXPASS );
			irc_prefmsg( hs_bot, cmdparams->source, "Password changed" );
			CommandReport( hs_bot, "%s changed the password for %s",
					cmdparams->source->name, vhe->nick );
			SaveVhost( vhe );
		}
		return NS_SUCCESS;
	}
	irc_prefmsg( hs_bot, cmdparams->source, "Error, hostname mismatch" );
	irc_chanalert( hs_bot, "%s tried to change the password for %s, but the hosts do not match (%s -> %s)",
			cmdparams->source->name, vhe->nick, cmdparams->source->user->hostname, vhe->host );
	nlog( LOG_WARNING, "%s tried to change the password for %s but the hosts do not match (%s -> %s)",
			cmdparams->source->name, vhe->nick, cmdparams->source->user->hostname, vhe->host );
	return NS_SUCCESS;
}

/** @brief hs_check_vhost
 *
 *  Check the proposed vhost is valid 
 *
 *  @param cmdparams
 *    cmdparams = if the request came from a user, this is it, otherwise NULL
 *    vhe = the requested vhost
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */


static int hs_check_vhost( const CmdParams *cmdparams, const vhostentry *vhe)
{
	banentry *ban;
	ban = FindBan( vhe->vhost);
	if( ban )
	{
		if (cmdparams) {
			irc_prefmsg( hs_bot, cmdparams->source, 
				"%s has been matched against the vhost ban %s",
				vhe->vhost, ban->host );
			CommandReport( hs_bot, "%s tried to add a banned vhost %s",
			  	cmdparams->source->name, vhe->vhost);
			return NS_FAILURE;
		} else {
			CommandReport(hs_bot, "Attempt to add a banned vhost %s", vhe->vhost);
			return NS_FAILURE;
		}
	}
	if( IsJustWildcard( vhe->host, 1 ) == NS_TRUE )
	{
		if (cmdparams) 
			irc_prefmsg( hs_bot, cmdparams->source, "%s is too general a wildcard for realhost", vhe->host);
		else 
			CommandReport( hs_bot, "%s is too general a wildcard for realhost", vhe->host);
			
		return NS_FAILURE;
	}
	if( ValidateHostWild( vhe->vhost ) == NS_FAILURE )
	{
		if (cmdparams) 
			irc_prefmsg( hs_bot, cmdparams->source, 
				"%s is an invalid host", vhe->vhost );
		else 
			CommandReport( hs_bot, "%s is a invalid host", vhe->vhost);
			
		return NS_FAILURE;
		
	}
	return NS_SUCCESS;
}


/** @brief hs_cmd_add
 *
 *  Command handler for ADD
 *
 *  @param cmdparams
 *    cmdparams->av[0] = nick
 *    cmdparams->av[1] = real host mask
 *    cmdparams->av[2] = vhost
 *    cmdparams->av[3] = password
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */
static int hs_cmd_add( const CmdParams *cmdparams )
{
	Client *u;
	vhostentry *vhe;
	
	SET_SEGV_LOCATION();
	if (cmdparams->source->user->ulevel < hs_cfg.addlevel && ircstrcasecmp(cmdparams->source->name, cmdparams->av[0]))
	{
		irc_prefmsg( hs_bot, cmdparams->source, "VHOST may be added for current nick only (%s)", cmdparams->source->name );
		return NS_SUCCESS;
	}
	vhe = ns_malloc(sizeof(vhostentry));
	strlcpy(vhe->nick, cmdparams->av[0], MAXNICK);
	strlcpy(vhe->host, cmdparams->av[1], MAXHOST);
	strlcpy(vhe->vhost, cmdparams->av[2], MAXHOST);
	strlcpy(vhe->passwd, cmdparams->av[3], MAXPASS);
	strlcpy(vhe->added, cmdparams->source->name, MAXNICK);
	if (hs_check_vhost(cmdparams, vhe) == NS_FAILURE) {
		ns_free(vhe);
		return NS_FAILURE;
	}
	if( list_find( vhost_list, vhe->nick, findnick ) )
	{
		irc_prefmsg( hs_bot, cmdparams->source, "%s already has a vhost entry", vhe->nick);
		ns_free(vhe);
		return NS_FAILURE;
		
	}

	lnode_create_append( vhost_list, vhe );
	SaveVhost( vhe );
	irc_prefmsg( hs_bot, cmdparams->source, 
		"%s has successfully been registered under realhost: %s vhost: %s and password: %s",
		cmdparams->av[0], cmdparams->av[1], cmdparams->av[2], cmdparams->av[3] );
	CommandReport( hs_bot, "%s added a vhost for %s with realhost %s vhost %s",
	    cmdparams->source->name, cmdparams->av[0], cmdparams->av[1], cmdparams->av[2] );
	/* Apply hostname if user online */
	u = FindUser( cmdparams->av[0] );
	if( u && !IsMe( u ) )
	{
		if( match( cmdparams->av[1], u->user->hostname ) )
		{
			irc_svshost( hs_bot, u, cmdparams->av[2] );
			irc_prefmsg( hs_bot, cmdparams->source, 
				"%s is online now, setting vhost to %s",
				cmdparams->av[0], cmdparams->av[2] );
			irc_prefmsg( hs_bot, u, 
				"Your vhost has been created with hostmask of %s and username %s with password %s",
				cmdparams->av[1], cmdparams->av[0], cmdparams->av[3] );
			if( u != cmdparams->source )
				irc_prefmsg( hs_bot, u, 
					"For security, you should change your vhost password. See /msg %s help chpass",
					hs_bot->name );
		}
	}
	return NS_SUCCESS;
}

/** @brief hs_cmd_list
 *
 *  Command handler for LIST
 *
 *  @param cmdparams
 *    cmdparams->av[0] = Optional Number to start display after
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_list( const CmdParams *cmdparams )
{
	lnode_t *hn;
	vhostentry *vhe;
	listcount_t start = 0;
	listcount_t index;
	listcount_t vhostcount;

	SET_SEGV_LOCATION();
	vhostcount = list_count( vhost_list );
	if( vhostcount == 0 )
	{
		irc_prefmsg( hs_bot, cmdparams->source, "No vhosts are defined." );
		return NS_SUCCESS;
	}
	if( cmdparams->ac == 2 )
	{
		if( ircstrcasecmp(cmdparams->av[0], "nick") == 0 
			|| ircstrcasecmp(cmdparams->av[0], "host") == 0 
			|| ircstrcasecmp(cmdparams->av[0], "vhost") == 0 )
		{
			return hs_cmd_listwild(cmdparams);
		}
	}
	if( cmdparams->ac == 1 )
	{
		start = atoi( cmdparams->av[0] );
	}
	if( start >= vhostcount )
	{
		irc_prefmsg( hs_bot, cmdparams->source, "Value out of range. There are only %d entries", ( int )vhostcount );
		return NS_SUCCESS;
	}		
	index = 1;
	irc_prefmsg( hs_bot, cmdparams->source, "Current vhost list: " );
	irc_prefmsg( hs_bot, cmdparams->source, "Showing %ld to %ld entries of %d vhosts", start + 1, start + PAGESIZE, ( int )vhostcount );
	irc_prefmsg( hs_bot, cmdparams->source, "%-5s %-12s %-30s", "Num", "Nick", "Vhost" );
	hn = list_first( vhost_list );
	while( hn != NULL )
	{
		if( index <= start )
		{
			index++;
			hn = list_next( vhost_list, hn );
			continue;
		}
		vhe = lnode_get( hn );
		irc_prefmsg( hs_bot, cmdparams->source, "%-5d %-12s %-30s", (int)index,
			vhe->nick, vhe->vhost );
		index++;
		/* limit to PAGESIZE entries per screen */
		if( index > start + PAGESIZE ) 
			break;
		hn = list_next( vhost_list, hn );
	}
	irc_prefmsg( hs_bot, cmdparams->source, 
		"For detailed information on a vhost use /msg %s VIEW <nick>",
		hs_bot->name );
	irc_prefmsg( hs_bot, cmdparams->source, "End of list." );
	if( vhostcount >= index )
	{
		irc_prefmsg( hs_bot, cmdparams->source, "Type \2/msg %s LIST %ld\2 to see next %d", hs_bot->name, index - 1, PAGESIZE );
	}
	return NS_SUCCESS;
}

/** @brief hs_cmd_listwild
 *
 *  Command handler for LISTWILD
 *
 *  @param cmdparams
 *    cmdparams->av[0] = one of NICK | HOST | VHOST
 *    cmdparams->av[1] = wildcard match
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_listwild( const CmdParams *cmdparams )
{
	lnode_t *hn;
	vhostentry *vhe;
	listcount_t vhostcount;
	listcount_t index;
	listcount_t start;

	SET_SEGV_LOCATION();
	vhostcount = list_count( vhost_list );
	if( vhostcount == 0 )
	{
		irc_prefmsg( hs_bot, cmdparams->source, "No vhosts are defined." );
		return NS_SUCCESS;
	}
	if( ircstrcasecmp( cmdparams->av[1], "*" ) == 0 )
	{
		irc_prefmsg( hs_bot, cmdparams->source, "%s wildcard too broad, refine wildcard limit (%s).", cmdparams->av[0], cmdparams->av[1] );
		return NS_SUCCESS;
	}
	index = 1;
	start = 0;
	irc_prefmsg( hs_bot, cmdparams->source, "Current vhost list: " );
	irc_prefmsg( hs_bot, cmdparams->source, "Showing entries matching %s of %s", cmdparams->av[0], cmdparams->av[1]);
	irc_prefmsg( hs_bot, cmdparams->source, "%-5s %-12s %-30s", "Num", "Nick", "Vhost" );
	hn = list_first( vhost_list );
	while( hn != NULL )
	{
		vhe = lnode_get( hn );
		if ( ( ircstrcasecmp(cmdparams->av[0], "nick") == 0 && match(cmdparams->av[1], vhe->nick)) || (ircstrcasecmp(cmdparams->av[0], "host") == 0 && match(cmdparams->av[1], vhe->host)) || (ircstrcasecmp(cmdparams->av[0], "vhost") == 0 && match(cmdparams->av[1], vhe->vhost)) )
		{
			start++;
			/* limit to PAGESIZE entries per screen */
			if ( start > PAGESIZE )
				break;
			irc_prefmsg( hs_bot, cmdparams->source, "%-5d %-12s %-30s", (int)index, vhe->nick, vhe->vhost );
		}
		index++;
		hn = list_next( vhost_list, hn );
	}
	irc_prefmsg( hs_bot, cmdparams->source, 
		"For detailed information on a vhost use /msg %s VIEW <nick>",
		hs_bot->name );
	irc_prefmsg( hs_bot, cmdparams->source, "End of list." );
	if( start > PAGESIZE )
	{
		irc_prefmsg( hs_bot, cmdparams->source, "Not all matching entries shown, refine match to limit display");
	}
	return NS_SUCCESS;
}

/** @brief hs_cmd_view
 *
 *  Command handler for VIEW
 *
 *  @param cmdparams
 *    cmdparams->av[0] = login to view
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_view( const CmdParams *cmdparams )
{
	vhostentry *vhe;
	char ltime[80];

	SET_SEGV_LOCATION();
	vhe = lnode_find( vhost_list, cmdparams->av[0], findnick );
	if( !vhe )
	{
		irc_prefmsg( hs_bot, cmdparams->source, "No vhost for user %s", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	irc_prefmsg( hs_bot, cmdparams->source, "Vhost information:" );
	irc_prefmsg( hs_bot, cmdparams->source, "Nick:      %s", vhe->nick );
	irc_prefmsg( hs_bot, cmdparams->source, "Real host: %s", vhe->host );
	irc_prefmsg( hs_bot, cmdparams->source, "Vhost:     %s", vhe->vhost );
	irc_prefmsg( hs_bot, cmdparams->source, "Password:  %s", vhe->passwd );
	irc_prefmsg( hs_bot, cmdparams->source, "Added by:  %s",
		vhe->added ? vhe->added : "<unknown>" );
	strftime( ltime, 80, "%d/%m/%Y[%H:%M]", localtime( &vhe->tslastused ) );
	irc_prefmsg( hs_bot, cmdparams->source, "Last used: %s", ltime );
	irc_prefmsg( hs_bot, cmdparams->source, "--- End of information ---" );
	return NS_SUCCESS;
}

/** @brief hs_cmd_del
 *
 *  Command handler for DEL
 *    cmdparams->av[0] = login to delete
 *
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_del( const CmdParams *cmdparams )
{
	lnode_t *hn;
	vhostentry *vhe;

	SET_SEGV_LOCATION();
	hn = list_find( vhost_list, cmdparams->av[0], findnick );
	if( !hn )
	{
		irc_prefmsg( hs_bot, cmdparams->source, "No vhost for user %s", cmdparams->av[0] );
		return NS_SUCCESS;
	}
	vhe = ( vhostentry * ) lnode_get( hn );
	irc_prefmsg( hs_bot, cmdparams->source, "removed vhost %s for %s",
		vhe->nick, vhe->vhost );
	CommandReport( hs_bot, "%s removed vhost %s for %s",
			cmdparams->source->name, vhe->vhost, vhe->nick );
	DelVhost( vhe );
	list_delete_destroy_node( vhost_list, hn );
	return NS_SUCCESS;
}

/** @brief hs_cmd_login
 *
 *  Command handler for LOGIN
 *
 *  @param cmdparams
 *    cmdparams->av[0] = login
 *    cmdparams->av[1] = password
 *
 *  @return NS_SUCCESS if succeeds, else NS_FAILURE
 */

static int hs_cmd_login( const CmdParams *cmdparams )
{
	vhostentry *vhe;

	SET_SEGV_LOCATION();
	/* Check HostName Against Data Contained in vhosts.data */
	vhe = lnode_find( vhost_list, cmdparams->av[0], findnick );
	if( vhe )
	{
		if( ircstrcasecmp( vhe->passwd, cmdparams->av[1] ) == 0 )
		{
			irc_svshost( hs_bot, cmdparams->source, vhe->vhost );
			irc_prefmsg( hs_bot, cmdparams->source, 
				"Your vhost has been set to %s", vhe->vhost );
			nlog( LOG_NORMAL, "%s used LOGIN to obtain vhost of %s",
			    cmdparams->source->name, vhe->vhost );
			if( hs_cfg.verbose )
			{
				irc_chanalert( hs_bot, "\2VHOST\2 %s login to vhost %s", 
					cmdparams->source->name, vhe->vhost );
			}
			SaveVhost( vhe );
			return NS_SUCCESS;
		}
	}
	irc_prefmsg( hs_bot, cmdparams->source, 
		"Incorrect login or password. Do you have a vhost added?" );
	return NS_SUCCESS;
}
