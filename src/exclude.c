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
** $Id: exclude.c 3302 2008-03-03 03:50:17Z Fish $
*/

/* @file global exclusion handling functions
 */

/*  TODO:
 *  - (MAYBE) Real time exclusions??? possibly optional.
 */

#include "neostats.h"
#include "exclude.h"
#include "services.h"
#include "ircstring.h"
#include "helpstrings.h"

/* Prototype for module exclude command handler */
static int cmd_exclude( const CmdParams *cmdparams );

/* Exclude types */
typedef enum NS_EXCLUDE
{
	NS_EXCLUDE_HOST	= 0,
	NS_EXCLUDE_SERVER,
	NS_EXCLUDE_CHANNEL,
	NS_EXCLUDE_USERHOST,
	NS_EXCLUDE_LIMIT
	/* NS_EXCLUDE_MAX = ( NS_EXCLUDE_LIMIT - 1 ) */
} NS_EXCLUDE;

/* Maximum size of exclude lists */
#define MAX_EXCLUDES		100

/* Exclude struct */
typedef struct Exclude
{
	NS_EXCLUDE type;
	char pattern[USERHOSTLEN];
	char addedby[MAXNICK];
	char reason[MAXREASON];
	time_t addedon;
} Exclude;

/* List walk handler type */
typedef int (*ExcludeHandler) ( Exclude *exclude, void *v );

/* String descriptions of exclude types */
static const char *ExcludeDesc[ NS_EXCLUDE_LIMIT ] =
{
	"Host",
	"Server",
	"Channel",
	"Userhost"
};

/* Template bot exclude command struture */
static bot_cmd exclude_commands[] =
{
	{"EXCLUDE",		cmd_exclude,		1,	NS_ULEVEL_ADMIN,	ns_help_exclude, 0, NULL, NULL},
	NS_CMD_END()
};

/** @brief FindExclude
 *
 *  Searches for exclusions matching mask
 *  Exclusion sub system use only
 *
 *  @param exclude_list exclude list to search
 *  @param mask of exclude types to check
 *  @param pattern of exclude to match
 *
 *  @return exclude structure found or NULL if not found
 */

static Exclude *FindExclude( const list_t *exclude_list, NS_EXCLUDE type, const char *pattern )
{
	lnode_t *node;
	Exclude *exclude;

	SET_SEGV_LOCATION();
	if( !exclude_list )
		return NULL;
	node = list_first( exclude_list );
	while( node != NULL )
	{
		exclude = lnode_get( node );
		if( exclude->type == type )
		{
			if( match( exclude->pattern, pattern ) )
			{
				dlog( DEBUG1, "FindExclude: %s matches exclude %s", pattern, exclude->pattern );
				return exclude;
			}
		}				
		node = list_next( exclude_list, node );
	}
	return NULL;
}

/** @brief ProcessExcludeList
 *
 *  Walk through exclusion list calling handler
 *  Exclusion sub system use only
 *
 *  @param exclude_list exclude list to search
 *  @param mask of exclude types to check
 *  @param pattern of exclude to match
 *
 *  @return result of list walk passed from handler
 */

static int ProcessExcludeList( const list_t *exclude_list, ExcludeHandler handler, void *v )
{
	lnode_t *node;
	Exclude *exclude;
	int ret = 0;

	SET_SEGV_LOCATION();
	node = list_first( exclude_list );
	while( node != NULL )
	{
		exclude = lnode_get( node );
		ret = handler( exclude, v );
		if( ret != 0 )
			break;
		node = list_next( exclude_list, node );
	}
	return ret;
}

/** @brief new_exclude
 *
 *  Add an exclude to the selected exclude list
 *  Exclusion sub system use only
 *
 *  @param exclude_list exclude list to add to
 *  @param data exclude data
 *
 *  @return none
 */

static void new_exclude( list_t *exclude_list, const void *data )
{
	Exclude *exclude;

	exclude = ns_calloc( sizeof( Exclude ) );
	os_memcpy( exclude, data, sizeof( Exclude ) );
	lnode_create_append( exclude_list, exclude );
	dlog( DEBUG2, "Added exclusion %s (%d) by %s on %d", exclude->pattern, exclude->type, exclude->addedby, ( int )exclude->addedon );
}

/** @brief load_exclude
 *
 *  Table load handler
 *  Database row handler to load global exclude data
 *  Exclusion sub system use only
 *
 *  @param data pointer to table row data
 *  @param size of loaded data
 *
 *  @return NS_TRUE to abort load or NS_FALSE to continue loading
 */

