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
** $Id: modules.c 3316 2008-03-05 08:12:49Z Fish $
*/

#include "neostats.h"
#include "dl.h"
#include "timer.h"
#include "sock.h"
#include "services.h"
#include "modules.h"
#include "nsevents.h"
#include "bots.h"
#include "dns.h"
#include "protocol.h"
#include "auth.h"
#include "users.h"
#include "channels.h"
#include "servers.h"
#include "exclude.h"
#include "updates.h"
#ifdef USE_PERL
#undef _
#define PERLDEFINES
#include "perlmod.h"
#endif /* USE_PERL */
#include "namedvars.h"
#include "namedvars-core.h"

/** @brief Module list */
static Module *ModList[NUM_MODULES];
/** @brief Module run level stack control variables */
Module *RunModule[10];
int RunLevel = 0;
/* @brief Module hash list */
static hash_t *modulehash;

nv_struct nv_modules[] = {
	{ "name", NV_PSTR, offsetof(ModuleInfo, name), NV_FLG_RO, offsetof(Module, info), -1 },
	{ "description", NV_PSTR, offsetof(ModuleInfo, description), NV_FLG_RO, offsetof(Module, info), -1},
	{ "copyright", NV_PSTRA, offsetof(ModuleInfo, copyright), NV_FLG_RO, offsetof(Module, info), -1},
	{ "about_text", NV_PSTRA, offsetof(ModuleInfo, about_text), NV_FLG_RO, offsetof(Module, info), -1},
	{ "version", NV_PSTR, offsetof(ModuleInfo, version), NV_FLG_RO, offsetof(Module, info), -1},
	{ "build_date", NV_PSTR, offsetof(ModuleInfo, build_date), NV_FLG_RO, offsetof(Module, info), -1},
	{ "build_time", NV_PSTR, offsetof(ModuleInfo, build_time), NV_FLG_RO, offsetof(Module, info), -1},
	{ "type", NV_INT, offsetof(Module, type), NV_FLG_RO, -1, -1},
	{ "modnum", NV_INT, offsetof(Module, modnum), NV_FLG_RO, -1, -1},
	{ "status", NV_INT, offsetof(Module, status), NV_FLG_RO, -1, -1},
	NV_STRUCT_END()
};


/** @brief ProcessModuleList
 *
 *  Calls handler for all modules
 *
 *  @params handler to call
 *  @params v additional data to send to handler
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ProcessModuleList( const ModuleListHandler handler, void *v )
{
	Module *module_ptr;
	hscan_t ms;
	hnode_t *node;
	int ret = 0;

/*	SET_SEGV_LOCATION(); */

	hash_scan_begin( &ms, modulehash );
	while( ( node = hash_scan_next( &ms ) ) != NULL )
	{
		module_ptr = hnode_get( node );
		ret = handler( module_ptr, v );
		if( ret != 0 )
			break;
	}
	return ret;
}

/** @brief InitModules
 *
 *  initialises module list hashes
 *  For core use only
 *
 *  @params none
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int InitModules( void )
{
	SET_SEGV_LOCATION();
	modulehash = nv_hash_create( NUM_MODULES, 0, 0, "Modules", nv_modules, NV_FLAGS_RO, NULL);
	if( !modulehash ) {
		nlog( LOG_CRITICAL, "Unable to create module hash" );
		return NS_FAILURE;
	}
	return NS_SUCCESS;
}

/** @brief FiniModules
 *
 *  Fini module subsystem
 *
 *  @params none
 *
 *  @return none
 */

void FiniModules( void ) 
{
	hash_destroy( modulehash );
}

/** @brief SynchModule
 *
 *  Synch module
 *
 *  @params module_ptr pointer to module to synch
 *
 *  @return result of ModSynch
 */

