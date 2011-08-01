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
** $Id: timer.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _TIMER_H_
#define _TIMER_H_

int InitTimers( void );
void FiniTimers( void );
int ns_cmd_timerlist( const CmdParams *cmdparams );
void CheckTimers_cb (int notused, short event, void *arg);
int del_timers( const Module *mod_ptr );

#endif /* _TIMER_H_ */
