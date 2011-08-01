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
** $Id: main.c 2974 2005-12-06 21:39:11Z Mark $
*/

#include "neostats.h"
#include <libgen.h>
#include "services.h"
#include "main.h"
#include "dl.h"
#include "execinfo.h"

char *from;
char *to;

config nsconfig;
Module *RunModule[10];
int RunLevel = 0;
char segv_location[SEGV_LOCATION_BUFSIZE];
tme me;
Bot *ns_botptr = NULL;


static void *indbm_module_handle;
static void *outdbm_module_handle;

void *outdbhandle;
void *outtbhandle;


typedef struct dbm_sym {
	void **ptr;
	char *sym;
} dbm_sym;

static void *( *INDBMOpenDB )( const char *name );
static void ( *INDBMCloseDB )( void *dbhandle );
static void *( *INDBMOpenTable )( void *dbhandle, const char *name );
static void ( *INDBMCloseTable )( void *dbhandle, void *tbhandle );
static void *( *OUTDBMOpenDB )( const char *name );
static void ( *OUTDBMCloseDB )( void *dbhandle );
static void *( *OUTDBMOpenTable )( void *dbhandle, const char *name );
static void ( *OUTDBMCloseTable )( void *dbhandle, void *tbhandle );
static int ( *OUTDBMStore )( void *dbhandle, void *tbhandle, const char *key, void *data, int size );
static int ( *INDBMFetchRows2 )( void *dbhandle, void *tbhandle, DBRowHandler2 handler );
static char **( *INDBMListDB ) ();
static char **( *INDBMListTables ) (const char *database);

static dbm_sym indbm_sym_table[] = 
{
	{ ( void * )&INDBMOpenDB,		"DBMOpenDB" },
	{ ( void * )&INDBMCloseDB,	"DBMCloseDB" },
	{ ( void * )&INDBMOpenTable,	"DBMOpenTable" },
	{ ( void * )&INDBMCloseTable,	"DBMCloseTable" },
	{ ( void * )&INDBMFetchRows2,	"DBMFetchRows2" },
	{ ( void * )&INDBMListDB, 	"DBMListDB" },
	{ ( void * )&INDBMListTables,	"DBMListTables" },
	{NULL, NULL},
};

static dbm_sym outdbm_sym_table[] = 
{
	{ ( void * )&OUTDBMOpenDB,		"DBMOpenDB" },
	{ ( void * )&OUTDBMCloseDB,	"DBMCloseDB" },
	{ ( void * )&OUTDBMOpenTable,	"DBMOpenTable" },
	{ ( void * )&OUTDBMCloseTable,	"DBMCloseTable" },
	{ ( void * )&OUTDBMStore,		"DBMStore" },
	{NULL, NULL},
};


static int InitDBAMSymbols( void );
void FiniDBA( void );

/** @brief print_copyright
 *
 *  print copyright notice
 *  NeoStats core use only.
 *  Not used on Win32
 *
 *  @param none
 *
 *  @return none
 */

static void print_copyright( void )
{
	printf( "-----------------------------------------------\n" );
	printf( "Copyright: NeoStats Group. 2000-2006\n" );
	printf( "Justin Hammond (fish@neostats.net)\n" );
	printf( "-----------------------------------------------\n\n" );
}

static void print_help (char *name) {
	printf( "%s: Usage: %s [options] -f <DB Module> -t <DB Module>\n", basename(name), basename(name));
	printf( "     -h (Show this screen)\n" );
	printf( "     -v (Show version number)\n" );
	printf( "     -d (enable debug mode\n" );
	return;
}

