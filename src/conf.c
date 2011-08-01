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
** $Id: conf.c 3331 2008-03-18 14:22:22Z Fish $
*/

#include "neostats.h"
#include "confuse.h"
#include "conf.h"
#include "log.h"
#include "services.h"
#include "modules.h"
#include "dl.h"
#ifdef USE_PERL
#include "perlmod.h"
#endif /* USE_PERL */

#define CONFIG_NAME		"neostats.conf"

typedef struct validate_args {
	char name[BUFSIZE];
	cfg_validate_callback_t cb;
} validate_args;

/** @brief List of modules to load */
static void *load_mods[NUM_MODULES];

static void cb_module( char *name );
static int cb_verify_chan( cfg_t *cfg, cfg_opt_t *opt );
static int cb_verify_numeric( cfg_t *cfg, cfg_opt_t *opt );
static int cb_verify_numsocks (cfg_t *cfg, cfg_opt_t *opt);
static int cb_verify_bind( cfg_t *cfg, cfg_opt_t *opt );
static int cb_verify_file( cfg_t *cfg, cfg_opt_t *opt );
static int cb_verify_log( cfg_t *cfg, cfg_opt_t *opt );
static int cb_verify_mask( cfg_t *cfg, cfg_opt_t *opt );
static int cb_verify_nick( cfg_t *cfg, cfg_opt_t *opt );
static int cb_noload( cfg_t *cfg, cfg_opt_t *opt );
static int cb_verify_host( cfg_t *cfg, cfg_opt_t *opt );
static int cb_verify_neohost( cfg_t *cfg, cfg_opt_t *opt );
static int cb_verify_settime( cfg_t *cfg, cfg_opt_t *opt );

/** @brief Core Configuration Items
 * 
 * Contains Configuration Items for the Core NeoStats service
 */

static cfg_opt_t server_details[] = {
	CFG_STR ("Name", "neostats.nonetwork.org", CFGF_NONE),
	CFG_STR ("Info", "NeoStats 3.0 Services", CFGF_NONE),
	CFG_STR ("ServiceChannel", "#services", CFGF_NONE),
	CFG_INT ("ServerNumeric", 123, CFGF_NONE),
	CFG_STR ("BindTo", 0, CFGF_NODEFAULT),
	CFG_STR ("Protocol", "unreal32", CFGF_NONE),
	CFG_END()
};

static cfg_opt_t options[] = {
	CFG_INT ("ReconnectTime", 10, CFGF_NONE),
	CFG_BOOL ("UsePrivmsg", cfg_false, CFGF_NONE),
	CFG_BOOL ("OperOnly", cfg_false, CFGF_NONE),
	CFG_INT ("ServerSettime", 0, CFGF_NONE),
	CFG_STR ("DatabaseType", "gdbm", CFGF_NONE),
	CFG_STR ("LogFileNameFormat", "-%m-%d", CFGF_NONE),
	CFG_STR ("RootNick", "NeoStats", CFGF_NONE),
	CFG_STR ("ServicesHost", "services.neostats.net", CFGF_NONE),
	CFG_BOOL ("NOLOAD", cfg_true, CFGF_NONE),
	CFG_INT ("MaxSockets", 1024, CFGF_NONE),
	CFG_END()
};

static cfg_opt_t servers[] = {
	CFG_STR ("IpAddress", 0, CFGF_NONE),
	CFG_INT ("Port", 6667, CFGF_NONE),
	CFG_STR ("Password", 0, CFGF_NONE),
	CFG_END()
};

static cfg_opt_t serviceroot[] = {
	CFG_STR ("Mask", 0, CFGF_NODEFAULT),
	CFG_END()
};

static cfg_opt_t modules[] = {
	CFG_STR_LIST ("ModuleName", 0, CFGF_NODEFAULT),
	CFG_END()
};

static cfg_opt_t neonet[] = {
	CFG_STR ("HostName", "mqpool.neostats.net", CFGF_NONE),
	CFG_INT ("Port", 2960, CFGF_NONE),
	CFG_STR ("UserName", NULL, CFGF_NODEFAULT),
	CFG_STR ("Password", NULL,CFGF_NODEFAULT ),
	CFG_STR ("Connect", "yes", CFGF_NONE),
	CFG_END()
};

