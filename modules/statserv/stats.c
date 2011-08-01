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
** $Id: stats.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "statserv.h"
#include "stats.h"
#include "network.h"
#include "channel.h"
#include "server.h"
#include "version.h"
#include "tld.h"

/** macro to calculate week number from time */
#define WEEKNUM( t )( ( ( t )->tm_yday + 7 -( ( t )->tm_wday ) ) / 7 )

/** @brief PreSaveStatistic
 *
 *  Update statistic time prior to save
 *
 *  @param stat pointer to stat to update
 *
 *  @return none
 */

void PreSaveStatistic( statistic *stat )
{
	struct tm *ltm;

	ltm = localtime( &me.now );
	stat->month = ltm->tm_mon;
	stat->week = WEEKNUM( ltm );
	stat->day = ltm->tm_yday;	
}

/** @brief PostLoadStatistic
 *
 *  Update statistic based on time post load
 *
 *  @param stat pointer to stat to update
 *
 *  @return none
 */

void PostLoadStatistic( statistic *stat )
{
	struct tm *ltm;

	ltm = localtime( &me.now );
	stat->current = 0;
	if( stat->day != ltm->tm_yday )
		ResetStatisticEntry( &stat->daily, stat->current );
	/* Only load weekly stats if week not changed */
	if( stat->week != WEEKNUM( ltm ) )
		ResetStatisticEntry( &stat->weekly, stat->current );
	/* Only load monthly stats if month not changed */
	if( stat->month != ltm->tm_mon )
		ResetStatisticEntry( &stat->monthly, stat->current );
}

/** @brief AverageStatisticEntry
 *
 *  Average statistic entry
 *
 *  @param stat pointer to stat to update
 *  @param current value on network
 *
 *  @return none
 */

void AverageStatisticEntry( statisticentry *stat, unsigned int current )
{
	stat->average = ( stat->max + current ) / 2;
}

/** @brief AverageStatistic
 *
 *  Average statistic
 *
 *  @param stat pointer to stat to update
 *
 *  @return none
 */

void AverageStatistic( statistic *stat )
{
	AverageStatisticEntry( &stat->daily, stat->current );
	AverageStatisticEntry( &stat->weekly, stat->current );
	AverageStatisticEntry( &stat->monthly, stat->current );
	AverageStatisticEntry( &stat->alltime, stat->current );
}

/** @brief ResetStatisticEntry
 *
 *  Reset statistic entry
 *
 *  @param stat pointer to stat to update
 *  @param current value on network
 *
 *  @return none
 */

void ResetStatisticEntry( statisticentry *stat, unsigned int current )
{
	stat->runningtotal = current;
	stat->max = current;
	stat->ts_max = me.now;
}

/** @brief ResetStatistic
 *
 *  Reset statistic
 *
 *  @param stat pointer to stat to update
 *  @param current value on network
 *
 *  @return none
 */

void ResetStatistic( statistic *stat )
{
	struct tm *ts = localtime( &me.now );

	/* Reset daily on first hour of day( i.e. midnight ) */
	if( ts->tm_hour == 0 )
		ResetStatisticEntry( &stat->daily, stat->current );
	/* Reset weekly on first day of week */
	if( ts->tm_wday == 0 )
		ResetStatisticEntry( &stat->weekly, stat->current );
	/* Reset monthly on first day of month */
	if( ts->tm_mday == 1 )
		ResetStatisticEntry( &stat->monthly, stat->current );
}

/** @brief IncStatisticEntry
 *
 *  increment statistic entry
 *
 *  @param stat pointer to stat to update
 *  @param current value on network
 *
 *  @return none
 */

int IncStatisticEntry( statisticentry *stat, unsigned int current )
{
	int isrecord = 0;

	if( current > stat->max )
	{
		stat->max = current;
		stat->ts_max = me.now;
		isrecord = 1;
	}
	else if( current == stat->max )
	{
		stat->ts_max = me.now;
	}
	stat->runningtotal++;
	return isrecord;
}

/** @brief IncStatistic
 *
 *  increment statistic
 *
 *  @param stat pointer to stat to update
 *
 *  @return none
 */

int IncStatistic( statistic *stat )
{
	stat->current++;
	IncStatisticEntry( &stat->daily, stat->current );
	IncStatisticEntry( &stat->weekly, stat->current );
	IncStatisticEntry( &stat->monthly, stat->current );
	return IncStatisticEntry( &stat->alltime, stat->current );
}

/** @brief DecStatistic
 *
 *  decrement statistic
 *
 *  @param stat pointer to stat to update
 *
 *  @return none
 */

void DecStatistic( statistic *stat )
{
	if( stat->current > 0 )
		stat->current--;
}

/** @brief SetStatisticEntry
 *
 *  set statistic entry
 *
 *  @param stat pointer to stat to update
 *  @param current value on network
 *  @param diff between stored and current
 *
 *  @return none
 */

int SetStatisticEntry( statisticentry *stat, unsigned int current, int diff )
{
	int isrecord = 0;

	if( current > stat->max )
	{
		stat->max = current;
		stat->ts_max = me.now;
		isrecord = 1;
	}
	else if( current == stat->max )
	{
		stat->ts_max = me.now;
	}
	/* Only adjust running total if diff is > 0 */
	if( diff > 0 )
		stat->runningtotal += diff;
	return isrecord;
}

