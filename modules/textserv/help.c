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

const char *ts_about[] = 
{
	"\2TextServ\2 is a text messaging service",
	NULL
};

const char *ts_help_add[] = {
	"Add a bot",
	"Syntax: \2ADD <nickname> <database> <channel> [PUBLIC|PRIVATE] [user] [host]\2",
	"",
	"Register a bot with textserv.",
	"<nickname> is the name of the bot to add",
	"<database> is the name of the database to load into the bot",
	"<channel> is the main channel you want the bot to join.",
	"[PUBLIC|PRIVATE] public control allows chanops to set the bot to join their channel",
	"private control only allows service admins to add/delete channels from the bot.",
	"[user] is the user for the bot to use if any (default 'ts')",
	"[host] is the hostname for the bot to use.",
	NULL
};

const char *ts_help_del[] = {
	"Delete a bot",
	"Syntax: \2DEL <nickname>\2",
	"",
	"Delete a bot.",
	NULL
};

const char *ts_help_list[] = {
	"List bots",
	"Syntax: \2LIST\2",
	"",
	"Lists loaded bots.",
	NULL
};
