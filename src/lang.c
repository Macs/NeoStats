/*  LibLang - Message Translation and Retrival Library 
**  Copyright (c) 2004 Justin Hammond
** 
**  Portions Copyright Mark Hetherington, NeoStats Software
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
** LibLang CVS Identification
** $Id: lang.c 12 2004-08-10 12:47:20Z Fish $
*/

#include "neostats.h"
#if 0 /* Temp since berkeley is causing too many problems */
/* #if defined HAVE_DB_H && !defined WIN32 */
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif /* HAVE_STRINGS_H */
#ifdef HAVE_DB_H
#include <db.h>
#endif /* HAVE_DB_H */
#include "lang.h"

static DBT dbkey;
static DBT dbdata;
static int dbret;

typedef struct lang_type {
	DB* dbp;
	char langname[LANGNAMESIZE];
	int ok;
}lang_entry;

lang_entry lang_list[MAXLANG];

struct lang_info {
	int nooflangs;
	DB *dbp;
	int langdebug;
	LANGDebugFunc debugfunc;
	char dbpath[STRSIZE];
}lang_info;

char TranslatedLang[STRSIZE];
hash_t *langcache[MAXLANG];
static void* LANGGetData(void* key, int lang);

static void LANGDebug(char *file, int line, char *func, int err, char *fmt, ...) {
	va_list ap;
	char buf[800];
	char buf2[1024];
	if (lang_info.langdebug != 1 && err != 1) {
		return;
	}
	va_start(ap, fmt);
	ircsnprintf(buf, 800, fmt, ap);
	va_end(ap);
	ircsnprintf(buf2, 1024, "%s:%d(%s): %s", file, line, func, buf);
	if (lang_info.debugfunc) {
		lang_info.debugfunc("%s", buf2);
	} else {
		fprintf(stderr, "%s\n", buf2);
	}
}	

	

static int LANGOpenDatabase()
{
	int rc, i;
	DBT key, data;
	DBC *dbcp;
	
	lang_info.nooflangs = 0;
	
	LANGDEBUG("LANGOpenDatabase", "");
	if ((rc = db_create(&lang_info.dbp, NULL, 0)) != 0) {
		LANGERROR("Lang List Create Failed: %s", db_strerror(rc));
		return -1;
	}
	if ((rc = lang_info.dbp->open(lang_info.dbp, lang_info.dbpath, "LIST", DB_BTREE, DB_CREATE, 0)) != 0) {
		LANGERROR("Opening Lang LIst failed: %s", db_strerror(rc));
		lang_info.dbp->close(lang_info.dbp, 0);
		return -1;
	}
	/* Acquire a cursor for the database. */
	if ((rc = lang_info.dbp->cursor(lang_info.dbp, NULL, &dbcp, 0)) != 0) {
		LANGERROR("Getting a Cursor Failed: %s", db_strerror(rc));
		lang_info.dbp->close(lang_info.dbp, 0);
		dbcp->c_close(dbcp);
		return -1;
	}
	
	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	rc = 0;
	/* Walk through the database and print out the key/data pairs. */
	while ((rc = dbcp->c_get(dbcp, &key, &data, DB_NEXT)) == 0) {
		if (data.size+1 > LANGNAMESIZE) {
			LANGERROR("LangName is bigger than our defined size in %s at %d", __FILE__, __LINE__);
			abort();
			return -1;
		}
		if (lang_info.nooflangs+1 >= MAXLANG) {
			LANGERROR("Max No of Languages Reached %s. Please Recompile", MAXLANG);
			abort();
			continue;
		}
		ircsnprintf(lang_list[lang_info.nooflangs].langname, data.size+1, "%s", (char *)data.data);
		LANGDEBUG("Databases: %.*s : %.*s",
		    (int)key.size, (char *)key.data,
    		    (int)data.size, (char *)data.data);
		lang_info.nooflangs++;
	}
	if (rc != DB_NOTFOUND) {
		lang_info.dbp->close(lang_info.dbp, 0);
		dbcp->c_close(dbcp);
		LANGERROR("Lang List Database Not found: %s", db_strerror(rc));
		return -1;
	}							    		    				

	/* ok, we got the database list close the cursors */
	dbcp->c_close(dbcp);	
	lang_info.dbp->close(lang_info.dbp, 0);


	for (i=0; i < lang_info.nooflangs; i++) {
		LANGDEBUG("opening %s", lang_list[i].langname);
		if ((dbret = db_create(&lang_list[i].dbp, NULL, 0)) != 0) {
			LANGERROR("db_create: %s", db_strerror(dbret));
			return -1;
		}
		if ((rc = lang_list[i].dbp->open(lang_list[i].dbp, lang_info.dbpath, lang_list[i].langname, DB_BTREE, DB_CREATE, 0)) != 0) {
			LANGERROR("dbp->open: %s", db_strerror(rc));
			continue;
		}
		lang_list[i].ok = 1;
		lang_stats.lang_list[i] = sstrdup(lang_list[i].langname);
		lang_stats.noofloadedlanguages++;
	}
	LANGDEBUG("opened %d langs", lang_info.nooflangs);
	return 1;
}