int SynchModule( Module *module_ptr )
{
	int err = NS_SUCCESS;
	int( *ModSynch )( void );

	/* Start sync process */
	SetModuleInSynch( module_ptr );
	if( IS_STANDARD_MOD( module_ptr ) )
	{
		ModSynch = ns_dlsym( ( int * ) module_ptr->handle, "ModSynch" );
		if( ModSynch )
		{
			SET_RUN_LEVEL( module_ptr );
			err = ( *ModSynch )(); 
			RESET_RUN_LEVEL();
		}
	}
#ifdef USE_PERL
	else
	{
		SET_RUN_LEVEL( module_ptr );
		err = perl_sync_module( module_ptr );
		RESET_RUN_LEVEL();
	}
#endif /* USE_PERL */
	/* sync complete */
	SetModuleSynched( module_ptr );
	return err;
}
	
/** @brief SynchAllModule
 *
 *  Synch all modules
 *
 *  @params none
 *
 *  @return none
 */

void SynchAllModules( void )
{
	Module *module_ptr;
	hscan_t ms;
	hnode_t *node;

	SET_SEGV_LOCATION();
	hash_scan_begin( &ms, modulehash );
	while( ( node = hash_scan_next( &ms ) ) != NULL ) {
		module_ptr = hnode_get( node );
		if( SynchModule( module_ptr ) != NS_SUCCESS ) {
			unload_module( module_ptr->info->name, NULL );
		}
	}
}

/** @brief ModuleVersion
 *
 *  report module versions
 *
 *  @params module_ptr pointer to module to report
 *  @params v nick to send to
 *
 *  @return none
 */

static int ModuleVersion( Module *module_ptr, void *v )
{
	irc_numeric( RPL_VERSION, (char *)v, _( "Module %s version: %s %s %s" ),
		module_ptr->info->name, module_ptr->info->version, 
		module_ptr->info->build_date, module_ptr->info->build_time );
	return NS_FALSE;
}

/** @brief AllModuleVersions
 *
 *  report module versions
 *
 *  @params nick
 *  @params remoteserver
 *
 *  @return none
 */

void AllModuleVersions( const char* nick, const char *remoteserver )
{
	ProcessModuleList( ModuleVersion, (void *)nick );
}

/** @brief load_module_error
 *
 *  report load module error
 *
 *  @params target
 *  @params module_name
 *  @params fmt
 *  @params ...
 *
 *  @return none
 */

void load_module_error( const Client *target, const char *module_name, const char *fmt, ... )
{
	static char buf[BUFSIZE];
	va_list ap;

	va_start( ap, fmt );
	ircvsnprintf( buf, BUFSIZE, fmt, ap );
	va_end( ap );
	if( target ) {
		irc_prefmsg( ns_botptr, target, __( "Unable to load module %s: %s", u ), module_name, buf );
	}
	nlog( LOG_WARNING, buf );
}

/** @brief ns_load_module
 *
 *  determine the type of module based on extension
 *  and then call the relevent procedure to load it
 *
 *  @params modfilename name of module to load
 *  @params u client requesting load
 *
 *  @return pointer to loaded module
 */

