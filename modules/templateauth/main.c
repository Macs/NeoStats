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
** $Id$
*/

/** TemplateAuth Module
 *
 *  You can copy this file as a template for writing your own auth modules
 */

/* neostats.h is the only required include for your module to access the 
 * NeoStats module API. You should not need to include any other NeoStats
 * files in order to develop your module.
 */
#include "neostats.h"

/** 
 *  Example copyright text
 *  You must change this or your module will not load.
 */

const char *template_copyright[] = {
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

/** Module info */
ModuleInfo module_info = 
{
	"TemplateAuth",
	"Template Authentication Module",
	template_copyright,
	template_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	MODULE_FLAG_AUTH,
	0,
	0,
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
 *  @return none
 */

int ModFini (void)
{
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
	return 0;
}