int LANGDumpDB(char *lang, int missing, void *mylist)
{
	int rc;
	DBT key, data;
	DBC *dbcp;
	char tmpbuf[STRSIZE];
	int count;
	hnode_t *node;
	langdump *dump;
	int mylang;
	
	rc = LANGfindlang(DEFDATABASE);
	if (rc == -1) {
		LANGERROR("LANGDumpDB: Missing Default Database %s", DEFDATABASE);
		return -1;
	}
	mylang = LANGfindlang(lang);
	/* Acquire a cursor for the default database. */
	if ((rc = lang_list[rc].dbp->cursor(lang_list[rc].dbp, NULL, &dbcp, 0)) != 0) {
		LANGERROR("DBDump: Getting a Cursor Failed: %s (%d)", db_strerror(rc), missing);
		dbcp->c_close(dbcp);
		return -1;
	}
	
	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	rc = 0;
	count = 0;
	/* Walk through the database and print out the key/data pairs. */
	while ((rc = dbcp->c_get(dbcp, &key, &data, DB_NEXT)) == 0) {
		ircsnprintf(tmpbuf, STRSIZE, "%.*s", data.size, (char *)data.data);
		if (!missing) {
			/* only dump the missing values) */
			if (LANGGotData(tmpbuf, lang) == 1) {
				continue;
			}
		}
		LANGDEBUG("Entries: %.*s : %.*s",
		    (int)key.size, (char *)key.data,
		    (int)data.size, (char *)data.data);
		count++;
		dump = malloc(sizeof(langdump));
		ircsnprintf(dump->key, KEYSIZE, "%.*s", key.size, (char *)key.data);
		ircsnprintf(dump->realstring, STRSIZE, "%.*s", data.size, (char *)data.data);
		ircsnprintf(dump->string, STRSIZE, "%s", (char *)LANGGetData(tmpbuf, mylang));
		node = hnode_create(dump);
		hash_insert((hash_t *)mylist, node, dump->realstring);
	}
	return count;
}	

