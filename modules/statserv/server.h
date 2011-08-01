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
** $Id: server.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _SERVER_H_
#define _SERVER_H_

typedef struct serverstat {
	char name[MAXHOST];
	Client *s;
	time_t ts_start;
	time_t ts_lastseen;
	statistic users;
	statistic opers;
	statistic operkills;
	statistic serverkills;
	statistic splits;
	time_t lowest_ping;
	time_t ts_lowest_ping;
	time_t highest_ping;
	time_t ts_highest_ping;
}serverstat;

typedef void( *ServerStatHandler )( const serverstat *cs, const void *v );
void GetServerStats( const ServerStatHandler handler, const void *v );

int ss_event_server( const CmdParams *cmdparams );
int ss_event_squit( const CmdParams *cmdparams );
int ss_event_pong( const CmdParams *cmdparams );
int ss_cmd_map( const CmdParams *cmdparams );
int ss_cmd_server( const CmdParams *cmdparams );
int InitServerStats( void );
void FiniServerStats( void );
void SaveServerStats( void );
void AddServerUser( const Client *u );
void DelServerUser( const Client *u );
void AddServerOper( const Client *u );
void DelServerOper( const Client *u );
void AverageServerStatistics( void );
void ResetServerStatistics( void );

#endif /* _SERVER_H_ */
