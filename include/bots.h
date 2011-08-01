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
** $Id: bots.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _BOTS_H_
#define _BOTS_H_

int InitBots( void );
void FiniBots( void );
int ns_cmd_botlist( const CmdParams *cmdparams );
int DelBot( const char *nick );
void DelModuleBots( const Module *mod_ptr );
int BotNickChange( const Bot *botptr, const char *newnick );
void bot_private( const char *origin, char *const *av, int ac );
void bot_notice( const char *origin, char *const *av, int ac );
void bot_chan_private( const char *origin, char *const *av, int ac );
void bot_chan_notice( const char *origin, char *const *av, int ac );
void handle_dead_channel( Channel *c );

#endif /* _BOTS_H_ */
