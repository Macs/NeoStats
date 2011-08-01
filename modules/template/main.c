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

/** template.c 
 *  You can copy this file as a template for writing your own modules
 */

/* neostats.h is the only required include for your module to access the 
 * NeoStats module API. You should not need to include any other NeoStats
 * files in order to develop your module.
 */
#include "neostats.h"	/* NeoStats API */

/** When we create a a bot, we must store the handle returned to us for use
 *  when calling API bot functions
 */
static Bot *template_bot;

/** 
 *  Example copyright text
 *  You must change this or your module will not load.
 */

const char *template_copyright[] = 
{
	"Copyright (c) <year>, <your name>",
	NULL
};

/** 
 *  Example about text
 *  Returned by an intrinsic command when a user requests
 *  /msg botname about
 *  You must change this or your module will not load.
 */

const char *template_about[] = {
	"About your module",
	NULL
};

/** 
 *  Help text for example command
 *  Help text is in two parts:
 *  1 ) A single line returned in the list when a user requests 
 *     /msg help botname
 *  2 ) Multi-line help text returned when a user requests:
 *     /msg help botname command
 */
const char *template_help_hello_world[] = {
	"Hello world example command",
	"Syntax: \2HELLOWORLD\2",
	"",
	"Example of a privmsg command which just sends HELLO WORLD",
	"to the services channel.",
	NULL
};

/** 
 *  Help text for example setting
 */
const char *example_help_set_example[] = {
	"Syntax: \2EXAMPLE\2",
	"",
	"Example of a setting.",
	NULL
};

/** Module Info definition 
 *	This describes the module to the NeoStats core and provides information
 *  to end users when modules are queried.
 *  The structure is required but some fields are optional.
 */
ModuleInfo module_info = 
{
	/* REQUIRED: 
	 * name of module e.g. StatServ */
	"Template",
	/* REQUIRED: 
	 * one line brief description of module */
	"Put your brief module description here",
	/* OPTIONAL: 
	 * pointer to a NULL terminated list with copyright information
	 * NeoStats will automatically provide a CREDITS command to output this
	 * use NULL for none */
	template_copyright,
	/* OPTIONAL: 
	 * pointer to a NULL terminated list with extended description
	 * NeoStats will automatically provide an ABOUT command to output this
	 * use NULL for none */
	template_about,
	/* REQUIRED: 
	 * version of neostats used to build module
	 * must be NEOSTATS_VERSION or your module will not load */
	NEOSTATS_VERSION,
	/* REQUIRED: 
	 * string containing version of module 
	 * returned when a user requests 
	 * /msg botname version */
	"1.0",
	/* REQUIRED: string containing build date of module 
	 * should be __DATE__ */
	__DATE__,
	/* REQUIRED: string containing build time of module 
	 * should be __TIME__ */
	__TIME__,
	/* OPTIONAL: 
	 * Module control flags, 
	 * use 0 if not needed */
	0,
	/* OPTIONAL: 
	 * Protocol flags for required protocol specfic features e.g. NICKIP
	 * use 0 if not needed */
	0,
	/* OPTIONAL: 
	 * Protocol flags for required protocol specfic features e.g. SETHOST
	 * use 0 if not needed */
	0,
};

/** 
 *  example command
 *  Just sends "Hello World!" to the services channel
 */
static int template_hello_world( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	irc_chanalert( template_bot, "%s says \"Hello World!\"",
		cmdparams->source->name );
	return NS_SUCCESS;
}

/** OPTIONAL:
 *  Table of commands available in our bot
 *  This lists commands that a user can send via privmsg or if your bot is 
 *  in a channel and your bot is not "DEAF", users can issue via a channel 
 *  of !command
 */
static bot_cmd template_commands[]=
{
	{
	/* Command string */
	"HELLO",
	/* Function to call when this command is received */
	template_hello_world,
	/* Minimum number of parameters for this command */
	0,
	/* Minimum user level for this command */
	0,
	/* Multi line help text for this command */
	template_help_hello_world,
	/* Command flags */
	0,
	/* Module specific pointer, NULL if not required */
	NULL,
	/* NeoStats internal use only, must be NULL */
	NULL,
	},
	/* End command list with a NULL entry */
	NS_CMD_END()
};

static int example_setting = 0;

/** OPTIONAL:
 *  Table of settings available in our bot
 *  This lists settings that a user can set via privmsg
 */
static bot_setting template_settings[]=
{
	{
	/* Set string */
	"EXAMPLE",	
	/* Address of vaue holding current setting */
	&example_setting,	
	/* Set type, see SET_TYPE in neostats.h for available types */
	SET_TYPE_BOOLEAN,	
	/* Minimum value for setting, only valid for certain types */
	0, 
	/* Maximum value for setting, only valid for certain types */
	0, 	
	/* Minimum user level for this set command */
	NS_ULEVEL_ADMIN, 
	/* Description of value e.g. seconds, only valid for certain types */
	NULL,	
	/* pointer to help text */
	example_help_set_example, 
	/* handler for custom/post-set processing */
	NULL, 
	/* default value for setting, must be cast to( void * ) */
	( void* )1 
	},
	/* End setting list with a NULL entry */
	NS_SETTING_END()
};

/** Define information about our bot
 */
BotInfo template_bot_info = 
{
	/* REQUIRED: 
	 * nick */
	"changeme",
	/* OPTIONAL: 
	 * altnick, use "" if not needed */
	"altnick",
	/* REQUIRED: 
	 * user */
	"changeme",
	/* REQUIRED: 
	 * host that this bot will use 
	 * for the neostats host, use BOT_COMMON_HOST */
	BOT_COMMON_HOST,
	/* REQUIRED: 
	 * realname */
	"Example NeoStats module",
	/* OPTIONAL: 
	 * flags */
	0, 
	/* OPTIONAL: 
	 * bot command list pointer, use NULL if not needed */
	template_commands, 
	/* OPTIONAL: 
	 * bot command setting pointer, use NULL if not needed */
	template_settings,
};

/** Module event list
 *  What events we will act on
 *  This is required if you want your module to respond to events on IRC
 *  see events.h for a list of all events available
 */
ModuleEvent module_events[] = 
{
	NS_EVENT_END()
};

/** @brief ModInit
 *
 *  Init module
 *
 *  @param pointer to our module structure
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit( void )
{
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
	/* Introduce a bot onto the network saving the bot handle */
	template_bot = AddBot( &template_bot_info );
	if( !template_bot ) 
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
	return NS_SUCCESS;
}
