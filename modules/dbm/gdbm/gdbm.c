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
** $Id: gdbm.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "gdbmdefs.h"
#include "gdbmerrno.h"
#include "nsdbm.h"
#include <sys/types.h>
#ifndef WIN32
/* Does not compile under Win32 and not actually used by Neostats so not needed */
#include <dirent.h>
#include <glob.h>
#endif /* !WIN32 */
#include <errno.h>

extern const char *gdbm_strerror __P( ( gdbm_error ) );
char **DBMListDB();

int convertdone = 0;
char *dbnametmp;


void *DBMOpenDB(const char *name) 
{
#ifndef WIN32 /* Workaround compiler error */
	int i = 0;
	glob_t g;
	char search[MAXPATH];
	char *table;
	char newname[MAXPATH];
	
	if (convertdone == 0) {
		ircsnprintf(search, MAXPATH, "data/%s*.gdbm", name);
		if (glob(search, 0, NULL, &g) == 0) {
			for (i = 0; i < g.gl_pathc; i++) {
				/* start of table name is 5 ("data/") + length of Database */
				table = strdup(g.gl_pathv[i]+strlen(name)+5);
				if (table[0] == '-') {
					/* already converted */
					break;
				}
				ircsnprintf(newname, MAXPATH, "data/%s-%s", name, table);
				nlog(LOG_WARNING, "Renaming GDBM Datafiles for %s (%s) from Alpha2 Format", name, table);
				if (rename(g.gl_pathv[i], newname) != 0) {
					nlog(LOG_WARNING, "Rename Failed for %s", g.gl_pathv[i]);
				};
				free(table);
			}
		} else {
			nlog(LOG_WARNING, "Glob Failed: %s", strerror(errno));
		}
	}
#endif /* !WIN32 */
	/* not required for GDBM */
	return strndup(name, strlen(name));;
}

void DBMCloseDB(void *dbhandle)
{	
	/* not required for gdbm */
	ns_free(dbhandle)
	return;
}

/** @brief DBMOpenTable
 *
 *  Open gdbm table
 *
 *  @param table name
 *
 *  @return handle to table or NULL on error
 */

void *DBMOpenTable( void *dbname, const char *name )
{
	static char filename[MAXPATH];
	gdbm_file_info *gdbm_file;
	int cache_size = DEFAULT_CACHESIZE;

	dlog( DEBUG10, "DBMOpenTable %s", name );
	ircsprintf( filename, "data/%s-%s.gdbm", (char *)dbname, name );
	gdbm_file = gdbm_open( filename, 0, GDBM_WRCREAT | GDBM_NOLOCK, 00600, NULL );
	if( gdbm_file == NULL )
	{
		nlog( LOG_ERROR, "gdbm_open fail: %s", gdbm_strerror( gdbm_errno ) );
		return NULL;
	}
	if( gdbm_setopt( gdbm_file, GDBM_CACHESIZE, &cache_size, sizeof( int ) ) == -1 )
	{
		nlog( LOG_ERROR, "gdbm_setopt fail: %s", gdbm_strerror( gdbm_errno ) );
		return NULL;
	}
	return ( void * )gdbm_file;
}

/** @brief DBMCloseTable
 *
 *  Close gdbm table
 *
 *  @param handle of table to close
 *
 *  @return none
 */

void DBMCloseTable( void *unused, void *handle )
{
	if( handle ) 
	{
		gdbm_close( ( gdbm_file_info * )handle ); 
	}
}

/** @brief DBMFetch
 *
 *  Fetch data from table record
 *
 *  @param handle of table
 *  @param record key
 *  @param pointer to data to fetch data into
 *  @param size of record
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBMFetch( void *unused, void *handle, char *key, void *data, int size )
{
	datum dbkey;
	datum dbdata;

	dbkey.dptr = key;
	dbkey.dsize = strlen( key ) + 1;
	dbdata = gdbm_fetch( ( gdbm_file_info * )handle, dbkey );
	if( dbdata.dptr != NULL )
	{
		if( dbdata.dsize != size )
		{
			nlog( LOG_WARNING, "DBMFetch: gdbm_fetch fail: %s data size mismatch", key );
			free( dbdata.dptr );
			return NS_FAILURE;
		}
		os_memcpy( data, dbdata.dptr, size );
		free( dbdata.dptr );
		return NS_SUCCESS;
	}
	dlog( DEBUG10, "DBMFetch: gdbm_fetch fail: %s %s", key, gdbm_strerror( gdbm_errno ) );
	return NS_FAILURE;
}

/** @brief DBMStore
 *
 *  Store data in table record
 *
 *  @param handle of table
 *  @param record key
 *  @param pointer to data to fetch data into
 *  @param size of record
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBMStore( void *unused, void *handle, char *key, void *data, int size )
{
	datum dbkey;
	datum dbdata;

	dbkey.dptr = key;
	dbkey.dsize = strlen( key ) + 1;
	dbdata.dptr = data;
	dbdata.dsize = size;
	if( gdbm_store( ( gdbm_file_info * )handle, dbkey, dbdata, GDBM_REPLACE ) != 0 )
	{
		nlog( LOG_WARNING, "DBMStore: gdbm_store fail: %s %s", key, gdbm_strerror( gdbm_errno ) );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief DBMFetchRows
 *
 *  Walk through table and pass records in turn to handler
 *
 *  @param table handle
 *  @param handler for records
 *
 *  @return number of rows processed by handler
 */