/** @brief get_options
 *
 *  Processes command line options
 *  NeoStats core use only.
 *  Not used in Win32.
 *
 *  @param argc count of command line parameters
 *  @param argv array of command line parameters
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int get_options( int argc, char **argv )
{
	int c;

	while( ( c = getopt( argc, argv, "hvdf:t:" ) ) != -1 ) {
		switch( c ) {
		case 'h':
			print_help(argv[0]);
			return NS_FAILURE;
			break;
		case 'v':
			printf( "NeoStats: http://www.neostats.net\n" );
			printf( "Version:  1\n");
			print_copyright();
			return NS_FAILURE;
			break;
		case 'd':
			nsconfig.debuglevel = 10;
			break;
		case 'f':
			from = strdup(optarg);
			break;
		case 't':
			to = strdup(optarg);		
			break;
		default:
			printf( "Unknown command line switch %c\n", optopt );
		}
	}
	return NS_SUCCESS;
}


int GotRows (char *key, void *data, int size) {
	if (nsconfig.debuglevel == 0) {
		printf(".");
	} else {
		printf("Processing Key %s\n", key);
	}
	OUTDBMStore(outdbhandle, outtbhandle, key, data, size);
	return 0;
}


/** @brief main
 *
 *  Program entry point
 *  NeoStats core use only.
 *  Under Win32 this is used purely as a startup function and not 
 *  the main entry point
 *
 *  @param argc count of command line parameters
 *  @param argv array of command line parameters
 *
 *  @return EXIT_SUCCESS if succeeds, EXIT_FAILURE if not 
 */

int main( int argc, char *argv[] )
{
	char **DBList;
	char **TBList;
	int i = 0;
	int j = 0;
	void *indbhandle;
	void *intbhandle;
	/* get our commandline options */
	printf("%s starting...\n", basename(argv[0]));
	if( get_options( argc, argv ) != NS_SUCCESS )
		return EXIT_FAILURE;
	nsconfig.foreground = 1;
	if (from != NULL && to != NULL) {
		printf("Preparing to Convert Database from %s to %s\n", from, to);
	} else {
		print_help(argv[0]);
	}	
	InitDBAMSymbols();	
	DBList = INDBMListDB();
	/* this is the list of Databases */
	if (DBList == NULL) {
		printf("No %s Databases to process\n", from);
		exit(-1);
	}
	while (DBList[i] != NULL) {
		printf("Opening Database: %s\n", DBList[i]);
		/* now list tables */
		TBList = INDBMListTables(DBList[i]);
		j = 0;
		indbhandle = INDBMOpenDB(DBList[i]);
		outdbhandle = OUTDBMOpenDB(DBList[i]);
		if (indbhandle != NULL) {
			while (TBList[j] != NULL) {
				printf("\tProcessing Table: %s\n", TBList[j]);
				if (nsconfig.debuglevel == 0) printf("\t\t");
				intbhandle = INDBMOpenTable(indbhandle, TBList[j]);
				outtbhandle = OUTDBMOpenTable(outdbhandle, TBList[j]);
				INDBMFetchRows2(indbhandle, intbhandle, GotRows);
				INDBMCloseTable(indbhandle, intbhandle);
				OUTDBMCloseTable(outdbhandle, outtbhandle);
				outtbhandle = NULL;
				intbhandle = NULL;
				j++;
				printf("\n");
			}
		} else {
			printf("Couldn't Open Database\n");
		}
		INDBMCloseDB(indbhandle);
		OUTDBMCloseDB(outdbhandle);
		indbhandle = NULL;
		i++;
	}

	FiniDBA();
	return 1;
}


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

	ircsnprintf( dbm_path, 255, "modules/%s%s", from, MOD_STDEXT );
	indbm_module_handle = ns_dlopen( dbm_path, RTLD_NOW | RTLD_GLOBAL );
	if( !indbm_module_handle )
	{
		printf("Unable to load input dbm module %s: %s\n", dbm_path, ns_dlerror());
		exit(-1);
	}
	pdbm_sym = indbm_sym_table;
	while( pdbm_sym->ptr != NULL )
	{
		*pdbm_sym->ptr = ns_dlsym( indbm_module_handle, pdbm_sym->sym );
		if( *pdbm_sym->ptr == NULL)
		{
			printf("Unable to load input dbm module %s: missing handler for %s\n", dbm_path, pdbm_sym->sym );
			exit(-1);
		}
		pdbm_sym ++;
	}
	ircsnprintf( dbm_path, 255, "modules/%s%s", to, MOD_STDEXT );
	outdbm_module_handle = ns_dlopen( dbm_path, RTLD_NOW | RTLD_GLOBAL );
	if( !outdbm_module_handle )
	{
		printf("Unable to load output dbm module %s: %s\n", dbm_path, ns_dlerror());
		exit(-1);
	}
	pdbm_sym = outdbm_sym_table;
	while( pdbm_sym->ptr != NULL )
	{
		*pdbm_sym->ptr = ns_dlsym( outdbm_module_handle, pdbm_sym->sym );
		if( *pdbm_sym->ptr == NULL)
		{
			printf("Unable to load output dbm module %s: missing handler for %s\n", dbm_path, pdbm_sym->sym );
			exit(-1);
		}
		pdbm_sym ++;
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
	ns_dlclose( indbm_module_handle );
	ns_dlclose( outdbm_module_handle );
}



