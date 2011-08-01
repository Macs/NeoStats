/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2008 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Based on adns, which is
**    Copyright (C) 1997-2008 Ian Jackson <ian@davenant.greenend.org.uk>
**    Copyright (C) 1999-2008 Tony Finch <dot@dotat.at>
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
** $Id: adnstvarith.h 3294 2008-02-24 02:45:41Z Fish $
*/
/*
 * tvarith.h
 * - static inline functions for doing arithmetic on timevals
 */

#ifndef ADNS_TVARITH_H_INCLUDED
#define ADNS_TVARITH_H_INCLUDED

static inline void timevaladd(struct timeval *tv_io, long ms)
{
	struct timeval tmp;
	assert(ms >= 0);
	tmp = *tv_io;
	tmp.tv_usec += (ms % 1000) * 1000;
	tmp.tv_sec += ms / 1000;
	if (tmp.tv_usec >= 1000000) {
		tmp.tv_sec++;
		tmp.tv_usec -= 1000000;
	}
	*tv_io = tmp;
}

#endif
