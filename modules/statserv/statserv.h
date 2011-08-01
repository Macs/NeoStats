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
** $Id: statserv.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _STATSERV_H_
#define _STATSERV_H_

/* this is the how often to save a portion of the DB. Don't alter this unless you need to */
/* DO NOT set PROGCHANTIME less than ((DBSAVETIME + (DBSAVETIME/2)) * 4) otherwise you will not have the enitre database progressively saved! */

/* try a save every 10 minutes */
#define DBSAVETIME 600
/* but only save data older than 1 hour! */
#define PROGCHANTIME TS_ONE_HOUR

extern Bot *statbot;

typedef struct tStatServ { 
	int lagtime; 
	int lagalert; 
	int recordalert; 
	int html; 
	char htmlpath[MAXPATH]; 
	int htmltime; 
	int channeltime;
	int msginterval; 
	int msglimit; 
	int exclusions; 
	int flatmap; 
} tStatServ;

extern tStatServ StatServ;

/* ss_help.c */
extern const char *ss_about[];
extern const char *ss_help_server[];
extern const char *ss_help_map[];
extern const char *ss_help_netstats[];
extern const char *ss_help_daily[];
extern const char *ss_help_tldmap[];
extern const char *ss_help_operlist[];
extern const char *ss_help_botlist[];
extern const char *ss_help_htmlstats[];
extern const char *ss_help_forcehtml[];
extern const char *ss_help_channel[];
extern const char *ss_help_set_htmlpath[];
extern const char *ss_help_set_html[];
extern const char *ss_help_set_exclusions[];
extern const char *ss_help_set_flatmap[];
extern const char *ss_help_set_msginterval[];
extern const char *ss_help_set_msglimit[];
extern const char *ss_help_set_lagtime[];
extern const char *ss_help_set_htmltime[];
extern const char *ss_help_set_channeltime[];
extern const char *ss_help_set_lagalert[];
extern const char *ss_help_set_recordalert[];
extern const char *ss_help_ctcpversion[];

void announce_record (const char *msg, ...);
void announce_lag(const char *msg, ...);

#endif /* _STATSERV_H_ */
