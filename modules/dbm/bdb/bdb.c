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
** $Id: bdb.c 3300 2008-02-27 02:26:39Z Fish $
*/

#include "neostats.h"
#include "nsdbm.h"

#ifdef DB_HEADER
#include DB_HEADER
#include <sys/types.h>
#include <dirent.h>
          
static DBT dbkey;
static DBT dbdata;

static DB_ENV *db_env = NULL;

static int dbopened = 0;

/* some older versions of BDB dun have this */
#ifndef DB_VERB_REGISTER
#define DB_VERB_REGISTER 0
#endif
#ifndef DB_REGISTER
#define DB_REGISTER 0
#endif


#define BDB_VERSION_AT_LEAST(major,minor) \
    (DB_VERSION_MAJOR > (major) \
         || (DB_VERSION_MAJOR == (major) && DB_VERSION_MINOR >= (minor)))

/* In BDB 4.3, the error gatherer function grew a new DBENV parameter,
   and the MSG parameter's type changed. */
#if BDB_VERSION_AT_LEAST(4,3)
/* Prevents most compilers from whining about unused parameters. */
#define BDB_ERROR_GATHERER_IGNORE(varname) ((void)(varname))
#else
#define bdb_error_gatherer(param1, param2, param3) \
        bdb_error_gatherer(param2, char *msg)
#define BDB_ERROR_GATHERER_IGNORE(varname) ((void)0)
#endif


void bdb_error_gatherer(const DB_ENV *dbenv, const char *prefix, const char *msg) {
	BDB_ERROR_GATHERER_IGNORE(dbenv);
	nlog(LOG_WARNING, "BDB Error: %s", msg);
}

void bdb_msg_gatherer(const DB_ENV *dbenv, const char *msg) {
	dlog(DEBUG10, "BDB Info: %s", msg);
}


int CheckPointBDB( void *userptr ) {
	if (db_env)
		db_env->txn_checkpoint(db_env, 0, 0, DB_FORCE);
	return NS_SUCCESS;
}

void *DBMOpenDB (const char *name)
{
	int dbret;
	
	if (!db_env) {
		if ((dbret = db_env_create(&db_env, 0)) != 0) {
			nlog(LOG_WARNING, "db_env_create failed: %s", db_strerror(dbret));
			return NULL;
		}
		db_env->set_verbose(db_env, DB_VERB_RECOVERY|DB_VERB_REGISTER, 1);
		db_env->set_flags(db_env, DB_TXN_WRITE_NOSYNC, 1);
#ifdef DB_LOG_AUTOREMOVE
		db_env->set_flags(db_env, DB_LOG_AUTOREMOVE, 1);
#endif
		if ((dbret = db_env->open(db_env, "data/", DB_RECOVER|DB_REGISTER|DB_CREATE|DB_INIT_TXN|DB_INIT_MPOOL|DB_INIT_LOCK|DB_INIT_LOG, 0600)) != 0) {
			nlog(LOG_WARNING, "db evn open failed: %s", db_strerror(dbret));
			db_env->close(db_env, 0);
			return NULL;
		}
#if BDB_VERSION_AT_LEAST(4,3)
		db_env->set_errcall(db_env, (bdb_error_gatherer));
		db_env->set_msgcall(db_env, (bdb_msg_gatherer));
		db_env->stat_print(db_env, DB_STAT_ALL|DB_STAT_SUBSYSTEM);
#endif
		/* timer to checkpoint the databases */
		AddTimer( TIMER_TYPE_INTERVAL, CheckPointBDB, "CheckPintBDB", 3600, NULL );
		
	}
	dbopened++;
	return strdup(name);
}

void DBMCloseDB (void *dbhandle)
{
	ns_free(dbhandle);
	dbopened--;
	if (dbopened <= 0) {
#if BDB_VERSION_AT_LEAST(4,3)
		db_env->stat_print(db_env, DB_STAT_ALL|DB_STAT_SUBSYSTEM);
#endif
		db_env->close(db_env, 0);
		db_env = NULL;
	} else {
		dlog(DEBUG10, "DBMClose: Databases still opened, not destroying enviroment");
	}
	return;
}

