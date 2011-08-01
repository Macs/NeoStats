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
** $Id: ircprotocol.h 3294 2008-02-24 02:45:41Z Fish $
*/
#ifndef _IRCPROTOCOL_H_
#define _IRCPROTOCOL_H_

int InitIrcd( void );
void FiniIrcd( void );
int irc_connect( const char *name, const int numeric, const char *infoline, const char *pass, const time_t tsboot, const time_t tslink );
int irc_nick( const char *nick, const char *user, const char *host, const char *realname, const char *modes );
int irc_server( const char *name, const int numeric, const char *infoline );
int irc_squit( const char *server, const char *quitmsg );
/*int snetinfo_cmd( void );*/
/*int ssvinfo_cmd( void );*/
/*int sburst_cmd( int b );*/
/*int seob_cmd( const char *server );*/
int irc_smo( const char *source, const char *umodetarget, const char *msg );

extern int (*irc_parse) (void *notused, void *rline, int len);

#endif /* _IRCPROTOCOL_H_ */
