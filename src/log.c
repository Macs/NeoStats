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
** NeoStats CVS Identification
** $Id: log.c 3320 2008-03-05 09:22:44Z Fish $
*/

#include "neostats.h"
#include "main.h"
#include "log.h"
#include "services.h"
#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

/* LogEntry structure */
typedef struct LogEntry {
	FILE *logfile;
	char name[MAX_MOD_NAME];
	char logname[MAXPATH];
	unsigned int flush;
} LogEntry;

/* Format string for log file names */
char LogFileNameFormat[MAX_LOGFILENAME] = "-%m-%d";
/* Log level strings */
static const char *loglevels[LOG_LEVELMAX] = {
	"CRITICAL",
	"ERROR",
	"WARNING",
	"NOTICE",
	"NORMAL",
	"INFO",
};
/* Debug log level strings */
static const char *dloglevels[DEBUGMAX] = {
	"DEBUGRX",
	"DEBUGTX",
	"DEBUG1",
	"DEBUG2",
	"DEBUG3",
	"DEBUG4",
	"DEBUG5",
	"DEBUG6",
	"DEBUG7",
	"DEBUG8",
	"DEBUG9",
	"DEBUG10",
};
/* log scratchpad buffer */
static char log_buf[BUFSIZE];
/* log time string scratchpad buffer */
static char log_fmttime[TIMEBUFSIZE];
/* log hash pointer */
static hash_t *logs;

/*  @brief InitLogs
 * 
 *  initialize the logging functions 
 *  NeoStats core use only.
 * 
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */
int InitLogs( void )
{
	SET_SEGV_LOCATION();
	logs = hash_create( HASHCOUNT_T_MAX, 0, 0 );
	if( logs == NULL )
	{
#ifndef WIN32
		printf( "ERROR: Can't initialize log subsystem." );
#endif /* WIN32 */
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/*  @brief make_log_filename
 * 
 *  Generate log filename
 *  NeoStats core use only.
 * 
 *  @param modname module name
 *  @param logname pointer to buffer in which to make logname
 *
 *  @return none
 */

static void make_log_filename( char *modname, char *logname )
{
	static char log_file_fmttime[TIMEBUFSIZE];
	
	time_t t = time( NULL );
	strftime( log_file_fmttime, TIMEBUFSIZE, LogFileNameFormat, localtime( &t ) );
	ns_strlwr( modname );
	ircsnprintf( logname, MAXPATH, "logs/%s%s.log", modname, log_file_fmttime );
}

/*  @brief make_log_timestring
 * 
 *  Generate log time string
 *  NeoStats core use only.
 * 
 *  @param modname module name
 *  @param logname pointer to buffer in which to make logname
 *
 *  @return none
 */

static void make_log_timestring( void )
{
	/* we update me.now here, because some functions might be busy and not call the loop a lot */
	strftime( log_fmttime, TIMEBUFSIZE, "%d/%m/%Y[%H:%M:%S]", localtime( &me.now ) );
}

/** @brief FiniLogs
 *
 *  cleanup log subsystem
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */

void FiniLogs( void ) 
{
	CloseLogs();
	hash_destroy( logs );
}

/*  @brief CloseLogs
 * 
 *  Flush log files
 *  NeoStats core use only.
 * 
 *  @param none
 *
 *  @return none
 */

void CloseLogs( void )
{
	hscan_t hs;
	hnode_t *hn;
	LogEntry *logentry;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, logs );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		logentry = hnode_get( hn );
		logentry->flush = 0;
#ifdef DEBUG
		printf( "Closing Logfile %s (%s)\n", logentry->name, ( char * ) hnode_getkey( hn ) );
#endif
		if( logentry->logfile )
		{
			fflush( logentry->logfile );
			fclose( logentry->logfile );
			logentry->logfile = NULL;
		}
		hash_scan_delete_destroy_node( logs, hn );
		ns_free( logentry );
	}
}
/* @breif FlushLogs
 * 
 *  Flush Log files out. Called from timers 
 *  Neostats Core use only.
 * 
 *  @param arg - user supplied pointer. Not used
 *
 *  @return SUCCESS - Keep Timer around. FAILURE - Delete Timer
 */
int FlushLogs(void *arg) {
	fflush(NULL);
	return NS_SUCCESS;
}	

/*  @brief ResetLogs
 * 
 *  rotate logs, called at midnight
 *  NeoStats core use only.
 * 
 *  @param none
 *
 *  @return none
 */
int ResetLogs( void *arg )
{
	hscan_t hs;
	hnode_t *hn;
	LogEntry *logentry;

	SET_SEGV_LOCATION();
	hash_scan_begin( &hs, logs );
	while( ( hn = hash_scan_next( &hs ) ) != NULL ) {
		logentry = hnode_get( hn );
		/* If file handle is vald we must have used the log */
		if( logentry->logfile ) {
			if( logentry->flush > 0 ) {
				fflush( logentry->logfile );
			}
			fclose( logentry->logfile );		
			logentry->logfile = NULL;
		}
		logentry->flush = 0;
#ifdef DEBUG
		printf( "Closing Logfile %s (%s)\n", logentry->name, ( char * ) hnode_getkey( hn ) );
#endif
		/* make new file name but do not open until needed to avoid 0 length files*/
		make_log_filename( logentry->name, logentry->logname );
	}
	return NS_SUCCESS;
}

