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
** $Id: ircprotocol.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "ircprotocol.h"
#include "protocol.h"
#include "ircsend.h"
#include "ircrecv.h"
#include "numerics.h"
#include "modes.h"
#include "dl.h"
#include "services.h"

ircd_server ircd_srv;
static char protocol_path[MAXPATH];
ProtocolInfo *protocol_info;
void *protocol_module_handle;
irc_cmd *cmd_list;
int (*irc_parse) (void *notused, void *rline, int len);
 
/** @brief process_ircd_cmd
 *
 *  process ircd commands
 *
 *  @param cmdptr source flag
 *  @param cmd command string
 *  @param origin of command
 *  @param av array of arguments
 *  @param ac count of args in av
 *
 *  @return none
 */

void process_ircd_cmd( int cmdptr, const char *cmd, char *origin, char **av, int ac )
{
	irc_cmd *ircd_cmd_ptr;
	irc_cmd *intrinsic_cmd_ptr;

	SET_SEGV_LOCATION();
	ircd_cmd_ptr = cmd_list;
	while( ircd_cmd_ptr->name != NULL )
	{
		if( ircstrcasecmp( *ircd_cmd_ptr->name, cmd ) == 0 || 
		  ( ( ircd_srv.protocol & PROTOCOL_TOKEN ) && ircd_cmd_ptr->token && strcmp( *ircd_cmd_ptr->token, cmd ) == 0 ) ) {
			if( ircd_cmd_ptr->handler )
			{
				dlog( DEBUG3, "process_ircd_cmd: running command %s", *ircd_cmd_ptr->name );
				ircd_cmd_ptr->handler( origin, av, ac, cmdptr );
			}
			else
			{
				dlog( DEBUG3, "process_ircd_cmd: ignoring command %s", cmd );
			}
			ircd_cmd_ptr->usage++;
			return;
		}
		ircd_cmd_ptr ++;
	}
	intrinsic_cmd_ptr = intrinsic_cmd_list;
	while( intrinsic_cmd_ptr->handler != NULL )
	{
		if( *intrinsic_cmd_ptr->name )
		{
			if( ircstrcasecmp( *intrinsic_cmd_ptr->name, cmd ) == 0 || 
			  ( ( ircd_srv.protocol & PROTOCOL_TOKEN ) && *intrinsic_cmd_ptr->token && ircstrcasecmp( *intrinsic_cmd_ptr->token, cmd ) == 0 ) ) 
			{
				dlog( DEBUG3, "process_ircd_cmd: running command %s", *intrinsic_cmd_ptr->name );
				intrinsic_cmd_ptr->handler( origin, av, ac, cmdptr );
				intrinsic_cmd_ptr->usage++;
				return;
			}
		}
		intrinsic_cmd_ptr ++;
	}
	ircd_cmd_ptr = numeric_cmd_list;	
	/* Process numeric replies */
	while( ircd_cmd_ptr->name != NULL )
	{
		if( ircstrcasecmp( *ircd_cmd_ptr->name, cmd ) == 0 )
		{
			if( ircd_cmd_ptr->handler ) {
				dlog( DEBUG3, "process_ircd_cmd: running command %s", *ircd_cmd_ptr->name );
				ircd_cmd_ptr->handler( origin, av, ac, cmdptr );
			}
			else
			{
				dlog( DEBUG3, "process_ircd_cmd: ignoring command %s", cmd );
			}
			ircd_cmd_ptr->usage++;
			return;
		}
		ircd_cmd_ptr ++;
	}

	nlog( LOG_INFO, "No support for %s", cmd );
}

