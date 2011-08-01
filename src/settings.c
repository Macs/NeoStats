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
** $Id: settings.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "services.h"
#include "commands.h"
#include "settings.h"

typedef int( *bot_cmd_set_handler )( const CmdParams *cmdparams, const bot_setting *set_ptr );

static int bot_cmd_set_boolean( const CmdParams *cmdparams, const bot_setting *set_ptr );
static int bot_cmd_set_int( const CmdParams *cmdparams, const bot_setting *set_ptr );
static int bot_cmd_set_string( const CmdParams *cmdparams, const bot_setting *set_ptr );
static int bot_cmd_set_channel( const CmdParams *cmdparams, const bot_setting *set_ptr );
static int bot_cmd_set_msg( const CmdParams *cmdparams, const bot_setting *set_ptr );
static int bot_cmd_set_nick( const CmdParams *cmdparams, const bot_setting *set_ptr );
static int bot_cmd_set_user( const CmdParams *cmdparams, const bot_setting *set_ptr );
static int bot_cmd_set_host( const CmdParams *cmdparams, const bot_setting *set_ptr );
static int bot_cmd_set_realname( const CmdParams *cmdparams, const bot_setting *set_ptr );
static int bot_cmd_set_ipv4( const CmdParams *cmdparams, const bot_setting *set_ptr );
static int bot_cmd_set_custom( const CmdParams *cmdparams, const bot_setting *set_ptr );

static bot_cmd_set_handler bot_cmd_set_handlers[] = 
{
	bot_cmd_set_boolean,
	bot_cmd_set_int,
	bot_cmd_set_string,
	bot_cmd_set_msg,
	bot_cmd_set_nick,
	bot_cmd_set_user,
	bot_cmd_set_host,
	bot_cmd_set_realname,
	bot_cmd_set_channel,
	bot_cmd_set_ipv4,
	bot_cmd_set_custom,
};

/** @brief bot_cmd_set_list
 *
 *  Process SET LIST command
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_cmd_set_list( const CmdParams *cmdparams )
{
	hnode_t *setnode;
	hscan_t hs;
	bot_setting *set_ptr;
	int userlevel;

	irc_prefmsg( cmdparams->bot, cmdparams->source, __( "Current %s settings:", cmdparams->source ), cmdparams->bot->name );
	userlevel = getuserlevel( cmdparams );
	hash_scan_begin( &hs, cmdparams->bot->botsettings );
	while( ( setnode = hash_scan_next( &hs ) ) != NULL )
	{
		set_ptr = hnode_get( setnode );
		/* Only list authorised SETTINGS */
		if( userlevel >= set_ptr->ulevel )
		{
			switch( set_ptr->type )
			{
				case SET_TYPE_BOOLEAN:
					irc_prefmsg( cmdparams->bot, cmdparams->source, "%s: %s",
						set_ptr->option, *( int* )set_ptr->varptr ? __( "Enabled", cmdparams->source ) : __( "Disabled", cmdparams->source ) );
					break;
				case SET_TYPE_INT:
					if( set_ptr->desc )
					{
						irc_prefmsg( cmdparams->bot, cmdparams->source, "%s: %d %s",
							set_ptr->option, *( int* )set_ptr->varptr, set_ptr->desc );
					}
					else
					{
						irc_prefmsg( cmdparams->bot, cmdparams->source, "%s: %d",
							set_ptr->option, *( int* )set_ptr->varptr );
					}
					break;				
				case SET_TYPE_MSG:
				case SET_TYPE_STRING:
				case SET_TYPE_NICK:
				case SET_TYPE_USER:
				case SET_TYPE_HOST:
				case SET_TYPE_REALNAME:
				case SET_TYPE_IPV4:	
				case SET_TYPE_CHANNEL:							
					irc_prefmsg( cmdparams->bot, cmdparams->source, "%s: %s",
						set_ptr->option, ( char * )set_ptr->varptr );
					break;
				case SET_TYPE_CUSTOM:
					if( set_ptr->handler )
					{
						( void )set_ptr->handler( cmdparams, SET_LIST );
					}
					break;
				default:
					irc_prefmsg( cmdparams->bot, cmdparams->source, __( "%s: uses an unsupported type", cmdparams->source ),
						set_ptr->option );
					break;
			}
		}
		set_ptr++;
	}
	return NS_SUCCESS;
}

