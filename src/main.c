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
** $Id: main.c 3295 2008-02-24 02:51:03Z Fish $
*/

/*  TODO:
 *  - Improve exit code
 *  - Improve reconnect system 
 */

#include "neostats.h"
#include "main.h"
#include "ircprotocol.h"
#include "conf.h"
#include "log.h"
#include "sock.h"
#include "users.h"
#include "servers.h"
#include "channels.h"
#include "dns.h"
#include "transfer.h"
#include "exclude.h"
#include "bans.h"
#include "services.h"
#include "modules.h"
#include "bots.h"
#include "timer.h"
#include "signals.h"
#ifndef WIN32
#include "lang.h"
#endif /* !WIN32 */
#include "updates.h"
#include "nsdba.h"
#include "dcc.h"
#ifdef USE_PERL
#include "perlmod.h"
#endif /* USE_PERL */
#ifndef WIN32
#include "nxml.h"
#include "mrss.h"
#endif
#include "namedvars.h"

#ifndef WIN32
#define PID_FILENAME	"neostats.pid"
#endif /* !WIN32 */

#ifndef WIN32
static void do_reconnect( void );
#endif /* !WIN32 */

static int in_do_exit = 0;

#ifdef WIN32
static void *( *old_malloc )( size_t );
static void( *old_free )( void * );
#endif /* WIN32 */

static struct
{
	int exit_code;
	char *exit_message;
} exit_reports[]=
{
	/* NS_EXIT_NORMAL */
	{ EXIT_SUCCESS, "Normal shut down subsystems" },
	/* NS_EXIT_RELOAD */
	{ EXIT_SUCCESS, "Reloading NeoStats" },
	/* NS_EXIT_RECONNECT */
	{ EXIT_SUCCESS, "Restarting NeoStats subsystems" },
	/* NS_EXIT_ERROR */
	{ EXIT_FAILURE, "Exiting due to error" },
	/* NS_EXIT_SEGFAULT */
	{ EXIT_FAILURE, "Shutting down subsystems without saving data due to core" },
};

char segv_location[SEGV_LOCATION_BUFSIZE];

/*! Date we were compiled */
const char version_date[] = __DATE__;
/*! Time we were compiled */
const char version_time[] = __TIME__;

static int attempts = 0;
jmp_buf sigvbuf;

/** @brief update_time_now
 *
 *  update the me.now struct entries
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */
void update_time_now( void )
{
	me.now = time(NULL);
	ircsnprintf (me.strnow, STR_TIME_T_SIZE, "%lu", (long)me.now);
}

#ifndef WIN32
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
	printf( "Copyright (c) 2000-2008\n" );
	printf( "Justin Hammond (fish@neostats.net)\n" );
	printf( "Mark Hetherington (m@neostats.net)\n" );
	printf( "-----------------------------------------------\n\n" );
}
#endif /* !WIN32 */

#ifndef WIN32
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
	int level;

	while( ( c = getopt( argc, argv, "hvrd:nqf" ) ) != -1 ) {
		switch( c ) {
		case 'h':
			printf( "NeoStats: Usage: \"neostats [options]\"\n" );
			printf( "     -h (Show this screen)\n" );
			printf( "	  -v (Show version number)\n" );
			printf( "	  -d 1-10 (Debug log output level 1 = lowest, 10 = highest)\n" );
			printf( "	  -n (Do not load any modules on startup)\n" );
			printf( "	  -q (Quiet start - for cron scripts)\n" );
			printf( "     -f (Do not fork into background)\n" );
			return NS_FAILURE;
		case 'v':
			printf( "NeoStats: http://www.neostats.net\n" );
			printf( "Version:  %s\n", me.version );
			printf( "Compiled: %s at %s\n", ns_module_info.build_date, ns_module_info.build_time );
			print_copyright();
			return NS_FAILURE;
		case 'd':
			level = atoi( optarg );
			if( ( level > DEBUGMAX ) ||( level < 1 ) ) {
				printf( "Invalid debug level %d\n", level );
				return NS_FAILURE;
			}
			printf( "debug.log enabled at level %d. Watch your disk space\n", level );
			nsconfig.debuglevel = level;
			nsconfig.debug = 1;
			break;
		case 'n':
			nsconfig.modnoload = 1;
			break;
		case 'q':
			nsconfig.quiet = 1;
			break;
		case 'f':
			nsconfig.foreground = 1;
			break;
		default:
			printf( "Unknown command line switch %c\n", optopt );
		}
	}
	return NS_SUCCESS;
}
#endif /* !WIN32 */