static Module *load_stdmodule( const char *modfilename, Client * u )
{
	int err;
	void *handle;
	ModuleInfo *infoptr = NULL;
	ModuleEvent *eventlistptr = NULL;
	Module *mod_ptr = NULL;
	int( *ModInit )( void );
	CmdParams *cmd;

	SET_SEGV_LOCATION();
	if( hash_isfull( modulehash ) ) {
		load_module_error( u, modfilename, __( "module list is full", u ) );
		return NULL;
	} 
	handle = ns_dlopen( modfilename, RTLD_NOW | RTLD_GLOBAL );
	if( !handle ) {
		load_module_error( u, modfilename, ns_dlerrormsg, modfilename );
		return NULL;
	}
	infoptr = ns_dlsym( handle, "module_info" );
	if( infoptr == NULL ) {
		load_module_error( u, modfilename, __( "missing module_info", u ) );
		ns_dlclose( handle );
		return NULL;
	}
	/* Check module was built for this version of NeoStats */
	if( ircstrncasecmp( NEOSTATS_VERSION, infoptr->neostats_version, VERSIONSIZE ) !=0 ) {
		load_module_error( u, modfilename, __( "module built with an old version of NeoStats and must be rebuilt.", u ) );
		ns_dlclose( handle );
		return NULL;
	}
	if( !infoptr->copyright || ircstrcasecmp( infoptr->copyright[0], "Copyright (c) <year>, <your name>" ) ==0 ) {
		load_module_error( u, modfilename, __( "missing copyright text.", u ) );
		ns_dlclose( handle );
		return NULL;
	}	
	if( !infoptr->about_text || ircstrcasecmp( infoptr->about_text[0], "About your module" ) ==0 ) {
		load_module_error( u, modfilename, __( "missing about text.", u ) );
		ns_dlclose( handle );
		return NULL;
	}	
	/* Check that the Module hasn't already been loaded */
	if( hash_lookup( modulehash, infoptr->name ) ) {
		ns_dlclose( handle );
		load_module_error( u, modfilename, __( "already loaded", u ) );
		return NULL;
	}
	/* Check we have require PROTOCOL/FEATURE support for module */
	if( ( infoptr->features & ircd_srv.features ) != infoptr->features ) {
		load_module_error( u, modfilename, __( "Required module features not available on this IRCd.", u ), modfilename );
		ns_dlclose( handle );
		return NULL;
	}
	/* Lookup ModInit( replacement for library __init() call */
	ModInit = ns_dlsym( ( int * ) handle, "ModInit" );
	if( !ModInit ) {
		load_module_error( u, modfilename, __( "missing ModInit.", u ) );
		ns_dlclose( handle );
		return NULL;
	}
	/* Allocate module */
	mod_ptr = ( Module * ) ns_calloc( sizeof( Module ) );
	dlog( DEBUG1, "Module internal name: %s", infoptr->name );
	dlog( DEBUG1, "Module description: %s", infoptr->description );
	mod_ptr->info = infoptr;
	mod_ptr->handle = handle;
	insert_module( mod_ptr );
	mod_ptr->type = MOD_TYPE_STANDARD;
	/* Extract pointer to event list */
	eventlistptr = ns_dlsym( handle, "module_events" );
	if( eventlistptr ) {
		SET_RUN_LEVEL( mod_ptr );
		AddEventList( eventlistptr );
		RESET_RUN_LEVEL();
	}
    /* For Auth modules, register auth function */
	if( infoptr->flags & MODULE_FLAG_AUTH ) {
		if( AddAuthModule( mod_ptr ) != NS_SUCCESS ) {
			load_module_error( u, modfilename, __( "Unable to load auth module: %s missing ModAuthUser function",u ), infoptr->name );
			unload_module( mod_ptr->info->name, NULL );
			return NULL;
		}
	}
    /* Module side user authentication for e.g. SecureServ helpers 
     * Not available on auth modules */
	if( !( infoptr->flags & MODULE_FLAG_AUTH ) )
		mod_ptr->authcb = ns_dlsym( ( int * ) handle, "ModAuthUser" );
	if( infoptr->flags & MODULE_FLAG_CTCP_VERSION )
		me.versionscan ++;		
	/* assign a module number to this module */
	assign_mod_number( mod_ptr );

	SET_SEGV_LOCATION();
	SET_RUN_LEVEL( mod_ptr );
	DBAOpenDatabase();
	err = ( *ModInit )(); 
	RESET_RUN_LEVEL();
	if( err < 1 || IsModuleError( mod_ptr ) ) {
		load_module_error( u, modfilename, __( "See %s.log for further information.",u ), mod_ptr->info->name );
		unload_module( mod_ptr->info->name, NULL );
		return NULL;
	}
	if( infoptr->flags & MODULE_FLAG_LOCAL_EXCLUDES ) 
	{
		SET_RUN_LEVEL( mod_ptr );	
		InitExcludes( mod_ptr );
		RESET_RUN_LEVEL();
	}
	SET_SEGV_LOCATION();

	/* Let this module know we are online if we are! */
	if( IsNeoStatsSynched() ) {
		if( SynchModule( mod_ptr ) != NS_SUCCESS || IsModuleError( mod_ptr ) )
		{
			load_module_error( u, modfilename, __( "See %s.log for further information.", u ), mod_ptr->info->name );
			unload_module( mod_ptr->info->name, NULL );
			return NULL;
		}
	}
	cmd = ns_calloc( sizeof( CmdParams ) );
	cmd->param = ( char * )infoptr->name;
	SendAllModuleEvent( EVENT_MODULELOAD, cmd );
	ns_free( cmd );
	if( u ) {
		irc_prefmsg( ns_botptr, u, __( "Module %s loaded, %s",u ), infoptr->name, infoptr->description );
		irc_globops( NULL, _( "Module %s loaded" ), infoptr->name );
	}
	return mod_ptr;
}