/* all these functions are just stubs so we can compile without ifdef flags in the main code */

void do_exit( NS_EXIT_TYPE exitcode, const char *quitmsg )
{
	printf("%s\n", quitmsg);
	exit(-1);
}

void fatal_error( char *file, int line, const char *func, char *error_text )
{
	nlog( LOG_CRITICAL, "Fatal Error: %s %d %s %s", file, line, func, error_text );
	do_exit( NS_EXIT_ERROR, "Fatal Error - check log file" );
}
void update_time_now( void )
{
	me.now = time(NULL);
	ircsnprintf (me.strnow, STR_TIME_T_SIZE, "%lu", (long)me.now);
}

int irc_chanprivmsg( const Bot *botptr, const char *chan, const char *fmt, ... ) {
	return NS_SUCCESS;
}
/** @brief ns_strlwr
 * 
 *  make string string lowercase
 * 
 *  @param string to convert to lowercase; string is modified
 *
 *  @returns pointer to the new string
 */

char *ns_strlwr( char *s )
{
	char *t;
	
	t = s;
	while( *t != '\0' )
	{
		*t = tolower( *t );
		t++;
	}
	return s;
}


void *os_memcpy( void *dest, const void *src, size_t count )
{
	return memcpy( dest, src, count );
}

/** @brief AddStringToList
 * 
 *  Adds a string to an array of strings
 * 
 *  @param List the array you wish to append S to 
 *  @param S the string you wish to append
 *  @param C current size of the array
 *
 *  @returns none
 */

void AddStringToList( char ***List, char S[], int *C )
{
	static unsigned int numargs = 8;

	if( *C == 0 ) {
		numargs = 8;
		*List = ns_calloc( sizeof( char * ) * numargs );
	} else if( *C  == numargs ) {
		numargs += 8;
		*List = ns_realloc( *List, sizeof( char * ) * numargs );
	}
	( *List )[*C] = S;
	++*C;
}

void dlog( NS_DEBUG_LEVEL level, const char *fmt, ... )
{
	va_list ap;
	char log_buf[BUFSIZE];
	if (nsconfig.debuglevel > 0) {	
		va_start( ap, fmt );
		ircvsnprintf( log_buf, BUFSIZE, fmt, ap );
		va_end( ap );
		printf( "Debug: %s\n", log_buf );
	}
}

/*  @brief nlog
 * 
 *  log messages
 *  NeoStats core use only.
 * 
 *  @param level of message
 *  @param fmt string
 *  @param ... parameters to format string
 *
 *  @return none
 */

void nlog( NS_LOG_LEVEL level, const char *fmt, ... )
{
	va_list ap;
	char log_buf[BUFSIZE];
		va_start( ap, fmt );
		ircvsnprintf( log_buf, BUFSIZE, fmt, ap );
		va_end( ap );
		printf( "%s\n", log_buf );
}

/*  @brief nassert_fail
 * 
 *  neostats assertion handler
 *  NeoStats core use only.
 * 
 *  @param expression tested
 *  @param file name
 *  @param line number
 *  @param function name
 *
 *  @return none
 */

void nassert_fail( const char *expr, const char *file, const int line, const char *func )
{
#ifdef HAVE_BACKTRACE
	void *array[MAXBACKTRACESIZE];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace( array, MAXBACKTRACESIZE );
	strings = backtrace_symbols( array, size );
#endif /* HAVE_BACKTRACE */
	nlog( LOG_CRITICAL, "Assertion Failure!" );
	nlog( LOG_CRITICAL, "Function: %s %s %d", func, file, line );
	nlog( LOG_CRITICAL, "Expression: %s", expr );
#ifdef HAVE_BACKTRACE
	for( i = 1; i < size; i++ ) {
		nlog( LOG_CRITICAL, "BackTrace (%d) : %s", (int)i - 1, strings[i] );
	}
#endif /* HAVE_BACKTRACE */
	nlog( LOG_CRITICAL, "Shutting Down!" );
	exit( EXIT_FAILURE );
}