/** @brief SetStatistic
 *
 *  set statistic
 *
 *  @param stat pointer to stat to update
 *  @param current value on network
 *
 *  @return none
 */

int SetStatistic( statistic *stat, int current )
{
	int diff = 0;

	if( current != stat->current )
	{
		/* Subtract this way so that diff is the correct sign for a 
		 * later add to running total. I.e.
		 *   current >  stat->current : +ve
		 *   current == stat->current : 0
		 *   current <  stat->current : -ve
		 */
		diff = current - stat->current;
		stat->current = current;
	}
	SetStatisticEntry( &stat->daily, stat->current, diff );
	SetStatisticEntry( &stat->weekly, stat->current, diff );
	SetStatisticEntry( &stat->monthly, stat->current, diff );
	return SetStatisticEntry( &stat->alltime, stat->current, diff );
}

/** @brief check_interval
 *
 *  check interval between announcements
 *
 *  @param none
 *
 *  @return NS_TRUE if OK else NS_FALSE
 */

static int check_interval( void )
{
	static time_t lasttime;
	static int count;

	if( !ModuleSynched() )
		return NS_FALSE;
	if( ( me.now - lasttime ) < StatServ.msginterval  )
	{
		if( ++count > StatServ.msglimit )
			return NS_FALSE;
	}
	else
	{
		lasttime = me.now;
		count = 0;
	}
	return NS_TRUE;
}

/** @brief announce
 *
 *  Make announcement of selected type
 *
 *  @param announcetype type of announcement
 *  @param msg to send
 *
 *  @return none
 */

static void announce( int announcetype, const char *msg )
{
	switch( announcetype )
	{
		case 3:
			irc_wallops( statbot, "%s", msg );
			break;
		case 2:
			irc_globops( statbot, "%s", msg );
			break;
		case 1:
		default:
			irc_chanalert( statbot, "%s", msg );
			break;
	}
}

/** @brief announce_record
 *
 *  Make announcement
 *
 *  @param msg to send
 *
 *  @return none
 */

void announce_record( const char *msg, ... )
{
	static char announce_buf[BUFSIZE];
	va_list ap;

	if( StatServ.recordalert <= 0 || check_interval() < 0 )
		return;
	va_start( ap, msg );
	ircvsnprintf( announce_buf, BUFSIZE, msg, ap );
	va_end( ap );
	announce( StatServ.recordalert, announce_buf );
}

/** @brief announce_lag
 *
 *  Make announcement
 *
 *  @param msg to send
 *
 *  @return none
 */

void announce_lag( const char *msg, ... )
{
	static char announce_buf[BUFSIZE];
	va_list ap;

	if( StatServ.lagalert <= 0 || check_interval() < 0 )
		return;
	va_start( ap, msg );
	ircvsnprintf( announce_buf, BUFSIZE, msg, ap );
	va_end( ap );
	announce( StatServ.lagalert, announce_buf );
}

/** @brief ResetStatisticsTimer
 *
 *  Timer handler for rest of statistics
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ResetStatisticsTimer( void *userptr )
{
	SET_SEGV_LOCATION();
	dlog( DEBUG1, "Reset Statistics" );
	ResetNetworkStatistics();
	ResetServerStatistics();
	ResetChannelStatistics();
	ResetTLDStatistics();
	return NS_SUCCESS;
}

/** @brief AverageStatistics
 *
 *  Average of statistics
 *
 *  @param none
 *
 *  @return none
 */

void AverageStatistics( void )
{
	SET_SEGV_LOCATION();
	dlog( DEBUG1, "Average Statistics" );
	AverageNetworkStatistics();
	AverageServerStatistics();
	AverageChannelStatistics();
	AverageTLDStatistics();
}

/** @brief AverageStatisticsTimer
 *
 *  Timer handler for average of statistics
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int AverageStatisticsTimer( void *userptr )
{
	SET_SEGV_LOCATION();
	AverageStatistics();
	return NS_SUCCESS;
}

/** @brief GetAllTimePercent
 *
 *  Generate all time stat percent
 *
 *  @param stat pointer to stat to calculate
 *
 *  @return percent
 */

int GetAllTimePercent( const statistic *stat )
{
	return( int )( ( ( float ) stat->current / ( float ) stat->alltime.max ) * 100 );
}

/** @brief GetDailyPercent
 *
 *  Generate daily percent
 *
 *  @param stat pointer to stat to calculate
 *
 *  @return percent
 */

int GetDailyPercent( const statistic *stat )
{
	return( int )( ( ( float ) stat->current / ( float ) stat->daily.max ) * 100 );
}

/** @brief GetWeeklyPercent
 *
 *  Generate weekly stat percent
 *
 *  @param stat pointer to stat to calculate
 *
 *  @return percent
 */

int GetWeeklyPercent( const statistic *stat )
{
	return( int )( ( ( float ) stat->current / ( float ) stat->weekly.max ) * 100 );
}

/** @brief GetMonthlyPercent
 *
 *  Generate monthly stat percent
 *
 *  @param stat pointer to stat to calculate
 *
 *  @return percent
 */

int GetMonthlyPercent( const statistic *stat )
{
	return( int )( ( ( float ) stat->current / ( float ) stat->monthly.max ) * 100 );
}