/** @brief ns_load_module
 *
 *  determine the type of module based on extension
 *  and then call the relevent procedure to load it
 *
 *  @params modfilename name of module to load
 *
 *  @return pointer to loaded module
 */

Module *ns_load_module( const char *modfilename, Client * u )
{ 
	static char path[255];
	static char loadmodname[255];
	struct stat buf;

	strlcpy( loadmodname, modfilename, 255 );
	ns_strlwr( loadmodname );
	ircsnprintf( path, 255, "%s/%s%s", MOD_PATH, loadmodname, MOD_STDEXT );
	if( stat( path, &buf ) != -1 ) {
		return load_stdmodule( path, u );
	}
#ifdef USE_PERL
	ircsnprintf( path, 255, "%s/%s%s", MOD_PATH, loadmodname, MOD_PERLEXT );
	if( stat( path, &buf ) != -1 ) {
		Module *mod;
		mod = load_perlmodule( path, u );
		if (!mod) {
			return NULL;
		}
		mod->info->build_date = ns_malloc( 10 );
		strftime( ( char * )mod->info->build_date, 9, "%d/%m/%y", gmtime( &buf.st_mtime ) );
		mod->info->build_time = ns_malloc( 11 );
		strftime( ( char * )mod->info->build_time, 10, "%H:%M", gmtime( &buf.st_mtime ) );
		return mod;
	}
#endif /* USE_PERL */
	/* if we get here, ehhh, doesn't exist */
	load_module_error( u, modfilename, __( "Module file not found", u ) );
	return NULL;

}

/** @brief insert_module
 *
 *  generate module number and assign it
 *
 *  @params mod_ptr module pointer 
 *
 *  @return none
 */

void assign_mod_number( Module *mod_ptr )
{
	int moduleindex = 0;

	while( ModList[moduleindex] != NULL )
		moduleindex++;
	ModList[moduleindex] = mod_ptr;
	mod_ptr->modnum = moduleindex;
	dlog( DEBUG1, "Assigned %d to module %s for modulenum", moduleindex, mod_ptr->info->name );
}

/** @brief insert_module
 *
 *  insert module pointer into module hash. 
 *
 *  @params mod_ptr module pointer 
 *
 *  @return none
 */

void insert_module( Module *mod_ptr )
{
	hnode_create_insert( modulehash, mod_ptr, mod_ptr->info->name );
}

/** @brief ReportModuleInfo
 *
 *  MODLIST helper
 *  report module info
 *
 *  @params module_ptr pointer to module to report
 *  @params v client to send to
 *
 *  @return none
 */

static int ReportModuleInfo( Module *module_ptr, void *v )
{
	Client *u = (Client *)v;
	irc_prefmsg( ns_botptr, u, __( "Module: %d %s (%s)", u ), module_ptr->modnum, module_ptr->info->name, module_ptr->info->version );
	irc_prefmsg( ns_botptr, u, "      : %s", module_ptr->info->description );
	return NS_FALSE;
}

/** @brief ns_cmd_modlist
 *
 *  MODLIST command handler
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ns_cmd_modlist( const CmdParams* cmdparams )
{
	SET_SEGV_LOCATION();
	ProcessModuleList( ReportModuleInfo, (void *)cmdparams->source );
	irc_prefmsg( ns_botptr, cmdparams->source, __( "End of Module List", cmdparams->source ) );
	return 0;
}

/** @brief ns_cmd_load
 *
 *  LOAD command handler
 *  Load module
 *   
 *  @param cmdparams structure with command information
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ns_cmd_load( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	if( ns_load_module( cmdparams->av[0], cmdparams->source ) )
		irc_chanalert( ns_botptr, _( "%s loaded module %s" ), cmdparams->source->name, cmdparams->av[0] );
	else
		irc_chanalert( ns_botptr, _( "%s tried to load module %s, but load failed" ), cmdparams->source->name, cmdparams->av[0] );
   	return NS_SUCCESS;
}

/** @brief ns_cmd_unload
 *
 *  UNLOAD command handler
 *  Unload module
 *   
 *  @param cmdparams structure with command information
 *
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
 */

