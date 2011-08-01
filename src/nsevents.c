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
** $Id: nsevents.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "nsevents.h"
#include "modules.h"
#ifdef USE_PERL
#undef _
#define PERLDEFINES
#include "perlmod.h"
#endif /* USE_PERL */

typedef struct ModuleAllEvent
{
	Event event;
	CmdParams* cmdparams;
} ModuleAllEvent;

/** String descriptions of events for debug use.
  * must match enum in events.h
  */
static const char *EventStrings[] =
{
	"EVENT_MODULELOAD",
	"EVENT_MODULEUNLOAD",
	"EVENT_SERVER",
	"EVENT_SQUIT",
	"EVENT_PING",
	"EVENT_PONG",
	"EVENT_SIGNON",
	"EVENT_QUIT",
	"EVENT_NICKIP",
	"EVENT_KILL",
	"EVENT_LOCALKILL",
	"EVENT_GLOBALKILL",
	"EVENT_SERVERKILL",
	"EVENT_BOTKILL",
	"EVENT_NICK",
	"EVENT_AWAY",
	"EVENT_UMODE",
	"EVENT_SMODE",
	"EVENT_NEWCHAN",
	"EVENT_DELCHAN",
	"EVENT_JOIN",
	"EVENT_PART",
	"EVENT_PARTBOT",
	"EVENT_EMPTYCHAN",
	"EVENT_KICK",
	"EVENT_KICKBOT",
	"EVENT_TOPIC",
	"EVENT_CMODE",
	"EVENT_PRIVATE",
	"EVENT_NOTICE",
	"EVENT_CPRIVATE",
	"EVENT_CNOTICE",
	"EVENT_GLOBOPS",
	"EVENT_CHATOPS",
	"EVENT_WALLOPS",
	"EVENT_CTCPVERSIONRPL",
	"EVENT_CTCPVERSIONRPLBC",
	"EVENT_CTCPVERSIONREQ",
	"EVENT_CTCPVERSIONREQBC",
	"EVENT_CTCPFINGERRPL",
	"EVENT_CTCPFINGERRPLBC",
	"EVENT_CTCPFINGERREQ",
	"EVENT_CTCPFINGERREQBC",
	"EVENT_CTCPACTIONREQ",
	"EVENT_CTCPACTIONREQBC",
	"EVENT_CTCPTIMERPL",
	"EVENT_CTCPTIMERPLBC",
	"EVENT_CTCPTIMEREQ",
	"EVENT_CTCPTIMEREQBC",
	"EVENT_CTCPPINGRPL",
	"EVENT_CTCPPINGRPLBC",
	"EVENT_CTCPPINGREQ",
	"EVENT_CTCPPINGREQBC",
	"EVENT_CTCPUNHANDLEDRPL",
	"EVENT_CTCPUNHANDLEDRPLBC",
	"EVENT_CTCPUNHANDLEDREQ",
	"EVENT_CTCPUNHANDLEDREQBC",
	"EVENT_DCCSEND",
	"EVENT_DCCCHAT",
	"EVENT_DCCCHATMSG",
	"EVENT_ADDBAN",
	"EVENT_DELBAN",
	"EVENT_COUNT",
};

/** @brief SendEvent
 *
 *	Call event handler
 *  Event subsystem use only
 *
 *  @param event to send
 *  @param cmdparams
 *  @param module_ptr pointer to module to raise event for
 *
 *  @return none
 */