/** @brief InitMe
 *
 *  init me structure and set pre config defaults
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int InitMe( void )
{
	os_memset( &me, 0, sizeof( me ) );
	/* initialise version */
	strlcpy( me.version, NEOSTATS_VERSION, VERSIONSIZE );
	/* our default lang is always -1 */
	me.lang = -1;
	me.numeric = 1;
	update_time_now();
	me.ts_boot = me.now;
	/* Clear config */
	os_memset( &nsconfig, 0 , sizeof( config ) );
	/* Default reconnect time */
	nsconfig.r_time = 10;
	/* Debug mode overrides */
#ifdef DEBUG
	nsconfig.debug = 1;
	nsconfig.loglevel = LOG_INFO;
	nsconfig.debuglevel = DEBUG10;
	nsconfig.foreground = 1;
#endif /* DEBUG */
	/* default debugmodule to all */
	strlcpy( nsconfig.debugmodule, "all", MAX_MOD_NAME );
#ifdef WIN32
	nsconfig.loglevel = LOG_NORMAL;
#endif /* WIN32 */
	/* default DBM */
	strlcpy(me.dbm, "gdbm" ,MAXHOST );
#ifndef WIN32
	/* this is a hack to compile in pcre and nxml and mrss static libary files */
	if (me.numeric != 1) {
		pcre_config(0, NULL);
		nxml_new(NULL);
		mrss_parse_file(NULL, NULL);
	}
#endif
	return NS_SUCCESS;
}

/** @brief InitCore
 *
 *  init core sub systems
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

static int InitCore( void )
{
	/* init named vars first */
	if (nv_init() != NS_SUCCESS) 
		return NS_FAILURE;
	if( InitSocks() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitTimers() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitDBA() != NS_SUCCESS )
		return NS_FAILURE;
	/* Open core database */
	DBAOpenDatabase();
	/* initialize Module subsystem */
	if( InitModules() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitBots() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitDns() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitServers() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitUsers() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitChannels() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitBans() != NS_SUCCESS )
		return NS_FAILURE;	
	if( InitCurl() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitIrcd() != NS_SUCCESS )
		return NS_FAILURE;
	if( InitDCC() != NS_SUCCESS )
		return NS_FAILURE;
#ifdef USE_PERL
	if( Init_Perl() != NS_SUCCESS )
		return NS_FAILURE;
#endif /* USE_PERL */
	InitServices();
	dlog( DEBUG1, "Core init successful" );
	return NS_SUCCESS;
}

/** @brief FiniCore
 *
 *  cleanup core sub systems
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */

