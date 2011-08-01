/* NeoStats - IRC Statistical Services 
** Copyright (c) 2006 Justin Hammond, Mark Hetherington
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
** $Id: main.c 3021 2006-01-26 15:30:22Z Fish $
*/

#include "neostats.h"

/** DCCPartyLine Module
 *
 *  Adds partyline to DCC system
 */

/** Copyright info */
static const char *dccpartyline_copyright[] = 
{
	"Copyright (c) 2006, NeoStats",
	"http://www.neostats.net/",
	NULL
};

/** About info */
static const char *dccpartyline_about[] = 
{
	"Adds partyline to DCC system.",
	NULL
};

/** Module info */
ModuleInfo module_info = 
{
	"dccpartyline",
	"DCC Party Line module",
	dccpartyline_copyright,
	dccpartyline_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
	0,
};

static int dccpartyline_event_dccchatmsg( const CmdParams *cmdparams );

/** Module Events */
ModuleEvent module_events[] = 
{
	{EVENT_DCCCHATMSG,	dccpartyline_event_dccchatmsg, 0},
	NS_EVENT_END()
};

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

static int dccpartyline_event_dccchatmsg( const CmdParams *cmdparams )
{
	static char buf[BUFSIZE];
 
 	ircsnprintf( buf, BUFSIZE, "\2%s\2: %s", cmdparams->source->name, cmdparams->param );
	irc_dccmsgall( "%s", buf );
	irc_chanalert( NULL, "%s", buf );
	return NS_SUCCESS;
}
