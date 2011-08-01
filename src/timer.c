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
** $Id: timer.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "servers.h"
#include "services.h"
#include "modules.h"
#include "log.h"
#include "timer.h"
#include "event.h"
#include "main.h"

#define TIMER_TABLE_SIZE	300	/* Number of Timers */

/* @brief Module Timer hash list */
static list_t *timerlist;
static struct event *evtimers;

static void TimerCalcNextRun(Timer *timer, lnode_t *node);
static void NextSchedule();
/** @brief InitTimers
 *
 *  Init timer subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitTimers( void )
{
	timerlist = list_create( TIMER_TABLE_SIZE);
	if( !timerlist )
	{
		nlog( LOG_CRITICAL, "Unable to create timer hash" );
		return NS_FAILURE;
	}
	evtimers = ns_malloc( sizeof( struct event ) );
	evtimer_set( evtimers, CheckTimers_cb, NULL );
	AddTimer(TIMER_TYPE_INTERVAL, PingServers, "PingServers", nsconfig.pingtime, NULL);
	AddTimer(TIMER_TYPE_INTERVAL, FlushLogs, "FlushLogs", nsconfig.pingtime, NULL);
	if (nsconfig.setservertimes > 0) 
		AddTimer(TIMER_TYPE_INTERVAL, SetServersTime, "SetServersTime", nsconfig.setservertimes, NULL);
	AddTimer(TIMER_TYPE_DAILY, ResetLogs, "ResetLogs", 0, NULL);
	return NS_SUCCESS;
}

/** @brief FiniTimers
 *
 *  Fini timer subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */

void FiniTimers( void )
{
	lnode_t *tn;
	Timer *timer;
	
	evtimer_del( evtimers );
	os_free( evtimers );
	DelTimer("PingServers");
	DelTimer("FlushLogs");
	if (nsconfig.setservertimes > 0) 
		DelTimer("SetServersTime");
	DelTimer("ResetLogs");

	if (list_count(timerlist) > 0) {
		tn = list_first(timerlist);
		while( tn != NULL )
		{
			timer = lnode_get( tn );
			dlog( DEBUG3, "FiniTimers() BUG: Timer %s not deleted for module %s", timer->name, timer->moduleptr->info->name );
			tn = list_next(timerlist, tn);
		}
	}
	list_destroy( timerlist );
}

/** @brief CheckTimers_cb
 *
 *  CheckTimers_cb
 *  NeoStats core use only.
 *
 *  @param notused Justin????
 *  @param event Justin????
 *  @param arg Justin????
 *
 *  @return none
 */

void CheckTimers_cb( int notused, short event, void *arg )
{
	Timer *timer = NULL;
	lnode_t *tn;

	SET_SEGV_LOCATION();

	update_time_now();
	/* We only run Timers one at a time */
	tn = list_first(timerlist);
	timer = lnode_get( tn );
	/* If a module is not yet synched, reset it's lastrun */
	if( !IsModuleSynched( timer->moduleptr ) ) {
		timer->lastrun = ( int ) me.now;
	} else if (timer->nextrun <= me.now) {
		if( setjmp( sigvbuf ) == 0 ) {
			dlog( DEBUG10, "run_mod_timers: Running timer %s for module %s", timer->name, timer->moduleptr->info->name );
			SET_RUN_LEVEL( timer->moduleptr );
			if( timer->handler( timer->userptr ) < 0 ) {
				dlog( DEBUG2, "run_mod_timers: Deleting Timer %s for Module %s as requested", timer->name, timer->moduleptr->info->name );
				list_delete_destroy_node(timerlist, tn);
				ns_free( timer );
				NextSchedule();
			} else {
				timer->lastrun = ( int ) me.now;
				TimerCalcNextRun(timer, tn);
			}
			RESET_RUN_LEVEL();
#if 0
			if( timer->type == TIMER_TYPE_COUNTDOWN ) {
					hash_scan_delete_destroy_node( timerlist, tn );
					ns_free( timer );
			}
#endif
		} else {
			nlog( LOG_CRITICAL, "run_mod_timers: setjmp() failed, can't call module %s", timer->moduleptr->info->name );
			NextSchedule();
		}
	}
}

