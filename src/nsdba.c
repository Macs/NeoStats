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
** $Id: nsdba.c 3294 2008-02-24 02:45:41Z Fish $
*/

/*	
 *  DBMs issue internal mallocs so these cannot be freed with ns_free
 *  This module handles the copy of data from the db to the variable
 */

#include "neostats.h"
#include "dl.h"

typedef struct dbentry {
	char name[MAX_MOD_NAME];
	hash_t *tablehash;
	void *handle;
} dbentry;

typedef struct tableentry {
	char name[MAXPATH];
	void *handle;
} tableentry;

typedef struct dbm_sym {
	void **ptr;
	char *sym;
} dbm_sym;

static void *( *DBMOpenDB )( const char *name );
static void ( *DBMCloseDB )( void *dbhandle );
static void *( *DBMOpenTable )( void *dbhandle, const char *name );
static void ( *DBMCloseTable )( void *dbhandle, void *tbhandle );
static int ( *DBMFetch )( void *dbhandle, void *tbhandle, const char *key, void *data, int size );
static int ( *DBMStore )( void *dbhandle, void *tbhandle, const char *key, void *data, int size );
static int ( *DBMFetchRows )( void *dbhandle, void *tbhandle, DBRowHandler handler );
static int ( *DBMFetchRows2 )( void *dbhandle, void *tbhandle, DBRowHandler2 handler );
static int ( *DBMDelete )( void *dbhandle, void *tbhandle, const char *key );

static dbm_sym dbm_sym_table[] = 
{
	{ ( void * )&DBMOpenDB,		"DBMOpenDB" },
	{ ( void * )&DBMCloseDB,	"DBMCloseDB" },
	{ ( void * )&DBMOpenTable,	"DBMOpenTable" },
	{ ( void * )&DBMCloseTable,	"DBMCloseTable" },
	{ ( void * )&DBMFetch,		"DBMFetch" },
	{ ( void * )&DBMStore,		"DBMStore" },
	{ ( void * )&DBMFetchRows,	"DBMFetchRows" },
	{ ( void * )&DBMFetchRows2,	"DBMFetchRows2" },
	{ ( void * )&DBMDelete,		"DBMDelete" },
	{NULL, NULL},
};

static hash_t *dbhash;
static void *dbm_module_handle;

/** @brief InitDBAMSymbols
 *
 *  Lookup DBM symbols for DBA layer
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int InitDBAMSymbols( void )
{
	static char dbm_path[MAXPATH];
	dbm_sym *pdbm_sym;

	ircsnprintf( dbm_path, 255, "%s/%s%s", MOD_PATH, me.dbm, MOD_STDEXT );
	nlog( LOG_NORMAL, "Using dbm module %s", dbm_path );
	dbm_module_handle = ns_dlopen( dbm_path, RTLD_NOW | RTLD_GLOBAL );
	if( !dbm_module_handle )
	{
		nlog( LOG_CRITICAL, "Unable to load dbm module %s: file not found", dbm_path );
		return NS_FAILURE;	
	}
	pdbm_sym = dbm_sym_table;
	while( pdbm_sym->ptr != NULL )
	{
		*pdbm_sym->ptr = ns_dlsym( dbm_module_handle, pdbm_sym->sym );
		if( *pdbm_sym->ptr == NULL)
		{
			nlog( LOG_CRITICAL, "Unable to load dbm module %s: missing handler for %s", dbm_path, pdbm_sym->sym );
			return NS_FAILURE;	
		}
		pdbm_sym ++;
	}
	return NS_SUCCESS;
}

/** @brief InitDBA
 *
 *  Init DBA layer
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitDBA( void )
{
	if( InitDBAMSymbols() != NS_SUCCESS )
		return NS_FAILURE;
	dbhash = hash_create( HASHCOUNT_T_MAX, 0, 0 );
	if( !dbhash )
	{
		nlog( LOG_CRITICAL, "Unable to create db hash" );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief FiniDBA
 *
 *  Finish DBA layer
 *
 *  @param none
 *
 *  @return none
 */

