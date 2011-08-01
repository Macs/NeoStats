/* NeoStats - IRC Statistical Services
** Copyright (c) 1999-2008 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000 - 2001 ^Enigma^
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
** $Id: namedvars.c 3294 2008-02-24 02:45:41Z Fish $
*/

/** @file namedvars.c 
 *  @brief named variable support for NeoStats Hash and List features
 */ 

#include "neostats.h"
#include "namedvars.h"
#include "namedvars-core.h"

hash_t *namedvars;

void nv_printstruct(void *data, nv_list *item);

nv_struct nv_nvlist[] = {
	{"name", NV_PSTR, offsetof(nv_list, name), NV_FLAGS_RO, -1, -1},
	{"type", NV_INT, offsetof(nv_list, type), NV_FLAGS_RO, -1, -1},
	{"flags", NV_INT, offsetof(nv_list, flags), NV_FLAGS_RO, -1, -1},
	{"noflds", NV_INT, offsetof(nv_list, no_flds), NV_FLAGS_RO, -1, -1},
	NV_STRUCT_END()
};


char *fldtypes[] = {
	"Pointer String",
	"String",
	"Integer",
	"Long",
	"Void",
	"String Array"
};

nv_list *FindNamedVars(char *name) {
	nv_list *nv;
	nv = hnode_find(namedvars, name);
	if (nv) {
		return nv;
	} else {
		nlog(LOG_WARNING, "FindNamedVars: Can't find NamedVar %s", name);
		return NULL;
	}
}

int nv_init() {
	nv_list *newitem;
	namedvars = hash_create(HASHCOUNT_T_MAX, 0, 0);
	newitem = ns_malloc(sizeof(nv_list));
	newitem->name = strdup("NamedVars");
	newitem->type = NV_TYPE_HASH;
	newitem->flags = NV_FLG_RO;
	newitem->format = nv_nvlist;
	newitem->mod = GET_CUR_MODULE();
	newitem->data = namedvars;
	newitem->updatehandler = NULL;
	newitem->no_flds = 0;
	while (newitem->format[newitem->no_flds].fldname != NULL) {
		newitem->no_flds++;
	}
	hnode_create_insert(namedvars, newitem, newitem->name);
	return NS_SUCCESS;
}

hash_t *nv_hash_create(hashcount_t count, hash_comp_t comp, hash_fun_t fun, char *name2, nv_struct *nvstruct, nv_flags flags, nv_set_handler set_handler) {
	nv_list *newitem;
	if (!FindNamedVars(name2)) {
		newitem = ns_malloc(sizeof(nv_list));
		newitem->name = strdup(name2);
		newitem->type = NV_TYPE_HASH;
		newitem->flags = flags;
		newitem->format = nvstruct;
		newitem->mod = GET_CUR_MODULE();
		newitem->data = (void *)hash_create(count, comp, fun);
		newitem->updatehandler = set_handler;
		newitem->no_flds = 0;
		while (newitem->format[newitem->no_flds].fldname != NULL) {
			newitem->no_flds++;
		}
		hnode_create_insert(namedvars, newitem, newitem->name);
		return (hash_t *) newitem->data;
	} else {
		nlog(LOG_WARNING, "Can not create NamedVars Hash %s - Already Exists", name2);
		return hash_create(count, comp, fun);
	}
}
void nv_hash_destroy(hash_t *hash, char *name) {
	/* XXX: Delete out of NV list */
	hash_destroy(hash);
}

list_t *nv_list_create(listcount_t count, char *name2, nv_struct *nvstruct, nv_flags flags, nv_set_handler set_handler) {
	nv_list *newitem;
	if (!FindNamedVars(name2)) {
		newitem = ns_malloc(sizeof(nv_list));
		newitem->name = strdup(name2);
		newitem->type = NV_TYPE_LIST;
		newitem->flags = flags;
		newitem->format = nvstruct;
		newitem->mod = GET_CUR_MODULE();
		newitem->data = (void *)list_create(count);
		newitem->updatehandler = set_handler;
		newitem->no_flds = 0;
		while (newitem->format[newitem->no_flds].fldname != NULL) {
			newitem->no_flds++;
		}
		hnode_create_insert(namedvars, newitem, newitem->name);
		return (list_t *) newitem->data;
	} else {
		nlog(LOG_WARNING, "Can not create NamedVars List %s - Already Exists", name2);
		return list_create(count);
	}
}