static void SendEvent( const ModuleEvent *eventptr, Event event, const CmdParams *cmdparams, Module *module_ptr )
{
	if( !eventptr )
	{
		dlog( DEBUG9, "SendEvent: %s has no event handler for %s", module_ptr->info->name, EventStrings[event] );
		return;
	}
	/* If we are not yet synched, check that the module supports 
	 * the event before we are synched. */
	if( !IsModuleSynched( module_ptr ) && !( eventptr->flags & EVENT_FLAG_IGNORE_SYNCH ) )
	{
		dlog( DEBUG9, "Skipping module %s for %s since module is not yet synched", module_ptr->info->name, EventStrings[event] );
		return;
	}
	if( ( eventptr->flags & EVENT_FLAG_DISABLED ) )
	{
		dlog( DEBUG9, "Skipping module %s for %s since it is disabled", module_ptr->info->name, EventStrings[event] );
		return;
	}
	if( ( eventptr->flags & EVENT_FLAG_EXCLUDE_ME ) && IsMe( cmdparams->source ) )
	{
		dlog( DEBUG9, "Skipping module %s for %s since %s is excluded as a NeoStats client", module_ptr->info->name, EventStrings[event], cmdparams->source->name );
		return;
	}
	if( eventptr->flags & EVENT_FLAG_EXCLUDE_MODME )
	{
		if( cmdparams->source && cmdparams->source->user && cmdparams->source->user->bot && cmdparams->source->user->bot->moduleptr == module_ptr )
		{
			dlog( DEBUG9, "Skipping module %s for %s since %s is excluded as a Module client", module_ptr->info->name, EventStrings[event], cmdparams->source->name );
			return;
		}
	}			
	if( ( eventptr->flags & EVENT_FLAG_USE_EXCLUDE ) && IsExcluded( cmdparams->source ) )
	{
		dlog( DEBUG9, "Skipping module %s for %s since %s is excluded", module_ptr->info->name, EventStrings[event], cmdparams->source->name );
		return;
	}			
	dlog( DEBUG9, "Running module %s with %s", module_ptr->info->name, EventStrings[event] );
	SET_SEGV_LOCATION();
#ifdef USE_PERL
	if( ( eventptr->flags & EVENT_FLAG_PERLCALL ) )
	{
		SET_RUN_LEVEL( module_ptr );
		perl_event_cb( event, cmdparams, module_ptr );
		RESET_RUN_LEVEL();
		return;
	}			
#endif /* USE_PERL */
	if( setjmp( sigvbuf ) == 0 )
	{
		SET_RUN_LEVEL( module_ptr );
		( void )module_ptr->event_list[event]->handler( cmdparams );
		RESET_RUN_LEVEL();
	}
	else
	{
		nlog( LOG_CRITICAL, "SendEvent: setjmp() failed, not calling module %s", module_ptr->info->name );
	}
}

/** @brief SendModuleEvent
 *
 *	Call event handler for a specific module
 *  NeoStats core use only
 *
 *  @param event to send
 *  @param cmdparams
 *  @param module_ptr pointer to module to raise event for
 *
 *  @return none
 */

void SendModuleEvent( Event event, CmdParams *cmdparams, Module *module_ptr )
{
	SET_SEGV_LOCATION();
	dlog( DEBUG9, "SendModuleEvent: %s to module %s", EventStrings[event], module_ptr->info->name );

	cmdparams->eventid = event;
	if( module_ptr->event_list )
		SendEvent( module_ptr->event_list[event], event, cmdparams, module_ptr );
#ifdef USE_PERL
	if( ( module_ptr->pm && module_ptr->pm->event_list ) )
		SendEvent( module_ptr->pm->event_list[event], event, cmdparams, module_ptr );
#endif /* USE_PERL */
}

/** @brief SendAllModuleEventHandler
 *
 *	List walk handler to call event handler for all modules
 *  NeoStats core use only
 *
 *  @param module_ptr pointer to module
 *  @param v pointer to cmdparams and event
 *
 *  @return none
 */

static int SendAllModuleEventHandler( Module *module_ptr, void *v )
{
	ModuleAllEvent *mae = (ModuleAllEvent *)v;

	SendModuleEvent( mae->event, mae->cmdparams, module_ptr );
	return NS_FALSE;
}

/** @brief SendAllModuleEvent
 *
 *	Call event handler for all modules
 *  NeoStats core use only
 *
 *  @param event to send
 *  @param cmdparams
 *
 *  @return none
 */