int DBMFetchRows( void *unused, void *handle, DBRowHandler handler )
{
	datum dbkey;
	datum dbdata;
	int rowcount = 0;

	dbkey = gdbm_firstkey( ( gdbm_file_info * )handle );
	while( dbkey.dptr != NULL )
	{
		rowcount++;
		dlog( DEBUG10, "DBMFetchRows: key %s", dbkey.dptr );
		dbdata = gdbm_fetch( ( gdbm_file_info * )handle, dbkey );
		/* Allow handler to exit the fetch loop */
		if( handler( dbdata.dptr, dbdata.dsize ) != 0 )
		{
			free( dbdata.dptr );
			free( dbkey.dptr );
			break;
		}
		free( dbdata.dptr );
		dbdata = gdbm_nextkey( ( gdbm_file_info * )handle, dbkey );
		free( dbkey.dptr );
		dbkey = dbdata;
	}
	return rowcount;
}

int DBMFetchRows2 (void *dbhandle, void *tbhandle, DBRowHandler2 handler)
{
	datum dbkey;
	datum dbdata;
	int rowcount = 0;

	dbkey = gdbm_firstkey( ( gdbm_file_info * )tbhandle );
	while( dbkey.dptr != NULL )
	{
		rowcount++;
		dlog( DEBUG10, "DBMFetchRows2: key %s", dbkey.dptr );
		dbdata = gdbm_fetch( ( gdbm_file_info * )tbhandle, dbkey );
		/* Allow handler to exit the fetch loop */
		if( handler( dbkey.dptr, dbdata.dptr, dbdata.dsize ) != 0 )
		{
			free( dbdata.dptr );
			free( dbkey.dptr );
			break;
		}
		free( dbdata.dptr );
		dbdata = gdbm_nextkey( ( gdbm_file_info * )tbhandle, dbkey );
		free( dbkey.dptr );
		dbkey = dbdata;
	}
	return rowcount;
}


/** @brief DBMDelete
 *
 *  delete table row
 *
 *  @param table handle
 *  @param record key
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBMDelete( void *unused, void *handle, char *key )
{
	datum dbkey;

	dbkey.dptr = key;
	dbkey.dsize = strlen( key ) + 1;
	if( gdbm_delete( ( gdbm_file_info * )handle, dbkey ) != 0 )
	{
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/* Does not compile under Win32 and not actually used by Neostats so not needed */
#ifndef WIN32
int file_select (struct dirent *entry) {
	char *ptr;
	if ((ircstrcasecmp(entry->d_name, ".")==0) || (ircstrcasecmp(entry->d_name, "..")==0)) 
		return 0;
	/* check filename extension */
	ptr = strrchr(entry->d_name, '.');
	if ((ptr) && !(ircstrcasecmp(ptr, ".gdbm"))) {
		return NS_SUCCESS;
	}
	return 0;	
}


char **DBMListDB()
{
	struct dirent **files;
	int count, i, sl = 0;
	int j = 0;
	char *filename;
	char *dbname;
	char **DBList = NULL;
	int gotit;
	
	count = scandir ("data/", &files, file_select, alphasort);
	for (i = 2; i <= count; i++) 
	{
		filename = strdup(files[i-1]->d_name);
		dbname = strchr(filename, '-');
		if (dbname) {
			filename[dbname-filename] = '\0';
			if (sl > 0) {
				gotit = 0;
				for (j = 0; j < sl; j++) {
					if (!(ircstrcasecmp(DBList[j], filename))) {
						gotit = 1;
					} 
				}
				if (gotit != 1) {
					AddStringToList(&DBList, filename, &sl);
				}
			} else {
				AddStringToList(&DBList, filename, &sl);
			}
		}
	}
	AddStringToList(&DBList, '\0', &sl);
	return DBList;
}


char **DBMListTables(char *Database)
{
	char **Tables = NULL;
	int tl = 0;
	int i = 0;
	glob_t g;
	char search[MAXPATH];
	char *table;
	

	dlog(DEBUG10, "DBMListTables %s\n", Database);
	ircsnprintf(search, MAXPATH, "data/%s*.gdbm", Database);
	if (glob(search, 0, NULL, &g) == 0) {
		for (i = 0; i < g.gl_pathc; i++) {
			/* start of table name is 5 ("data/") + length of Database */
			table = strdup(g.gl_pathv[i]+strlen(Database)+6);

			if (table[strlen(table)-5] == '.') {
				table[strlen(table)-5] = '\0';
			}
			AddStringToList(&Tables, table, &tl);
		}
	} else {
		nlog(LOG_WARNING, "Glob Failed: %s", strerror(errno));
	}
	AddStringToList(&Tables, '\0', &tl); 
	return Tables;
}
#endif /* !WIN32 */