void nv_list_destroy(list_t *list, char *name) {
	/* XXX: Delete out of NamedVars listing */
	list_destroy(list);
}


int dump_namedvars(char *name2)
{
	hnode_t *node, *node2;
	lnode_t *lnode;
	hscan_t scan, scan1;
	void *data;	
	nv_list *item;
	int j;;
	hash_scan_begin(&scan1, namedvars);
	while ((node = hash_scan_next(&scan1)) != NULL ) {
		item = hnode_get(node);
		printf("%s (%p) Details: Type: %d Flags: %d Module: %s\n", item->name, item, item->type, item->flags, item->mod->info->name);
		printf("Entries:\n");
		printf("===================\n");
		if (item->type == NV_TYPE_HASH) {
			j = 0;
			hash_scan_begin(&scan, (hash_t *)item->data);
			while ((node2 = hash_scan_next(&scan)) != NULL) {
				data = hnode_get(node2);
				printf("Entry %d (%d)\n", j, (int)hash_count((hash_t *)item->data));
				nv_printstruct(data, item);
				j++;
			}
		} else if (item->type == NV_TYPE_LIST) {
			j = 0;
			lnode = list_first((list_t *)item->data);
			while (lnode) {
				data = lnode_get(lnode);
				printf("Entry %d (%d)\n", j, (int)list_count((list_t *)item->data));
				nv_printstruct(data, item);
				j++;
				lnode = list_next((list_t *)item->data, lnode);
			}
		}
	}
	return NS_SUCCESS;
}

char *nv_gf_string(const void *data, const nv_list *item, const int field) {
	char *output;
	void *data2;
	if (item->format[field].type == NV_PSTR) {
		if (item->format[field].fldoffset != -1) {
			data2 = (void *)((int)data + item->format[field].fldoffset);
			data2 = (void *)*((int *)data2);
		} else {
			data2 = (void *)data;
		}		

		output = (char *)*(int *)((int)data2 + item->format[field].offset);
	} else if (item->format[field].type == NV_STR) {
		if (item->format[field].fldoffset != -1) {
			data2 = (void *)((int)data + item->format[field].fldoffset);
			data2 = (void *)*((int *)data2);
		} else {
			data2 = (void *)data;
		}		
		output = (char *)((int)data2 + item->format[field].offset);
	} else {
		nlog(LOG_WARNING, "nv_gf_string: Field is not a string %d", field);
		return NULL;
	}
#ifdef DEBUG
	if (!ValidateString(output)) {
		nlog(LOG_WARNING, "nv_gf_string: Field is not string %d", field);
		return NULL;
	}
#endif
	return output;
}

int nv_gf_int(const void *data, const nv_list *item, const int field) {
	int output;
	size_t offset;
	if (item->format[field].type != NV_INT) {
		nlog(LOG_WARNING, "nv_gf_int: field is not a int %d", field);
		return 0;
	}
	if (item->format[field].fldoffset != -1) {
		offset = item->format[field].fldoffset + item->format[field].offset;
	} else {
		offset = item->format[field].offset;
	}		
	output = *((int *)((int)data + offset));
	return output;
}

long nv_gf_long(const void *data, const nv_list *item, const int field) {
	long output;
	size_t offset;
	if (item->format[field].type != NV_LONG) {
		nlog(LOG_WARNING, "nv_gf_long: field is not a long %d", field);
		return 0;
	}
	if (item->format[field].fldoffset != -1) {
		offset = item->format[field].fldoffset + item->format[field].offset;
	} else {
		offset = item->format[field].offset;
	}		
	output = *((long *)((int)data + offset));
	return output;
}

char **nv_gf_stringa(const void *data, const nv_list *item, const int field) {
	char **output;
	void *data2;
	int k;
	if (item->format[field].type != NV_PSTRA) {
		nlog(LOG_WARNING, "nv_gf_long: field is not a string array %d", field);
		return NULL;
	}
	if (item->format[field].fldoffset != -1) {
		data2 = (void *)((int)data + item->format[field].fldoffset);
		data2 = (void *)*((int *)data2);
	} else {
		data2 = (void *)data;
	}		
	output = (char **)*(int *)((int)data2 + item->format[field].offset);
#ifdef DEBUG
	k = 0;
	while (output && output[k] != NULL) {
		if (!ValidateString(output[k])) 
			return NULL;
		k++;
	}
#endif
	return output;
}