int LANGNewLang(char *lang) 
{
	DBT dbkey, dbdata;
	int rc;
	if (strlen(lang) >= LANGNAMESIZE) {
		LANGERROR("LANGNewLang: Error, Name to long %d", strlen(lang));
		return -1;
	}
	if (lang_info.nooflangs +1 >= MAXLANG) {
		LANGERROR("LANGNewLang: Max no of langs reached. %d", lang_info.nooflangs);
		abort();
		return -1;
	}

	if ((rc = db_create(&lang_info.dbp, NULL, 0)) != 0) {
		LANGERROR("Lang List Create Failed: %s", db_strerror(rc));
		return -1;
	}
	if ((rc = lang_info.dbp->open(lang_info.dbp, lang_info.dbpath, "LIST", DB_BTREE, DB_CREATE, 0)) != 0) {
		LANGERROR("Opening Lang LIst failed: %s", db_strerror(rc));
		lang_info.dbp->close(lang_info.dbp, 0);
		return -1;
	}

	
	ircsnprintf(lang_list[lang_info.nooflangs].langname, LANGNAMESIZE, "%s", lang);
	if ((dbret = db_create(&lang_list[lang_info.nooflangs].dbp, NULL, 0)) != 0) {
		LANGERROR("db_create: %s", db_strerror(dbret));
		return -1;
	}

	if ((rc = lang_list[lang_info.nooflangs].dbp->open(lang_list[lang_info.nooflangs].dbp, lang_info.dbpath, lang_list[lang_info.nooflangs].langname, DB_BTREE, DB_CREATE, 0)) != 0) {
		LANGERROR("dbp->open: %s", db_strerror(rc));
		return -1;
	} else {
		lang_list[lang_info.nooflangs].ok = 1;
		lang_info.nooflangs++;
		memset(&dbkey, 0, sizeof(dbkey));
		memset(&dbdata, 0, sizeof(dbdata));
		dbkey.data = lang;
		dbkey.size = strlen(lang);
		dbdata.data = lang;
		dbdata.size = strlen(lang);
		if ((dbret = lang_info.dbp->put(lang_info.dbp, NULL, &dbkey, &dbdata, DB_NOOVERWRITE)) != 0) {
			LANGERROR("dbp->put: %s", db_strerror(dbret));
		}
		lang_info.dbp->close(lang_info.dbp, 0);		
		lang_stats.noofloadedlanguages++;
		lang_stats.lang_list[lang_info.nooflangs] = sstrdup(lang);
		return lang_info.nooflangs - 1;
	}	
}		

static void LANGCloseDatabase(int lang)
{
	if (lang_list[lang].ok == 1) {
		lang_list[lang].dbp->close(lang_list[lang].dbp, 0); 
		LANGDEBUG("LANGCloseDatabase %d", lang);
	}
	free(lang_stats.lang_list[lang]);
	lang_stats.lang_list[lang] = NULL;
	lang_stats.noofloadedlanguages--;
	lang_info.nooflangs--;
}

static void* LANGGetData(void* key, int lang)
{

	char tmpbuf[KEYSIZE];
	
	
	LANGDEBUG("LANGGetData %d", lang);
	if (lang_list[lang].ok != 1) {
		LANGDEBUG("Invalid Lang %d", lang);
		return NULL;
	}
	lang_stats.dbhits[lang]++;
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	/* XXX Fix this */
	ircsnprintf(tmpbuf, KEYSIZE, "%x", (unsigned int)hash_fun_default(key));
	LANGDEBUG("LangGetData: Looking for %s (%s)", (char *)key, tmpbuf);
	dbkey.data = tmpbuf;
	dbkey.size = strlen(tmpbuf);
	if ((dbret = lang_list[lang].dbp->get(lang_list[lang].dbp, NULL, &dbkey, &dbdata, 0)) == 0)
	{
		lang_stats.dbhits[lang]++;
		LANGDEBUG("found %.*s",dbdata.size, (char *)dbdata.data);
		if (dbdata.size > STRSIZE) {
			LANGERROR("LANGGetData: Translated string is to big %d", dbdata.size);
			abort();
			/* continue anyway, so at least we can return a partial string */
		}
		ircsnprintf(TranslatedLang, STRSIZE, "%.*s", dbdata.size, (char *)dbdata.data);
		/* XXX Fixme, we shouldn't use a global var here*/
		return TranslatedLang;
	}
	LANGDEBUG("LANGGetData: dbp->get: fail %s for %s", db_strerror(dbret), (char *)key);
	lang_stats.dbfails[lang]++;
	return NULL;
}
int LANGfindlang(char *lang) {
	int i;
	for (i = 0; i < lang_info.nooflangs; i++) {
		if (!ircstrcasecmp(lang_list[i].langname, lang)) {
			LANGDEBUG("Found Lang %s as %d", lang, i);
			return i;
		}
	}
	return -1;
}