static cfg_opt_t fileconfig[] = {
	CFG_SEC ("ServerConfig", server_details, CFGF_NONE),
	CFG_SEC ("Options", options, CFGF_NONE),
#if 0
/* XXX do we want to specify backup linking servers? */
	CFG_SEC ("Servers", servers, CFGF_MULTI | CFGF_TITLE),
#else /* 0 */
	CFG_SEC ("Servers", servers, CFGF_NONE),
#endif /* 0 */
	CFG_SEC ("ServiceRoot", serviceroot, CFGF_NONE),
	CFG_SEC ("Modules", modules, CFGF_NONE),
	CFG_SEC ("NeoNet", neonet, CFGF_NONE),
	CFG_END()
};

static validate_args arg_validate[] = {
	{"ServerConfig|Name", cb_verify_neohost},
	{"ServerConfig|ServiceChannel", cb_verify_chan},
	{"ServerConfig|ServerNumeric", cb_verify_numeric},
	{"ServerConfig|BindTo", cb_verify_bind},
	{"ServerConfig|Protocol", cb_verify_file},
	{"Options|ServerSettime", cb_verify_settime},
	{"Options|DataBaseType", cb_verify_file},
	{"Options|LogFileNameFormat", cb_verify_log},
	{"Options|RootNick", cb_verify_nick},
	{"Options|NOLOAD", cb_noload},
	{"Options|MaxSockets", cb_verify_numsocks},
	{"Servers|IpAddress", cb_verify_host},
	{"ServiceRoot|Mask", cb_verify_mask},
	{"Modules|ModuleName", cb_verify_file},
	{"NeoNet|HostName", cb_verify_host}
};

/** @brief ConfParseError
 *
 *  Report configuration parse error 
 *  Config subsystem use only
 *
 *  @param err error value from parse 
 *
 *  @return none
 */

static void ConfParseError( int err )
{
#ifdef WIN32
	switch( err )
	{
		case CFG_FILE_ERROR:
			nlog( LOG_ERROR, "Config file not found" );
			break;
		case CFG_PARSE_ERROR:
			nlog( LOG_ERROR, "Config Parse Error" );
			break;
		default:
			nlog( LOG_ERROR, "Unknown Error" );
			break;
	}
#else /* WIN32 */
	printf( "***************************************************\n" );
	printf( "*                  Error!                         *\n" );
	printf( "*                                                 *\n" );
	switch( err )
	{
		case CFG_FILE_ERROR:
			printf( "*           Config file not found                 *\n" );
			break;
		case CFG_PARSE_ERROR:
			printf( "*            Config Parse Error                   *\n" );
			break;
		default:
			printf( "*               Unknown Error                     *\n" );
			break;
	}
	printf ( "*                                                 *\n" );
	printf ( "*             NeoStats NOT Started                *\n" );
	printf ( "***************************************************\n" );
#endif /* WIN32 */
}

/** @brief set_config_values
 *
 *  set initial NeoStats config based on config file
 *  Config subsystem use only
 *
 *  @param cfg pointer to config struct
 *
 *  @return NS_SUCCESS or NS_FAILURE
 */