void *nv_gf_complex(const void *data, const nv_list *item, const int field) {
	void *output, *data2;
#if 0
	if (item->format[field].type != NV_COMPLEX) {
		nlog(LOG_WARNING, "nv_gf_complex: field is not complex %d", field);
		return NULL;
	}
#endif
	if (item->format[field].fldoffset != -1) {
		data2 = (void *)((int)data + item->format[field].fldoffset);
		data2 = (void *)*((int *)data2);
	} else {
		data2 = (void *)data;
	}		
	output = (void *)((int)data2 + item->format[field].offset);
	return output;
}

int nv_get_field(const nv_list *item, const char *name) {
	int i = 0;
	while (item->format[i].fldname != NULL) {
		if (!ircstrcasecmp(item->format[i].fldname, name)) 
			return i;
		i++;
	}
	return -1;
}

void nv_printstruct(void *data, nv_list *item) {
	int i, k;
	char **outarry;
	
	i = 0;
	while (item->format[i].fldname != NULL) {
		printf("\tField: Name: %s, Type: %s, Flags: %d ", item->format[i].fldname, fldtypes[item->format[i].type], item->format[i].flags);
		switch (item->format[i].type) {
			case NV_PSTR:
				printf("Value: %s\n", nv_gf_string(data, item, i));
				break;
			case NV_STR:
				printf("Value: %s\n", nv_gf_string(data, item, i));
				break;
			case NV_INT:
				printf("Value: %d\n", nv_gf_int(data, item, i));
				break;
			case NV_LONG:
				printf("Value: %ld\n", nv_gf_long(data, item, i));
				break;
			case NV_VOID:
				printf("Value: Complex (%p)!\n", nv_gf_complex(data, item, i));
				break;
			case NV_PSTRA:
				k = 0;
				printf("\n");
				outarry = nv_gf_stringa(data, item, i);
				while (outarry && outarry[k] != NULL) {
					printf("\t\tValue [%d]: %s\n", k, outarry[k]);
					k++;
				}
				break;			
			default:
				printf("Value: Unhandled!\n");
				break;
		}
		i++;
	}
}
nv_item *nv_new_item(nv_list *data) {
	nv_item *newitem;
	newitem = ns_calloc(sizeof(nv_item));
	newitem->fields = ns_calloc(data->no_flds * sizeof(nv_fields));
	return newitem;
}

void nv_free_item(nv_item *item) {
	int i;
	for (i = 0; i < item->no_fields; i++) {
		ns_free(item->fields[i]->name);
		if (item->fields[i]->type == NV_PSTR)
			ns_free(item->fields[i]->values.v_char);
		ns_free(item->fields[i]);
	}
	ns_free(item->fields);
	ns_free(item);
}		

