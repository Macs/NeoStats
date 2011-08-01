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
** $Id: nsdbm.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _NSDBM_H_
#define _NSDBM_H_

MODULEFUNC void *DBMOpenDB( const char *name );
MODULEFUNC void DBMCloseDB( void *dbhandle );
MODULEFUNC void *DBMOpenTable( void *dbhandle, const char *name );
MODULEFUNC void DBMCloseTable( void *dbhandle, void *tbhandle );
MODULEFUNC int DBMFetch( void *dbhandle, void *tbhandle, char *key, void *data, int size );
MODULEFUNC int DBMStore( void *dbhandle, void *tbhandle, char *key, void *data, int size );
MODULEFUNC int DBMFetchRows( void *dbhandle, void *tbhandle, DBRowHandler handler );
MODULEFUNC int DBMFetchRows2( void *dbhandle, void *tbhandle, DBRowHandler2 handler );
MODULEFUNC int DBMDelete( void *dbhandle, void *tbhandle, char * key );

#endif /* _NSDBM_H_ */
