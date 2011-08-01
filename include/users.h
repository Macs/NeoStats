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
** $Id: users.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _USERS_H_
#define _USERS_H_

int InitUsers( void );
void FiniUsers( void );
Client *AddUser( const char *nick, const char *user, const char *host, const char *realname, const char *server, const char *ip, const char *TS, const char *numeric );
void KillUser( const char *source, const char *nick, const char *reason );
void QuitUser( const char *nick, const char *reason );
void SetUserVhost( const char *nick, const char *vhost );
void SetUserServicesTS( const char *nick, const char *ts );
void SetUserVersion( Client *client, const char *version );
void UserNickChange( const char *oldnick, const char *newnick, const char *ts );
void UserMode( const char *nick, const char *modes );
void UserSMode( const char *nick, const char *modes );
void UserAway( const char *nick, const char *awaymsg );
Client *find_user_base64( const char *num );
int ns_cmd_userlist( const CmdParams *cmdparams );
void QuitServerUsers( Client *s );
EXPORTFUNC void AddFakeUser( const char *mask );
EXPORTFUNC void DelFakeUser( const char *mask );
void CleanupUserModdata( void );
void SyncServerClients(Client *s);

#endif /* _USERS_H_ */