static int set_config_values( cfg_t *cfg )
{
	unsigned int i;
	/* Server name has a default */
	strlcpy (me.name, cfg_getstr (cfg, "ServerConfig|Name"), sizeof (me.name));
	
	/* Server Port has a default */
	me.port = cfg_getint (cfg, "Servers|Port");
	/* Connect To */
	if (cfg_size (cfg, "Servers|IpAddress") == 0)
	{
#ifndef WIN32
		printf ("ERROR: No Server was configured for Linking. Please fix this\n");
#endif /* WIN32 */
		return NS_FAILURE;
	}
	strlcpy (me.uplink, cfg_getstr (cfg, "Servers|IpAddress"), sizeof (me.uplink));
	/* Connect Pass */
	if (cfg_size (cfg, "Servers|Password") == 0)
	{
#ifndef WIN32
		printf ("ERROR: No Password was specified for Linking. Please fix this\n");
#endif /* WIN32 */
		return NS_FAILURE;
	}
	strlcpy (nsconfig.pass, cfg_getstr (cfg, "Servers|Password"), sizeof (nsconfig.pass));
	dlog( DEBUG6, "NeoStats ServerName: %s", me.name );
	dlog( DEBUG6, "Connecting To:       %s:%d", me.uplink, me.port );
	/* Server InfoLine has a default */
	strlcpy (me.infoline, cfg_getstr (cfg, "ServerConfig|Info"), sizeof (me.infoline));
	/* Service host has a default */
	strlcpy (me.servicehost, cfg_getstr (cfg, "Options|ServicesHost"), sizeof (me.servicehost));
	/* Reconnect time has a default */
	nsconfig.r_time = cfg_getint (cfg, "Options|ReconnectTime");
	/* want privmsg  has a default */
	nsconfig.want_privmsg = cfg_getbool (cfg, "Options|UsePrivMsg");
	/* service chan has a default */
	strlcpy (me.serviceschan, cfg_getstr (cfg, "ServerConfig|ServiceChannel"), sizeof (me.serviceschan));
	/* only opers has a default */
	nsconfig.onlyopers = cfg_getbool (cfg, "Options|OperOnly");
	/* vhost has no default, nor is it required */
	if (cfg_size (cfg, "ServerConfig|BindTo") > 0) {
		strlcpy (me.local, cfg_getstr (cfg, "ServerConfig|BindTo"), sizeof (me.local));
		dlog( DEBUG6, "Source IP:          %s\n", me.local );
	}
	/* LogFile Format has a default */
	strlcpy (LogFileNameFormat, cfg_getstr (cfg, "Options|LogFileNameFormat"), MAX_LOGFILENAME);
	/* max socks has a default */
	me.maxsocks = cfg_getint(cfg, "Options|MaxSockets");
	/* numeric has a default */
	me.numeric = cfg_getint (cfg, "ServerConfig|ServerNumeric");
	/* has a default */
	nsconfig.setservertimes = cfg_getint (cfg, "Options|ServerSettime") * 60 * 60;
	/* serviceroot has no default */
	if (cfg_size (cfg, "ServiceRoot|Mask") > 0) {
		char *nick, *user , *host, *arg;

		/* already validate */
		arg = cfg_getstr (cfg, "ServiceRoot|Mask");
		if( arg )
		{
			nick = strtok (arg, "!");
			user = strtok (NULL, "@");
			host = strtok (NULL, "");
			strlcpy (nsconfig.rootuser.nick, nick, MAXNICK);
			strlcpy (nsconfig.rootuser.user, user, MAXUSER);
			strlcpy (nsconfig.rootuser.host, host, MAXHOST);
			dlog( DEBUG6, "ServiceRoot:         %s!%s@%s", nsconfig.rootuser.nick, nsconfig.rootuser.user, nsconfig.rootuser.host );
		}
		else
			dlog( DEBUG6, "WARNING: No ServiceRoot Entry Defined");
	}
	else
		dlog( DEBUG6, "WARNING: No ServiceRoot Entry Defined");
	/* protocol is required, but defaults to unreal32 */
	strlcpy (me.protocol, cfg_getstr (cfg, "ServerConfig|Protocol"), MAXHOST);
	/* dbm has a default */
	strlcpy (me.dbm, cfg_getstr (cfg, "Options|DataBaseType"), MAXHOST);
	/* has a default */
	strlcpy (me.rootnick, cfg_getstr (cfg, "Options|RootNick"), MAXNICK);
	/* now load the modules */
	dlog( DEBUG6, "Modules Loaded:");
	for (i = 0; i < cfg_size(cfg, "Modules|ModuleName"); i++) {
		cb_module( cfg_getnstr( cfg, "Modules|ModuleName", i ) );
		dlog( DEBUG6, "                     %s", cfg_getnstr( cfg, "Modules|ModuleName", i ));
	}	
	dlog( DEBUG6, "-----------------------------------------------" );
	if (cfg_size(cfg, "NeoNet|UserName") > 0) 
		strlcpy(mqs.username, cfg_getstr(cfg, "NeoNet|UserName"), MAXUSER);
	if (cfg_size(cfg, "NeoNet|Password") > 0)
		strlcpy(mqs.password, cfg_getstr(cfg, "NeoNet|Password"), MAXUSER);
	/* has a default */
	strlcpy(mqs.hostname, cfg_getstr(cfg, "NeoNet|HostName"), MAXHOST);
	/* has a default */
	mqs.port = cfg_getint(cfg, "NeoNet|Port");
	if (!ircstrcasecmp(cfg_getstr(cfg, "NeoNet|Connect"), "yes")) {
		mqs.connect = MQ_CONNECT_YES;
	} else if (!ircstrcasecmp(cfg_getstr(cfg, "NeoNet|Connect"), "demand")) {
		mqs.connect = MQ_CONNECT_DEMAND;
	} else {
		mqs.connect = MQ_CONNECT_NO;
	}
	return NS_SUCCESS;
}

