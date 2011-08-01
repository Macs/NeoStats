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
** $Id: log.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _LOG_H_
#define _LOG_H_

/*
 * log.h
 * dynamic configuration runtime libary
 */

/* buffer size for new customisable filenames 
   20 should be more than sufficient */
#define MAX_LOGFILENAME	20

void CloseLogs( void );
int InitLogs( void );
int ResetLogs( void *arg );
int FlushLogs(void *arg);
void FiniLogs( void );
/* Configurable log filename format string */
extern char LogFileNameFormat[MAX_LOGFILENAME];

#endif /* _LOG_H_ */
