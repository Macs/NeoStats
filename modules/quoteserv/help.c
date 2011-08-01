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
** $Id: help.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"

const char *qs_about[] = 
{
	"\2QuoteServ\2 is a Quote messaging service",
	NULL
};

const char *qs_help_add[] = {
	"Add a database",
	"Syntax: \2ADD <database>\2",
	"",
	"Register a database with quoteserv.",
	"<database> is the name of the database to load",
	NULL
};

const char *qs_help_del[] = {
	"Delete a database",
	"Syntax: \2DEL <database>\2",
	"",
	"Delete a database.",
	NULL
};

const char *qs_help_list[] = {
	"List loaded databases",
	"Syntax: \2LIST\2",
	"",
	"List loaded databases.",
	NULL
};

const char *qs_help_quote[] = {
	"Fetch quote",
	"Syntax: \2QUOTE [database]\2",
	"",
	"Get a random quote from all loaded databases.",
	"The optional database parameter allows selection of a random quote",
	"from that database.",
	NULL
};

const char *help_set_signonquote[] = {
	"\2SIGNONQUOTE <ON|OFF>\2",
	"Whether quoteserv gives a quote to users when the join the network",
	NULL
};

const char *help_set_exclusions[] = {
	"\2EXCLUSIONS <ON|OFF>\2",
	"Whether quoteserv uses the Global Exclusion list",
	NULL
};
