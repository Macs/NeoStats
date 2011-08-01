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
** $Id: helpstrings.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _HELPSTRINGS_H_
#define _HELPSTRINGS_H_

/* ns.c */
extern const char *ns_help_shutdown[];
extern const char *ns_help_reload[];
extern const char *ns_help_exclude[];
#ifdef USE_RAW
extern const char *ns_help_raw[];
#endif /* USE_RAW */
extern const char *ns_help_userlist[];
extern const char *ns_help_channellist[];
extern const char *ns_help_serverlist[];
extern const char *ns_help_banlist[];
extern const char *ns_help_load[];
extern const char *ns_help_unload[];
extern const char *ns_help_modlist[];
extern const char *ns_help_jupe[];
extern const char *ns_help_level[];
extern const char *ns_help_botlist[];
extern const char *ns_help_socklist[];
extern const char *ns_help_timerlist[];
extern const char *ns_help_status[];
extern const char *ns_help_lookup[];

extern const char *ns_help_set_nick[];
extern const char *ns_help_set_altnick[];
extern const char *ns_help_set_user[];
extern const char *ns_help_set_host[];
extern const char *ns_help_set_realname[];
extern const char *ns_help_set_joinserviceschan[];
extern const char *ns_help_set_splittime[];
extern const char *ns_help_set_msgsampletime[];
extern const char *ns_help_set_msgthreshold[];
extern const char *ns_help_set_pingtime[];
extern const char *ns_help_set_servicecmode[];
extern const char *ns_help_set_serviceumode[];
extern const char *ns_help_set_loglevel[];
extern const char *ns_help_set_cmdchar[];
extern const char *ns_help_set_cmdreport[];
extern const char *ns_help_set_debug[];
extern const char *ns_help_set_debuglevel[];
extern const char *ns_help_set_debugchan[];
extern const char *ns_help_set_debugtochan[];
extern const char *ns_help_set_debugmodule[];
extern const char *ns_help_set_recvq[];
extern const char *ns_help_set_sendhelp[];
extern const char *ns_help_set_allhelp[];

extern const char *cmd_help_help[];
extern const char *cmd_help_about[];
extern const char *cmd_help_credits[];
extern const char *cmd_help_version[];
extern const char *cmd_help_levels[];
extern const char *cmd_help_set[];

#endif /* _HELPSTRINGS_H_ */
