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
** $Id: ss_help.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"

const char *ss_about[] = {
	"\2StatServ\2 provides detailed statistics about your",
	"IRC network users, channels and servers.",
	NULL
};

const char *ss_help_ctcpversion[] = {
	"Display client versions statistics",
	"Syntax: \2ctcpversion [limit]\2",
	"",
	"Display statistics on the client versions used on the network",
	"optional parameter <limit> specifies how many results to show.",
	NULL
};

const char *ss_help_set_htmlpath[] = {
	"\2HTMLPATH <path>\2",
	"Set the full pathname including filename used to write HTML",
	"statistics when HTML Statistics are enabled",
	NULL
};

const char *ss_help_set_exclusions[] = {
	"\2EXCLUSIONS <ON|OFF>\2",
	"Whether to ignore exlcuded items from statistics calculations",
	NULL
};

const char *ss_help_set_flatmap[] = {
	"\2FLATMAP <ON|OFF>\2",
	"Whether StatServ shows map with links or as flat list",
	NULL
};

const char *ss_help_set_html[] = {
	"\2HTML <ON|OFF>\2",
	"Enable or disable HTML statistics generation.",
	NULL
};

const char *ss_help_set_msginterval[] = {
	"\2MSGINTERVAL <seconds>\2",
	"Limit alerts to MSGLIMIT in <seconds>.",
	NULL
};

const char *ss_help_set_msglimit[] = {
	"\2MSGLIMIT <count>\2",
	"Limit alerts to <count> in MSGINTERVAL seconds.",
	NULL
};

const char *ss_help_set_lagtime[] = {
	"\2LAGTIME <seconds>\2",
	"Time in seconds at which a server is considered lagged.",
	NULL
};

const char *ss_help_set_htmltime[] = {
	"\2HTMLTIME <seconds>\2",
	"Interval in seconds at which statserv updates the HTML file.",
	NULL
};

const char *ss_help_set_lagalert[] = {
	"\2LAGALERT <alerttype>\2",
	"Announce when a server on the network is lagged",
	"Options for <alerttype> are:",
	"    0 - Never",
	"    1 - Announce in services channel",
	"    2 - Announce by globops",
	"    3 - Announce by wallops",
	NULL
};

const char *ss_help_set_recordalert[] = {
	"\2RECORDALERT <alerttype>\2",
	"Announce new records on the network",
	"Options for <alerttype> are:",
	"    0 - Never",
	"    1 - Announce in services channel",
	"    2 - Announce by globops",
	"    3 - Announce by wallops",
	NULL
};

const char *ss_help_set_channeltime[] = {
	"\2CHANNELTIME <seconds>\2",
	"Time in seconds a channel is unused before it is deleted from statistics.",
	NULL
};

const char *ss_help_channel[] = {
	"Display channel statistics",
	"Syntax: \2CHANNEL\2",
	"        \2CHANNEL <Channame>\2",
	"        \2CHANNEL <POP>\2",
	"        \2CHANNEL <KICKS>\2",
	"        \2CHANNEL <TOPICS>\2",
	"",
	"Display statistics about current channels",
	"With no parameters, display top 10 channels by current member count",
	"",
	"\2<name>\2 display detailed statistics on channel",
	"",
	"\2POP\2 display top 10 channels by total joins",
	"",
	"\2KICKS\2 display top 10 channels by total kicks",
	"",
	"\2TOPICS\2 display top 10 channels by total topic changes",
	NULL
};

const char *ss_help_server[] = {
	"Display server statistics",
	"Syntax: \2SERVER <server name>\2",
	"        \2SERVER LIST\2",
	"        \2SERVER DEL <servername>\2",
	"        \2SERVER RENAME <oldservername> <newservername>\2",
	"",
	"Display server statistics for passed server name.",
	"",
	"LIST displays all database entries.",
	"",
	"DEL removes an entry.",
	"",
	"RENAME renames an entry.",
	NULL
};

const char *ss_help_map[] = {
	"Display network map",
	"Syntax: \2MAP\2",
	"",
	"Display a server map with basic statistics.",
	NULL
};

const char *ss_help_netstats[] = {
	"Display network statistics",
	"Syntax: \2NETSTATS\2",
	"",
	"Display network wide statistics",
	NULL
};

const char *ss_help_daily[] = {
	"Display daily statistics",
	"Syntax: \2DAILY\2",
	"",
	"Display statistics based on today.",
	NULL
};

const char *ss_help_tldmap[] = {
	"Display TLD statistics",
	"Syntax: \2TLDMAP\2",
	"",
	"Display map of top level domains on the network.",
	NULL
};

const char *ss_help_operlist[] = {
	"Display list of IRC operators",
	"Syntax: \2OPERLIST\2",
	"        \2OPERLIST NOAWAY\2",
	"        \2OPERLIST SERVER <servername>\2",
	"",
	"Display current list of IRC operators.",
	"",
	"NOAWAY will ignore opers that are set away", 
	"",
	"SERVER will only show opers on the named server.",
	NULL
};

const char *ss_help_botlist[] = {
	"Display list of BOTS",
	"Syntax: \2BOTLIST\2",
	"",
	"Display current list of bots on the network.",
	"i.e. Clients with UMODE_BOT set.",
	NULL
};

const char *ss_help_forcehtml[] = {
	"Force output of the HTML statistics",
	"Syntax: \2FORCEUPDATE\2",
	"",
	"Forces an immediate update of the HTML statistics output.",
	NULL
};
