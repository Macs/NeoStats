/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2008 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
** ( at your option)any later version.
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
** $Id: extauth.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "namedvars.h"

/** ExtAuth Module
 *
 *  User authentication based on nick!user@host masking
 */

static int ea_cmd_access( const CmdParams *cmdparams );

/** Access list struct */
typedef struct AccessEntry
{
	char nick[MAXNICK];
	char mask[USERHOSTLEN];
	int level;
}AccessEntry;

nv_struct nv_extauth[] = {
	{"nick", NV_STR, offsetof(AccessEntry, nick), NV_FLG_RO, -1, MAXNICK},
	{"mask", NV_STR, offsetof(AccessEntry, mask), NV_FLG_RO, -1, USERHOSTLEN},
	{"level", NV_INT, offsetof(AccessEntry, level), NV_FLG_RO, -1, -1},
	NV_STRUCT_END()
};


/** Copyright info */
static const char *extauth_copyright[] = 
{
	"Copyright (c) 1999-2008, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** About info */
static const char *extauth_about[] = 
{
	"\2ExtAuth\2 authorises users based on an access list.",
	NULL
};

/** Help text */
const char *ea_help_access[] = 
{
	"Manage NeoStats user access list",
	"Syntax: \2ACCESS ADD <nick> <mask> <level>\2",
	"        \2ACCESS DEL <nick>\2",
	"        \2ACCESS LIST\2",
	"",
	"Manage the list of users having access to NeoStats",
	"<mask> must be of the form user@host",
	"<level> must be between 0 and 200",
	NULL
};

/** Module info */
ModuleInfo module_info = 
{
	"ExtAuth",
	"Access List Authentication Module",
	extauth_copyright,
	extauth_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	MODULE_FLAG_AUTH,
	0,
	0,
};

/** hash for storing access list */
static hash_t *accesshash;

/** Bot command table */
static bot_cmd extauth_commands[] =
{
	{"ACCESS",	ea_cmd_access,	1,	NS_ULEVEL_ROOT, ea_help_access, 0, NULL, NULL},
	NS_CMD_END()
};

/** @brief LoadAccessListEntry
 *
 *  Table load handler
 *
 *  @param pointer to table row data
 *  @param size of loaded data
 *
 *  @return none
 */

static int LoadAccessListEntry( void *data, int size )
{
	AccessEntry *access;
	
	if( size != sizeof( AccessEntry ) )
		return NS_FALSE;
	access = ns_calloc( sizeof( AccessEntry ) );
	os_memcpy( access, data, sizeof( AccessEntry ) );
	hnode_create_insert( accesshash, access, access->nick );
	return NS_FALSE;
}

/** @brief LoadAccessList
 *
 *  Load access list 
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int LoadAccessList( void )
{
	accesshash = nv_hash_create( HASHCOUNT_T_MAX, 0, 0, "ExtAuth", nv_extauth, NV_FLAGS_RO, NULL);
	if( !accesshash )
	{
		nlog( LOG_CRITICAL, "Unable to create accesslist hash" );
		return NS_FAILURE;
	}
	DBAFetchRows( "AccessList", LoadAccessListEntry );
	return NS_SUCCESS;
}

/** @brief AccessAdd
 *
 *  Access ADD sub-command handler
 *
 *  @param cmdparam struct
 *
 *  @return NS_SUCCESS if suceeds else result of command
 */

static int AccessAdd( const CmdParams *cmdparams )
{
	int level = 0;
	AccessEntry *access;
	
	SET_SEGV_LOCATION();
	dlog(DEBUG1, "Current Run Level %s", GET_CUR_MODNAME());
	if( cmdparams->ac < 3 ) 
	{
		return NS_ERR_NEED_MORE_PARAMS;
	}
	if( hash_lookup( accesshash, cmdparams->av[1] ) ) 
	{
		irc_prefmsg( NULL, cmdparams->source, "Entry for %s already exists", cmdparams->av[1] );
		return NS_SUCCESS;
	}
	if( !strstr( cmdparams->av[2], "@" ) ) 
	{
		irc_prefmsg( NULL, cmdparams->source, "Invalid format for hostmask. Must be of the form user@host." );
		return NS_ERR_SYNTAX_ERROR;
	}
	level = atoi( cmdparams->av[3] );
	if( level < 0 || level > NS_ULEVEL_ROOT) 
	{
		irc_prefmsg( NULL, cmdparams->source, "Level out of range. Valid values range from 0 to 200." );
		return NS_ERR_PARAM_OUT_OF_RANGE;
	}
	access = ns_calloc( sizeof( AccessEntry) );
	strlcpy( access->nick, cmdparams->av[1], MAXNICK );
	strlcpy( access->mask, cmdparams->av[2], USERHOSTLEN );
	access->level = level;
 	hnode_create_insert( accesshash, access, access->nick );
	/* save the entry */
	DBAStore( "AccessList", access->nick, ( void *)access, sizeof( AccessEntry) );
	irc_prefmsg( NULL, cmdparams->source, "Successfully added %s for host %s with level %d to access list", access->nick, access->mask, access->level );
	return NS_SUCCESS;
}

/** @brief AccessDel
 *
 *  Access DEL sub-command handler
 *
 *  @param cmdparam struct
 *
 *  @return NS_SUCCESS if suceeds else result of command
 */

static int AccessDel( const CmdParams *cmdparams )
{
	hnode_t *node;

	SET_SEGV_LOCATION();
	if( cmdparams->ac < 1 ) 
	{
		return NS_ERR_SYNTAX_ERROR;
	}
	node = hash_lookup( accesshash, cmdparams->av[1] );
	if( node ) 
	{
		AccessEntry *access = ( AccessEntry * )hnode_get( node );
		hash_delete_destroy_node( accesshash, node );
		ns_free( access );
		DBADelete( "AccessList", cmdparams->av[1] );
		irc_prefmsg( NULL, cmdparams->source, "Deleted %s from access list", cmdparams->av[1] );
	} 
	else 
	{
		irc_prefmsg( NULL, cmdparams->source, "Error, %s not found in access list.", cmdparams->av[1] );
	}
	return NS_SUCCESS;
}

/** @brief AccessList
 *
 *  Access LIST command handler
 *
 *  @param cmdparam struct
 *
 *  @return NS_SUCCESS if suceeds else result of command
 */

static int AccessList( const CmdParams *cmdparams )
{
	hscan_t accessscan;
	hnode_t *node;
	AccessEntry *access;

	SET_SEGV_LOCATION();	
	irc_prefmsg( NULL, cmdparams->source, "Access List (%d):", ( int )hash_count( accesshash ) );
	hash_scan_begin( &accessscan, accesshash );
	while( ( node = hash_scan_next( &accessscan ) ) != NULL) 
	{
		access = hnode_get( node );
		irc_prefmsg( NULL, cmdparams->source, "%s %s (%d)", access->nick, access->mask, access->level );
	}
	irc_prefmsg( NULL, cmdparams->source, "End of list." );
	return NS_SUCCESS;
}

/** @brief ea_cmd_access
 *
 *  Access command handler
 *
 *  @param cmdparam struct
 *
 *  @return NS_SUCCESS if suceeds else result of command
 */

static int ea_cmd_access( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	if( ircstrcasecmp( cmdparams->av[0], "ADD" ) == 0 )
		return AccessAdd( cmdparams );
	if( ircstrcasecmp( cmdparams->av[0], "DEL" ) == 0 )
		return AccessDel( cmdparams );
	if( ircstrcasecmp( cmdparams->av[0], "LIST" ) == 0 )
		return AccessList( cmdparams );
	return NS_ERR_SYNTAX_ERROR;
}

/** @brief ModInit
 *
 *  Init handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit( void )
{
	if( LoadAccessList() != NS_SUCCESS )
		return NS_FAILURE;
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch( void )
{
	if( add_services_cmd_list( extauth_commands ) != NS_SUCCESS ) 
	{
		return NS_FAILURE;
	}
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
	hscan_t accessscan;
	hnode_t *node;
	AccessEntry *access;

	del_services_cmd_list( extauth_commands );


	hash_scan_begin( &accessscan, accesshash );
	while( ( node = hash_scan_next( &accessscan ) ) != NULL) 
	{
		access = hnode_get( node );
		ns_free (access);
		hash_scan_delete_destroy_node(accesshash, node);
	}
	hash_destroy(accesshash);

	return NS_SUCCESS;
}

/** @brief ModAuthUser
 *
 *  Lookup authentication level for user
 *
 *  @param pointer to user
 *
 *  @return authentication level for user
 */

int ModAuthUser( const Client *u )
{
	static char hostmask[USERHOSTLEN];
	AccessEntry *access;

	dlog( DEBUG2, "ModAuthUser for %s", u->name );
	access = ( AccessEntry *)hnode_find( accesshash, u->name );
	if( access) 
	{
		ircsnprintf( hostmask, USERHOSTLEN, "%s@%s", u->user->username, u->user->hostname );
		if( match( access->mask, hostmask ) ) 
		{
			return access->level;		
		}
	}		
	return 0;
}
