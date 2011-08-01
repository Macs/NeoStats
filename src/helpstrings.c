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
** $Id: helpstrings.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"

const char *ns_help_level[] = {
	"Permission level",
	"Syntax: \2LEVEL [nick]\2",
	"",
	"Display permission level for NeoStats in the range",
	"0 (lowest) to 200 (highest).",
	"Optional nick parameter allows you to see the level",
	"for another user",
	NULL
};

const char *ns_help_jupe[] = {
	"Jupiter a server",
	"Syntax: \2JUPE <servername>\2",
	"",
	"Jupiter a server; i.e. create a fake \"server\" connected",
	"to the NeoStats host which prevents any real server of",
	"that name from connecting. To remove the jupe use the",
	"\2/SQUIT\2 command.",
	NULL
};

const char *ns_help_exclude[] = {
	"Maintain exclusion list",
	"Syntax: \2EXCLUDE ADD <HOST|SERVER|CHANNEL|USERHOST> <pattern> <reason>\2",
	"        \2EXCLUDE DEL <pattern>\2",
	"        \2EXCLUDE LIST\2",
	"",
	"Maintain the exclusion list which is used to exclude channels,",
	"users and servers from certain scans and events.",
	"",
	"\2ADD\2 Add a new exclusion to the list of the requested type.",
	"<pattern> is the userhost mask, host, server or channel name and",
	"may include wildcards such as * and ?.",
	"<reason> is the reason for the exclusion",
	"",
	"\2DEL\2 Delete an entry from the exclusion list.",
	"",
	"\2LIST\2 List the current exclusions",
	NULL
};

#ifdef USE_RAW
const char *ns_help_raw[] = {
	"Send a raw command",
	"Syntax: \2RAW <text>\2",
	"",
	"Sends a string of raw text directly to the server to which",
	"NeoStats is connected. Nothing is returned to the user",
	"after a raw command.",
	"Raw can cause a number of problems on the network and is",
	"used at your own risk. No support of any kind is provided",
	"for this command.",
	NULL
};
#endif

const char *ns_help_load[] = {
	"Load module",
	"Syntax: \2LOAD <module name>\2",
	"",
	"Load a module.",
	NULL
};

const char *ns_help_unload[] = {
	"Unload module",
	"Syntax: \2UNLOAD <module name>\2",
	"",
	"Unload a module.",
	NULL
};

const char *ns_help_modlist[] = {
	"List loaded modules",
	"Syntax: \2MODLIST\2",
	"",
	"Display names and descriptions of all loaded modules.",
	NULL
};

const char *ns_help_shutdown[] = {
	"Shutdown NeoStats",
	"Syntax: \2SHUTDOWN <reason>\2",
	"",
	"Cause NeoStats to save data files and exit.",
	"The reason provided will be broadcast to the services",
	"channel and other operators on the network.",
	NULL
};

const char *ns_help_reload[] = {
	"Reload NeoStats",
	"Syntax: \2RELOAD <reason>\2",
	"",
	"Cause NeoStats to leave the network, reload datafiles,",
	"then reconnect to the network.",
	"The reason provided will be broadcast to the services",
	"channel and other operators on the network.",
	NULL
};

const char *ns_help_userlist[] = {
	"List users",
	"Syntax: \2USERLIST\2",
	"        \2USERLIST <name>\2",
	"",
	"Display list of users on the network.",	
	"Optional name parameter limits display to that user.",	
	"This command is only available in debug mode and is only",
	"useful for debugging Neostats.",
	NULL
};

const char *ns_help_serverlist[] = {
	"List servers",
	"Syntax: \2SERVERLIST\2",
	"        \2SERVERLIST <name>\2",
	"",
	"Display list of servers on the network.",	
	"Optional parameter name limits display to that server.",	
	"This command is only available in debug mode and is only",
	"useful for debugging Neostats.",
	NULL
};

const char *ns_help_banlist[] = {
	"List bans",
	"Syntax: \2BANLIST\2",
	"",
	"Display list of bans on the network.",	
	"This command is only available in debug mode and is only",
	"useful for debugging Neostats.",
	NULL
};

const char *ns_help_channellist[] = {
	"List channels",
	"Syntax: \2CHANNELLIST\2",
	"        \2CHANNELLIST <name>\2",
	"",
	"Display list of channels on the network.",	
	"Optional parameter name limits display to that channel.",	
	"This command is only available in debug mode and is only",
	"useful for debugging Neostats.",
	NULL
};

const char *ns_help_botlist[] = {
	"List module bots",
	"Syntax: \2BOTLIST\2",
	"",
	"Display list of neostats bots being used on the network.",	
	NULL
};

const char *ns_help_socklist[] = {
	"List module sockets",
	"Syntax: \2SOCKLIST\2",
	"",
	"Display list of sockets being used on the network.",	
	NULL
};

const char *ns_help_timerlist[] = {
	"List module timers",
	"Syntax: \2TIMERLIST\2",
	"",
	"Display list of timer functions being used on the network.",	
	NULL
};

const char *ns_help_status[] = {
	"Status information",
	"Syntax: \2STATUS\2",
	"",
	"Display info about NeoStats uptime and other stats.",
	NULL
};

const char *ns_help_lookup[] = {
	"Lookup DNS record",
	"Syntax: \2LOOKUP <IP|HOSTNAME> [type]\2",
	"",
	"Lookup DNS records for an ip address or hostname.",
	"The default lookup is the ip address for a hostname",
	"or the hostname for an ip address.",
	"",
	"Options for type are:",
	"    txt - text records",
	"    rp  - responsible person for this record",
	"    ns  - name servers for this record",
	"    soa - SOA for this record",
	"",
	NULL
};

