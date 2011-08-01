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
**  Foundation, Inc., 59 Temple+++ Place, Suite 330, Boston, MA  02111-1307
**  USA
**
** NeoStats CVS Identification
** $Id: auth.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "services.h"
#include "dl.h"
#include "helpstrings.h"

/** Auth subsystem
 *
 *  Handle user authentication and communication with authentication
 *  modules. 
 */

/** This will hard code root access to a nick agreed with the 
 *  development team and should only ever be enabled on 
 *  request by us on the networks of our testers to help 
 *  fix bugs in NeoStats where there is no other way to provide
 *  the necessary access.
 *  For security reasons, a nick will be agreed between us and
 *  the test network so at no point will this be a known nick to
 *  other users.
 *  Do not enable unless you are asked to by the development team.
 *  See function AuthUser for where this is used.
*/
#ifdef DEBUG
/* #define CODERHACK "WeWillTellYouWhatToPutHere" */
#endif /* DEBUG */

/** Struct for auth list walk */
typedef struct ModuleAuthInfo
{
	int auth;
	const Client *u;
} ModuleAuthInfo;

/** Command prototypes */
static int cmd_level( const CmdParams *cmdparams );

/** Auth command table */
static bot_cmd auth_command_list[] =
{
	{"LEVEL", cmd_level, 0, 0, ns_help_level, 0, NULL, NULL},
	NS_CMD_END()
};

/** @brief IsServiceRoot
 *
 *  Is user the master Service Root?
 *  Auth subsystem use only.
 *
 *  @param u pointer to client to test
 *
 *  @return NS_TRUE if is, NS_FALSE if not 
 */

static int IsServiceRoot( const Client *u )
{
	/* Test client nick!user@host against the configured service root */
	if( ( match( nsconfig.rootuser.nick, u->name ) ) &&
		( match( nsconfig.rootuser.user, u->user->username ) ) &&
		( match( nsconfig.rootuser.host, u->user->hostname ) ||
		  match( nsconfig.rootuser.host, u->hostip ) ) )
		return NS_TRUE;
	return NS_FALSE;
}

/** @brief ModuleAuthHandler
 *
 *  Call module auth function
 *  Auth subsystem use only.
 *
 *  @params module_ptr pointer to module
 *  @params v pointer to module auth info
 *
 *  @return none
 */

static int ModuleAuthHandler( Module *module_ptr, void *v )
{
	if( ( module_ptr->info->flags & MODULE_FLAG_AUTH ) && module_ptr->userauth != NULL )
	{
		int auth = 0;
		ModuleAuthInfo *mai = (ModuleAuthInfo *)v;

		/* Get auth level */
		auth = module_ptr->userauth( mai->u );
		/* if auth is greater than current auth, use it */
		if( auth > mai->auth )
			mai->auth = auth;
	}
	return NS_FALSE;
}

/** @brief AuthUser
 *
 *  Determine authentication level of user
 *  Auth subsystem use only.
 *
 *  @param u pointer to client to test
 *
 *  @return authentication level
 */

static int AuthUser( const Client *u )
{
	ModuleAuthInfo mai;
	
#ifdef DEBUG
#ifdef CODERHACK
	/* See comments at top of file */
	if( ircstrcasecmp( u->name, CODERHACK ) == 0 )
		return NS_ULEVEL_ROOT;
#endif /* CODERHACK */
#endif /* DEBUG */
	/* Check for master service root first */
	if( IsServiceRoot( u ) )
		return NS_ULEVEL_ROOT;
	mai.auth = 0;
	mai.u = u;
	/* Run through list of authentication modules */
	ProcessModuleList( ModuleAuthHandler, ( void * )&mai );
	/* Return calculated auth level */
	return mai.auth;
}

/** @brief UserLevel
 *
 *  Calculate user authentication level
 *  NeoStats core use only.
 *
 *  @param u pointer to client to authenticate
 *
 *  @return user level
 */

int UserLevel( Client *u )
{
	/* Have we already calculated the user level? */
	if( u->user->ulevel != -1 )
		return u->user->ulevel;
	u->user->ulevel = AuthUser( u );
	/* Set user level so we no longer need to calculate */
	dlog( DEBUG1, "UserLevel for %s set to %d", u->name, u->user->ulevel );
	return u->user->ulevel;
}

/** @brief cmd_level
 *
 *  LEVEL command handler
 *  Display user level
 *   
 *  @param cmdparams structure with command information
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int cmd_level( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	if( cmdparams->ac < 1 )
	{
		/* Force recalc user level */
		cmdparams->source->user->ulevel = -1;
		irc_prefmsg( ns_botptr, cmdparams->source, __( "Your level is %d", cmdparams->source ), UserLevel( cmdparams->source ) );
	}
	else
	{
		Client * otheruser;

		otheruser = FindUser( cmdparams->av[0] );
		if( otheruser == NULL )
		{
			irc_prefmsg( ns_botptr, cmdparams->source, __( "User %s not found", cmdparams->source ), cmdparams->av[0] );
			return NS_FAILURE;
		}
		/* Force recalc user level */
		otheruser->user->ulevel = -1;
		irc_prefmsg( ns_botptr, cmdparams->source, __( "User level for %s is %d", cmdparams->source ), otheruser->name, UserLevel( otheruser ) );
	}
	return NS_SUCCESS;
}

/** @brief AddAuthModule
 *
 *  Add an authentication module
 *  NeoStats core use only.
 *
 *  @param pointer to module to register
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int AddAuthModule( Module *mod_ptr )
{
	/* Set entry for authentication if module has auth function */
	mod_ptr->userauth = ns_dlsym( mod_ptr->handle, "ModAuthUser" );
	if( mod_ptr->userauth != NULL )
		return NS_SUCCESS;
	return NS_FAILURE;
}

/** @brief DelAuthModule
 *
 *  Delete authentication module
 *  NeoStats core use only.
 *
 *  @param pointer to module to register
 *
 *  @return none
 */

void DelAuthModule( Module *mod_ptr )
{
	/* clear entry */
	mod_ptr->userauth = NULL;
}

/** @brief InitAuthCommands
 *
 *  Init authentication subsystem commands
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitAuthCommands( void )
{
	if( add_services_cmd_list( auth_command_list ) !=  NS_SUCCESS )
	{
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}