/** @brief parse
 *
 *  parser
 *
 *  @param notused
 *  @param rline
 *  @param len
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int parse( void *notused, void *rline, int len )
{
	char origin[64], cmd[64], *coreLine;
	char *line = (char *)rline;
	int cmdptr = 0;
	int ac = 0;
	char **av = NULL;

	SET_SEGV_LOCATION();
	if( *line == '\0' )
		return NS_FAILURE;
	dlog( DEBUG1, "------------------------BEGIN PARSE-------------------------" );
	dlog( DEBUGRX, "%s", line );
	if( *line == ':' )
	{
		coreLine = strpbrk( line, " " );
		if( coreLine == NULL )
			return NS_FAILURE;
		*coreLine = 0;
		while( isspace( *++coreLine ) )
			;
		strlcpy( origin, line + 1, sizeof( origin ) );
		memmove( line, coreLine, strnlen( coreLine, BUFSIZE ) + 1 );
		cmdptr = 1;
	}
	else
	{
		cmdptr = 0;
		*origin = 0;
	}
	if( *line == '\0' )
		return NS_FAILURE;
	coreLine = strpbrk( line, " " );
	if( coreLine )
	{
		*coreLine = 0;
		while( isspace( *++coreLine ) )
			;
	}
	else
	{
		coreLine = line + strlen( line );
	}
	strlcpy( cmd, line, sizeof( cmd ) ); 
	dlog( DEBUG1, "origin: %s", origin );
	dlog( DEBUG1, "cmd   : %s", cmd );
	dlog( DEBUG1, "args  : %s", coreLine );
	ac = ircsplitbuf( coreLine, &av, 1 );
	process_ircd_cmd( cmdptr, cmd, origin, av, ac );
	ns_free( av );
	dlog( DEBUG1, "-------------------------END PARSE--------------------------" );
	return NS_SUCCESS;
}

/** @brief parsep10
 *
 *  P10 parser
 *  formats:
 *    :<source> <command> <param1> <paramN> :<last parameter>
 *    <source> <command> <param1> <paramN> :<last parameter>
 *
 *  @param notused Justin????
 *  @param rline Justin????
 *  @param len Justin????
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int parsep10( void *notused, void *rline, int len )
{
	char origin[64], cmd[64], *coreLine;
	char *line = ( char * )rline;
	int cmdptr = 0;
	int ac = 0;
	char **av = NULL;

	SET_SEGV_LOCATION();
	if( *line == '\0' )
		return NS_FAILURE;
	dlog( DEBUG1, "------------------------BEGIN PARSE-------------------------" );
	dlog( DEBUGRX, "%s", line );
	coreLine = strpbrk( line, " " );
	if( coreLine )
	{
		*coreLine = 0;
		while( isspace( *++coreLine ) )
			;
	}
	else
	{
		coreLine = line + strlen( line );
	}
	if( ( ircstrcasecmp( line, "SERVER" ) ) == 0 || ( ircstrcasecmp( line, "PASS" ) == 0 ) )
	{
		strlcpy( cmd, line, sizeof( cmd ) );
		dlog( DEBUG1, "cmd   : %s", cmd );
		dlog( DEBUG1, "args  : %s", coreLine );
		ac = ircsplitbuf( coreLine, &av, 1 );
		cmdptr = 2;
		dlog( DEBUG1, "0 %d", ac );
		/* really needs to be in AddServer since this is a NeoStats wide bug
		 if config uplink name does not match our uplinks server name we can
		 never find the uplink!
		*/
		if( strcmp( cmd, "SERVER" ) == 0 )
			strlcpy( me.uplink, av[0], MAXHOST );
	}
	else
	{
		strlcpy( origin, line, sizeof( origin ) );	
		cmdptr = 0;
		line = strpbrk( coreLine, " " );
		if( line )
		{
			*line = 0;
			while( isspace( *++line ) )
				;
		} 
		/*else
		{
			coreLine = line + strlen( line );
		}*/
		strlcpy( cmd, coreLine, sizeof( cmd ) );
		dlog( DEBUG1, "origin: %s", origin );
		dlog( DEBUG1, "cmd   : %s", cmd );
		dlog( DEBUG1, "args  : %s", line );
		if( line )
			ac = ircsplitbuf( line, &av, 1 );
		dlog( DEBUG1, "0 %d", ac );
	}
	process_ircd_cmd( cmdptr, cmd, origin, av, ac );
	ns_free( av );
	dlog( DEBUG1, "-------------------------END PARSE--------------------------" );
	return NS_SUCCESS;
}

