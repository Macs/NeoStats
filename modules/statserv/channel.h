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
** $Id: channel.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _CHANNEL_H_
#define _CHANNEL_H_

typedef enum CHANNEL_SORT 
{
	CHANNEL_SORT_NONE = 0,
	CHANNEL_SORT_MEMBERS,
	CHANNEL_SORT_JOINS,
	CHANNEL_SORT_KICKS,
	CHANNEL_SORT_TOPICS
} CHANNEL_SORT;

typedef struct channelstat 
{
	char name[MAXCHANLEN];
	Channel *c;
	time_t ts_start;
	time_t ts_lastseen;
	time_t lastsave;
	statistic users;
	statistic kicks;
	statistic topics;
	statistic joins;
} channelstat;

typedef void (*ChannelStatHandler)( channelstat *cs, const void *v );

void GetChannelStats( const ChannelStatHandler handler, CHANNEL_SORT sortstyle, int limit, int ignorehidden, const void *v );
int ss_event_newchan( const CmdParams *cmdparams );
int ss_event_delchan( const CmdParams *cmdparams );
int ss_event_join( const CmdParams *cmdparams );
int ss_event_part( const CmdParams *cmdparams );
int ss_event_topic( const CmdParams *cmdparams );
int ss_event_kick( const CmdParams *cmdparams );
int ss_cmd_channel( const CmdParams *cmdparams );
int DelOldChanTimer( void *v );
int InitChannelStats( void );
void FiniChannelStats( void );
void SaveChanStats( void );
void SaveChanStatsProgressive( void );
void ResetChannelStatistics( void );
void AverageChannelStatistics( void );

#endif /* _CHANNEL_H_ */
