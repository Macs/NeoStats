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
**  Portions borrowed from kazlib. Original Header follows:
*/
/*
 * List Abstract Data Type
 * Copyright (C) 1997 Kaz Kylheku <kaz@ashi.footprints.net>
 *
 * Free Software License:
 *
 * All rights are reserved by the author, with the following exceptions:
 * Permission is granted to freely reproduce and distribute this software,
 * possibly in exchange for a fee, provided that this copyright notice appears
 * intact. Permission is also granted to adapt this software to produce
 * derivative works, as long as the modified versions carry this copyright
 * notice and additional notices stating that the work has been modified.
 * This source code may be translated into executable form and incorporated
 * into proprietary software; there is no requirement for such software to
 * contain a copyright notice related to this source.
 *
 * $Id: list.h 3294 2008-02-24 02:45:41Z Fish $
 * $Name:  $
 */

#ifndef _LIST_H_
#define _LIST_H_

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */
#ifdef KAZLIB_SIDEEFFECT_DEBUG
#include "sfx.h"
#define LIST_SFX_CHECK(E) SFX_CHECK(E)
#else /* KAZLIB_SIDEEFFECT_DEBUG */
#define LIST_SFX_CHECK(E) (E)
#endif /* KAZLIB_SIDEEFFECT_DEBUG */

/*
 * Blurb for inclusion into C++ translation units
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	typedef unsigned long listcount_t;
#define LISTCOUNT_T_MAX ULONG_MAX

	typedef struct lnode_t {
#if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
		struct lnode_t *list_next;
		struct lnode_t *list_prev;
		void *list_data;
#else /* defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG) */
		int list_dummy;
#endif /* defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG) */
	} lnode_t;

	typedef struct lnodepool_t {
#if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
		struct lnode_t *list_pool;
		struct lnode_t *list_free;
		listcount_t list_size;
#else /* defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG) */
		int list_dummy;
#endif /* defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG) */
	} lnodepool_t;

	typedef struct list_t {
#if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
		lnode_t list_nilnode;
		listcount_t list_nodecount;
		listcount_t list_maxcount;
#else /* defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG) */
		int list_dummy;
#endif /* defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG) */
	} list_t;

EXPORTFUNC	lnode_t *lnode_create (void *);
	lnode_t *lnode_init (lnode_t *, void *);
EXPORTFUNC	void lnode_destroy (lnode_t *);
	void lnode_put (lnode_t *, void *);
	void *lnode_get (const lnode_t *);
	int lnode_is_in_a_list (const lnode_t *);

#if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
#define lnode_put(N, D)		((N)->list_data = (D))
#define lnode_get(N)		((N)->list_data)
#endif /* defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG) */

	lnodepool_t *lnode_pool_init (lnodepool_t *, lnode_t *, listcount_t);
	lnodepool_t *lnode_pool_create (listcount_t);
	void lnode_pool_destroy (lnodepool_t *);
	lnode_t *lnode_borrow (lnodepool_t *, void *);
	void lnode_return (lnodepool_t *, lnode_t *);
	int lnode_pool_isempty (const lnodepool_t *);
	int lnode_pool_isfrom (const lnodepool_t *, const lnode_t *);

EXPORTFUNC	int comparef (const void *, const void *);


	list_t *list_init (list_t *, listcount_t);
EXPORTFUNC	list_t *list_create (listcount_t);
EXPORTFUNC	void list_destroy (list_t *);
EXPORTFUNC	void list_destroy_nodes (list_t *);
	void list_return_nodes (list_t *, lnodepool_t *);

	listcount_t list_count (const list_t *);
	int list_isempty (const list_t *);
	int list_isfull (const list_t *);
	int list_contains (list_t *, const lnode_t *);

	void list_append (list_t *, lnode_t *);
	void list_prepend (list_t *, lnode_t *);
EXPORTFUNC	void list_ins_before (list_t *, lnode_t *, lnode_t *);
EXPORTFUNC	void list_ins_after (list_t *, lnode_t *, lnode_t *);

	lnode_t *list_first (const list_t *);
	lnode_t *list_last (const list_t *);
	lnode_t *list_next (list_t *, const lnode_t *);
	lnode_t *list_prev (list_t *, const lnode_t *);

	lnode_t *list_del_first (list_t *);
	lnode_t *list_del_last (list_t *);
EXPORTFUNC	lnode_t *list_delete (list_t *, lnode_t *);

EXPORTFUNC	void list_process (list_t *, void *, void (*)(list_t *, lnode_t *, void *));

	int list_verify (list_t *);

#if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
#define lnode_pool_isempty(P)	((P)->list_free == 0)
#define list_count(L)		((L)->list_nodecount)
#define list_isempty(L)		((L)->list_nodecount == 0)
#define list_isfull(L)		(LIST_SFX_CHECK(L)->list_nodecount == (L)->list_maxcount)
#define list_next(L, N)		(LIST_SFX_CHECK(N)->list_next == &(L)->list_nilnode ? NULL : (N)->list_next)
#define list_prev(L, N)		(LIST_SFX_CHECK(N)->list_prev == &(L)->list_nilnode ? NULL : (N)->list_prev)
#define list_first(L)		list_next(LIST_SFX_CHECK(L), &(L)->list_nilnode)
#define list_last(L)		list_prev(LIST_SFX_CHECK(L), &(L)->list_nilnode)
#endif /* defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG) */

#if defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG)
#define list_append(L, N)	list_ins_before(LIST_SFX_CHECK(L), N, &(L)->list_nilnode)
#define list_prepend(L, N)	list_ins_after(LIST_SFX_CHECK(L), N, &(L)->list_nilnode)
#define list_del_first(L)	list_delete(LIST_SFX_CHECK(L), list_first(L))
#define list_del_last(L)	list_delete(LIST_SFX_CHECK(L), list_last(L))
#endif /* defined(LIST_IMPLEMENTATION) || !defined(KAZLIB_OPAQUE_DEBUG) */

/* destination list on the left, source on the right */

	void list_extract (list_t *, list_t *, lnode_t *, lnode_t *);
	void list_transfer (list_t *, list_t *, lnode_t * first);
	void list_merge (list_t *, list_t *, int (const void *, const void *));
EXPORTFUNC	void list_sort (list_t *, int (const void *, const void *));
EXPORTFUNC	lnode_t *list_find (const list_t *, const void *, int (const void *, const void *));
EXPORTFUNC	int list_is_sorted (list_t *, int (const void *, const void *));

EXPORTFUNC	void list_destroy_auto (list_t * list);
EXPORTFUNC	lnode_t *lnode_create_append (list_t *, void *);
EXPORTFUNC	lnode_t *lnode_create_prepend (list_t *list, void *data);
EXPORTFUNC	void *lnode_find (const list_t * list, const void *key, int compare (const void *, const void *));
EXPORTFUNC	void list_delete_destroy_node( list_t * list, lnode_t * del );

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _LIST_H_ */
