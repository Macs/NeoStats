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
** $Id: exclude.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _EXCLUDE_H_
#define _EXCLUDE_H_

int InitExcludes( Module *mod_ptr );
void FiniExcludes( void );
void AddBotExcludeCommands( Bot *botptr );
void FiniModExcludes( Module *mod_ptr );

void ns_do_exclude_chan( Channel *c );
void ns_do_exclude_server( Client *s );
void ns_do_exclude_user( Client *u );

#endif /* _EXCLUDE_H_ */
