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
** $Id: modes.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _MODES_H_
#define _MODES_H_

int InitModeTables( const mode_init *chan_umodes, const mode_init *chan_modes, const mode_init *user_umodes, const mode_init *user_smodes );
void ChanMode( char *origin, char **av, int ac );
void ChanModeHandler( Channel *c, const char *modes, int avindex, char **av, int ac );
void ChanUserMode( const char *chan, const char *nick, int add, const unsigned int mode );
void ListChannelModes( const CmdParams* cmdparams, const Channel* c );

#endif /* _MODES_H_ */