/*  @brief new_logentry
 * 
 *  Create log entry
 *  NeoStats core use only.
 * 
 *  @param none
 *
 *  @return pointer to newly created log entry
 */

static LogEntry *new_logentry( void )
{
	LogEntry *logentry;

	logentry = ns_calloc( sizeof( LogEntry ) );
	strlcpy( logentry->name, GET_CUR_MODNAME() , MAX_MOD_NAME );
	make_log_filename( logentry->name, logentry->logname );
	hnode_create_insert( logs, logentry, logentry->name );
	return logentry;
}

/*  @brief dlog_write
 * 
 *  write debug messages
 *  NeoStats core use only.
 * 
 *  @param time string 
 *  @param level string
 *  @param line to log
 *
 *  @return none
 */

static void dlog_write( const char *time, const char *level, const char *line )
{
	static int chanflag = 0;
	FILE *logfile;

	logfile = os_fopen( "logs/debug.log", "at" );
	if( logfile ) {
		os_fprintf( logfile, "%s %s %s - %s\n", time, level, GET_CUR_MODNAME(), line );
		fclose( logfile );
	}
	/* chanflag is used to avoid endless loop when sending debug messages to channel */
	if( nsconfig.debugtochan && !chanflag ) {
		chanflag = 1;
		irc_chanprivmsg( ns_botptr, nsconfig.debugchan, "%s %s %s - %s\n", time, level, GET_CUR_MODNAME(), line );
		chanflag = 0;
	}
}

/*  @brief dlog
 * 
 *  debug message handler
 *  NeoStats core use only.
 * 
 *  @param level of debug message
 *  @param fmt string
 *  @param ... parameters to format string
 *
 *  @return none
 */

void dlog( NS_DEBUG_LEVEL level, const char *fmt, ... )
{
	va_list ap;
	
	if( (nsconfig.debug == 1) && (level - 2) <= nsconfig.debuglevel ) {
		/* Support for module specific only debug info */
		if( ircstrcasecmp( nsconfig.debugmodule, "all" ) == 0 || ircstrcasecmp( nsconfig.debugmodule, GET_CUR_MODNAME() ) == 0 )
		{
			make_log_timestring();
			va_start( ap, fmt );
			ircvsnprintf( log_buf, BUFSIZE, fmt, ap );
			va_end( ap );
			if( nsconfig.foreground )
				printf( "%s %s - %s\n", dloglevels[level - 1], GET_CUR_MODNAME(), log_buf );
			dlog_write( log_fmttime, dloglevels[level - 1], log_buf );
		}
	}
}

/*  @brief nlog_write
 * 
 *  write log messages
 *  NeoStats core use only.
 * 
 *  @param time string 
 *  @param level string
 *  @param line to log
 *
 *  @return none
 */

static void nlog_write( const char *time, const char *level, const char *line )
{
	LogEntry *logentry;

	logentry = ( LogEntry * ) hnode_find( logs, GET_CUR_MODNAME() );
	if( !logentry )
		logentry = new_logentry();
	if( !logentry->logfile )
		logentry->logfile = os_fopen( logentry->logname, "at" );
	if( !logentry->logfile ) {
#ifdef DEBUG
		printf( "LogFile Error: %s: %s\n", logentry->logname, strerror( errno ) );
#endif
		/* if the log file can't be opened now, there is something seriously wrong. straight Exit required, otherwise we might end up in recursive hell:
		** http://www.neostats.net/boards/viewtopic.php?t=2352&start=15 */
		exit(-1);
	}
	os_fprintf( logentry->logfile, "(%s) %s %s - %s\n", time, level, GET_CUR_MODNAME(), line );
	logentry->flush = 1;
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
	
	if( nsconfig.debuglevel || level <= nsconfig.loglevel ) {
		make_log_timestring();
		va_start( ap, fmt );
		ircvsnprintf( log_buf, BUFSIZE, fmt, ap );
		va_end( ap );
		if( level <= nsconfig.loglevel ) 
			nlog_write( log_fmttime, loglevels[level - 1], log_buf );
		if( nsconfig.foreground )
			printf( "%s %s - %s\n", loglevels[level - 1], GET_CUR_MODNAME(), log_buf );
		if( nsconfig.debuglevel )
			dlog_write( log_fmttime, loglevels[level - 1], log_buf );
	}
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
		nlog( LOG_CRITICAL, "BackTrace (%d) : %s", (int) i - 1, strings[i] );
	}
#endif /* HAVE_BACKTRACE */
	nlog( LOG_CRITICAL, "Shutting Down!" );
	exit( EXIT_FAILURE );
}

void CaptureBackTrace (const char *file, const int line, const char *func) {
#ifdef HAVE_BACKTRACE
	void *array[MAXBACKTRACESIZE];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace( array, MAXBACKTRACESIZE );
	strings = backtrace_symbols( array, size );
#endif /* HAVE_BACKTRACE */
	nlog( LOG_CRITICAL, "Abnormal Alert:!" );
	nlog( LOG_CRITICAL, "Function: %s %s %d", func, file, line );
#ifdef HAVE_BACKTRACE
	for( i = 1; i < size; i++ ) {
		nlog( LOG_CRITICAL, "BackTrace (%d) : %s", (int) i - 1, strings[i] );
	}
#endif /* HAVE_BACKTRACE */
}