static int load_exclude( void *data, int size )
{
	/* Only add exclude data of the correct struct size */
	if( size == sizeof( Exclude ) )
		new_exclude( GET_CUR_MODULE()->exclude_list, data );
	return NS_FALSE;
}

/** @brief InitExcludes
 *
 *  Initialise global exclusion system and load existing exclusions
 *  NeoStats core use only
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitExcludes( Module *mod_ptr )
{
	SET_SEGV_LOCATION();
	mod_ptr->exclude_list = list_create( MAX_EXCLUDES );
	if( !mod_ptr->exclude_list )
	{
		nlog( LOG_CRITICAL, "Unable to create exclude list" );
		return NS_FAILURE;
	}
	mod_ptr->exclude_cmd_list = ns_malloc( sizeof( exclude_commands ) );
	os_memcpy( mod_ptr->exclude_cmd_list, exclude_commands, sizeof( exclude_commands ) );
	DBAFetchRows( "exclusions", load_exclude );
	return NS_SUCCESS;
} 

/** @brief FiniExcludes
 *
 *  Finish global exclusion system
 *  NeoStats core use only
 *
 *  @param none
 *
 *  @return none
 */

void FiniExcludes( void )
{
	list_destroy_auto( GET_CUR_MODULE()->exclude_list );
}

/** @brief FiniModExcludes
 *
 *  Finish global exclusion system
 *  NeoStats core use only
 *
 *  @param mod_ptr pointer to module to initialise
 *
 *  @return none
 */

void FiniModExcludes( Module *mod_ptr )
{
	ns_free( mod_ptr->exclude_cmd_list );
	list_destroy_auto( mod_ptr->exclude_list );
}

/** @brief AddBotExcludeCommands
 *
 *  Add exclude command list to bot that uses module exclusions
 *  NeoStats core use only
 *
 *  @param botptr pointer to bot to add comands to
 *
 *  @return none
 */

void AddBotExcludeCommands( Bot *botptr )
{
	add_bot_cmd_list( botptr, GET_CUR_MODULE()->exclude_cmd_list );
}

/** @brief AddExclude
 *
 *  Add exclude to exclude list
 *  Exclude subsystem use only
 *
 *  @param exclude_list exclusion list to process
 *  @param cmdparams
 *    cmdparams->av[1] = type( one of HOST, CHANNEL, SERVER, USERHOST )
 *    cmdparams->av[2] = mask
 *    cmdparams->av[3..cmdparams->ac] = reason
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int AddExclude( list_t *exclude_list, NS_EXCLUDE type, const CmdParams *cmdparams )
{
	char *buf;
	Exclude *exclude;
	Exclude *foundexclude;

	foundexclude = FindExclude( exclude_list, type, cmdparams->av[2] );
	if( foundexclude )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, "%s already added as %s", cmdparams->av[2], foundexclude->pattern );
		return NS_SUCCESS;
	}
	exclude = ns_calloc( sizeof( Exclude ) );
	exclude->type = type;
	exclude->addedon = me.now;
	strlcpy( exclude->pattern, collapse( cmdparams->av[2] ), MAXHOST );
	strlcpy( exclude->addedby, cmdparams->source->name, MAXNICK );
	buf = joinbuf( cmdparams->av, cmdparams->ac, 3 );
	strlcpy( exclude->reason, buf, MAXREASON );
	ns_free( buf );
	/* if we get here, then exclude is valid */
	lnode_create_append( exclude_list, exclude );
	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "Added %s (%s) to exclusion list", cmdparams->source ), exclude->pattern, cmdparams->av[1] );
	if( nsconfig.cmdreport )
		irc_chanalert( cmdparams->bot, _( "%s added %s (%s) to the exclusion list" ), cmdparams->source->name, exclude->pattern, cmdparams->av[1] );
	/* now save the exclusion list */
	DBAStore( "exclusions", exclude->pattern, ( void * )exclude, sizeof( Exclude ) );
	return NS_SUCCESS;
}

