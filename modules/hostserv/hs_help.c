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
** $Id: hs_help.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"

const char *hs_about[] = {
	"\2HostServ\2 provides users with a unique virtual host while",
	"on the network. Virtual hosts are set automatically when",
	"users join the network or by using the LOGIN command.",
	"",
	"If you find your host is not working, it may have been",
	"removed due to abuse and you should contact an operator.",
	NULL
};

const char *hs_help_add[] = {
	"Add a vhost",
	"Syntax: \2ADD <NICK> <HOSTNAME> <VHOST> <PASSWORD>\2",
	"",
	"Register a vhost with hostserv. e.g. vhost.com.",
	"HOSTNAME is the host the user is connecting from",
	"and can include wildcards e.g. *.realhost.com.",
	"Users can also set their vhost with the LOGIN command.",
	"This allows them use of the vhost from multiple hosts or",
	"multiple users to share a vhost.",
	NULL
};

const char *hs_help_del[] = {
	"Delete a vhost",
	"Syntax: \2DEL <nick>\2",
	"",
	"Delete the vhost for <nick>.",
	NULL
};

const char *hs_help_view[] = {
	"Detailed vhost list",
	"Syntax: \2VIEW <nick>\2",
	"",
	"View detailed information about the vhost for <nick>.",
	NULL
};

const char *hs_help_list[] = {
	"List vhosts",
	"Syntax: \2LIST\2",
	"        \2LIST <startpos>\2",
	"",
	"Lists the vhosts stored in the database.",
	"For detailed information on a vhost see \2HELP VIEW\2",
	"20 vhosts are displayed at a time. To view other vhosts the",
	"<startpos> parameter allows listing from that position.",
	NULL
};

const char *hs_help_listwild[] = {
	"List vhosts",
	"Syntax: \2LISTWILD <NICK|HOST|VHOST> <limit>\2",
	"",
	"Lists the vhosts stored in the database of the requested.",
	"type matching the wildcard <limit>.",
	NULL
};

const char *hs_help_login[] = {
	"Login to HostServ",
	"Syntax: \2LOGIN <NICK> <PASSWORD>\2",
	"",
	"Sets your vhost if it was not set when you connect to IRC,",
	"or you are connecting from a different host, or share a",
	"vhost with other users.",
	NULL
};

const char *hs_help_chpass[] = {
	"Change password for a vhost",
	"Syntax: \2CHPASS <NICK> <OLDPASS> <NEWPASS>\2",
	"",
	"Change the password for your vhost.",
	NULL
};

const char *hs_help_bans[] = {
	"List banned vhosts",
	"Syntax: \2BANS LIST\2",
	"        \2BANS ADD <hostname> <reason>\2",
	"        \2BANS DEL <hostname>\2",
	"",
	"Maintain the list of banned vhosts.",
	"",
	"\2LIST\2 list banned vhosts.",
	"",
	"\2ADD\2 add a banned vhost.",
	"Wildcards, like *host* are permitted.",
	"",
	"\2DEL\2 delete a banned vhost.",
	"hostname must match the one used to add the vhost.",
	NULL
};

const char *hs_help_set_expire[] = {
	"\2EXPIRE <TIME>\2",
	"How long before unused HostServ entries should be deleted",
	"A value of 0 makes all vhosts permanent",
	NULL
};

const char *hs_help_set_hiddenhost[] = {
	"\2HIDDENHOST <ON/OFF>\2",
	"Undernet style hidden hosts set when users identify to",
	"nickserv using HOSTNAME",
	NULL
};

const char *hs_help_set_hostname[] = {
	"\2HOSTNAME <hostname>\2",
	"Hidden host to set on users in the style nick.<hostname>.",
	NULL
};

const char *hs_help_set_operhosts[] = {
	"\2OPERHOSTS <ON/OFF>\2",
	"Whether HostServ will set oper vhosts or not. Useful if",
	"your IRCd does not provide oper hosts.",
	NULL
};

const char *hs_help_set_verbose[] = {
	"\2VERBOSE <ON/OFF>\2",
	"Control HostServ verbosity",
	NULL
};

const char *hs_help_set_addlevel[] = {
	"\2ADDLEVEL <level>\2",
	"Restricts the adding of VHOSTS for nicks,",
	"other than the nick currently in use,",
	"to this level or above",
	NULL
};
