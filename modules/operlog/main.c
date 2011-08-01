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
** $Id: main.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "operlog.h"

/** Bot event function prototypes */
static int operlog_event_globops( const CmdParams *cmdparams );
static int operlog_event_chatops( const CmdParams *cmdparams );
static int operlog_event_wallops( const CmdParams *cmdparams );
static int operlog_event_localkill( const CmdParams *cmdparams );
static int operlog_event_globalkill( const CmdParams *cmdparams );
static int operlog_event_serverkill( const CmdParams *cmdparams );
static int operlog_event_umode( const CmdParams *cmdparams );

/** Bot pointer */
static Bot *operlog_bot;

/** Copyright info */
static const char *operlog_copyright[] = 
{
	"Copyright (c) 1999-2008, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** Module info */
ModuleInfo module_info = 
{
	"Operlog",
	"Operator command logging service",
	operlog_copyright,
	operlog_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
	0,
};

/** Bot setting table */
static bot_setting operlog_settings[] =
{
	NS_SETTING_END()
};

static bot_cmd operlog_commands[] = 
{
	NS_CMD_END()
};

/** BotInfo */
static BotInfo operlog_botinfo = 
{
	"Operlog", 
	"Operlog1", 
	"operlog", 
	BOT_COMMON_HOST, 
	"Operator command logging service",
	BOT_FLAG_ROOT|BOT_FLAG_RESTRICT_OPERS|BOT_FLAG_DEAF, 
	operlog_commands, 
	operlog_settings,
};

/** Module Events */
ModuleEvent module_events[] = 
{
	{EVENT_GLOBOPS,		operlog_event_globops, 0 },
	{EVENT_CHATOPS,		operlog_event_chatops, 0 },
	{EVENT_WALLOPS,		operlog_event_wallops, 0 },
	{EVENT_LOCALKILL,	operlog_event_localkill, 0 },
	{EVENT_GLOBALKILL,	operlog_event_globalkill, 0 },
	{EVENT_SERVERKILL,	operlog_event_serverkill, 0 },
	{EVENT_UMODE,		operlog_event_umode, 0 },
	NS_EVENT_END()
};

/** @brief ModInit
 *
 *  Init handler
 *  Loads configuration
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit( void )
{
	SET_SEGV_LOCATION();
	/* Load stored configuration */
	ModuleConfig( operlog_settings );
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *  Introduce bot onto network
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch( void )
{
	SET_SEGV_LOCATION();
	/* Create module bot */
	operlog_bot = AddBot( &operlog_botinfo );
	/* If failed to create bot, module will terminate */
	if( !operlog_bot ) 
		return NS_FAILURE;
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
	SET_SEGV_LOCATION();
	return NS_SUCCESS;
}

/** @brief operlog_event_globops
 *
 *  globops handler
 *  log globops
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int operlog_event_globops( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	nlog( LOG_NOTICE, "GLOBOPS: %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

/** @brief operlog_event_chatops
 *
 *  chatops handler
 *  log chatops
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int operlog_event_chatops( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	nlog( LOG_NOTICE, "CHATOPS: %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

/** @brief operlog_event_wallops
 *
 *  wallops handler
 *  log wallops
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int operlog_event_wallops( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	nlog( LOG_NOTICE, "WALLOPS: %s %s", cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

/** @brief operlog_event_localkill
 *
 *  local kill handler
 *  log local kill
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int operlog_event_localkill( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	nlog( LOG_NOTICE, "LOCALKILL: %s killed by %s for %s", cmdparams->target->name, cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

/** @brief operlog_event_globalkill
 *
 *  local kill handler
 *  log local kill
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int operlog_event_globalkill( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	nlog( LOG_NOTICE, "GLOBALKILL: %s killed by %s for %s", cmdparams->target->name, cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

/** @brief operlog_event_serverkill
 *
 *  local kill handler
 *  log local kill
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int operlog_event_serverkill( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	nlog( LOG_NOTICE, "SERVERKILL: %s killed by %s for %s", cmdparams->target->name, cmdparams->source->name, cmdparams->param );
	return NS_SUCCESS;
}

/** @brief operlog_event_mode
 *
 *  mode handler
 *  log operator mode
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int operlog_event_umode( const CmdParams *cmdparams )
{
	int add = 1;
	char *modes = cmdparams->param;

	SET_SEGV_LOCATION();
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
			case 'O':
			case 'o':
				if( add )
				{
					nlog( LOG_NOTICE, "OPER: %s is now an IRC Operator", cmdparams->source->name );
				}
				break;
			default:
				break;
		}
		modes++;
	}
	return NS_SUCCESS;
}