void *DBMOpenTable (void *dbhandle, const char *name)
{
	static char filename[MAXPATH];
	int dbret;
	DB *dbp;

	dlog (DEBUG10, "DBMOpenTable %s", name);
	if (db_env == NULL) {
		nlog(LOG_WARNING, "DataBase Enviroment is not created\n");
		return NULL;
	}
	ircsprintf (filename, "%s.bdb", (char *)dbhandle);
	if ((dbret = db_create(&dbp, db_env, 0)) != 0) {
		dlog(DEBUG10, "db_create: %s", db_strerror(dbret));
		return NULL;
	}
#if (DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR >= 1)
	if ((dbret = dbp->open(dbp, NULL, filename, name, DB_BTREE, DB_CREATE|DB_AUTO_COMMIT, 0600)) != 0) {
#else
	if ((dbret = dbp->open(dbp, filename, name, DB_BTREE, DB_CREATE|DB_AUTO_COMMIT, 0600)) != 0) {
#endif
		nlog(LOG_WARNING, "dbp->open: %s", db_strerror(dbret));
		return NULL;
	}
	return (void *)dbp;
}

void DBMCloseTable (void *dbhandle, void *tbhandle)
{
	DB *dbp = (DB *)tbhandle;

	dlog(DEBUG10, "DBMCloseTable");
	dbp->close(dbp, 0); 
}

int DBMFetch (void *dbhandle, void *tbhandle, char *key, void *data, int size)
{
	int dbret;
	DB *dbp = (DB *)tbhandle;

	dlog(DEBUG10, "DBMFetch %s", key);
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	dbkey.data = key;
	dbkey.size = strlen(key);
	if ((dbret = dbp->get(dbp, NULL, &dbkey, &dbdata, 0)) == 0)
	{
		os_memcpy (data, dbdata.data, size);
		return NS_SUCCESS;
	}
	if (dbret != DB_NOTFOUND) 
		dlog(DEBUG10, "dbp->get fail: %s", db_strerror(dbret));
	return NS_FAILURE;
}

int DBMStore (void *dbhandle, void *tbhandle, char *key, void *data, int size)
{
	int dbret;
	DB *dbp = (DB *)tbhandle;
	DB_TXN *txn = NULL;

	dlog(DEBUG10, "DBMStore %s %s", key, (char *)data);
	if ((dbret = db_env->txn_begin(db_env, NULL, &txn, 0)) != 0) {
		nlog(LOG_WARNING, "db_env->txn_begin: %s", db_strerror(dbret));
		return NS_FAILURE;
	}
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	dbkey.data = key;
	dbkey.size = strlen(key);
	dbdata.data = data;
	dbdata.size = size;
	if ((dbret = dbp->put(dbp, txn, &dbkey, &dbdata, 0)) != 0) {
		if (dbret != DB_NOTFOUND) {
			nlog(LOG_WARNING, "dbp->put: %s", db_strerror(dbret));
			if ((dbret = txn->abort(txn)) != 0) {
				nlog(LOG_WARNING, "txn->commit: %s", db_strerror(dbret));
			}
			return NS_FAILURE;
		} else {
			if ((dbret = txn->commit(txn, 0)) != 0) {
				nlog(LOG_WARNING, "txn->commit: %s", db_strerror(dbret));
			}
			return NS_SUCCESS;
		}
	}
	if ((dbret = txn->commit(txn, 0)) != 0) {
		nlog(LOG_WARNING, "txn->commit: %s", db_strerror(dbret));
	}

	return NS_SUCCESS;
}

int DBMFetchRows (void *dbhandle, void *tbhandle, DBRowHandler handler)
{
	int rowcount = 0;
	int dbret;
	DB *dbp = (DB *)tbhandle;
	DBC *dbcp;

	dlog(DEBUG10, "DBMFetchRows here");
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	/* initilize the cursors */
	if ((dbret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
		nlog(LOG_WARNING, "DB Cursor failed: %s", db_strerror(dbret));
		return rowcount;
	}
	
	while ((dbret = dbcp->c_get(dbcp, &dbkey, &dbdata, DB_NEXT)) == 0)
	{
		rowcount++;
                if( handler( dbdata.data, dbdata.size ) != 0 ) {
                        break;
		}
	
	} 
	if (dbret != 0 && dbret != DB_NOTFOUND) {
		dlog(DEBUG10, "dbp->c_get failed: %s", db_strerror(dbret));
	}
	if ((dbret = dbcp->c_close(dbcp)) != 0) {
		nlog(LOG_WARNING, "dbcpp->close failed: %s", db_strerror(dbret));
	}	
	return rowcount;
}

int DBMFetchRows2 (void *dbhandle, void *tbhandle, DBRowHandler2 handler)
{
	int rowcount = 0;
	int dbret;
	DB *dbp = (DB *)tbhandle;
	DBC *dbcp;

	dlog(DEBUG10, "DBMFetchRows2 here");
	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	/* initilize the cursors */
	if ((dbret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
		nlog(LOG_WARNING, "DB Cursor failed: %s", db_strerror(dbret));
		return rowcount;
	}
	
	while ((dbret = dbcp->c_get(dbcp, &dbkey, &dbdata, DB_NEXT)) == 0)
	{

		rowcount++;
                if( handler( dbkey.data, dbdata.data, dbdata.size ) != 0 ) {
                        break;
		}
	} 
	if (dbret != 0 && dbret != DB_NOTFOUND) {
		dlog(DEBUG10, "dbp->c_get failed: %s", db_strerror(dbret));
	}
	if ((dbret = dbcp->c_close(dbcp)) != 0) {
		nlog(LOG_WARNING, "dbcpp->close failed: %s", db_strerror(dbret));
	}	
	return rowcount;
}

int DBMDelete (void *dbhandle, void *tbhandle, char * key)
{
	int dbret;
	DB *dbp = (DB *)tbhandle;

	dlog(DEBUG10, "DBMDelete %s", key);
	memset(&dbkey, 0, sizeof(dbkey));
	dbkey.data = key;
	dbkey.size = strlen(key);
	if ((dbret = dbp->del(dbp, NULL, &dbkey, 0)) != 0)
	{
		if (dbret != DB_NOTFOUND) {
			nlog(LOG_WARNING, "dbp->del failed: %s", db_strerror(dbret));
			return NS_FAILURE;
		}
		return NS_SUCCESS;
	}
	return NS_SUCCESS;
}

int file_select (struct dirent *entry) {
	char *ptr;
	if ((ircstrcasecmp(entry->d_name, ".")==0) || (ircstrcasecmp(entry->d_name, "..")==0)) 
		return 0;
	/* check filename extension */
	ptr = strrchr(entry->d_name, '.');
	if ((ptr) && !(ircstrcasecmp(ptr, ".bdb"))) {
		return NS_SUCCESS;
	}
	return 0;	
}

char **DBMListDB()
{
	struct dirent **files;
	int count, i, sl = 0;
	char *filename;
	char **DBList = NULL;;
	
	count = scandir ("data/", &files, file_select, alphasort);
	for (i = 2; i <= count; i++) 
	{
		
		filename = ns_malloc(strlen(files[i-1]->d_name) - 3);
		strlcpy(filename, files[i-1]->d_name, strlen(files[i-1]->d_name) - 3);
		AddStringToList(&DBList, filename, &sl);
	}
	AddStringToList(&DBList, '\0', &sl);
	return DBList;;
}

char **DBMListTables(char *Database)
{
	static char filename[MAXPATH];
	int dbret;
	DB *dbp;
	int rowcount = 0;
	DBC *dbcp;
	char *table;
	char **Tables = NULL;;
	int tl = 0;

	dlog(DEBUG10, "DBMListTables %s\n", Database);
	ircsprintf (filename, "data/%s.bdb", Database);
	if ((dbret = db_create(&dbp, NULL, 0)) != 0) {
		nlog(LOG_WARNING, "db_create: %s\n", db_strerror(dbret));
		return NULL;
	}
#if (DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR >= 1)
	if ((dbret = dbp->open(dbp, NULL, filename, NULL, DB_UNKNOWN, DB_RDONLY, 0600)) != 0) {
#else
	if ((dbret = dbp->open(dbp, filename, NULL, DB_UNKNOWN, DB_RDONLY, 0600)) != 0) {
#endif
		nlog(LOG_WARNING,"dbp->open: %s\n", db_strerror(dbret));
		return NULL;
	}

	memset(&dbkey, 0, sizeof(dbkey));
	memset(&dbdata, 0, sizeof(dbdata));
	/* initilize the cursors */
	if ((dbret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
		nlog(LOG_WARNING, "DB Cursor failed: %s\n", db_strerror(dbret));
		return NULL;
	}
	
	while ((dbret = dbcp->c_get(dbcp, &dbkey, &dbdata, DB_NEXT)) == 0)
	{
		rowcount++;
		if (dbkey.size > 0) {
			table = ns_malloc(dbkey.size+1);
			strlcpy(table, dbkey.data, dbkey.size+1);
			AddStringToList(&Tables, table, &tl);
		} else {
			nlog(LOG_WARNING, "Hrm, Table name is null?\n");
		}
	} 
	if (dbret != 0 && dbret != DB_NOTFOUND) {
		dlog(DEBUG10, "dbp->c_get failed: %s\n", db_strerror(dbret));
	}
	if ((dbret = dbcp->c_close(dbcp)) != 0) {
		nlog(LOG_WARNING, "dbcpp->close failed: %s\n", db_strerror(dbret));
	}	
	dbp->close(dbp, 0); 
	AddStringToList(&Tables, '\0', &tl);
	return Tables;

}


#endif