/** @brief ConfLoad
 *
 *  Load and parse configuration file
 *
 *  @param none
 *
 *  @return NS_SUCCESS or NS_FAILURE
 */

int ConfLoad( void )
{
	cfg_t *cfg;
	unsigned int i;
	int ret;

#ifndef WIN32
	printf( "Reading the Config File. Please wait ...\n" );
#endif /* WIN32 */
	cfg = cfg_init (fileconfig, CFGF_NOCASE);
	for( i = 0; i < ARRAYLEN (arg_validate); i++ )
		cfg_set_validate_func (cfg, arg_validate[i].name, arg_validate[i].cb);
	if( ( ret = cfg_parse( cfg, CONFIG_NAME ) ) != 0 )
	{
		ConfParseError( ret );
		cfg_free( cfg );
		return NS_FAILURE;
	}
	if( set_config_values( cfg ) != NS_SUCCESS )
	{
		cfg_free( cfg );
		return NS_FAILURE;
	}
	cfg_free( cfg );
#ifndef WIN32
	printf( "Successfully loaded config file, booting NeoStats\n" );
	printf( "If NeoStats does not connect, please check logs/neostats-<date>.log for further information\n" );
#endif /* WIN32 */
	return NS_SUCCESS;
}

/** @brief ConfLoadModules 
 *
 *  Load the modules that selected by the configuration file
 *
 *  @param opt pointer to option
 *
 *  @return none
 */

void ConfLoadModules( void )
{
	int i;

	SET_SEGV_LOCATION();
	if( load_mods[0] == 0 )
	{
		nlog( LOG_NORMAL, "No modules configured for loading" );
		return;
	}
	nlog( LOG_NORMAL, "Loading configured modules" );
	for( i = 0; ( i < NUM_MODULES ) && ( load_mods[i] != 0 ); i++ )
	{
		dlog( DEBUG1, "ConfLoadModules: Loading Module %s", ( char * ) load_mods[i] );
		if( ns_load_module( load_mods[i], NULL ) )
			nlog( LOG_NORMAL, "Loaded module %s", ( char * ) load_mods[i] );
		else
			nlog( LOG_WARNING, "Failed to load module %s. Please check above error messages", ( char * ) load_mods[i] );
		ns_free (load_mods[i]);
	}
	nlog( LOG_NORMAL, "Completed loading configured modules" );
	return;
}


/** @brief cb_verify_chan
 *
 *  Verify channel name configuration value
 *  Config subsystem use only
 *
 *  @param cfg pointer to config structure
 *  @param opt pointer to option
 *
 *  @return none
 */