/** @brief bot_cmd_set_report
 *
 *  Report successful SET command
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @set_ptr pointer to setting struct
 *  @new_setting string with new value
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_cmd_set_report( const CmdParams *cmdparams, const bot_setting *set_ptr, char *new_setting )
{
	if( nsconfig.cmdreport )
	{
		irc_chanalert( cmdparams->bot, _( "%s set to %s by \2%s\2" ), 
			set_ptr->option, new_setting, cmdparams->source->name );
	}
	nlog( LOG_NORMAL, "%s!%s@%s set %s to %s", 
		cmdparams->source->name, cmdparams->source->user->username, cmdparams->source->user->hostname, set_ptr->option, new_setting );
	irc_prefmsg( cmdparams->bot, cmdparams->source, 
		__( "%s set to %s", cmdparams->source ), set_ptr->option, new_setting );
	return NS_SUCCESS;
} 

/** @brief bot_cmd_set helper functions
 *  validate the pamater based on type and perform appropriate action
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

/** @brief bot_cmd_set_boolean
 *
 *  SET handler for boolean
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @set_ptr pointer to setting struct
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_cmd_set_boolean( const CmdParams *cmdparams, const bot_setting *set_ptr )
{
	int newsetting;

	if( ircstrcasecmp( cmdparams->av[1], "ON" ) == 0 )
	{
		newsetting = 1;
	} 
	else if( ircstrcasecmp( cmdparams->av[1], "OFF" ) == 0 )
	{
		newsetting = 0;
	}
	else
	{
		msg_syntax_error( cmdparams );
		return NS_ERR_SYNTAX_ERROR;
	}
	/* Module specific handling */
	if( set_ptr->handler )
	{
		if( set_ptr->handler( cmdparams, SET_VALIDATE ) != NS_SUCCESS )
			return NS_FAILURE;
	}
	*( int* )set_ptr->varptr = newsetting;
	DBAStoreConfigBool( set_ptr->option, set_ptr->varptr );
	bot_cmd_set_report( cmdparams, set_ptr, cmdparams->av[1] );
	return NS_SUCCESS;
}

/** @brief bot_cmd_set_int
 *
 *  SET handler for int
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @set_ptr pointer to setting struct
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_cmd_set_int( const CmdParams *cmdparams, const bot_setting *set_ptr )
{
	int intval;

	intval = atoi( cmdparams->av[1] );	
	/* atoi will return 0 for a string instead of a digit so check it! */
	if( intval == 0 && ( ircstrcasecmp( cmdparams->av[1], "0" ) != 0 ) )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, 
			__( "%s invalid setting for %s", cmdparams->source ), cmdparams->av[1], set_ptr->option );
		irc_prefmsg( cmdparams->bot, cmdparams->source, 
			__( "Valid values are %d to %d", cmdparams->source ), set_ptr->min, set_ptr->max );
		return NS_ERR_SYNTAX_ERROR;
	}
	/* Check limits */
	if( ( set_ptr->min != -1 && intval < set_ptr->min ) ||( set_ptr->max != -1 && intval > set_ptr->max ) )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, 
			__( "%d out of range for %s", cmdparams->source ), intval, set_ptr->option );
		irc_prefmsg( cmdparams->bot, cmdparams->source, 
			__( "Valid values are %d to %d", cmdparams->source ), set_ptr->min, set_ptr->max );
		return NS_ERR_SYNTAX_ERROR;
	}
	/* Module specific handling */
	if( set_ptr->handler )
	{
		if( set_ptr->handler( cmdparams, SET_VALIDATE ) != NS_SUCCESS )
			return NS_FAILURE;
	}
	/* Set the new value */
	*( int* )set_ptr->varptr = intval;
	DBAStoreConfigInt( set_ptr->option, set_ptr->varptr );
	bot_cmd_set_report( cmdparams, set_ptr, cmdparams->av[1] );
	return NS_SUCCESS;
}