/** @brief cmd_exclude_add
 *
 *  EXCLUDE ADD command handler
 *  Adds exclude to exclusion list
 *  Exclusion sub system use only
 *
 *  @param exclude_list exclusion list to process
 *  @param cmdparams
 *    cmdparams->av[1] = type( one of HOST, CHANNEL, SERVER, USERHOST )
 *    cmdparams->av[2] = mask
 *    cmdparams->av[3..cmdparams->ac] = reason
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int cmd_exclude_add( list_t *exclude_list, const CmdParams *cmdparams )
{
	if( cmdparams->ac < 4 )
		return NS_ERR_NEED_MORE_PARAMS;
	if( list_isfull( exclude_list ) )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, "Error, exclusion list is full" );
		return NS_SUCCESS;
	}
	if( ircstrcasecmp( "HOST", cmdparams->av[1] ) == 0 )
	{
		if( !ValidateHostWild( cmdparams->av[2] ) )
		{
			irc_prefmsg( cmdparams->bot, cmdparams->source, "Invalid host name" );
			return NS_SUCCESS;
		}
		return AddExclude( exclude_list, NS_EXCLUDE_HOST, cmdparams );
	} 
	if( ircstrcasecmp( "CHANNEL", cmdparams->av[1] ) == 0 )
	{
		if( !ValidateChannelWild( cmdparams->av[2]) )
		{
			irc_prefmsg( cmdparams->bot, cmdparams->source, "Invalid channel name" );
			return NS_SUCCESS;
		}
		return AddExclude( exclude_list, NS_EXCLUDE_CHANNEL, cmdparams );
	} 
	if( ircstrcasecmp( "SERVER", cmdparams->av[1] ) == 0 )
	{
		if( !ValidateHostWild( cmdparams->av[2] ) )
		{
			irc_prefmsg( cmdparams->bot, cmdparams->source, "Invalid host name" );
			return NS_SUCCESS;
		}
		return AddExclude( exclude_list, NS_EXCLUDE_SERVER, cmdparams );
	} 
	if( ircstrcasecmp( "USERHOST", cmdparams->av[1] ) == 0 )
	{
		if( !ValidateUserHostWild( cmdparams->av[2] ) )
		{
			irc_prefmsg( cmdparams->bot, cmdparams->source, "Invalid userhost mask" );
			return NS_SUCCESS;
		}
		return AddExclude( exclude_list, NS_EXCLUDE_USERHOST, cmdparams );
	} 
	irc_prefmsg( cmdparams->bot, cmdparams->source, "Invalid exclude type" );
	return NS_SUCCESS;
} 

/** @brief cmd_exclude_del
 *
 *  EXCLUDE DEL command handler
 *  Deletes exclusion from exclusion list
 *  Exclusion sub system use only
 *
 *  @param exclude_list exclusion list to process
 *  @param cmdparams
 *    cmdparams->av[1] = mask
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int cmd_exclude_del( list_t *exclude_list, const CmdParams *cmdparams ) 
{
	lnode_t *node;
	Exclude *exclude;
	
	if( cmdparams->ac < 2 )
		return NS_ERR_NEED_MORE_PARAMS;
	node = list_first( exclude_list );
	while( node != NULL )
	{
		exclude = lnode_get( node );
		if( ircstrcasecmp( exclude->pattern, cmdparams->av[1] ) == 0 )
		{
			list_delete_destroy_node( exclude_list, node );
			DBADelete( "exclusions", exclude->pattern );
			irc_prefmsg( cmdparams->bot, cmdparams->source, __( "%s delete from exclusion list",cmdparams->source ), exclude->pattern );
			ns_free( exclude );
			return NS_SUCCESS;
		}
		node = list_next( exclude_list, node );
	}
	/* if we get here, means that we never got a match */
	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "%s not found in the exclusion list",cmdparams->source ), cmdparams->av[1] );
	return NS_SUCCESS;
} 

/** @brief ReportExcludeHandler
 *
 *  Report exclusions to user
 *  Exclusion sub system use only
 *
 *  @param exclude to report
 *  @param v pointer to cmdparams
 *
 *  @return NS_FALSE to continue or NS_TRUE to quit
 */

static int ReportExcludeHandler( Exclude *exclude, void *v )
{
	CmdParams *cmdparams = ( CmdParams * )v;

	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "%s (%s) Added by %s on %s for %s", cmdparams->source ), exclude->pattern, ExcludeDesc[exclude->type], exclude->addedby, sftime( exclude->addedon ), exclude->reason );
	return NS_FALSE;
}