void FiniDBA( void )
{
	tableentry *tbe;
	dbentry *dbe;
	hnode_t *node;
	hnode_t *tnode;
	hscan_t ds;
	hscan_t ts;

	hash_scan_begin( &ds, dbhash );
	while( ( node = hash_scan_next( &ds ) ) != NULL )
	{
		dbe = ( dbentry * ) hnode_get( node );
		dlog( DEBUG10, "Closing Database %s", dbe->name );
		hash_scan_begin( &ts, dbe->tablehash );
		while( ( tnode = hash_scan_next( &ts ) ) != NULL )
		{
			tbe = (tableentry *) hnode_get( tnode );
			dlog( DEBUG10, "Closing Table %s", tbe->name );
			DBMCloseTable( dbe->handle, tbe->handle );
			hash_scan_delete_destroy_node( dbe->tablehash, tnode );
			ns_free( tbe );
		}
		DBMCloseDB(dbe->handle);
		hash_destroy( dbe->tablehash );
		hash_scan_delete_destroy_node( dbhash, node );
		ns_free( dbe );
	}
	hash_destroy( dbhash );
	ns_dlclose( dbm_module_handle );
}

/** @brief DBAOpenDatabase
 *
 *  Open NeoStats database
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBAOpenDatabase( void )
{
	dbentry *dbe;

	dlog( DEBUG10, "DBAOpenDatabase %s", GET_CUR_MODNAME() );
	if( hash_isfull( dbhash ) )
	{
		nlog (LOG_CRITICAL, "DBAOpenDatabase: db hash is full");
		return NS_FAILURE;
	}
	dbe = ns_calloc( sizeof( dbentry ) );
	strlcpy( dbe->name, GET_CUR_MODNAME(), MAX_MOD_NAME );

	dbe->handle = DBMOpenDB( dbe->name );
	if (!dbe->handle) {
		nlog (LOG_CRITICAL, "DBAOpenDatabase: Unable to open Database");
		ns_free(dbe);
		return NS_FAILURE;
	}
	dbe->tablehash = hash_create( HASHCOUNT_T_MAX, 0, 0 );
	if( !dbe->tablehash )
	{
		nlog( LOG_CRITICAL, "DBAOpenDatabase: Unable to create table hash" );
		ns_free(dbe);
		return NS_FAILURE;
	}
	hnode_create_insert( dbhash, dbe, dbe->name );
	return NS_SUCCESS;
}

/** @brief DBACloseDatabase
 *
 *  Close NeoStats database
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBACloseDatabase( void )
{
	tableentry *tbe;
	dbentry *dbe;
	hnode_t *node;
	hnode_t *tnode;
	hscan_t ts;

	dlog( DEBUG10, "DBACloseDatabase %s", GET_CUR_MODNAME() );
	node = hash_lookup( dbhash, GET_CUR_MODNAME() );
	if (node)
	{
		dbe = ( dbentry * ) hnode_get( node );
		dlog( DEBUG10, "Closing Database %s", dbe->name );
		hash_scan_begin( &ts, dbe->tablehash );
		while( ( tnode = hash_scan_next( &ts ) ) != NULL )
		{
			tbe = (tableentry *) hnode_get( tnode );
			dlog( DEBUG10, "Closing Table %s", tbe->name );
			DBMCloseTable( dbe->handle, tbe->handle );
			hash_scan_delete_destroy_node( dbe->tablehash, tnode );
			ns_free( tbe );
		}
		DBMCloseDB(dbe->handle);
		hash_destroy( dbe->tablehash );
		hash_scan_delete_destroy_node( dbhash, node );
		ns_free( dbe );
	}
	return NS_SUCCESS;
}

/** @brief DBAOpenTable
 *
 *  Open table in NeoStats database
 *
 *  @param table name
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBAOpenTable( const char *table )
{
	dbentry *dbe;
	tableentry *tbe;

	dlog( DEBUG10, "DBAOpenTable %s", table );
	dbe = (dbentry *)hnode_find( dbhash, GET_CUR_MODNAME() );
	if( !dbe )
	{
		nlog( LOG_WARNING, "Database %s for table %s not open", GET_CUR_MODNAME(), table );
		return NS_FAILURE;
	}
	tbe = ns_calloc( sizeof( tableentry ) );
	ircsnprintf( tbe->name, MAXPATH, "%s", table);
	if( hnode_find( dbe->tablehash, tbe->name ) )
	{
		dlog( DEBUG10, "DBAOpenTable %s already open", table );
		ns_free (tbe);
		return NS_SUCCESS;
	}
	tbe->handle = DBMOpenTable( dbe->handle, tbe->name );
	if( !tbe->handle )
	{
		ns_free( tbe );
		FATAL_ERROR( "DBAOpenTable failed. Check log file for details" );
		return NS_FAILURE;
	}
	hnode_create_insert( dbe->tablehash, tbe, tbe->name );
	return NS_SUCCESS;
}

/** @brief DBAFetchDBEntry 
 *
 *  Get the Database entry info
 *  DBA subsystem use only
 */