static int cb_verify_chan( cfg_t *cfg, cfg_opt_t *opt )
{
	if( ValidateChannel( opt->values[0]->string ) == NS_FAILURE )
	{
		cfg_error( cfg, "Invalid channel name %s for option %s", opt->values[0]->string, opt->name );
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

/** @brief cb_verify_numeric
 *
 *  Verify server numeric configuration value
 *  Config subsystem use only
 *
 *  @param cfg pointer to config structure
 *  @param opt pointer to option
 *
 *  @return none
 */

static int cb_verify_numeric( cfg_t *cfg, cfg_opt_t *opt )
{
	long int num = opt->values[0]->number;

	if( ( num <= 0 ) || ( num > 254 ) )
	{
		cfg_error( cfg, "%d is out of range for %s", num, opt->name );
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

static int cb_verify_numsocks (cfg_t *cfg, cfg_opt_t *opt)
{
	long int num = opt->values[0]->number;
	long int socks;
#ifndef WIN32
	struct rlimit *lim;
#endif
#ifdef WIN32
	socks = 1024;
#else
	lim = ns_calloc (sizeof (struct rlimit));
	getrlimit (RLIMIT_NOFILE, lim);
	socks = lim->rlim_max;
	ns_free (lim);
	if(socks<0)
		socks = 1024;
#endif
	if( ( num <= 0 ) || ( socks > 254 ) )
	{
		cfg_error( cfg, "%d is out of range for %s", num, opt->name );
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}


/** @brief cb_verify_bind
 *
 *  Verify bindto configuration value
 *  Config subsystem use only
 *
 *  @param cfg pointer to config structure
 *  @param opt pointer to option
 *
 *  @return none
 */

static int cb_verify_bind( cfg_t *cfg, cfg_opt_t *opt )
{
	OS_SOCKET s;
	struct hostent *hp;

	/* test if we can bind */
	s = os_sock_socket( AF_INET, SOCK_STREAM, 0 );
	if( s == INVALID_SOCKET )
	{
		cfg_error( cfg, "Error testing bind setting." );
		return CFG_PARSE_ERROR;
	}
	if ( !ircstrcasecmp(opt->values[0]->string, "localhost") || !ircstrcasecmp(opt->values[0]->string, "127.0.0.1")) {
		cfg_error( cfg, "Error. %s is not a valid value for BindTo", opt->values[0]->string);
		return CFG_PARSE_ERROR;
	}
	if ( ( hp = gethostbyname( opt->values[0]->string ) ) == NULL )
	{
		cfg_error( cfg, "Unable to bind to address %s for option %s: %s", opt->values[0]->string, opt->name, strerror( errno ) );
		return CFG_PARSE_ERROR;
	}
	os_memset(&me.lsa, 0, sizeof(me.lsa));
    	os_memcpy( ( char * )&me.lsa.sin_addr, hp->h_addr, hp->h_length );
	me.lsa.sin_family = hp->h_addrtype;
	if( os_sock_bind( s, ( struct sockaddr * ) &me.lsa, sizeof( me.lsa ) ) == SOCKET_ERROR )
	{
		cfg_error( cfg, "Unable to bind to address %s for option %s: %s", opt->values[0]->string, opt->name, os_sock_getlasterrorstring() );
		return CFG_PARSE_ERROR;
	}
	/* if we get here, the socket is ok */
	os_sock_close( s );
	me.dobind = 1;
	return CFG_SUCCESS;
}

/** @brief cb_verify_file
 *
 *  Verify file exists
 *  Config subsystem use only
 *
 *  @param cfg pointer to config structure
 *  @param opt pointer to option
 *
 *  @return none
 */

static int cb_verify_file( cfg_t *cfg, cfg_opt_t *opt )
{
	char *file = opt->values[0]->string;
	static char buf[MAXPATH];
	struct stat fileinfo;

	ircsnprintf( buf, MAXPATH, "%s/%s%s", MOD_PATH, file, MOD_STDEXT );
	if( stat( buf, &fileinfo ) == -1 )
	{
#ifdef USE_PERL
		ircsnprintf( buf, MAXPATH, "%s/%s%s", MOD_PATH, file, MOD_PERLEXT );
		if( stat( buf, &fileinfo ) == -1 )
		{
#endif /* USE_PERL */
			cfg_error( cfg, "Unable to find file %s specified in option %s: %s", buf, opt->name, strerror( errno ) );
			return CFG_PARSE_ERROR;
#ifdef USE_PERL
		}
#endif /* USE_PERL */
	}
	if( !S_ISREG( fileinfo.st_mode ) )
	{
		cfg_error( cfg, "File %s specified in option %s is not a regular file", buf, opt->name );
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

/** @brief cb_verify_log
 *
 *  Verify log filename format configuration value
 *  Config subsystem use only
 *
 *  @param cfg pointer to config structure
 *  @param opt pointer to option
 *
 *  @return none
 */

static int cb_verify_log( cfg_t *cfg, cfg_opt_t *opt )
{
	return CFG_SUCCESS;
}

/** @brief cb_verify_mask
 *
 *  Verify nick!user@host mask configuration value
 *
 *  @param cfg pointer to config structure
 *  @param opt pointer to option
 *
 *  @return none
 */

static int cb_verify_mask( cfg_t *cfg, cfg_opt_t *opt )
{
	char *value = opt->values[0]->string;
	if( !strstr( value, "!" ) | !strstr( value, "@" ) )
	{
		cfg_error( cfg, "Invalid hostmask %s for %s", value, opt->name );
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

/** @brief cb_verify_nick
 *
 *  Verify nickconfiguration value
 *
 *  @param cfg pointer to config structure
 *  @param opt pointer to option
 *
 *  @return none
 */

static int cb_verify_nick( cfg_t *cfg, cfg_opt_t *opt )
{
	char *value = opt->values[0]->string;
	if( !ValidateNick(value) ) 
	{
		cfg_error( cfg, "Invalid Nickname %s for %s", value, opt->name );
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

/** @brief cb_noload
 *
 *  Verify NOLOAD configuration value
 *  Config subsystem use only
 *
 *  @param cfg pointer to config structure
 *  @param opt pointer to option
 *
 *  @return none
 */

static int cb_noload( cfg_t *cfg, cfg_opt_t *opt )
{
	if( opt->values[0]->boolean == cfg_true )
	{
		cfg_error( cfg, "Error. You didn't edit %s", CONFIG_NAME );
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

/** @brief cb_verify_host
 *
 *  Verify hostname/ip address configuration value
 *  Config subsystem use only
 *
 *  @param cfg pointer to config structure
 *  @param opt pointer to option
 *
 *  @return none
 */

static int cb_verify_host( cfg_t *cfg, cfg_opt_t *opt )
{
	if( ValidateHost( opt->values[0]->string ) == NS_FAILURE )
	{
		cfg_error( cfg, "Invalid hostname %s for option %s", opt->values[0]->string, opt->name );
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

/** @brief cb_verify_neohost
 *
 *  Verify hostname/ip address configuration value
 *  Config subsystem use only
 *
 *  @param cfg pointer to config structure
 *  @param opt pointer to option
 *
 *  @return none
 */

static int cb_verify_neohost( cfg_t *cfg, cfg_opt_t *opt )
{
	if( ValidateHost( opt->values[0]->string ) == NS_FAILURE )
	{
		cfg_error( cfg, "Invalid hostname %s for option %s", opt->values[0]->string, opt->name );
		return CFG_PARSE_ERROR;
	}
	if ((ircstrcasecmp(opt->values[0]->string, "stats.neostats.net")) == 0 || (ircstrcasecmp(opt->values[0]->string, "stats.somenet.net") == 0 )) {
		cfg_error( cfg, "You must use a hostname other than %s", opt->values[0]->string);
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

/** @brief cb_verify_settime
 *
 *  Verify settime configuration value
 *  Config subsystem use only
 *
 *  @param cfg pointer to config structure
 *  @param opt pointer to option
 *
 *  @return none
 */

static int cb_verify_settime( cfg_t *cfg, cfg_opt_t *opt )
{
	long int time = opt->values[0]->number;

	if( time < 0 )
	{
		cfg_error( cfg, "%d is out of range for %s", time, opt->name );
		return CFG_PARSE_ERROR;
	}
	return CFG_SUCCESS;
}

/** @brief cb_module
 *
 *  Add module to list of modules to load
 *  Config subsystem use only
 *
 *  @param name of module
 *
 *  @return none
 */

static void cb_module( char *name )
{
	int i;

	SET_SEGV_LOCATION();
	if( !nsconfig.modnoload )
	{
		for ( i = 0; ( i < NUM_MODULES ) && ( load_mods[i] != 0 ); i++ )
		{
			if( ircstrcasecmp( load_mods[i], name ) == 0 )
				return;
		}
		if( i < NUM_MODULES )
			load_mods[i] = sstrdup( name );
	}
}
