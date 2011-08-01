#include "neostats.h"
#include "namedvars.h"
#undef _
#ifdef WIN32
#undef getpid
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

XS(XS_NeoStats__NV_new); /* prototype to pass -Wmissing-prototypes */
XS(XS_NeoStats__NV__HashVars_DeleteNode); /* prototype to pass -Wmissing-prototypes */
XS(XS_NeoStats__NV__HashVars_AddNode); /* prototype to pass -Wmissing-prototypes */
XS(XS_NeoStats__NV__HashVars_FETCH); /* prototype to pass -Wmissing-prototypes */
XS(XS_NeoStats__NV__HashVars_EXISTS); /* prototype to pass -Wmissing-prototypes */
XS(XS_NeoStats__NV__HashVars_FIRSTKEY); /* prototype to pass -Wmissing-prototypes */
XS(XS_NeoStats__NV__HashVars_NEXTKEY); /* prototype to pass -Wmissing-prototypes */
XS(XS_NeoStats__NV__HashVars_ModNode); /* prototype to pass -Wmissing-prototypes */
void Init_Perl_NV() {

        newXSproto("NeoStats::NV::new", XS_NeoStats__NV_new, __FILE__, "$$");
        newXSproto("NeoStats::NV::HashVars::DeleteNode", XS_NeoStats__NV__HashVars_DeleteNode, __FILE__, "$$");
        newXSproto("NeoStats::NV::HashVars::AddNode", XS_NeoStats__NV__HashVars_AddNode, __FILE__, "$$$");
        newXSproto("NeoStats::NV::HashVars::ModNode", XS_NeoStats__NV__HashVars_ModNode, __FILE__, "$$$");
        newXSproto("NeoStats::NV::HashVars::FETCH", XS_NeoStats__NV__HashVars_FETCH, __FILE__, "$$");
        newXSproto("NeoStats::NV::HashVars::EXISTS", XS_NeoStats__NV__HashVars_EXISTS, __FILE__, "$$");
        newXSproto("NeoStats::NV::HashVars::FIRSTKEY", XS_NeoStats__NV__HashVars_FIRSTKEY, __FILE__, "$");
        newXSproto("NeoStats::NV::HashVars::NEXTKEY", XS_NeoStats__NV__HashVars_NEXTKEY, __FILE__, "$$");
}

/* XXX TODO: implement svREADONLY */

HV *perl_encode_namedvars(nv_list *nv, void *data) {
	HV *ret;
	int i =0;
	ret = newHV();
	while (nv->format[i].fldname != NULL) {
		switch(nv->format[i].type) {
			case NV_PSTR:
			case NV_STR:
				hv_store(ret, nv->format[i].fldname, strlen(nv->format[i].fldname),
					newSVpv(nv_gf_string(data, nv, i), strlen(nv_gf_string(data, nv, i))), 0);
				break;
			case NV_INT:
			case NV_LONG:
				hv_store(ret, nv->format[i].fldname, strlen(nv->format[i].fldname),
					newSViv(nv_gf_int(data, nv, i)), 0);
				break;
			case NV_VOID:
			case NV_PSTRA:
				nlog(LOG_WARNING, "perl_encode_namedvars: void/string todo!");
				break;
		}
	i++;
	}
	return ret;
}

nv_item *perl_store_namedvars(nv_list *nv, HV *values) {
	int i, j;
    nv_item *item;
	SV **value;
	i = 0;
	j = 0;
	item = nv_new_item(nv);
	while (nv->format[i].fldname != NULL) {
		if (hv_exists(values, nv->format[i].fldname, strlen(nv->format[i].fldname))) {
			value = hv_fetch(values, nv->format[i].fldname, strlen(nv->format[i].fldname), FALSE);
		} else {
			i++;
			continue;
		}
		switch (nv->format[i].type) {
			case NV_PSTR:
			case NV_STR:
				nv_sf_string(item, nv->format[i].fldname, SvPV_nolen(*value));
				break;
			case NV_INT:
				nv_sf_int(item, nv->format[i].fldname, SvIV(*value));
				break;
			case NV_LONG:
				nv_sf_long(item, nv->format[i].fldname, SvIV(*value));
				break;
			case NV_VOID:
			default:
				printf("Value: Unhandled!\n");
				break;
		}
		i++;
	}
	return item;
}



#define	RETURN_UNDEF_IF_FAIL { if ((int)RETVAL < 0) XSRETURN_UNDEF; }

MODULE = NeoStats::NV		PACKAGE = NeoStats::NV		

void
new(class, varname)
   char      *varname;
   char      *class;
PREINIT:
   HV        *tie;
   SV        *nv_link;
   HV        *tieref;
   nv_list   *nv;
