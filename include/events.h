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
** $Id: events.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _EVENTS_H_
#define _EVENTS_H_

/*  Events. 
 *    An event handler is passed a structure pointer of type
 *    CmdParams.
 */
typedef enum Event {
/*  EVENT_NULL
 *    dummy value to end lists
 */
	EVENT_NULL = -1,

/*  NeoStats events 
 *    Generated in response to neostats specific events
 */

/*  EVENT_MODULELOAD
 *    Called when a module is loaded. 
 *    parameters:
 *      module name in cmdparams->param
 */
	EVENT_MODULELOAD,

/*  EVENT_MODULEUNLOAD
 *    Called when a module is unloaded. 
 *    parameters:
 *      module name in cmdparams->param
 */
	EVENT_MODULEUNLOAD,

/*  Server events 
 *    Generated in response to server events
 */

/*  EVENT_SERVER 
 *    Called when a server connects to the network. 
 *    parameters:
 *      server in cmdparams->source
 */
	EVENT_SERVER,

/*  EVENT_SQUIT 
 *    Called when a server squits the network 
 *    parameters:
 *      server in cmdparams->source
 *      reason in cmdparams->param
 */
	EVENT_SQUIT,

/*  EVENT_PING 
 *    parameters:
 *      server in cmdparams->source
 */
	EVENT_PING,

/*  EVENT_PONG 
 *    parameters:
 *      server in cmdparams->source
 */
	EVENT_PONG,

/*  User events 
 *    Generated in response to user events
 */

/*  EVENT_SIGNON 
 *    Called when a new user signs onto the network. 
 *    parameters:
 *      user in cmdparams->source
 */
	EVENT_SIGNON,

/*  EVENT_QUIT
 *    Called when a user quits the network. 
 *    parameters:
 *      user in cmdparams->source
 *      reason in cmdparams->param
 */
	EVENT_QUIT,

/*	EVENT_NICKIP
 *		Called when we get the IP address of a user. This is 
 *		either when the ircd sends the nickip as part of the 
 *		connect message or a dns lookup sucessfully completes.
 *	  parameters:
 *		user in cmdparams->source
 */ 
	EVENT_NICKIP,

/*  EVENT_KILL 
 *    Called when a user is killed.
 *    parameters:
 *      oper / server that issued kill in cmdparams->source
 *      user who was killed in cmdparams->target
 *      reason in cmdparams->param
 */
	EVENT_KILL,

/*  EVENT_LOCALKILL 
 *    Called when a user is killed by an operator.
 *    parameters:
 *      operator that issued kill in cmdparams->source
 *      user who was killed in cmdparams->target
 *      reason in cmdparams->param
 */
	EVENT_LOCALKILL,

/*  EVENT_GLOBALKILL 
 *    Called when a user is killed by an operator.
 *    parameters:
 *      operator that issued kill in cmdparams->source
 *      user who was killed in cmdparams->target
 *      reason in cmdparams->param
 */
	EVENT_GLOBALKILL,

/*  EVENT_SERVERKILL 
 *    Called when a user is killed by a server.
 *    parameters:
 *      server that issued kill in cmdparams->source
 *      user who was killed in cmdparams->target
 *      reason in cmdparams->param
 */
	EVENT_SERVERKILL,

/*  EVENT_BOTKILL 
 *    is called if one of the NeoStats bots gets killed. You would 
 *    use it to reinitialize the bot.
 *    parameters:
 *      user who was killed in cmdparams->target
 *      reason in cmdparams->param
 */
	EVENT_BOTKILL,

/*  EVENT_NICK
 *    Called when a user changes nick
 *    parameters:
 *      user in cmdparams->source
 *      oldnick in cmdparams->param
 */
	EVENT_NICK,

/*  EVENT_AWAY 
 *    parameters:
 *      user in cmdparams->source
 */
	EVENT_AWAY,

/*  EVENT_UMODE 
 *    Called when a user changes Umodes. (e.g., /mode +o fish) 
 *    parameters:
 *      user in cmdparams->source
 *      modes in cmdparams->param
 */
	EVENT_UMODE,

/*  EVENT_SMODE 
 *    Called when a user changes Smodes.
 *    parameters:
 *      user in cmdparams->source
 *      modes in cmdparams->param
 */
	EVENT_SMODE,

/*  Channel events 
 *    Generated in response to channel events
 */

/*  EVENT_NEWCHAN 
 *    parameters:
 *      channel in cmdparams->channel
 */
	EVENT_NEWCHAN,

/*  EVENT_DELCHAN 
 *    parameters:
 *      channel in cmdparams->channel
 */
	EVENT_DELCHAN,

/*  EVENT_JOIN
 *    parameters:
 *      user in cmdparams->source
 *      channel in cmdparams->channel
 */
	EVENT_JOIN,

/*  EVENT_PART
 *    parameters:
 *      user in cmdparams->source
 *      channel in cmdparams->channel
 *      reason in cmdparams->param
 */
	EVENT_PART,

/*  EVENT_PARTBOT 
 *    parameters:
 *      user in cmdparams->source
 *      channel in cmdparams->channel
 *      reason in cmdparams->param
 */
	EVENT_PARTBOT,

/*  EVENT_EMPTYCHAN
 *    When no real users are left in a channel that the module has a bot in,
 *    the core bot will PART the bot and issue this event.
 *    by the core
 *    parameters:
 *      user in cmdparams->source
 *      bot in cmdparams->bot
 *      channel in cmdparams->channel
 *      reason in cmdparams->param
 */
	EVENT_EMPTYCHAN,

/*  EVENT_KICK 
 *    parameters:
 *      user/server which kicked in cmdparams->source
 *      user in cmdparams->target
 *      channel in cmdparams->channel
 *      reason in cmdparams->param
 */
	EVENT_KICK,

/*  EVENT_KICKBOT 
 *    parameters:
 *      user/server which kicked in cmdparams->source
 *      user in cmdparams->target
 *      channel in cmdparams->channel
 *      reason in cmdparams->param
 */
	EVENT_KICKBOT,

/*  EVENT_TOPIC 
 *    parameters:
 *      channel in cmdparams->channel
 */
	EVENT_TOPIC,

/*  EVENT_CMODE
 *    parameters:
 *      channel in cmdparams->channel
 *      modes in cmdparams->av[0]..av[cmdparams->ac-1]
 */
	EVENT_CMODE,

/*  Messages event
 *    Generated in response to messages received 
 */

/*  EVENT_PRIVATE
 *    parameters:
 *      from in cmdparams->source
 *      to in cmdparams->bot
 *      message in cmdparams->param
 */
	EVENT_PRIVATE,

/*  EVENT_NOTICE
 *    parameters:
 *      from in cmdparams->source
 *      to in cmdparams->bot
 *      message in cmdparams->param
 */
	EVENT_NOTICE,

/*  EVENT_CPRIVATE
 *    parameters:
 *      from in cmdparams->source
 *      channel in cmdparams->channel
 *      message in cmdparams->param
 */
	EVENT_CPRIVATE,

/*  EVENT_CNOTICE
 *    parameters:
 *      from in cmdparams->source
 *      channel in cmdparams->channel
 *      message in cmdparams->param
 */
	EVENT_CNOTICE,

/*  EVENT_GLOBOPS
 *    parameters:
 *      from in cmdparams->source
 *      message in cmdparams->param
 */
	EVENT_GLOBOPS,

/*  EVENT_CHATOPS
 *    parameters:
 *      from in cmdparams->source
 *      message in cmdparams->param
 */
	EVENT_CHATOPS,

/*  EVENT_WALLOPS
 *    parameters:
 *      from in cmdparams->source
 *      message in cmdparams->param
 */
	EVENT_WALLOPS,

/*  CTCP events 
 *    Generated in response to ctcp events
 */

/*  EVENT_CTCPVERSIONRPL 
 *    parameters:
 *      user in cmdparams->source
 *      version in cmdparams->param
 */
	EVENT_CTCPVERSIONRPL,
	EVENT_CTCPVERSIONRPLBC,

/*  EVENT_CTCPVERSIONREQ
 *    parameters:
 *      user in cmdparams->source
 */
	EVENT_CTCPVERSIONREQ,
	EVENT_CTCPVERSIONREQBC,

/*  EVENT_CTCPFINGERRPL 
 *    parameters:
 *      user in cmdparams->source
 *      finger in cmdparams->param
 */
	EVENT_CTCPFINGERRPL,
	EVENT_CTCPFINGERRPLBC,

/*  EVENT_CTCPFINGERREQ
 *    parameters:
 *      user in cmdparams->source
 */
	EVENT_CTCPFINGERREQ,
	EVENT_CTCPFINGERREQBC,

/*  EVENT_CTCPACTIONREQ
 *    parameters:
 *      user in cmdparams->source
 */
	EVENT_CTCPACTIONREQ,
	EVENT_CTCPACTIONREQBC,

/*  EVENT_CTCPTIMERPL 
 *    parameters:
 *      user in cmdparams->source
 *      time in cmdparams->param
 */
	EVENT_CTCPTIMERPL,
	EVENT_CTCPTIMERPLBC,

/*  EVENT_CTCPTIMEREQ
 *    parameters:
 *      user in cmdparams->source
 */
	EVENT_CTCPTIMEREQ,
	EVENT_CTCPTIMEREQBC,

/*  EVENT_CTCPPINGRPL 
 *    parameters:
 *      user in cmdparams->source
 *      ping in cmdparams->param
 */
	EVENT_CTCPPINGRPL,
	EVENT_CTCPPINGRPLBC,

/*  EVENT_CTCPPINGREQ
 *    parameters:
 *      user in cmdparams->source
 */
	EVENT_CTCPPINGREQ,
	EVENT_CTCPPINGREQBC,

/*  EVENT_CTCPUNHANDLEDRPL 
 *    parameters:
 *      user in cmdparams->source
 *      ???
 */
	EVENT_CTCPUNHANDLEDRPL,
	EVENT_CTCPUNHANDLEDRPLBC,

/*  EVENT_CTCPUNHANDLEDREQ
 *    parameters:
 *      user in cmdparams->source
 *      ???
 */
	EVENT_CTCPUNHANDLEDREQ,
	EVENT_CTCPUNHANDLEDREQBC,

/*  DCC events 
 *    Generated in response to ctcp dcc events
 */

/*  EVENT_DCCSEND 
 *    parameters:
 *      user in cmdparams->source
 *      parameters in cmdparams->param
 */
	EVENT_DCCSEND,

/*  EVENT_DCCCHAT
 *    parameters:
 *      user in cmdparams->source
 *      parameters in cmdparams->param
 */
	EVENT_DCCCHAT,

/*  EVENT_DCCCHATMSG
 *    parameters:
 *      user in cmdparams->source
 *      parameters in cmdparams->param
 */
	EVENT_DCCCHATMSG,

/*  Ban events 
 *    Generated in response to ban events
 */

/*  EVENT_ADDBAN
 *    parameters:
 *      cmdparams->param pointer to Ban struct
 */
	EVENT_ADDBAN,

/*  EVENT_DELBAN
 *    parameters:
 *      cmdparams->param pointer to Ban struct
 */
	EVENT_DELBAN,

/*  EVENT_COUNT
 *    dummy value to provide last event number
 */
	EVENT_COUNT,

} Event;

#endif /* _EVENTS_H_ */
