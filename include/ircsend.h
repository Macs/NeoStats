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
** $Id: ircsend.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _IRC_SEND_H_
#define _IRC_SEND_H_

int InitIrcdSymbols( void );
int irc_eob( const char *server );
int irc_netinfo( const char *source, const char *maxglobalcnt, const unsigned long ts, const int prot, const char *cloak, const char *netname );
int irc_snetinfo( const char *source, const char *maxglobalcnt, const unsigned long ts, const int prot, const char *cloak, const char *netname );
int irc_vctrl( const int uprot, const int nicklen, const int modex, const int gc, const char *netname );
int irc_burst( int b );
int irc_version( const char *source, const char *target );
int irc_svinfo( const int tscurrent, const int tsmin, const unsigned long tsnow );

#endif /* _IRC_SEND_H_ */
