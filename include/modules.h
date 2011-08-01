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
** $Id: modules.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _MODULES_H_
#define _MODULES_H_

extern jmp_buf sigvbuf;

int InitModules( void );
void FiniModules( void );
Module *ns_load_module( const char *path, Client *u );
int unload_module( const char *module_name, Client *u );
void unload_modules( void );
int ns_cmd_modlist( const CmdParams *cmdparams );
int ns_cmd_load( const CmdParams *cmdparams );
int ns_cmd_unload( const CmdParams *cmdparams );
void AllModuleVersions( const char *nick, const char *remoteserver );
int SynchModule( Module *module_ptr );
void SynchAllModules( void );
void assign_mod_number( Module *mod_ptr );
void insert_module( Module *mod_ptr );
void load_module_error( const Client *target, const char *module_name, const char *fmt, ... );
#endif /* _MODULES_H_ */