/** @brief cmd_exclude_list
 *
 *  EXCLUDE LIST command handler
 *  List exclusions
 *  Exclusion sub system use only
 *
 *  @param exclude_list exclusion list to process
 *  @param cmdparams
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int cmd_exclude_list( list_t *exclude_list, const CmdParams *cmdparams ) 
{
	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "Exclusion list:", cmdparams->source ) );
	ProcessExcludeList( exclude_list, ReportExcludeHandler, ( void * )cmdparams );
	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "End of list.", cmdparams->source ) );
	return NS_SUCCESS;
} 

/** @brief cmd_exclude
 *
 *  EXCLUDE command handler
 *  Manage module exclusions
 *  Exclusion sub system use only
 *
 *  @param cmdparams
 *    cmdparams->av[0] = subcommand( one of ADD, DEL, LIST )
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int cmd_exclude( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	if( ircstrcasecmp( cmdparams->av[0], "ADD" ) == 0 )
		return cmd_exclude_add( GET_CUR_MODULE()->exclude_list, cmdparams );
	if( ircstrcasecmp( cmdparams->av[0], "DEL" ) == 0 )
		return cmd_exclude_del( GET_CUR_MODULE()->exclude_list, cmdparams );
	if( ircstrcasecmp( cmdparams->av[0], "LIST" ) == 0 )
		return cmd_exclude_list( GET_CUR_MODULE()->exclude_list, cmdparams );
	return NS_ERR_SYNTAX_ERROR;
}

/** @brief ExcludeUserHandler
 *
 *  List walk handler to check if a user is excluded
 *  Exclusion sub system use only
 *
 *  @param exclude to test
 *  @param v pointer to client to test
 *
 *  @return NS_FALSE to continue or NS_TRUE to quit
 */

static int ExcludeUserHandler( Exclude *exclude, void *v )
{
	Client *u = ( Client * )v;

	switch( exclude->type )
	{
		case NS_EXCLUDE_HOST:
			if( match( exclude->pattern, u->user->hostname ) )
			{
				u->flags |= NS_FLAG_EXCLUDED;
				return NS_TRUE;
			}
			if ( match( exclude->pattern, u->hostip ) ) {
				u->flags |= NS_FLAG_EXCLUDED;
				return NS_TRUE;
			}
			break;
		case NS_EXCLUDE_USERHOST:
			if( match( exclude->pattern, u->user->userhostmask ) )
			{
				u->flags |= NS_FLAG_EXCLUDED;
				return NS_TRUE;
			}
			break;
		default:
			break;
	}
	return NS_FALSE;
}

/** @brief ns_do_exclude_user
 *
 *  Check user against global exclusion list and set appropriate flags 
 *  on user connect
 *  NeoStats core use only 
 *
 *  @param u pointer to Client struct of user to check
 *
 *  @return none
 */

void ns_do_exclude_user( Client *u ) 
{
	/* Assume not excluded until proven otherwise */
	u->flags &= ~NS_FLAG_EXCLUDED;
	/* if the server is excluded, user is excluded as well */
	if( u->uplink->flags & NS_FLAG_EXCLUDED )
	{
	 	u->flags |= NS_FLAG_EXCLUDED;
		return;
	}	
	ProcessExcludeList( ns_module.exclude_list, ExcludeUserHandler, ( void * )u );
}

/** @brief ModExcludeUserHandler
 *
 *  List walk handler to check if a user is excluded
 *  Exclusion sub system use only
 *
 *  @param exclude to test
 *  @param v pointer to client to test
 *
 *  @return NS_FALSE to continue or NS_TRUE if found
 */

static int ModExcludeUserHandler( Exclude *exclude, void *v )
{
	Client *u = ( Client * )v;

	switch( exclude->type )
	{
		case NS_EXCLUDE_SERVER:
			dlog( DEBUG4, "Testing server %s against %s", u->uplink->name, exclude->pattern );
			if( match( exclude->pattern, u->uplink->name ) )
			{
				dlog( DEBUG1, "User %s excluded by server entry %s", u->name, exclude->pattern );
				return NS_TRUE;
			}
			break;
		case NS_EXCLUDE_HOST:
			dlog( DEBUG4, "Testing host %s against %s", u->user->hostname, exclude->pattern );
			if( match( exclude->pattern, u->user->hostname ) )
			{
				dlog( DEBUG1, "User %s is excluded by host entry %s", u->name, exclude->pattern );
				return NS_TRUE;
			}
			if ( match( exclude->pattern, u->hostip ) ) {
				dlog( DEBUG1, "user %s is excluded by IP entry %s", u->name, exclude->pattern );
				return NS_TRUE;
			}
			break;
		case NS_EXCLUDE_USERHOST:
			dlog( DEBUG4, "Testing userhost %s against %s", u->user->userhostmask, exclude->pattern );
			if( match( exclude->pattern, u->user->userhostmask ) )
			{
				dlog( DEBUG1, "User %s is excluded by userhost entry %s", u->name, exclude->pattern );
				return NS_TRUE;
			}
			break;
		default:
			break;
	}
	return NS_FALSE;
}