/** @brief bot_cmd_set_string
 *
 *  SET handler for string
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @set_ptr pointer to setting struct
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_cmd_set_string( const CmdParams *cmdparams, const bot_setting *set_ptr )
{
	/* Module specific handling */
	if( set_ptr->handler )
	{
		if( set_ptr->handler( cmdparams, SET_VALIDATE ) != NS_SUCCESS )
			return NS_FAILURE;
	}
	strlcpy( ( char * )set_ptr->varptr, cmdparams->av[1], set_ptr->max );
	DBAStoreConfigStr( set_ptr->option, cmdparams->av[1], set_ptr->max );
	bot_cmd_set_report( cmdparams, set_ptr, cmdparams->av[1] );
	return NS_SUCCESS;
}

/** @brief bot_cmd_set_channel
 *
 *  SET handler for channel
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @set_ptr pointer to setting struct
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_cmd_set_channel( const CmdParams *cmdparams, const bot_setting *set_ptr )
{
	if( ValidateChannel( cmdparams->av[1] ) == NS_FAILURE )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, 
			__( "%s contains invalid characters", cmdparams->source ), cmdparams->av[1] );
		return NS_ERR_SYNTAX_ERROR;
	}
	/* Module specific handling */
	if( set_ptr->handler )
	{
		if( set_ptr->handler( cmdparams, SET_VALIDATE ) != NS_SUCCESS )
			return NS_FAILURE;
	}
	strlcpy( ( char * )set_ptr->varptr, cmdparams->av[1], set_ptr->max );
	DBAStoreConfigStr( set_ptr->option, cmdparams->av[1], set_ptr->max );
	bot_cmd_set_report( cmdparams, set_ptr, cmdparams->av[1] );
	return NS_SUCCESS;
}