int ns_cmd_unload( const CmdParams *cmdparams )
{
	SET_SEGV_LOCATION();
	if( unload_module( cmdparams->av[0], cmdparams->source ) > 0 )
		irc_chanalert( ns_botptr, _( "%s unloaded module %s" ), cmdparams->source->name, cmdparams->av[0] );
   	return NS_SUCCESS;
}

/** @brief unload_module
 *
 *  Unloads module
 *
 *  @param modname name of module to unload
 *  @param u pointer to client requesting unload if appropriate
 * 
 *  @return NS_SUCCESS if succeeds, NS_FAILURE if not 
*/

int unload_module( const char *modname, Client * u )
{
	Module *mod_ptr;
	hnode_t *modnode;
	int moduleindex;
	int( *ModFini )( void );
	CmdParams *cmdparams;

	SET_SEGV_LOCATION();
	/* Check to see if module is loaded */
	modnode = hash_lookup( modulehash, modname );
	if( !modnode ) {
		if( u ) {
			irc_prefmsg( ns_botptr, u, __( "Module %s not loaded, try /msg %s modlist", u ), modname, ns_botptr->name );
			irc_chanalert( ns_botptr, _( "%s tried to unload %s but its not loaded" ), u->name, modname );
		}
		return NS_FAILURE;
	}
	mod_ptr = hnode_get( modnode );
	irc_chanalert( ns_botptr, _( "Unloading module %s" ), modname );

	MQModuleDelcmd(mod_ptr);
	
	if( mod_ptr->info->flags & MODULE_FLAG_AUTH )
		DelAuthModule( mod_ptr );
	if( mod_ptr->info->flags & MODULE_FLAG_CTCP_VERSION )
		me.versionscan --;
	moduleindex = mod_ptr->modnum;
	/* canx any DNS queries used by this module */
	canx_dns( mod_ptr );
	/* Delete any timers used by this module */
	del_timers( mod_ptr );
	/* Delete any sockets used by this module */
	del_sockets( mod_ptr );
	/* Delete any associated event list */
	FreeEventList( mod_ptr );

#ifdef USE_PERL
	/* unload any extensions first */
	if ((mod_ptr->pm) && (mod_ptr->pm->type == TYPE_EXTENSION)) {
		PerlExtensionFini(mod_ptr);
		unload_perlextension(mod_ptr);
	}
#endif /* USE_PERL */

	/* Remove from the module hash so we dont call events for this module 
	 * during signoff 
	 */
	dlog( DEBUG1, "Deleting Module %s from Hash", modname );
	hash_delete_destroy_node( modulehash, modnode );		

	/* now determine if its perl, or standard module */
	if( IS_STANDARD_MOD( mod_ptr ) ) {
		/* call ModFini( replacement for library __fini() call */
		ModFini = ns_dlsym( ( int * ) mod_ptr->handle, "ModFini" );
		if( ModFini ) {
			SET_RUN_LEVEL( mod_ptr );
			( *ModFini )();
			RESET_RUN_LEVEL();
			SET_SEGV_LOCATION();
		}
#ifdef USE_PERL
	} else {
		SET_RUN_LEVEL( mod_ptr );
		PerlModFini( mod_ptr );
		RESET_RUN_LEVEL();
		SET_SEGV_LOCATION();
#endif /* USE_PERL */
	}
	/* Delete any bots used by this module. Done after ModFini, so the bot 
	 * can still send messages during ModFini 
	 */
	DelModuleBots( mod_ptr );
	/* Close module */
	irc_globops( NULL, _( "%s Module unloaded" ), modname );
	SET_RUN_LEVEL( mod_ptr );
	if( mod_ptr->info->flags & MODULE_FLAG_LOCAL_EXCLUDES ) 
	{
		FiniModExcludes( mod_ptr );
	}
	cmdparams = ns_calloc( sizeof( CmdParams ) );
	cmdparams->param = ( char * )modname;
	SendAllModuleEvent( EVENT_MODULEUNLOAD, cmdparams );
	ns_free( cmdparams );
	RESET_RUN_LEVEL();

	SET_RUN_LEVEL( mod_ptr );
	DBACloseDatabase();
	/* Cleanup moddata */
	CleanupUserModdata();
	CleanupServerModdata();
	CleanupChannelModdata();

	if( IS_STANDARD_MOD( mod_ptr ) )
		ns_dlclose( mod_ptr->handle );
#ifdef USE_PERL
	else
		unload_perlmod( mod_ptr );
#endif /* USE_PERL */
	RESET_RUN_LEVEL();
	ns_free( mod_ptr );
	/* free the module number */
	if( moduleindex >= 0 ) {
		dlog( DEBUG1, "Free %d from Module Numbers", moduleindex );
		ModList[moduleindex] = NULL;
	}
	return NS_SUCCESS;
}