/** @brief InitIrcdProtocol
 *
 *  Init protocol info
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int InitIrcdProtocol( void )
{
	protocol_info = ns_dlsym( protocol_module_handle, "protocol_info" );
	if( protocol_info == NULL )
	{
		nlog( LOG_CRITICAL, "Unable to find protocol_info in protocol module %s", protocol_path );
		return NS_FAILURE;	
	}
	if( protocol_info->required & PROTOCOL_CLIENTMODE )
		nsconfig.singlebotmode = 1;
	strlcpy( me.servicescmode, protocol_info->services_cmode, MODESIZE );
	strlcpy( me.servicesumode, protocol_info->services_umode, MODESIZE );
	/* set min protocol */
	ircd_srv.protocol = protocol_info->required;
	/* Allow protocol module to "override" the parser */
	irc_parse = ns_dlsym( protocol_module_handle, "parse" );
	/* Use internal parser */
	if( irc_parse == NULL )
	{
		if( ircd_srv.protocol & PROTOCOL_P10 )
			irc_parse = parsep10;
		else
			irc_parse = parse;
	}
	cmd_list = ns_dlsym( protocol_module_handle, "cmd_list" );
	if( cmd_list == NULL )
	{
		nlog( LOG_CRITICAL, "Unable to find command list in selected IRCd module" );
		return NS_FAILURE;	
	}
	return NS_SUCCESS;
}

/** @brief InitIrcdModes
 *
 *  Lookup mode tables and init neostats mode lookup tables
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int InitIrcdModes( void )
{
	mode_init *chan_umodes;
	mode_init *chan_modes;
	mode_init *user_umodes;
	mode_init *user_smodes;

	chan_umodes = ns_dlsym( protocol_module_handle, "chan_umodes" );
	chan_modes  = ns_dlsym( protocol_module_handle, "chan_modes" );
	user_umodes = ns_dlsym( protocol_module_handle, "user_umodes" );
	/* Not required */
	user_smodes = ns_dlsym( protocol_module_handle, "user_smodes" );
	if( user_smodes )
		ircd_srv.features |= FEATURE_USERSMODES;
	if( InitModeTables( chan_umodes, chan_modes, user_umodes, user_smodes ) != NS_SUCCESS ) 
		return NS_FAILURE;
	return NS_SUCCESS;
}

/** @brief InitIrcd
 *
 *  ircd initialisation
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitIrcd( void )
{
	/* Clear IRCD info */
	os_memset( &ircd_srv, 0, sizeof( ircd_srv ) );
	/* Open protocol module */
	ircsnprintf( protocol_path, 255, "%s/%s%s", MOD_PATH, me.protocol,MOD_STDEXT );
	nlog( LOG_NORMAL, "Using protocol module %s", protocol_path );
	protocol_module_handle = ns_dlopen( protocol_path, RTLD_NOW | RTLD_GLOBAL );
	if( protocol_module_handle == NULL )
	{
		nlog( LOG_CRITICAL, "Unable to load protocol module %s", protocol_path );
		return NS_FAILURE;	
	}
	/* Setup protocol options */
	if( InitIrcdProtocol() != NS_SUCCESS )
		return NS_FAILURE;	
	/* Setup IRCD function calls */
	if( InitIrcdSymbols() != NS_SUCCESS ) 
		return NS_FAILURE;
	/* Build mode tables */
	if( InitIrcdModes() != NS_SUCCESS ) 
		return NS_FAILURE;
	return NS_SUCCESS;
}

/** @brief FiniIrcd
 *
 *  fini ircd
 *
 *  @param none
 *
 *  @return none
 */

void FiniIrcd( void )
{
	ns_dlclose( protocol_module_handle );
}

/** @brief HaveFeature
 *
 *  Check to see if a feature is supported
 *
 *  @param mask of features to check
 *
 *  @return 1 if have else 0
 */

int HaveFeature( int mask )
{
	return( ircd_srv.features & mask );
}