static void FiniCore( void )
{
	FiniDCC();
	FiniCurl();
	FiniUsers();
	FiniChannels();
	FiniExcludes();
	FiniServers();
	FiniBans();
	FiniDns();
	FiniModules();
	FiniServices();
	FiniBots();
	FiniUpdate();
	FiniTimers();
	FiniSocks();
	FiniIrcd();
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

#ifdef WIN32
int neostats( void )
#else /* WIN32 */
int main( int argc, char *argv[] )
#endif /* WIN32 */
{
#ifndef WIN32
   	char dbpath[MAXPATH];
#endif /* WIN32 */

	if( InitMe() != NS_SUCCESS )
		return EXIT_FAILURE;
#ifndef WIN32
	/* get our commandline options */
	if( get_options( argc, argv ) != NS_SUCCESS )
		return EXIT_FAILURE;
	/* keep quiet if we are told to : ) */
	if( !nsconfig.quiet )
	{
		printf( "NeoStats %s Loading...\n", me.version );
		print_copyright();
	}
    /* make sure any files we create are not group/world readable (password info?) */
    umask(077);
#endif /* !WIN32 */
#if 0
	/* Change to the working Directory */
	if( chdir( NEO_PREFIX ) < 0 ) {
		printf( "NeoStats Could not change to %s\n", NEO_PREFIX );
		printf( "Did you 'make install' after compiling?\n" );
		printf( "Error Was: %s\n", strerror( errno ) );
		return EXIT_FAILURE;
	}
#endif

	/* Init run level to NeoStats core */
	RunModule[0] = &ns_module;
	/* before we do anything, make sure logging is setup */
	if( InitLogs() != NS_SUCCESS )
		return EXIT_FAILURE;
	/* our crash trace variables */
	SET_SEGV_LOCATION();
    /* init the major subsystems and config first */
	/* prepare to catch errors */
	InitSignals();
	/* load the config files */
	if( ConfLoad() != NS_SUCCESS )
		return NS_FAILURE;
	if( !me.servicehost[0] )
		strlcpy( me.servicehost, me.name, sizeof( me.name ) );
#ifndef WIN32
	/* initialize Lang Subsystem */
	ircsnprintf( dbpath, MAXPATH, "%s/data/lang.db", NEO_PREFIX );
	LANGinit( 1, dbpath, NULL );
#endif /* !WIN32 */
#ifndef WIN32
#ifndef DEBUG
	/* if we are compiled with debug, or forground switch was specified, DONT FORK */
	if( !nsconfig.foreground )
	{
		int pid;
		/* fix the double log message problem by closing logs prior to fork() */ 
		CloseLogs(); 
		pid = fork();
		/* Error check fork() */ 
		if( pid < 0 )
		{
			perror( "fork" ); 
			return EXIT_FAILURE; /* fork error */ 
		} 
		/* we are the parent */ 
		if( pid > 0 )
		{
			FILE *fp;

			/* write PID file */
			fp = fopen( PID_FILENAME, "wt" );
			fprintf( fp, "%i", pid );
			fclose( fp );
			if( !nsconfig.quiet )
			{
				printf( "\n" );
				printf( _( "NeoStats %s Successfully Launched into Background\n" ), me.version );
				printf( _( "PID: %i - Wrote to %s\n" ), pid, PID_FILENAME );
			}
			return EXIT_SUCCESS; /* parent exits */ 
		}
		/* child (daemon) continues */ 
		/* close standard file descriptors since they are invalid for a daemon */
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		/* reopen logs for child */ 
		if( InitLogs() != NS_SUCCESS )
			return EXIT_FAILURE;
		/* detach from parent process */
		if( setpgid( 0, 0 ) < 0 )
			nlog( LOG_WARNING, "setpgid() failed" );
	}
#endif  /* !DEBUG */
#endif /* !WIN32 */

  	/* Init NeoStats remaining sub systems now because:
  	 * `man kevent`
     *  The kqueue() system call creates a new kernel event queue and returns a
  	 *  descriptor.  The queue is not inherited by a child created with fork(2).
  	 *  However, if rfork(2) is called without the RFFDG flag, then the descrip-
  	 *  tor table is shared, which will allow sharing of the kqueue between two
  	 *  processes.
  	 *
  	 * <!@fish>SON OF A BITCH - This bug was a bastard to find!
  	 */
	if( InitCore() != NS_SUCCESS )
		return EXIT_FAILURE;
	dlog( DEBUG1, "NeoStats will use %s", event_get_method() );
	nlog( LOG_NOTICE, "NeoStats \"%s\" started.", me.version );
#ifdef WIN32
	/* override pcre lib malloc calls with our own version */
	old_malloc = pcre_malloc;
	old_free = pcre_free;
	pcre_malloc = os_malloc;
	pcre_free = os_free;
#endif /* WIN32 */
	/* Load modules after we fork. This fixes the load->fork-exit->call 
	   _fini problems when we fork */
	ConfLoadModules();
	InitUpdate();
	/* Connect to server */
	Connect();
#ifdef WIN32
	return EXIT_SUCCESS;
#else /* WIN32 */
	do_reconnect();
	/* We should never reach here but the compiler does not realise and may
	   complain about not all paths control returning values without the return 
	   Since it should never happen, treat as an error condition! */
	return EXIT_FAILURE;
#endif /* WIN32 */
}

#ifndef WIN32
/** @brief do_reconnect
 *
 *  Reconnect routine. Cleans up systems and flushes data files
 *  then exits and reconnects.
 *  NeoStats core use only.
 *
 *  @param none
 *
 *  @return none
 */
static void do_reconnect( void )
{
	if( nsconfig.r_time > 0 )
		nlog( LOG_NOTICE, "Reconnecting to the server in %d seconds (Attempt %i)", nsconfig.r_time, attempts );
	else
		nlog( LOG_NOTICE, "Reconnect time is zero, shutting down" );
	do_exit( NS_EXIT_RECONNECT, NULL );
}
#endif /* !WIN32 */

/** @brief do_exit
 *
 *  Exit routine. Cleans up systems and flushes data files
 *  then exits cleanly. During a segfault data is not saved.
 *  NeoStats core use only.
 *
 *  @param exitcode reason for exit
 *  @param quitmsg optional quit message to send over IRC
 *
 *  @return none
 */

void do_exit( NS_EXIT_TYPE exitcode, const char *quitmsg )
{
	int return_code;

	if( in_do_exit )
	{
		nlog( LOG_CRITICAL, "BUG: recursive do_exit calls, report this log entry to the NeoStats team" );
	}
	in_do_exit = 1;
	return_code = exit_reports[exitcode].exit_code;
	nlog( LOG_CRITICAL, exit_reports[exitcode].exit_message );

	if( exitcode != NS_EXIT_SEGFAULT ) {
		unload_modules();
		DBACloseDatabase();
		if( quitmsg )
		{
			irc_quit( ns_botptr, quitmsg );
			irc_squit( me.name, quitmsg );
		}
		event_loop( EVLOOP_ONCE );
		sleep( 1 );
		/* cleanup up core subsystems */
		FiniCore();
	}
	FiniDBA();
	FiniLogs();
#ifdef WIN32
	/* restore pcre lib malloc pointers */
	pcre_malloc = old_malloc;
	pcre_free = old_free;
#endif /* WIN32 */
#ifndef WIN32
	LANGfini();
	if( ( exitcode == NS_EXIT_RECONNECT && nsconfig.r_time > 0 ) || exitcode == NS_EXIT_RELOAD ) {
		sleep( nsconfig.r_time );
		execve( "./bin/neostats", NULL, NULL );
		return_code = EXIT_FAILURE;	/* exit code to error */
	}
	remove( PID_FILENAME );
#endif /* !WIN32 */
	exit( return_code );
}

/** @brief fatal_error
 *
 *  fatal error messsage handler
 *  NeoStats core use only.
 *
 *  @param file name of calling file
 *  @param line line of calling code
 *  @param func name of calling function
 *  @param error_text text of error
 *
 *  @return none
 */

void fatal_error( char *file, int line, const char *func, char *error_text )
{
	nlog( LOG_CRITICAL, "Fatal Error: %s %d %s %s", file, line, func, error_text );
	do_exit( NS_EXIT_ERROR, "Fatal Error - check log file" );
}