static dbentry *DBAFetchDBEntry()
{
	dbentry *dbe;

	dlog( DEBUG10, "DBAFetchDBEntry %s", GET_CUR_MODNAME() );
	dbe = (dbentry *)hnode_find( dbhash, GET_CUR_MODNAME() );
	if( !dbe )
	{
		nlog( LOG_WARNING, "Database %s not open", GET_CUR_MODNAME());
		return NULL;
	}
	return dbe;
} 
/** @brief DBAFetchTableEntry
 *
 *  Get table entry info
 *  DBA subsystem use only
 *
 *  @param table name
 *
 *  @return table entry or NULL for none
 */

static tableentry *DBAFetchTableEntry( const char *table, int *islocalopen )
{
	dbentry *dbe;
	tableentry *tbe;

	dlog( DEBUG10, "DBAFetchTableEntry %s", table );
	dbe = DBAFetchDBEntry();
	if( !dbe )
	{
		nlog( LOG_WARNING, "Database %s for table %s not open", GET_CUR_MODNAME(), table );
		return NULL;
	}
	tbe = (tableentry *)hnode_find( dbe->tablehash, table );
	if( !tbe )
	{
		DBAOpenTable( table );
		tbe = (tableentry *)hnode_find( dbe->tablehash, table );
		*islocalopen = 1;
	}
	return tbe;
}

/** @brief DBACloseTable
 *
 *  Close table in NeoStats database
 *
 *  @param table name
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBACloseTable( const char *table )
{
	dbentry *dbe;
	tableentry *tbe;
	hnode_t *node;

	dlog( DEBUG10, "DBACloseTable %s", table );
	dbe = (dbentry *)hnode_find( dbhash, GET_CUR_MODNAME() );
	if( !dbe )
	{
		nlog( LOG_WARNING, "Database %s for table %s not open", GET_CUR_MODNAME(), table );
		return NS_FAILURE;
	}
	node = hash_lookup( dbe->tablehash, table);
	if( node )
	{
		tbe = (tableentry *)hnode_get( node );
		DBMCloseTable( dbe->handle, tbe->handle );
		hash_delete_destroy_node( dbe->tablehash, node );
		ns_free( tbe );
	}
	return NS_SUCCESS;
}

/** @brief DBAFetch
 *
 *  Fetch data from table record
 *
 *  @param table name
 *  @param record key
 *  @param pointer to data to fetch data into
 *  @param size of record
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBAFetch( const char *table, const char *key, void *data, int size )
{
	int islocalopen = 0;
	int ret = 0;
	tableentry *tbe;
	dbentry *dbe = DBAFetchDBEntry();

	dlog( DEBUG10, "DBAFetch %s %s", table, key );
	tbe = DBAFetchTableEntry( table, &islocalopen );
	if (!dbe ) {
		nlog(LOG_WARNING, "No Such Database %s", dbe->name);
		return NS_FAILURE;
	}
	if( !tbe ) {
		nlog(LOG_WARNING, "No Such Table %s in database %s", tbe->name, dbe->name);
		return NS_FAILURE;
	}
	ret = DBMFetch( dbe->handle, tbe->handle, key, data, size );
	if( islocalopen )
		DBACloseTable( table );
	return ret;
}

/** @brief DBAStore
 *
 *  Store data in table record
 *
 *  @param table name
 *  @param record key
 *  @param pointer to data to store
 *  @param size of record
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBAStore( const char *table, const char *key, void *data, int size )
{
	int islocalopen = 0;
	int ret = 0;
	tableentry *tbe;
	dbentry *dbe = DBAFetchDBEntry();
	
	if (data == NULL) {
		nlog(LOG_WARNING, "Error Trying to save NULL data in table %s for key %s", table, key);
		return NS_FAILURE;
	}
	
	dlog( DEBUG10, "DBAStore %s %s", table, key );
	tbe = DBAFetchTableEntry( table, &islocalopen );
	if (!dbe ) {
		nlog(LOG_WARNING, "No Such Database %s", dbe->name);
		return NS_FAILURE;
	}
	if( !tbe ) {
		nlog(LOG_WARNING, "No Such Table %s in database %s", tbe->name, dbe->name);
		return NS_FAILURE;
	}
	ret = DBMStore( dbe->handle, tbe->handle, key, data, size );
	if( islocalopen )
		DBACloseTable( table );
	return ret;
}

/** @brief DBAFetchRows
 *
 *  Walk through table and pass records in turn to handler
 *
 *  @param table name
 *  @param handler for records
 *
 *  @return number of rows processed by handler
 */