/** @brief create new new_timer
 *
 *  For core use only, creates a timer
 *
 *  @param name of new timer
 * 
 *  @return pointer to new timer on success, NULL on error
 */

static Timer *new_timer( const char *name )
{
	Timer *timer;

	SET_SEGV_LOCATION();
	if( list_isfull( timerlist ) )
	{
		nlog( LOG_WARNING, "new_timer: timer hash is full" );
		return NULL;
	}
	dlog( DEBUG2, "new_timer: %s", name );
	timer = ns_calloc( sizeof( Timer ) );
	strlcpy( timer->name, name, MAX_MOD_NAME );
	return timer;
}

/** @brief FindTimer
 *
 *  Finds a timer in the current list of timers
 *
 *  @param name the name of timer to find
 * 
 *  @return pointer to timer if found, NULL if not found
 */

Timer *FindTimer( const char *name )
{
	lnode_t *tn;
	Timer *t;

	tn = list_first(timerlist);
	while (tn != NULL) {
		t = lnode_get(tn);
		if (!ircstrcasecmp(t->name, name))
			return t;
		tn = list_next(timerlist, tn);
	}
	dlog( DEBUG3, "FindTimer: %s not found", name );
	return NULL;
}

static void TimerCalcNextRun(Timer *timer, lnode_t *node) {
	struct tm *newtime;
	Timer *CurTimer, *NextTimer;
	lnode_t *CurNode, *NextNode;

	newtime = localtime(&timer->lastrun);
	switch(timer->type) {
		case TIMER_TYPE_DAILY:
			newtime->tm_hour = 0;
			newtime->tm_min = 0;
			newtime->tm_sec = 0;
			newtime->tm_mday += 1;
			break;
		case TIMER_TYPE_WEEKLY:
			/* weekly and monthly timers can run at any time */
			newtime->tm_mday += 7;
			break;
		case TIMER_TYPE_MONTHLY:
			newtime->tm_mon += 1;
			break;
		case TIMER_TYPE_INTERVAL:
			newtime->tm_sec += timer->interval;
			break;
		case TIMER_TYPE_COUNTDOWN:
#if 0
					if( me.now - timer->lastrun < timer->interval )
					{
						timer->interval -= ( me.now - timer->lastrun );
						timer->lastrun = me.now;
 						continue;
					}
#endif
			newtime->tm_sec += timer->interval;
			break;
	}	
	timer->nextrun = mktime(newtime);
	if (list_count(timerlist) == 0) {
		/* the list is empty, insert at the top */
		node = lnode_create_prepend(timerlist, timer);
		return;
	} else {
		if (!node) 
			node = lnode_create(timer);
		else 
			list_delete(timerlist, node);
	}		
	/* now move though the timer list and insert this at the right order */
	CurNode = list_first(timerlist);
	NextNode = list_next(timerlist, CurNode);
	while (CurNode != NULL) {
		if (CurNode) CurTimer = lnode_get(CurNode);
		if (NextNode) {
			/* if the NextNode is NULL, we are at the end of the list already */
			NextTimer = lnode_get(NextNode);
		} else {
			/* if the timer is after the CurNode, then */
			if (CurTimer->nextrun < timer->nextrun) 
				/* insert it afterwards */
				list_ins_after(timerlist, node, CurNode);
			else
				/* else insert it before */
				list_ins_before(timerlist, node, CurNode);
			/* and exit the while loop */
			break;
		}
		/* if the Curent Timer is going to run before this one */
		if (CurTimer->nextrun < timer->nextrun) {
			/* and the next timer is also going to run before this one */
			if (NextTimer->nextrun < timer->nextrun) {
				/* then Swap CurNode for NextNode */
				CurNode = NextNode;
				/* and get a new NextNode */
				NextNode = list_next(timerlist, CurNode);
			} else {
				/* otherwise insert it after the NextNode */
				list_ins_after(timerlist, node, CurNode);
				break;
			}
		} else {
			/* its before CurTimer, so insert it beforehand */
			list_ins_before(timerlist, node, CurNode);
			break;
		}
	}	
	NextSchedule();
}
static void NextSchedule() {
	lnode_t *CurNode;
	Timer *CurTimer;
	struct timeval tv;
	/* finally, update the Event Based Timers */
	update_time_now();
	CurNode = list_first(timerlist);
	if (CurNode) {
		CurTimer = lnode_get(CurNode);
		timerclear( &tv );
		tv.tv_sec = CurTimer->nextrun - me.now;;
		if (tv.tv_sec <= 0) {
			/* only run one timer at a time, so wake up in 10 seconds */
			tv.tv_sec = 10;
		}
		evtimer_add( evtimers, &tv );
		dlog(DEBUG3, "Timers will next run in %d Seconds (or around %s)", (int)tv.tv_sec, sftime(CurTimer->nextrun));
	} 
}