PPCODE:
   nv = FindNamedVars(varname);
   if (!nv) {
	nlog(LOG_WARNING, "Perl NV: Can't find NamedVar list %s %s", varname, class);
     	tieref = (HV *)&PL_sv_undef;
   } else {
	/* tie the hash to the package (FETCH/STORE) below */
   	tie = newHV();
	tieref = (HV *)newRV_noinc((SV*)tie);
   	sv_bless((SV *)tieref, gv_stashpv("NeoStats::NV::HashVars", TRUE));
	hv_magic(tie, (GV*)tieref, 'P'); 
	/* this one allows us to store a "pointer" 
         */
   	nv_link = newSViv((int)nv);
   	sv_magic(SvRV(tieref), nv_link, '~', 0, 0);
   	SvREFCNT_dec(nv_link);
   }
   /* return the hash */
   EXTEND(SP,1);
   PUSHs(sv_2mortal((SV *)tieref));

MODULE = NeoStats::NV		PACKAGE = NeoStats::NV::HashVars


IV
DeleteNode(self, key)
   SV		*self;
   SV		*key;
PREINIT:
   STRLEN        klen;
   MAGIC        *mg;
   nv_list	*nv;
   nv_item	*item;
CODE:
   RETVAL = (IV)-1;
   /* find our magic */
   mg   = mg_find(SvRV(self),'~');
   if(!mg) { croak("lost ~ magic"); }
   /* this is the nv_hash we are point at */
   nv = (nv_list *)SvIV(mg->mg_obj);
   item = nv_new_item(nv);
   /* make sure its a hash, not a list */
   if (nv->type == NV_TYPE_HASH) {
	/* get the "key" they want */
	item->index.key    = SvPV(key, klen);
	/* search for the key */
	item->type = nv->type;
   } else if (nv->type == NV_TYPE_LIST) {
	/* get the position */ 
	item->index.pos = SvIV(key);
	item->type = nv->type;
   }
   RETVAL = (IV)nv_update_structure(nv, item, NV_ACTION_DEL);
POSTCALL:
	RETURN_UNDEF_IF_FAIL;
OUTPUT:
	RETVAL

IV
AddNode(self, key, data)
   SV		*self;
   SV		*key;
   HV		*data
PREINIT:
   STRLEN        klen;
   MAGIC        *mg;
   nv_list	*nv;
   nv_item	*item;
CODE:
   RETVAL = (IV)-1;
   /* find our magic */
   mg   = mg_find(SvRV(self),'~');
   if(!mg) { croak("AddNode: lost ~ magic"); }
   /* this is the nv_hash we are point at */
   nv = (nv_list *)SvIV(mg->mg_obj);
   /* encode the data into item */
   item = perl_store_namedvars(nv, data);
   if (item) {
	   /* make sure its a hash, not a list */
	   if (nv->type == NV_TYPE_HASH) {
		/* get the "key" they want */
		item->index.key    = SvPV(key, klen);
		item->type = nv->type;
	   } else if (nv->type == NV_TYPE_LIST) {
		/* add on a list, pos will always be -1, so ignore key */
		item->index.pos = -1;
		item->type = nv->type;
	   }
   }
   RETVAL = (IV)nv_update_structure(nv, item, NV_ACTION_ADD);
POSTCALL:
	RETURN_UNDEF_IF_FAIL;
OUTPUT:
	RETVAL

IV
ModNode(self, key, data)
   SV		*self;
   SV		*key;
   HV		*data
PREINIT:
   STRLEN        klen;
   MAGIC        *mg;
   nv_list	*nv;
   nv_item	*item;
CODE:
   RETVAL = (IV)-1;
   /* find our magic */
   mg   = mg_find(SvRV(self),'~');
   if(!mg) { croak("AddNode: lost ~ magic"); }
   /* this is the nv_hash we are point at */
   nv = (nv_list *)SvIV(mg->mg_obj);
   /* encode the data into item */
   item = perl_store_namedvars(nv, data);
   if (item) {
	   /* make sure its a hash, not a list */
	   if (nv->type == NV_TYPE_HASH) {
		/* get the "key" they want */
		item->index.key    = SvPV(key, klen);
		item->type = nv->type;
	   } else if (nv->type == NV_TYPE_LIST) {
		/* add on a list, pos will always be -1, so ignore key */
		item->index.pos = SvIV(key);
		item->type = nv->type;
	   }
   }
   RETVAL = (IV)nv_update_structure(nv, item, NV_ACTION_MOD);
POSTCALL:
	RETURN_UNDEF_IF_FAIL;
OUTPUT:
	RETVAL





#/* get a individual entry. self points to what we set with sv_magic in
#* new function above */

HV *
FETCH(self, key)
   SV           *self;
   SV           *key;
PREINIT:
   char         *k;
   STRLEN        klen;
   MAGIC        *mg;
   nv_list	*nv;
   void 	*data;
   int		pos, i;
   lnode_t 	*lnode;