int LANGGotData(char *key, char *lang) {
	int rc;
	rc = LANGfindlang(lang);
	if (rc == -1) {
		/* no Hit */
		return -1;
	}
	if (LANGGetData(key, rc) == NULL) {
		return -1;
	} else {
		return 1;
	}
}

void LANGSetData(char* key, void* data, int size, char *lang, int keydone)
{
	int rc;
	char tmpbuf[KEYSIZE];
	/* first, find out if this lang is loaded already */
restart:
	rc = LANGfindlang(lang);
	if (rc == -1) {
		/* open a new language */
		LANGNewLang(lang);
		goto restart;
	}


	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	if (keydone == 0) {
		ircsnprintf(tmpbuf, KEYSIZE, "%x", (unsigned int)hash_fun_default(key));
	} else {
		ircsnprintf(tmpbuf, KEYSIZE, "%s", key);
	}
	LANGDEBUG("LANGSetData %s = %.*s (%s) (%d)", key, size, (char *)data, tmpbuf, rc);
	dbkey.data = tmpbuf;
	dbkey.size = strlen(tmpbuf);
	dbdata.data = data;
	dbdata.size = size;
	if ((dbret = lang_list[rc].dbp->put(lang_list[rc].dbp, NULL, &dbkey, &dbdata, DB_NOOVERWRITE)) != 0) {
		LANGERROR("dbp->put: %s", db_strerror(dbret));
		lang_stats.dbfails[rc]++;
		return;
	}
	lang_stats.dbupdates[rc]++;
}

char *LANGgettext(const char *string, int mylang)
{
	hnode_t *node;
	char *transtring;
	char *mytransstring;
	
	if (mylang == -1) {
		LANGDEBUG("Default Lang Hit %d", mylang);
		return (char *)string;
	}
	LANGDEBUG("using Lang %d", mylang);
	node = hash_lookup(langcache[mylang], string);
	if (node) {
		transtring = hnode_get(node);
		LANGDEBUG("Translated %s to %s", string, transtring);
		lang_stats.cachehits[mylang]++;
		return transtring;
	} else {
		/* here, for the sake of clarity we should compute the hash, but for examples forget it */
		transtring = LANGGetData((char *)string, mylang);
		if (transtring) {
			/* because whats returned for BDB is only valid till the next call */
			mytransstring = malloc(strlen(transtring));
			strcpy(mytransstring, transtring);
			/* insert into the cache */
			node = hnode_create(mytransstring);
			hash_insert(langcache[mylang], node, string);
			return mytransstring;
		} else {
			LANGDEBUG("Cant find translation for key \"%s\"", string);
			/* if it can't be found, return the default */
			lang_stats.transfailed[mylang]++;
			return (char *)string;
		}
	}
	/* Never reached! */
	return "Translation Error";	
}	

void LANGinit(int debug, char *dbpath, LANGDebugFunc debugfunc) 
{
	int i;
	lang_info.langdebug = debug;
	lang_info.debugfunc = debugfunc;
	
	if (dbpath != NULL) {
		strncpy(lang_info.dbpath, dbpath, STRSIZE);
	} else {
		/* use the current directory */
		strncpy(lang_info.dbpath, "lang.db", STRSIZE);
	}

	for (i = 0; i < 10; i++) {
		langcache[i] = hash_create(-1, 0, 0);
		lang_list[i].ok = 0;
	}
	LANGOpenDatabase(i);
	
}

void LANGfini() 
{
	hscan_t hc;
	hnode_t *node;
	int i;
	for (i = 0; i < 10; i++) {
		hash_scan_begin(&hc, langcache[i]);
		while ((node = hash_scan_next(&hc)) != NULL) {
			free(hnode_get(node));
		}
		hash_free_nodes(langcache[i]);
		hash_destroy(langcache[i]);
		LANGCloseDatabase(i);
	}
}
#else /* HAVE_DB_H */
#include "lang.h"
void LANGinit(int debug, char *dbpath, LANGDebugFunc debugfunc) 
{
}

void LANGfini() 
{
}
#endif /* HAVE_DB_H */