/** @brief unload_modules
 *
 *  Unloads all loaded modules
 *
 *  @param none
 * 
 *  @return none
*/

void unload_modules( void )
{
	Module *mod_ptr;
	hscan_t ms;
	hnode_t *node;

	/* Walk through hash list unloading each module */
	hash_scan_begin( &ms, modulehash );
	while( ( node = hash_scan_next( &ms ) ) != NULL ) {
		mod_ptr = hnode_get( node );
		unload_module( mod_ptr->info->name, NULL );
	}
}

/** @brief ModuleConfig
 *
 *  Load module configuration
 *
 *  @param set_ptr pointer to list of module settings
 *
 *  @return none
 */

void ModuleConfig( bot_setting* set_ptr )
{
	SET_SEGV_LOCATION();
	DBAOpenTable( CONFIG_TABLE_NAME );
	while( set_ptr->option != NULL )
	{
		switch( set_ptr->type ) {
			case SET_TYPE_BOOLEAN:
				if( DBAFetchConfigBool( set_ptr->option, set_ptr->varptr ) != NS_SUCCESS ) {
					*( int * )set_ptr->varptr = ( int )set_ptr->defaultval;
					DBAStoreConfigBool( set_ptr->option, set_ptr->varptr );
				}
				if( set_ptr->handler ) {
					set_ptr->handler( NULL, SET_LOAD );
				}
				break;
			case SET_TYPE_INT:
				if( DBAFetchConfigInt( set_ptr->option, set_ptr->varptr ) != NS_SUCCESS ) {
					*( int * )set_ptr->varptr = ( int )set_ptr->defaultval;
					DBAStoreConfigInt( set_ptr->option, set_ptr->varptr );
				}
				if( set_ptr->handler ) {
					set_ptr->handler( NULL, SET_LOAD );
				}
				break;
			case SET_TYPE_STRING:
			case SET_TYPE_CHANNEL:							
			case SET_TYPE_MSG:
			case SET_TYPE_NICK:
			case SET_TYPE_USER:
			case SET_TYPE_HOST:
			case SET_TYPE_REALNAME:
			case SET_TYPE_IPV4:
				if( DBAFetchConfigStr( set_ptr->option, set_ptr->varptr, set_ptr->max ) != NS_SUCCESS ) {
					if( set_ptr->defaultval ) {
						strlcpy( set_ptr->varptr, set_ptr->defaultval, set_ptr->max );
					}
					DBAStoreConfigStr( set_ptr->option, set_ptr->varptr, set_ptr->max );
				}
				if( set_ptr->handler ) {
					set_ptr->handler( NULL, SET_LOAD );
				}
				break;			
			case SET_TYPE_CUSTOM:
				if( set_ptr->handler ) {
					set_ptr->handler( NULL, SET_LOAD );
				}
				break;
			default:
				nlog( LOG_WARNING, "Unsupported SET type %d in ModuleConfig %s", 
					set_ptr->type, set_ptr->option );
				break;
		}
		set_ptr++;
	}
	DBACloseTable( CONFIG_TABLE_NAME );
}