/** @brief ModIsUserExcluded
 *
 *  Check whether user is excluded by module exclusion list
 *  Module use
 *
 *  @param u pointer to Client struct of user to check
 *
 *  @return NS_TRUE if excluded else NS_FALSE if not
 */

int ModIsUserExcluded( const Client *u ) 
{
	SET_SEGV_LOCATION();
	if( ircstrcasecmp( u->uplink->name, me.name ) == 0 )
	{
		dlog( DEBUG1, "User %s excluded as neostats or module user.", u->name );
		return NS_TRUE;
	}
	return ProcessExcludeList( GET_CUR_MODULE()->exclude_list, ModExcludeUserHandler, ( void * )u );
}

/** @brief ns_do_exclude_server
 *
 *  Check server against global exclusion list and set appropriate flags
 *  on server connect
 *  NeoStats core use only
 *
 *  @param s pointer to Client struct of server to check
 *
 *  @return none
 */

void ns_do_exclude_server( Client *s ) 
{
	Exclude *foundexclude;

	foundexclude = FindExclude( ns_module.exclude_list, NS_EXCLUDE_SERVER, s->name );
	if( foundexclude )
	{
		dlog( DEBUG1, "Excluding server %s against %s", s->name, foundexclude->pattern );
		s->flags |= NS_FLAG_EXCLUDED;
		return;
	}
	/* if we are here, there is no match */
	s->flags &= ~NS_FLAG_EXCLUDED;
}

/** @brief ns_do_exclude_chan
 *
 *  Check channel against global exclusion list and set appropriate flags
 *  on channel creation
 *  NeoStats core use only
 *
 *  @param c pointer to Channel struct of channel to check
 *
 *  @return none
 */

void ns_do_exclude_chan( Channel *c ) 
{
	Exclude *foundexclude;

	foundexclude = FindExclude( ns_module.exclude_list, NS_EXCLUDE_CHANNEL, c->name );
	if( foundexclude )
	{
		dlog( DEBUG1, "Excluding channel %s against %s", c->name, foundexclude->pattern );
		c->flags |= NS_FLAG_EXCLUDED;
		return;
	}
	/* if we are here, there is no match */
	c->flags &= ~NS_FLAG_EXCLUDED;
}

/** @brief ModIsServerExcluded
 *
 *  Check whether server is excluded by module exclusion list
 *  Module use
 *
 *  @param s pointer to Client struct of server to check
 *
 *  @return NS_TRUE if excluded else NS_FALSE if not
 */

int ModIsServerExcluded( const Client *s )
{
	Exclude *foundexclude;

	foundexclude = FindExclude( GET_CUR_MODULE()->exclude_list, NS_EXCLUDE_SERVER, s->name );
	if( foundexclude )
	{
		dlog( DEBUG1, "Excluding server %s against %s", s->name, foundexclude->pattern );
		return NS_TRUE;
	}
	return NS_FALSE;
}

/** @brief ModIsChannelExcluded
 *
 *  Check whether channel is excluded by module exclusion list
 *  Module use
 *
 *  @param u pointer to Channel struct of channel to check
 *
 *  @return NS_TRUE if excluded else NS_FALSE if not
 */

int ModIsChannelExcluded( const Channel *c ) 
{
	Exclude *foundexclude;

	SET_SEGV_LOCATION();
	if( IsServicesChannel( c ) )
	{
		dlog( DEBUG1, "Excluding services channel %s", c->name );
		return NS_TRUE;
	}
	foundexclude = FindExclude( GET_CUR_MODULE()->exclude_list, NS_EXCLUDE_CHANNEL, c->name );
	if( foundexclude )
	{
		dlog( DEBUG1, "Excluding channel %s against %s", c->name, foundexclude->pattern );
		return NS_TRUE;
	}
	return NS_FALSE;
}