void SendAllModuleEvent( Event event, CmdParams *cmdparams )
{
	ModuleAllEvent mae;

	SET_SEGV_LOCATION();
	dlog( DEBUG9, "SendAllModuleEvent: %s to all modules", EventStrings[event] );
	mae.event = event;
	mae.cmdparams = cmdparams;
	ProcessModuleList( SendAllModuleEventHandler, (void *)&mae );
}

/** @brief AddEvent
 *
 *	Add event handler
 *  NeoStats core use only
 *
 *  @param eventptr pointer to event to add
 *
 *  @return none
 */

void AddEvent( ModuleEvent *eventptr )
{
	Module *mod_ptr;

	if( !eventptr )
	{
		nlog( LOG_ERROR, "AddEvent: eventptr passed as NULL" );
		return;
	}
	mod_ptr = GET_CUR_MODULE();
	dlog( DEBUG9, "AddEvent: adding %s to %s", EventStrings[eventptr->event], mod_ptr->info->name );
#ifdef USE_PERL
	if( ( eventptr->flags & EVENT_FLAG_PERLCALL ) )
	{
		if( ( mod_ptr->pm && !mod_ptr->pm->event_list ) )
			mod_ptr->pm->event_list = ns_calloc( sizeof( ModuleEvent * ) * EVENT_COUNT );
		mod_ptr->pm->event_list[eventptr->event] = eventptr;
	}
	else
#endif /* USE_PERL */
	{
		if( !eventptr->handler )
		{
			nlog( LOG_ERROR, "AddEvent: missing handler for %s in module %s", EventStrings[eventptr->event], mod_ptr->info->name );
			return;
		}
		if( !mod_ptr->event_list )
			mod_ptr->event_list = ns_calloc( sizeof( ModuleEvent * ) * EVENT_COUNT );
		mod_ptr->event_list[eventptr->event] = eventptr;
	}
	if( eventptr->event == EVENT_NICKIP )
		me.want_nickip = 1; 		
}

/** @brief AddEventList
 *
 *	Add list of event handlers
 *  NeoStats core use only
 *
 *  @param eventlistptr pointer to list of events to add
 *
 *  @return none
 */

void AddEventList( ModuleEvent *eventlistptr )
{
	if( !eventlistptr )
	{
		nlog( LOG_ERROR, "AddEventList: eventlistptr passed as NULL" );
		return;
	}
	while( eventlistptr->event != EVENT_NULL )
	{
		AddEvent( eventlistptr );
		eventlistptr ++;
	}
}

/** @brief DeleteEvent
 *
 *	Delete event handler
 *  NeoStats core use only
 *
 *  @param eventptr pointer to event to delete
 *
 *  @return none
 */

void DeleteEvent( Event event )
{
	Module *mod_ptr;

	mod_ptr = GET_CUR_MODULE();
	if( !mod_ptr->event_list )
	{
		dlog( DEBUG9, "DeleteEvent: module %s has no events", mod_ptr->info->name );
		return;
	}
	if( mod_ptr->event_list )
		mod_ptr->event_list[event] = NULL;
	dlog( DEBUG9, "DeleteEvent: deleting %s from %s", EventStrings[event], mod_ptr->info->name );
}

/** @brief DeleteEventList
 *
 *	Delete list of event handlers
 *  NeoStats core use only
 *
 *  @param eventlistptr pointer to list of events to delete
 *
 *  @return none
 */

void DeleteEventList( const ModuleEvent *eventlistptr )
{
	if( !eventlistptr )
	{
		nlog( LOG_ERROR, "DeleteEventList: eventlistptr passed as NULL" );
		return;
	}
	while( eventlistptr->event != EVENT_NULL )
	{
		DeleteEvent( eventlistptr->event );
		eventlistptr++;
	}
}

/** @brief FreeEventList
 *
 *	Free event list
 *  NeoStats core use only
 *
 *  @param mod_ptr pointer to module to free event list
 *
 *  @return none
 */