int DBAFetchRows( const char *table, DBRowHandler handler )
{
	int islocalopen = 0;
	int ret = 0;
	tableentry *tbe;
	dbentry *dbe = DBAFetchDBEntry();

	dlog( DEBUG10, "DBAFetchRows %s", table );
	if (!dbe) {
		nlog(LOG_WARNING, "No Such Database %s", dbe->name);
		return ret;
	}
	tbe = DBAFetchTableEntry( table, &islocalopen );
	if( !tbe ) {
		nlog(LOG_WARNING, "FetchRow: No Such Table %s in DB %s", table, dbe->name);
		return ret;
	}
	ret = DBMFetchRows( dbe->handle, tbe->handle, handler );	
	if( islocalopen )
		DBACloseTable( table );
	return ret;
}

/** @brief DBAFetchRows2
 *
 *  Walk through table and pass records in turn to handler
 *  This version also passes the Key used to store the record back to the handler
 *
 *  @param table name
 *  @param handler for records
 *
 *  @return number of rows processed by handler
 */

int DBAFetchRows2( const char *table, DBRowHandler2 handler )
{
	int islocalopen = 0;
	int ret = 0;
	tableentry *tbe;
	dbentry *dbe = DBAFetchDBEntry();

	dlog( DEBUG10, "DBAFetchRows %s", table );
	if (!dbe) {
		nlog(LOG_WARNING, "No Such Database %s", dbe->name);
		return ret;
	}
	tbe = DBAFetchTableEntry( table, &islocalopen );
	if( !tbe ) {
		nlog(LOG_WARNING, "FetchRow: No Such Table %s in DB %s", table, dbe->name);
		return ret;
	}
	ret = DBMFetchRows2( dbe->handle, tbe->handle, handler );	
	if( islocalopen )
		DBACloseTable( table );
	return ret;
}

/** @brief DBADelete
 *
 *  delete table row
 *
 *  @param table name
 *  @param record key
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int DBADelete( const char *table, const char *key )
{
	int islocalopen = 0;
	int ret = 0;
	tableentry *tbe;
	dbentry *dbe = DBAFetchDBEntry();
	

	dlog( DEBUG10, "DBADelete %s %s", table, key );
	tbe = DBAFetchTableEntry( table, &islocalopen );
	if( !tbe )
		return NS_FAILURE;
	if (!dbe )
		return NS_FAILURE;
	ret = DBMDelete( dbe->handle, tbe->handle, key );
	if( islocalopen )
		DBACloseTable( table );
	return ret;
}