/** @brief AddTimer
 *
 *  For module use. Adds a timer with the given function to the timer list
 *
 *  @param type of timer
 *  @param handler for timer
 *  @param name of timer
 *  @param interval the interval at which the timer triggers in seconds
 *  @param userptr User Baton to pass around.
 * 
 *  @return NS_SUCCESS if added, NS_FAILURE if not 
 */

int AddTimer( TIMER_TYPE type, timer_handler handler, const char *name, int interval, void *userptr )
{
	Timer *timer;
	Module *moduleptr;

	SET_SEGV_LOCATION();
	moduleptr = GET_CUR_MODULE();
	if( handler == NULL )
	{
		nlog( LOG_WARNING, "Module %s timer %s does not exist", moduleptr->info->name, name );
		return NS_FAILURE;
	}
	if( FindTimer( name ) )
	{
		nlog( LOG_WARNING, "Module %s timer %s already exists. Not adding.", moduleptr->info->name, name );
		return NS_FAILURE;
	}
	timer = new_timer( name );
	if( timer )
	{
		timer->type = type;
		timer->interval = interval;
		timer->lastrun = me.now;
		timer->moduleptr = moduleptr;
		timer->handler = handler;
		timer->userptr = userptr;
		TimerCalcNextRun(timer, NULL);
		dlog( DEBUG2, "AddTimer: Module %s added timer %s", moduleptr->info->name, name );
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief DelTimer
 *
 *  Deletes a timer with the given name from the timer list
 *  For module use.
 *
 *  @param name the name of timer to delete
 * 
 *  @return NS_SUCCESS if deleted, NS_FAILURE if not found
 */

int DelTimer( const char *name )
{
	Timer *timer;
	lnode_t *tn;

	SET_SEGV_LOCATION();
	tn = list_first(timerlist);
	while (tn != NULL) {
		timer = lnode_get(tn);
		if (!ircstrcasecmp(timer->name, name)) {
			dlog( DEBUG2, "DelTimer: removed timer %s for module %s", name, timer->moduleptr->info->name );
			list_delete_destroy_node(timerlist, tn);
#ifdef USE_PERL
			if( IS_PERL_MOD( timer->moduleptr ) )
				ns_free( timer->userptr );
#endif /* USE_PERL */
			ns_free( timer );
			return NS_SUCCESS;
		}
		tn = list_next(timerlist, tn);
	}
	dlog(DEBUG2, "DelTimer: Timer %s not found", name);
	return NS_FAILURE;
}

/** @brief del_timers
 *
 *  delete all timers from the timer list for given module
 *  For core use. 
 *
 *  @param mod_ptr pointer to module to delete timers from
 * 
 *  @return NS_SUCCESS if deleted, NS_FAILURE if not found
 */

int del_timers( const Module *mod_ptr )
{
	Timer *timer;
	lnode_t *tn, *tn2;

	tn = list_first(timerlist);
	while( tn != NULL )
	{
		tn2 = list_next(timerlist, tn);
		timer = lnode_get( tn );
		if( timer->moduleptr == mod_ptr )
		{
			dlog( DEBUG1, "del_timers: deleting timer %s from module %s.", timer->name, mod_ptr->info->name );
			list_delete_destroy_node(timerlist, tn);
#ifdef USE_PERL
			if( IS_PERL_MOD( timer->moduleptr ) )
				ns_free( timer->userptr );
#endif /* USE_PERL */
			ns_free( timer );
		}
		tn = tn2;
	}
	return NS_SUCCESS;
}

/** @brief SetTimerInterval
 *
 *  For module use. Sets interval for timer
 *
 *  @param name of timer to set
 *  @param interval to set
 * 
 *  @return NS_SUCCESS if deleted, NS_FAILURE if not found
 */

int SetTimerInterval( const char *name, int interval )
{
	Timer *timer;
	lnode_t *tn;

	SET_SEGV_LOCATION();
	tn = list_first(timerlist);
	while (tn != NULL) {
		timer = lnode_get(tn);
		if (!ircstrcasecmp(timer->name, name)) {
			timer->interval = interval;
			dlog( DEBUG2, "SetTimerInterval: timer interval for %s (%s) set to %d", name, timer->moduleptr->info->name, interval );
			TimerCalcNextRun(timer, tn);
			return NS_SUCCESS;
		}
		tn = list_next(timerlist, tn);	
	}
	dlog(DEBUG2, "SetTimerInterval: timer %s not found", name);
	return NS_FAILURE;
}

/** @brief ns_cmd_timerlist
 *
 *  NeoStats command to list the current timers from IRC
 *
 *  @param cmdparams
 * 
 *  @return none
*/

int ns_cmd_timerlist( const CmdParams *cmdparams )
{
	Timer *timer = NULL;
	lnode_t *tn;

	SET_SEGV_LOCATION();
	irc_prefmsg( ns_botptr, cmdparams->source, __( "Timer List:", cmdparams->source ) );
	tn = list_first(timerlist);
	while( tn != NULL )
	{
		timer = lnode_get( tn );
		irc_prefmsg( ns_botptr, cmdparams->source, "%s:", timer->moduleptr->info->name );
		irc_prefmsg( ns_botptr, cmdparams->source, __( "Timer: %s", cmdparams->source ), timer->name );
		switch( timer->type )
		{
			case TIMER_TYPE_DAILY:
				irc_prefmsg( ns_botptr, cmdparams->source, "Timer Type: Daily");
				break;
			case TIMER_TYPE_WEEKLY:
				irc_prefmsg( ns_botptr, cmdparams->source, "Timer Type: Weekly");
				break;
			case TIMER_TYPE_MONTHLY:
				irc_prefmsg( ns_botptr, cmdparams->source, "Timer Type: Monthly");
				break;
			case TIMER_TYPE_INTERVAL:
				irc_prefmsg( ns_botptr, cmdparams->source, "Timer Type: Interval");
				irc_prefmsg( ns_botptr, cmdparams->source, __( "Interval: %ld", cmdparams->source ), ( long )timer->interval );
				break;
			case TIMER_TYPE_COUNTDOWN:
				irc_prefmsg( ns_botptr, cmdparams->source, "Timer Type: Countdown");
				irc_prefmsg( ns_botptr, cmdparams->source, __( "Interval: %ld", cmdparams->source ), ( long )timer->interval );
				break;
		}
		irc_prefmsg( ns_botptr, cmdparams->source, __( "Next run at: %s", cmdparams->source ), sftime(timer->nextrun));
		irc_prefmsg( ns_botptr, cmdparams->source, __( "Last run at: %s", cmdparams->source ), sftime(timer->lastrun));
		tn = list_next(timerlist, tn);
	}
	irc_prefmsg( ns_botptr, cmdparams->source, __( "End of list.", cmdparams->source ) );
	return 0;
}
