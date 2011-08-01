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
** $Id: servers.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _SERVER_H_
#define _SERVER_H_

Client *AddServer( const char *name, const char *uplink, const char *hops, const char *numeric, const char *infoline );
void DelServer( const char *name, const char *reason );
int ns_cmd_serverlist( const CmdParams *cmdparams );
int InitServers( void );
int PingServers( void *);
int SetServersTime( void *arg);
void FiniServers( void );
Client *find_server_base64( const char *num );
void RequestServerUptimes( void );
void CleanupServerModdata( void );
void SyncServer( const char *name);

#endif /* _SERVER_H_ */
