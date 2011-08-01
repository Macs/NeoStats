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

const char *ls_about[] = 
{
	"\2LimitServ\2 manages channel limit settings",
	NULL
};

const char *help_add[] = {
	"Add a channel",
	"Syntax: \2ADD <channel>\2",
	"",
	"Register a channel with limitserv.",
	"<channel> is the name of the channel",
	NULL
};

const char *help_del[] = {
	"Delete a channel",
	"Syntax: \2DEL <channel>\2",
	"",
	"Delete a channel.",
	NULL
};

const char *help_list[] = {
	"List channels",
	"Syntax: \2LIST\2",
	"",
	"Lists channels.",
	NULL
};

const char *help_set_join[] = {
	"\2JOIN <ON|OFF>\2",
	"Whether to join channels managed by limitserv",
	NULL
};

const char *help_set_buffer[] = {
	"\2BUFFER <number>\2",
	"Sets the number of extra user slots allocated when adjusting the limit",
	NULL
};

const char *help_set_timer[] = {
	"\2TIMER <seconds>\2",
	"Sets the number of seconds between user limit updates",
	NULL
};
const char *help_set_grace[] = {
	"\2GRACE <number>\2",
	"Only Changes the Limit Setting when the users increase or decrease by this ammount.",
	"It can't be equal or bigger than the BUFFER setting",
	NULL
};