const char *ns_help_set_joinserviceschan[] = {
	"\2JOINSERVICESCHAN <ON|OFF>\2",
	"Whether NeoStats service bots join services channel",
	NULL
};

const char *ns_help_set_splittime[] = {
	"\2SPLITTIME <timediff>\2",
	"Used to determine if users connecting to the network",
	"are part of a net join (when two servers link together)",
	"This setting should not be changed unless you know the",
	"effects in full",
	NULL
};

const char *ns_help_set_msgsampletime[] = {
	"\2MSGSAMPLETIME <seconds>\2",
	"Sets the threshold for message floods.",
	"<seconds> is time used to measure floods.",
	NULL
};

const char *ns_help_set_msgthreshold[] = {
	"\2MSGTHRESHOLD <number>\2",
	"Sets the threshold for message floods.",
	"<number> is number of messages in MSGSAMPLETIME seconds.",
	NULL
};

const char *ns_help_set_pingtime[] = {
	"\2PINGTIME <seconds>\2",
	"Interval at which NeoStats pings servers",
	NULL
};

const char *ns_help_set_loglevel[] = {
	"\2LOGLEVEL <level>\2",
	"Controls the level of logging information recorded",
	"<level> is a value in the range 1 - 6",
	NULL
};

const char *ns_help_set_cmdchar[] = {
	"\2CMDCHAR <char>\2",
	"Character used to indicate a channel message is a command.",
	NULL
};

const char *ns_help_set_cmdreport[] = {
	"\2CMDREPORT <ON|OFF>\2",
	"Report command usage to the services channel",
	NULL
};

const char *ns_help_set_debug[] = {
	"\2DEBUG <ON|OFF>\2",
	"Send debugging information to debug.log and to the channel",
	"defined by DEBUGCHAN if DEBUGTOCHAN is enabled.",
	NULL
};

const char *ns_help_set_debuglevel[] = {
	"\2DEBUG <level>\2",
	"Controls the level of debug information reported",
	"<level> is a value in the range 1 - 10",
	NULL
};

const char *ns_help_set_debugchan[] = {
	"\2DEBUGCHAN <#channel>\2",
	"Channel name for debug output when DEBUGTOCHAN is ON",
	NULL
};

const char *ns_help_set_debugmodule[] = {
	"\2DEBUGMODULE <modulename|all>\2",
	"What module to report debug text for or all",
	NULL
};

const char *ns_help_set_debugtochan[] = {
	"\2DEBUGTOCHAN <ON|OFF>\2",
	"Enable debug output to channel DEBUGCHAN",
	NULL
};

const char *ns_help_set_nick[] = {
	"\2NICK <newnick>\2",
	"Change bot nickname",
	NULL
};

const char *ns_help_set_altnick[] = {
	"\2ALTNICK <newnick>\2",
	"Change bot alternate nickname",
	NULL
};

const char *ns_help_set_user[] = {
	"\2USER <username>\2",
	"Change bot username",
	"(may require restart to take effect).",
	NULL
};

const char *ns_help_set_host[] = {
	"\2HOST <host>\2",
	"Change bot host",
	"(may require restart to take effect).",
	NULL
};

const char *ns_help_set_realname[] = {
	"\2REALNAME <realname>\2",
	"Change bot realname",
	"(may require restart to take effect).",
	NULL
};

const char *ns_help_set_servicecmode[] = {
	"\2SERVICECMODE <mode>\2",
	"Channel modes assigned to service bots when they join",
	"the services channel. You must prefix with '+'",
	"e.g. +o or +a or +v etc",
	"(requires restart to affect existing service bots).",
	NULL
};

const char *ns_help_set_serviceumode[] = {
	"\2SERVICEUMODE <mode>\2",
	"User modes assigned to service bots when they join",
	"the network. You must prefix with '+'",
	"e.g. +S or +So etc",
	"(requires restart to affect existing service bots).",
	NULL
};

const char *ns_help_set_recvq[] = {
	"\2RECVQ <size>\2",
	"Receive queue size for sockets",
	NULL
};

const char *cmd_help_help[] = {
	"Online help",
	"Syntax: \2HELP [command]\2",
	"",
	"Display help on the bot commands",
	NULL
};

const char *cmd_help_about[] = {
	"Display about text",
	"Syntax: \2ABOUT\2",
	"",
	"Display information about the module",
	NULL
};

const char *cmd_help_credits[] = {
	"Display credits",
	"Syntax: \2CREDITS\2",
	"",
	"Display credits",
	NULL
};

const char *cmd_help_version[] = {
	"Display version",
	"Syntax: \2VERSION\2",
	"",
	"Display version information",
	NULL
};

const char *cmd_help_levels[] = {
	"Change command user level",
	"Syntax: \2LEVELS LIST\2",
	"        \2LEVELS <command> <level>\2",
	"",
	"List or change minimum user level for a command.",
	"<level> must be between 0 and 200.",
	NULL
};

const char *cmd_help_set[] = {
	"Syntax: \2SET LIST\2",
	"        \2SET <option> <value>\2",
	"",
	"LIST    display the current settings",
	"",
	"Available Options are:",
	"",
	NULL
};
const char *ns_help_set_sendhelp[] = {
	"Syntax: \2SET SENDHELP <on/off>\2",
	"",
	"Should we send full helptext for commands with invalid syntax's etc"
	"",
	NULL
};
const char *ns_help_set_allhelp[] = {
	"Syntax: \2SET ALLHELP <on/off>\2"
	"",
	"Should we send all available help to users (regardless if its channel or PM only help)"
	"or only send context sensitive help"
	"",
	NULL
};
