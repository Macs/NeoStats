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
** $Id: namedvars.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _NAMEDVARS_H_
#define _NAMEDVARS_H_
typedef enum {
	NV_STR,
	NV_PSTR,
	NV_INT,
	NV_LONG,
	NV_VOID,
	NV_PSTRA
} nv_struct_type;
typedef enum {
	NV_FLG_NONE,
	NV_FLG_RO
} nv_struct_flag;

typedef struct nv_struct {
	/* name of the field */
	char *fldname;
	/* type of filed */
	nv_struct_type type;
	/* offset of the field */
	int offset;
	/* flags of the field */
	nv_struct_flag flags;
	/* if its a substructure, the offset of the actual field, otherwise -1 */
	int fldoffset;
	/* if its a STRING, its length, -1 for other entries */
	int len;
} nv_struct;

#define NV_STRUCT_END() { NULL, NV_STR, 0, NV_FLG_NONE, -1}

typedef enum {
	NV_ACTION_ADD,
	NV_ACTION_MOD,
	NV_ACTION_DEL
} nv_write_action;

typedef struct nv_fields { 
	char *name;
	union {
		char *v_char;
		int v_int;
		long  v_long;
		void *v_void;
		char **v_chara;
		int n_chara;
	} values;
	nv_struct_type type;
} nv_fields;
typedef enum {
	NV_TYPE_LIST,
	NV_TYPE_HASH
} nv_type;

typedef struct nv_item {
	union {
		char *key;
		int pos;
	} index;
	nv_type type;
	int no_fields;
	nv_fields **fields;
	union {
		lnode_t *lnode;
		hnode_t *hnode;
	} node;
} nv_item;



typedef enum {
	NV_FLAGS_NONE,
	NV_FLAGS_RO
} nv_flags;


typedef int (*nv_set_handler) (nv_item *item, nv_write_action action );

typedef struct nv_list {
	/* name of the list/hash */
	char *name;
	/* type of list/hash */
	nv_type type;
	/* description of the fields */
	nv_struct *format;
	/* flags */
	nv_flags flags;
	/* module */
	Module *mod;
	/* ptr to the list */
	void *data;
	/* ptr to function to handle updates to the list */
	nv_set_handler updatehandler;
	union {
		struct hscan_t hscan;
		struct lnode_t *node;
	} iter;
	int itercount;
	int no_flds;
} nv_list;

extern hash_t *namedvars;


int nv_init();
EXPORTFUNC hash_t *nv_hash_create(hashcount_t count, hash_comp_t comp, hash_fun_t fun, char *name, nv_struct *nvstruct, nv_flags flags, nv_set_handler set_handler);
EXPORTFUNC void nv_hash_destroy(hash_t *hash, char *name);
EXPORTFUNC list_t *nv_list_create(listcount_t count, char *name2, nv_struct *nvstruct, nv_flags flags, nv_set_handler set_handler);
EXPORTFUNC void nv_list_destroy(list_t *list, char *name);
EXPORTFUNC nv_list *FindNamedVars(char *name);
EXPORTFUNC char *nv_gf_string(const void *, const nv_list *, const int);
EXPORTFUNC int nv_gf_int(const void *, const nv_list *, const int);
EXPORTFUNC long nv_gf_long(const void *, const nv_list *, const int);
EXPORTFUNC char **nv_gf_stringa(const void *, const nv_list *, const int);
EXPORTFUNC void *nv_gf_complex(const void *, const nv_list *, const int);
EXPORTFUNC int nv_get_field(const nv_list *, const char *);
EXPORTFUNC int nv_get_field_item(nv_item *item, char *fldname);
EXPORTFUNC int nv_update_structure (nv_list *data, nv_item *item, nv_write_action action);
EXPORTFUNC int nv_sf_string(nv_item *item, char *fldname, char *value);
EXPORTFUNC int nv_sf_int(nv_item *item, char *fldname, int value);
EXPORTFUNC int nv_sf_long(nv_item *item, char *fldname, long value);
EXPORTFUNC nv_item *nv_new_item(nv_list *data);

#endif /* _NAMEDVARS_H_ */
