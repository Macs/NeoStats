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
** $Id: dcc.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _DCC_H_
#define _DCC_H_

int InitDCC( void );
void FiniDCC( void );
void dcc_hook_1( fd_set *read_fd_set, fd_set *write_fd_set );
void dcc_hook_2( fd_set *read_fd_set, fd_set *write_fd_set );
int dcc_req ( CmdParams* cmdparams );
void dcc_send_msg( const Client* dcc, char * buf );

#endif /* _DCC_H_ */