/** @brief bot_cmd_set_msg
 *
 *  SET handler for msg
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @set_ptr pointer to setting struct
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_cmd_set_msg( const CmdParams *cmdparams, const bot_setting *set_ptr )
{
	char *buf;

	buf = joinbuf( cmdparams->av, cmdparams->ac, 1 );
	strlcpy( ( char * )set_ptr->varptr, buf, set_ptr->max );
	DBAStoreConfigStr( set_ptr->option, buf, set_ptr->max );
	bot_cmd_set_report( cmdparams, set_ptr, buf );
	ns_free( buf );
	return NS_SUCCESS;
}

/** @brief bot_cmd_set_nick
 *
 *  SET handler for nick
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @set_ptr pointer to setting struct
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_cmd_set_nick( const CmdParams *cmdparams, const bot_setting *set_ptr )
{
	if( ValidateNick( cmdparams->av[1] ) == NS_FAILURE )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, 
			__( "%s contains invalid characters", cmdparams->source ), cmdparams->av[1] );
		return NS_ERR_SYNTAX_ERROR;
	}
	strlcpy( ( char * )set_ptr->varptr, cmdparams->av[1], set_ptr->max );
	DBAStoreConfigStr( set_ptr->option, cmdparams->av[1], set_ptr->max );
	bot_cmd_set_report( cmdparams, set_ptr, cmdparams->av[1] );
	return NS_SUCCESS;
}

/** @brief bot_cmd_set_user
 *
 *  SET handler for user
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @set_ptr pointer to setting struct
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_cmd_set_user( const CmdParams *cmdparams, const bot_setting *set_ptr )
{
	if( ValidateUser( cmdparams->av[1] ) == NS_FAILURE )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, 
			__( "%s contains invalid characters", cmdparams->source ), cmdparams->av[1] );
		return NS_ERR_SYNTAX_ERROR;
	}
	strlcpy( ( char * )set_ptr->varptr, cmdparams->av[1], set_ptr->max );
	DBAStoreConfigStr( set_ptr->option, cmdparams->av[1], set_ptr->max );
	bot_cmd_set_report( cmdparams, set_ptr, cmdparams->av[1] );
	return NS_SUCCESS;
}

/** @brief bot_cmd_set_host
 *
 *  SET handler for host
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @set_ptr pointer to setting struct
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_cmd_set_host( const CmdParams *cmdparams, const bot_setting *set_ptr )
{
	if( !strchr( cmdparams->av[1], '.' ) )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, 
			__( "%s is an invalid hostname", cmdparams->source ), cmdparams->av[1] );
		return NS_ERR_SYNTAX_ERROR;
	}
	if( ValidateHost( cmdparams->av[1] ) == NS_FAILURE )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, 
			__( "%s contains invalid characters", cmdparams->source ), cmdparams->av[1] );
		return NS_ERR_SYNTAX_ERROR;
	}
	strlcpy( ( char * )set_ptr->varptr, cmdparams->av[1], set_ptr->max );
	DBAStoreConfigStr( set_ptr->option, cmdparams->av[1], set_ptr->max );
	bot_cmd_set_report( cmdparams, set_ptr, cmdparams->av[1] );
	return NS_SUCCESS;
}

/** @brief bot_cmd_set_realname
 *
 *  SET handler for realname
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @set_ptr pointer to setting struct
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_cmd_set_realname( const CmdParams *cmdparams, const bot_setting *set_ptr )
{
	char *buf;

	buf = joinbuf( cmdparams->av, cmdparams->ac, 1 );
	strlcpy( ( char * )set_ptr->varptr, buf, set_ptr->max );
	DBAStoreConfigStr( set_ptr->option, buf, set_ptr->max );
	bot_cmd_set_report( cmdparams, set_ptr, buf );
	ns_free( buf );
	return NS_SUCCESS;
}

/** @brief bot_cmd_set_ipv4
 *
 *  SET handler for ipv4
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @set_ptr pointer to setting struct
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_cmd_set_ipv4( const CmdParams *cmdparams, const bot_setting *set_ptr )
{
	if( !inet_addr( cmdparams->av[1] ) )
	{
		irc_prefmsg( cmdparams->bot, cmdparams->source, 
			__( "Invalid IPV4 format. Should be dotted quad, e.g. 1.2.3.4", cmdparams->source ) );
		return NS_ERR_SYNTAX_ERROR;
	}
	strlcpy( ( char * )set_ptr->varptr, cmdparams->av[1], set_ptr->max );
	DBAStoreConfigStr( set_ptr->option, cmdparams->av[1], set_ptr->max );
	bot_cmd_set_report( cmdparams, set_ptr, cmdparams->av[1] );
	return NS_SUCCESS;
}

/** @brief bot_cmd_set_custom
 *
 *  SET handler for custom
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @set_ptr pointer to setting struct
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int bot_cmd_set_custom( const CmdParams *cmdparams, const bot_setting *set_ptr )
{
	if( set_ptr->handler )
	{
		return set_ptr->handler( cmdparams, SET_CHANGE );
	}
	return NS_FAILURE;
}

/** @brief bot_cmd_set
 *
 *  SET handler
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int bot_cmd_set( const CmdParams *cmdparams )
{
	bot_cmd_set_handler set_handler;
	bot_setting *set_ptr;
	int userlevel;

	if( cmdparams->ac < 1 )
	{
		return NS_ERR_SYNTAX_ERROR;
	} 
	userlevel = getuserlevel( cmdparams );
	if( userlevel < cmdparams->bot->set_ulevel )
	{
		msg_permission_denied( cmdparams, NULL );
		return NS_ERR_NO_PERMISSION;
	}
	if( ircstrcasecmp( cmdparams->av[0], "LIST" ) == 0 )
	{
		bot_cmd_set_list( cmdparams );
		return NS_SUCCESS;
	}
	if( cmdparams->ac < 2 )
	{
		return NS_ERR_SYNTAX_ERROR;
	} 
	set_ptr = ( bot_setting *)hnode_find( cmdparams->bot->botsettings, cmdparams->av[0] );
	if( set_ptr )
	{
		if( userlevel < set_ptr->ulevel )
		{
			msg_permission_denied( cmdparams, cmdparams->av[0] );
			return NS_ERR_NO_PERMISSION;
		}
		set_handler = bot_cmd_set_handlers[set_ptr->type];
		if( set_handler( cmdparams, set_ptr ) != NS_SUCCESS )
		{
			return NS_FAILURE;
		}
		/* Call back after SET so that a module can "react" to a change in a setting */
		if( set_ptr->type != SET_TYPE_CUSTOM )
		{
			if( set_ptr->handler )
			{
				( void )set_ptr->handler( cmdparams, SET_CHANGE );
			}
		}
		return NS_SUCCESS;
	}
	irc_prefmsg( cmdparams->bot, cmdparams->source, 
		__( "Unknown set option. /msg %s HELP SET for more info", cmdparams->source ),
		cmdparams->bot->name );
	return NS_ERR_UNKNOWN_OPTION;
}

