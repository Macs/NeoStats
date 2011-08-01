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
** $Id: numerics.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "protocol.h"
#include "numerics.h"
#include "base64.h"

char *numeric219 = "219";
char *numeric242 = "242";
char *numeric351 = "351";

irc_cmd numeric_cmd_list[] = {
	/*Message	Token	handler	usage */
/*  RX: :irc.foo.com 219 NeoStats u :End of /STATS report */
	{&numeric219, NULL, _m_numericdefault, 0},
	{&numeric242, NULL, _m_numeric242, 0},
	{&numeric351, NULL, _m_numeric351, 0},
	{0, 0, 0, 0},
};

/*  RX: :irc.foo.com 250 NeoStats :Highest connection count: 3( 2 clients )
 */

/** @brief _m_numeric351
 *
 *  process numeric 351
 *  RX: :irc.foo.com 351 stats.neostats.net Unreal3.2. irc.foo.com :FinWXOoZ [*=2303]
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_numeric351( char *origin, char **argv, int argc, int srv )
{
	Client *s;

	if( ircd_srv.protocol & PROTOCOL_B64SERVER )
		s = FindServer( base64_to_server( origin ) );
	else
		s = FindServer( origin );
	if( s )
		strlcpy( s->version, argv[1], MAXHOST );
}

/** @brief _m_numeric242
 *
 *  process numeric 242
 *  RX: :irc.foo.com 242 NeoStats :Server Up 6 days, 23:52:55
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_numeric242( char *origin, char **argv, int argc, int srv )
{
	Client *s;

	if( ircd_srv.protocol & PROTOCOL_B64SERVER )
		s = FindServer( base64_to_server( origin ) );
	else
		s = FindServer( origin );
	if( s ) {
		/* Convert "Server Up d days, hh:mm:ss" to seconds*/
		char *ptr;
		time_t secs;

		/* current string: "Server Up d days, hh:mm:ss" */
		strtok( argv[argc-1], " " );
		/* current string: "Up d days, hh:mm:ss" */
		strtok( NULL, " " );
		/* current string: "d days, hh:mm:ss" */
		ptr = strtok( NULL, " " );
		if( ptr == NULL )
			return;
		secs = atoi( ptr ) * TS_ONE_DAY;
		/* current string: "days, hh:mm:ss" */
		strtok( NULL, " " );
		/* current string: ", hh:mm:ss" */
		ptr = strtok( NULL, "" );
		/* current string: "hh:mm:ss" */
		ptr = strtok( ptr , ":" );
		if( ptr == NULL )
			return;
		secs += atoi( ptr ) * TS_ONE_HOUR;
		/* current string: "mm:ss" */
		ptr = strtok( NULL, ":" );
		if( ptr == NULL )
			return;
		secs += atoi( ptr )*60;
		/* current string: "ss" */
		ptr = strtok( NULL, "" );
		if( ptr == NULL )
			return;
		secs += atoi( ptr );

		s->server->uptime = secs;
	}
}

/** @brief _m_numericdefault
 *
 *  dummy routine to "process" a numeric and avoid warnings for 
 *  unprocessed numerics. This mainly allows us to ignore end
 *  of list type numerics without having to specficially code them
 *  RX: :irc.foo.com nnn to :message
 *
 *  @param origin source of message (user/server)
 *  @param argv list of message parameters
 *  @param argc parameter count
 *  @param srv command flag
 *
 *  @return none
 */

void _m_numericdefault( char *origin, char **argv, int argc, int srv )
{
}