CODE:
   RETVAL = (HV *)-1;
   /* find our magic */
   mg   = mg_find(SvRV(self),'~');
   if(!mg) { croak("lost ~ magic"); }
   /* this is the nv_hash we are point at */
   nv = (nv_list *)SvIV(mg->mg_obj);
   /* make sure its a hash, not a list */
   if (nv->type == NV_TYPE_HASH) {
	   /* get the "key" they want */
	   k    = SvPV(key, klen);
	   /* search for the key */
	   data = hnode_find((hash_t *)nv->data, k);
	   if (!data) {
		RETVAL = (HV *)-1;
	   } else {
		RETVAL = (HV *)perl_encode_namedvars(nv, data);	   
	   }
   } else if (nv->type == NV_TYPE_LIST) {
	   /* get the position */
	   pos = SvIV(key);
	   lnode = list_first((list_t *)nv->data);;
	   if (!lnode) {
		RETVAL = (HV *)-1;
	   } else {
		   for (i = 0; i < pos; i++) {
				lnode = list_next((list_t *)nv->data, lnode);
		   }
		   if (lnode) {
			   RETVAL = perl_encode_namedvars(nv, lnode_get(lnode));
		   } else
			   RETVAL = (HV *)-1;
	  }
   }
POSTCALL:
	RETURN_UNDEF_IF_FAIL;
OUTPUT:
	RETVAL


# /* check if a entry is in the hash */

bool
EXISTS(self, key)
   SV   *self;
   SV   *key;
PREINIT:
   HV   *hash;
   char *k;
   nv_list *nv;
   MAGIC        *mg;
   STRLEN        klen;
   char 	*data;
   int 		pos;
CODE:
   RETVAL = 0;
   hash = (HV*)SvRV(self);
   mg   = mg_find(SvRV(self),'~');
   if(!mg) { croak("lost ~ magic"); }
   /* this is the nv_hash we are point at */
   k    = SvPV(key, PL_na);
   nv = (nv_list *)SvIV(mg->mg_obj);
   /* make sure its a hash, not a list */
   if (nv->type == NV_TYPE_HASH) {
	   /* get the "key" they want */
	   k    = SvPV(key, klen);
	   /* search for the key */
	   data = hnode_find((hash_t *)nv->data, k);
	   if (!data) {
		RETVAL = 0;
	   } else {
		RETVAL = 1;
	   }
   } else if (nv->type == NV_TYPE_LIST) {
	   pos = SvIV(key);
	   if (pos > list_count((list_t *)nv->data)) 
		RETVAL = 0;
	   else
		RETVAL = 1;
   }
OUTPUT:
   RETVAL

#/* get the first entry from a hash */

SV*
FIRSTKEY(self)
   SV *self;
PREINIT:
   HV *hash;
   MAGIC *mg;
   nv_list *nv;
   hnode_t *node;
CODE:
   RETVAL = &PL_sv_undef;
   hash = (HV*)SvRV(self);
   mg   = mg_find(SvRV(self),'~');
   if(!mg) { croak("lost ~ magic"); }
   nv = (nv_list *)SvIV(mg->mg_obj);
   /* make sure its a hash, not a list */
   if (nv->type == NV_TYPE_HASH) {
	hash_scan_begin( &nv->iter.hscan, (hash_t *)nv->data);
	node = hash_scan_next(&nv->iter.hscan);
	nv->itercount = 0;
	if (!node) {
		RETVAL = &PL_sv_undef;
	} else {
		RETVAL = newSVpv(hnode_getkey(node), 0);
	}
   } else if (nv->type == NV_TYPE_LIST) {
	nv->iter.node = list_first((list_t *)nv->data);
	nv->itercount = 0;
	RETVAL = newSVpv("0", 0);
   }
POSTCALL:
	RETURN_UNDEF_IF_FAIL;
OUTPUT:
	RETVAL

#/* get the next entry from a cache */

SV*
NEXTKEY(self, lastkey)
   SV *self;
PREINIT:
   HV *hash;
   MAGIC *mg;
   hnode_t *node;
   nv_list *nv;
   char tmpstr[BUFSIZE];
CODE:
   RETVAL = &PL_sv_undef;
   hash = (HV*)SvRV(self);
   mg   = mg_find(SvRV(self),'~');
   if(!mg) { croak("lost ~ magic"); }
   nv = (nv_list *)SvIV(mg->mg_obj);
   /* make sure its a hash, not a list */
   if (nv->type == NV_TYPE_HASH) {
	node = hash_scan_next(&nv->iter.hscan);
	nv->itercount++;
	if (!node) {
		RETVAL = &PL_sv_undef;
	} else {
		RETVAL = newSVpv(hnode_getkey(node), 0);
	}
   } else if (nv->type == NV_TYPE_LIST) {
	nv->itercount++;
	if (nv->itercount >= list_count((list_t *)nv->data)) {
		RETVAL = &PL_sv_undef;
	} else {
		ircsnprintf(tmpstr, BUFSIZE, "%d", nv->itercount);
		RETVAL =newSVpv(tmpstr,0);
	}
   }
POSTCALL:
	RETURN_UNDEF_IF_FAIL;
OUTPUT:
	RETVAL

#/* delete a entry */