void FreeEventList( Module *mod_ptr )
{
	if( mod_ptr->event_list )
	{
		ns_free( mod_ptr->event_list );
		mod_ptr->event_list = NULL;
	}
#ifdef USE_PERL
	if( mod_ptr->pm && mod_ptr->pm->event_list )
	{
		ns_free( mod_ptr->pm->event_list );
		mod_ptr->pm->event_list = NULL;
	}
#endif /* USE_PERL */
}

/** @brief SetAllEventFlags
 *
 *	Set a flag for all events used by a module
 *  NeoStats core and module use
 *
 *  @param flag to set
 *  @param enable whether to enable or disable flag
 *
 *  @return none
 */

void SetAllEventFlags( unsigned int flag, unsigned int enable )
{
	int i;
	ModuleEvent **eventlistptr;

	eventlistptr = GET_CUR_MODULE()->event_list;
	if( !eventlistptr )
	{
		nlog( LOG_ERROR, "SetAllEventFlags: %s has no eventlist", GET_CUR_MODULE()->info->name );
		return;
	}
	for( i = 0; i < EVENT_COUNT; i++ )
	{
		if( eventlistptr[i] )
		{
			if( enable )
				eventlistptr[i]->flags |= flag;
			else
				eventlistptr[i]->flags &= ~flag;
		}
	}
}

/** @brief SetEventFlags
 *
 *	Set a flag for a single event used by a module
 *  NeoStats core and module use
 *
 *  @param event to set
 *  @param flag to set
 *  @param enable or disable flag
 *
 *  @return none
 */

void SetEventFlags( Event event, unsigned int flag, unsigned int enable )
{
	ModuleEvent **eventlistptr;

	eventlistptr = GET_CUR_MODULE()->event_list;
	if( !eventlistptr )
	{
		nlog( LOG_ERROR, "SetEventFlags: %s has no eventlist", GET_CUR_MODULE()->info->name );
		return;
	}
	if( !eventlistptr[event] )
	{
		nlog( LOG_ERROR, "SetEventFlags: %s has no event %s", GET_CUR_MODULE()->info->name, EventStrings[event] );
		return;
	}
	if( enable )
		eventlistptr[event]->flags |= flag;
	else
		eventlistptr[event]->flags &= ~flag;
}

/** @brief EnableEvent
 *
 *	Enable a single event used by a module
 *  NeoStats core and module use
 *
 *  @param event to set
 *
 *  @return none
 */

void EnableEvent( Event event )
{
	ModuleEvent **eventlistptr;

	eventlistptr = GET_CUR_MODULE()->event_list;
	if( !eventlistptr )
	{
		nlog( LOG_ERROR, "EnableEvent: %s has no eventlist", GET_CUR_MODULE()->info->name );
		return;
	}
	if( !eventlistptr[event] )
	{
		nlog( LOG_ERROR, "EnableEvent: %s has no event %s", GET_CUR_MODULE()->info->name, EventStrings[event] );
		return;
	}
	eventlistptr[event]->flags &= ~EVENT_FLAG_DISABLED;
}

/** @brief DisableEvent
 *
 *	Disable a single event used by a module
 *  NeoStats core and module use
 *
 *  @param event to set
 *
 *  @return none
 */

void DisableEvent( Event event )
{
	ModuleEvent **eventlistptr;

	eventlistptr = GET_CUR_MODULE()->event_list;
	if( !eventlistptr )
	{
		nlog( LOG_ERROR, "DisableEvent: %s has no eventlist", GET_CUR_MODULE()->info->name );
		return;
	}
	if( !eventlistptr[event] )
	{
		nlog( LOG_ERROR, "DisableEvent: %s has no event %s", GET_CUR_MODULE()->info->name, EventStrings[event] );
		return;
	}
	eventlistptr[event]->flags |= EVENT_FLAG_DISABLED;
}
