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
** $Id: adnsdlist.h 3294 2008-02-24 02:45:41Z Fish $
*/
/*
 * dlist.h
 * - macros for handling doubly linked lists
 */

#ifndef ADNS_DLIST_H_INCLUDED
#define ADNS_DLIST_H_INCLUDED

#define ALIST_INIT(list) ((list).head= (list).tail= 0)
#define ALINK_INIT(link) ((link).next= (link).back= 0)

#define ALIST_UNLINK_PART(list,node,part) \
  do { \
    if ((node)->part back) (node)->part back->part next= (node)->part next; \
      else                                  (list).head= (node)->part next; \
    if ((node)->part next) (node)->part next->part back= (node)->part back; \
      else                                  (list).tail= (node)->part back; \
  } while(0)

#define ALIST_LINK_TAIL_PART(list,node,part) \
  do { \
    (node)->part next= 0; \
    (node)->part back= (list).tail; \
    if ((list).tail) (list).tail->part next= (node); else (list).head= (node); \
    (list).tail= (node); \
  } while(0)

#define ALIST_UNLINK(list,node) ALIST_UNLINK_PART(list,node,)
#define ALIST_LINK_TAIL(list,node) ALIST_LINK_TAIL_PART(list,node,)

#endif
