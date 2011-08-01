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
** $Id: main.c 2821 2005-09-19 20:11:48Z Mark $
*/

/** main.c 
 *  This module tests the perl extension interface
  */

#include "neostats.h"	/* NeoStats API */
#define PERLDEFINES
#include "perlmod.h"

static Bot *perl_bot;
static int perlext_pong (const CmdParams *cmds);

const char* perl_copyright[] = 
{
	"Copyright (c) 2006, Justin Hammond",
	NULL
};

const char *perl_about[] = {
	"Test the Perl Extension API",
	NULL
};

const char *load_extension_help[] = {
	"Test",
	"Syntax: \2LOADEXT\2",
	"",
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
	"PerlExtTest",
	/* REQUIRED: 
	 * one line brief description of module */
	"Test Perl Exetensions",
	/* OPTIONAL: 
	 * pointer to a NULL terminated list with copyright information
	 * NeoStats will automatically provide a CREDITS command to output this
	 * use NULL for none */
	perl_copyright,
	/* OPTIONAL: 
	 * pointer to a NULL terminated list with extended description
	 * NeoStats will automatically provide an ABOUT command to output this
	 * use NULL for none */
	perl_about,
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
	 * Protocol flags for required protocol specfic features e.g. SETHOST
	 * use 0 if not needed */
	0,
};

XS (XS_NeoStats_Test_PerlExt) 
{
	dXSARGS;
	if(items != 1) {
		nlog(LOG_WARNING, "Didn't get required no of params for TestPerlExt Call");
	} else {
		irc_chanalert(perl_bot, "TestPerlExt Got this: %s", SvPV_nolen(ST(0)));
	}
}

static int 
perl_ext_init() {
	newXS("NeoStats::PerlExt::TestPerlExt", XS_NeoStats_Test_PerlExt, __FILE__);
	nlog(LOG_INFO, "Loaded Perl Extensions Hooks");
	return NS_SUCCESS;
}


/** 
 *  example command
 *  Just sends "Hello World!" to the services channel
 */
static int load_extension( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	irc_chanalert( perl_bot, "%s is trying to load perl extension %s",
		cmdparams->source->name, cmdparams->av[0] );
	if (load_perlextension(cmdparams->av[0], perl_ext_init, cmdparams->source)) {
		perl_sync_module(GET_CUR_MODULE());
	} else {
		return NS_FAILURE;
	}
	execute_perl(GET_CUR_MODULE(), sv_2mortal (newSVpv ("NeoStats::Module::extension_2eple::TestCall", 0)), 1, "Hello World");
	return NS_SUCCESS;
}

/** OPTIONAL:
 *  Table of commands available in our bot
 *  This lists commands that a user can send via privmsg or if your bot is 
 *  in a channel and your bot is not "DEAF", users can issue via a channel 
 *  of !command
 */
static bot_cmd perl_commands[]=
{
	{
	/* Command string*/
	"LOADEXT",	
	/* Function to call when this command is received */
	load_extension,	
	/* Minimum number of parameters for this command */
	1, 	
	/* Minimum user level for this command */
	0,	
	/* Multi line help text for this command */
	load_extension_help,		
	},
	/* End command list with a NULL entry */
	NS_CMD_END()
};


/** Define information about our bot
 */
BotInfo perl_bot_info = 
{
	/* REQUIRED: 
	 * nick */
	"PerlExt",
	/* OPTIONAL: 
	 * altnick, use "" if not needed */
	"PerlExt2",
	/* REQUIRED: 
	 * user */
	"Perl",
	/* REQUIRED: 
	 * host that this bot will use 
	 * for the neostats host, use BOT_COMMON_HOST */
	BOT_COMMON_HOST,
	/* REQUIRED: 
	 * realname */
	"Perl Extensions Test",
	/* OPTIONAL: 
	 * flags */
	BOT_FLAG_SERVICEBOT, 
	/* OPTIONAL: 
	 * bot command list pointer, use NULL if not needed */
	perl_commands, 
	/* OPTIONAL: 
	 * bot command setting pointer, use NULL if not needed */
	NULL,
};

/** Module event list
 *  What events we will act on
 *  This is required if you want your module to respond to events on IRC
 *  see events.h for a list of all events available
 */
ModuleEvent module_events[] = 
{
	{EVENT_PONG, perlext_pong },
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
	perl_bot = AddBot( &perl_bot_info );
	if( !perl_bot ) 
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

static int
perlext_pong (const CmdParams *cmds) {
	irc_chanalert(perl_bot, "Got Pong fromm %s", cmds->source->name);
	return NS_SUCCESS;
}