/** @brief add_bot_setting
 *
 *  Add a single set option
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int add_bot_setting( hash_t *set_hash, bot_setting *set_ptr ) 
{
	hnode_create_insert( set_hash, set_ptr, set_ptr->option );
	dlog( DEBUG3, "add_bot_setting: added a new set option %s", set_ptr->option );
	return NS_SUCCESS;
}

/** @brief del_bot_setting 
 *
 *  delete a single set option
 *  SET subsystem use only.
 *
 *  @params cmdparams pointer to commands param struct
 *  @set_ptr pointer to setting struct
 *
 *  @return none
 */

static void del_bot_setting( hash_t *set_hash, const bot_setting *set_ptr ) 
{
	hnode_t *setnode;
	
	setnode = hash_lookup( set_hash, set_ptr->option );
	if( setnode )
	{
		hash_delete_destroy_node( set_hash, setnode );
	}
}

/** @brief add_bot_setting_list
 *
 *  adds a list of set options
 *  SET subsystem use only.
 *
 *  @bot_ptr pointer to bot
 *  @set_ptr pointer to setting struct
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int add_bot_setting_list( Bot *bot_ptr, bot_setting *set_ptr ) 
{
	if( !set_ptr )
	{
		return NS_FAILURE;
	}
	/* If no hash create */
	if( bot_ptr->botsettings == NULL )
	{
		bot_ptr->botsettings = hash_create( HASHCOUNT_T_MAX, 0, 0 );
		if( !bot_ptr->botsettings )
		{
			nlog( LOG_CRITICAL, "Unable to create botsettings hash" );
			return NS_FAILURE;
		}
	}
	/* Default SET to ROOT only */
	bot_ptr->set_ulevel = NS_ULEVEL_ROOT;
	/* Now calculate minimum defined user level */
	while( set_ptr->option != NULL )
	{
		if( set_ptr->ulevel < bot_ptr->set_ulevel )
		{
			bot_ptr->set_ulevel = set_ptr->ulevel;
		}
		add_bot_setting( bot_ptr->botsettings, set_ptr );
		set_ptr++;
	}
	return NS_SUCCESS;
}

/** @brief del_bot_setting_list
 *
 *  delete a list of set options
 *  SET subsystem use only.
 *
 *  @bot_ptr pointer to bot
 *  @set_ptr pointer to setting struct
 *
 *  @return none
 */

void del_bot_setting_list( const Bot *bot_ptr, const bot_setting *set_ptr ) 
{
	/* If no bot pointer return failure */
	if( bot_ptr != NULL && bot_ptr->botsettings != NULL )
	{
		/* Cycle through command list and delete them */
		while( set_ptr->option != NULL )
		{
			del_bot_setting( bot_ptr->botsettings, set_ptr );
			set_ptr++;
		}
	}
}

/** @brief del_all_bot_settings
 *
 *  delete all settings
 *  SET subsystem use only.
 *
 *  @bot_ptr pointer to bot
 *
 *  @return none
 */

void del_all_bot_settings( Bot *bot_ptr )
{
	hnode_t *setnode;
	hscan_t hs;

	/* Check we have a command hash */
	if( bot_ptr->botsettings != NULL )
	{
		/* Cycle through command hash and delete each command */
		hash_scan_begin( &hs, bot_ptr->botsettings );
		while( ( setnode = hash_scan_next( &hs ) ) != NULL )
		{
			hash_scan_delete_destroy_node( bot_ptr->botsettings, setnode );
		}
		/* Destroy command */
		hash_destroy( bot_ptr->botsettings );
		bot_ptr->botsettings = NULL;
	}
}

/** @brief add_services_set_list
 *
 *  add a list of set options to the main neostats bot
 *  SET subsystem use only.
 *
 *  @bot_setting_list pointer to setting list
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int add_services_set_list( bot_setting *bot_setting_list )
{
	return add_bot_setting_list( ns_botptr, bot_setting_list );
}

/** @brief del_services_set_list
 *
 *  delete a list of set options from the main neostats bot
 *  SET subsystem use only.
 *
 *  @bot_setting_list pointer to setting list
 *
 *  @return none
 */

void del_services_set_list( const bot_setting *bot_setting_list )
{
	del_bot_setting_list( ns_botptr, bot_setting_list );
}