int nv_update_structure (nv_list *data, nv_item *item, nv_write_action action) {
	int i, j;
	/* first, determine if the structure allows updates */
	if (data->flags & NV_FLAGS_RO) {
		nlog(LOG_WARNING, "Attempt to update read only structure %s", data->name);
		nv_free_item(item);
		return NS_FAILURE;
	}
	/* make sure index and node are filled in */
	if (item->type != data->type) {
		nlog(LOG_WARNING, "Type Field Differs in nv_update_structure");
		nv_free_item(item);
		return NS_FAILURE;
	}
	switch (item->type) {
		case NV_TYPE_LIST:
			if ((action == NV_ACTION_ADD) && (item->index.pos == -1)) {
				item->node.lnode = NULL;
			} else if (item->index.pos > -1) {
				/* find the lnode */
				if (item->index.pos > list_count((list_t *)data->data)) {
					nlog(LOG_WARNING, "Can't find position %d in list %s", item->index.pos, data->name);
					nv_free_item(item);
					return NS_FAILURE;
				}
                                item->node.lnode = list_first((list_t *)data->data);;
                                for (i = 0; i < item->index.pos; i++) {
                                	item->node.lnode = list_next((list_t *)data->data, item->node.lnode);
				}
				if (item->node.lnode == NULL) {
					nlog(LOG_WARNING, "can't find postition %d in list %s", item->index.pos, data->name);
					nv_free_item(item);
					return NS_FAILURE;
				} 
			} else {
				nlog(LOG_WARNING, "Invalid Index position");
				nv_free_item(item);
				return NS_FAILURE;
			}
			break;
		case NV_TYPE_HASH:
			if (item->index.key) {
				item->node.hnode = hnode_find((hash_t *)data->data, item->index.key);
				if (item->node.hnode == NULL) {
					nlog(LOG_WARNING, "Can't find Key %s in hash %s", item->index.key, data->name);
					nv_free_item(item);
					return NS_FAILURE;
				}
			} else {
				nlog(LOG_WARNING, "Invalid Key Name");
				nv_free_item(item);
				return NS_FAILURE;
			}
			break;
	}	
	if (action == NV_ACTION_ADD) {
		/* if its a add, make sure the index values dun exist */
		if ((item->type == NV_TYPE_LIST) && (item->node.lnode != NULL)) {
			nlog(LOG_WARNING, "Attempt to a new Node to list %s with a existing entry already %d in place. Use Modify instead", data->name, item->index.pos);
			nv_free_item(item);
			return NS_FAILURE;
		} else if ((item->type == NV_TYPE_HASH) && (item->node.hnode != NULL)) {
			nlog(LOG_WARNING, "Attempt to a new Node to hash %s with a existing entry already %s in place. Use Modify instead", data->name, item->index.key);
			nv_free_item(item);
			return NS_FAILURE;
		}			
	}
	if (action == NV_ACTION_ADD || action == NV_ACTION_MOD) {
		/* make sure all fields are present */
		i = 0;
		while (data->format[i].fldname != NULL) {
			if (nv_get_field_item(item, data->format[i].fldname) == -1) {
				nlog(LOG_WARNING, "Field %s is missing for AddOption for %s", data->format[i].fldname, data->name);
				nv_free_item(item);
				return NS_FAILURE;
			}
			i++;
		}
	}	
	/* second, check the fields are not RO if doing a modify */
	if (action == NV_ACTION_MOD) {
		i = 0;
		while (data->format[i].fldname != NULL) {
			for (j = 0; j > item->no_fields; j++) {
				if (!ircstrcasecmp(data->format[i].fldname, item->fields[j]->name)) {
					if (data->format[i].flags & NV_FLG_RO) {
						nlog(LOG_WARNING, "Attempt to update a read only field %s in structure %s", data->format[i].fldname, data->name);
						nv_free_item(item);
						return NS_FAILURE;
					}
					/* also check string length, if applicable */
					if (data->format[i].type == NV_STR) {
						if (strlen(item->fields[j]->values.v_char) > data->format[i].len) {
							nlog(LOG_WARNING, "Attempt to use too large a string in field %s in structure %s", data->format[i].fldname, data->name);
							nv_free_item(item);
							return NS_FAILURE;
						}
					}
				}
			}
	        	i++;
		}
	}
	/* if we get to hear, pass to the function to do the verification and actions */
	SET_RUN_LEVEL(data->mod);
	i = (int)data->updatehandler(item, action);
	RESET_RUN_LEVEL();
	/* free the item structure */
	nv_free_item(item);
	return i;;
}


int nv_get_field_item(nv_item *item, char *fldname) {
	int i = 0;
	while (item->no_fields > i) {
		if (!ircstrcasecmp(item->fields[i]->name, fldname)) 
			return i;
		i++;
	}
	return -1;
}

int nv_sf_string(nv_item *item, char *fldname, char *value) {
	int i;
	i = nv_get_field_item(item, fldname);
	if (i == -1) {
		i = item->no_fields++;
		item->fields[i] = ns_calloc(sizeof(nv_fields));
		item->fields[i]->name = strdup(fldname);
	}
	item->fields[i]->values.v_char = strdup(value);
	item->fields[i]->type = NV_PSTR;
	return NS_SUCCESS;
}
int nv_sf_int(nv_item *item, char *fldname, int value) {
	int i;

	i = nv_get_field_item(item, fldname);
	if (i == -1) {
		i = item->no_fields++;
		item->fields[i] = ns_malloc(sizeof(nv_fields));
		item->fields[i]->name = strdup(fldname); 
	}
	item->fields[i]->values.v_int = value;  
	item->fields[i]->type = NV_INT;
	return NS_SUCCESS;
}
int nv_sf_long(nv_item *item, char *fldname, long value) {
	int i;
	i = nv_get_field_item(item, fldname);
	if (i == -1) {
		i = item->no_fields++;
		item->fields[i] = ns_malloc(sizeof(nv_fields));
		item->fields[i]->name = strdup(fldname);
	}
	item->fields[i]->values.v_int = value;
	item->fields[i]->type = NV_LONG;
	return NS_SUCCESS;
}
