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
** $Id: update.c 3198 2007-08-19 06:23:08Z Fish $
*/

/*  TODO:
 *  - Nothing at present
 */

#include "neostats.h"
#include "services.h"
#include "namedvars.h"
#include "mrss.h"
#include "update.h"



/** local structures */
static int update_set_updateenabled_cb( const CmdParams *cmdparams, SET_REASON reason );
static int update_set_updateurl_cb( const CmdParams *cmdparams, SET_REASON reason );
static int update_check_umode( const CmdParams *cmdparams );
static int update_checkupdate(const CmdParams *cmdparams);
static int update_report_update( const CmdParams *cmdparams);

/** Configuration structure */
static struct update_cfg 
{ 
	char updateurl[BUFSIZE];
	unsigned int enabled;
} update_cfg;

/** Copyright info */
static const char *update_copyright[] = 
{
	"Copyright (c) 1999-2008, NeoStats",
	"http://www.neostats.net/",
	NULL
};

static const char *update_about[] = 
{
	"Automatically notify operators of New Versions"
	"of NeoStats and its Modules",
	NULL
};

static const char *update_help_updateinfo[] = 
{
	"Shows any available updates for NeoStats and its modules",
	"Syntax: \2UPDATEINFO\2",
	"",
	"Will display any newer versions of modules or the Core NeoStats",
	"that is available from the NeoStats website",
	NULL
};	

static const char *update_help_checkupdate[] =
{
	"Checkers for new versions of NeoStats or the loaded modules",
	"Syntax: \2CHECKUPDATE\2",
	"",
	"Will check for any new versions of NeoStats or the modules that are",
	"currently loaded",
	NULL
};

typedef struct updateinfo {
	char *mod;
	char *title;
	char *ver;
	char *description;
	char *url;
} updateinfo;

hash_t *availableupdates;

nv_struct nv_updateinfo[] =  {
	{"mod", NV_PSTR, offsetof(updateinfo, mod), NV_FLG_RO, -1, -1},
	{"ver", NV_PSTR, offsetof(updateinfo, ver), NV_FLG_RO, -1, -1},
	{"description", NV_PSTR, offsetof(updateinfo, description), NV_FLG_RO, -1, -1},
	{"url", NV_PSTR, offsetof(updateinfo, url), NV_FLG_RO, -1, -1},
	NV_STRUCT_END()
};

/** Module info */
ModuleInfo module_info = 
{
	"Update",
	"Update Notification Module",
	update_copyright,
	update_about,
	NEOSTATS_VERSION,
	CORE_MODULE_VERSION,
	__DATE__,
	__TIME__,
	0,
	0,
	0,
};

/** Bot setting table */
static bot_setting update_settings[] =
{
	{"UPDATEURL",		&update_cfg.updateurl,	SET_TYPE_STRING,	0, BUFSIZE, 	NS_ULEVEL_ADMIN, NULL,	update_help_set_updateurl, update_set_updateurl_cb, ( void *)"http://www.neostats.net/feed.php" },
	{"UPDATEENABLED",	&update_cfg.enabled,	SET_TYPE_BOOLEAN,	0, 0, 	NS_ULEVEL_ADMIN, NULL,	update_help_set_updateenabled, update_set_updateenabled_cb, ( void* )1 },
	NS_SETTING_END()
};

ModuleEvent module_events[] = 
{
	{EVENT_UMODE,		update_check_umode,		EVENT_FLAG_EXCLUDE_ME},
	NS_EVENT_END()
};

static bot_cmd update_cmds[] = {
	{"UPDATEINFO", 	update_report_update, 	0, 	NS_ULEVEL_LOCOPER,	update_help_updateinfo,		0,	NULL,	NULL},
	{"CHECKUPDATE",	update_checkupdate,	0,	NS_ULEVEL_LOCOPER,	update_help_checkupdate,	0,	NULL,	NULL},
	NS_CMD_END()
};


unsigned int string_to_ver(const char *str)
{
    static char lookup[] = "abcdef0123456789";
    unsigned int maj = 0, min = 0, rev = 0, ver;
    unsigned char *cptr, *idx;
    int bits;

    // do the major number
    cptr = (unsigned char *)str;
    for (; *cptr; cptr++)
    {
	if (*cptr == '.' || *cptr == '_')
	{
	    cptr++;
	    break;
	}
	idx = (unsigned char *)strchr(lookup, tolower(*cptr));
	if (!idx)
	    continue;
	
	maj = (maj << 4) | ((char *)idx - lookup);
    }
    
    // do the minor number
    for (bits = 2; *cptr && *cptr != '.' && *cptr != '_' && bits > 0; cptr++)
    {
	idx = (unsigned char *)strchr(lookup, tolower(*cptr));
	if (!idx)
	    continue;
	
	min = (min << 4) | ((char *)idx - lookup);
	bits--;
    }
    
    // do the revision number
    for (bits = 4; *cptr && bits > 0; cptr++)
    {
	idx = (unsigned char *)strchr(lookup, tolower(*cptr));
	if (!idx)
	    continue;

	rev = (rev << 4) | ((char *)idx - lookup);
	bits--;
    }

    ver = (maj << 24) | (min << 16) | (rev << (4*bits));

    return ver;
}
static int BuildMods( Module *mod_ptr, void *v) {
	hash_t *loadedmods = (hash_t *)v;
	hnode_create_insert(loadedmods, mod_ptr, mod_ptr->info->name);
	return NS_FALSE;
}

static void StoreReport(mrss_item_t *item, const char *mod, const char *version) {

	updateinfo *newitem;

	/* regardless if this is already known or not, announce it to the world every time we check */
	irc_chanalert(ns_botptr, "A new version of %s is available! /msg %s UPDATEINFO for more information", item->title, ns_botptr->u->name);	

	if ((newitem = hnode_find(availableupdates, mod)) != NULL) {
		if (string_to_ver(version) <= string_to_ver(newitem->ver))
			return;
		/* else free the descriptions, versions, url structs */
		ns_free(newitem->ver);
		ns_free(newitem->description);
		ns_free(newitem->url);
	} else {
		newitem = ns_malloc(sizeof(updateinfo));
		newitem->mod = strdup(mod);
		newitem->title = strdup(item->title);
		hnode_create_insert(availableupdates, newitem, newitem->mod);
	}
	/* if we get here, we have a newitem ready for new vals */
	newitem->ver = strdup(version);
	newitem->description = strdup(item->description);
	newitem->url = strdup(item->link);
	/* ok, its stored.... now we just wait for new clients to signon in our umode and smode handlers */
	return;

}

static void CheckModVersions(mrss_item_t *item, char *modname, Module *mod) {
	mrss_tag_t *othertags;
	char **av;
	int ac = 0;
	othertags = item->other_tags;
	while (othertags) {
		if (!ircstrcasecmp(othertags->name, "Version")) {
			if (mod) {
				/* av[0] - Main Version av[2] - SVN revison if ac > 1 */
				ac = split_buf((char *)mod->info->version, &av);
			} else {
				ac = split_buf(me.version, &av);
			}
			if (string_to_ver(othertags->value) > string_to_ver(av[0])) {
				StoreReport(item, modname, othertags->value);
			}				
			ns_free(av);
		}			
		othertags = othertags->next;
	}
}


static void UpdateRSSHandler(void *userptr, int status, char *data, int datasize)
{
	mrss_error_t ret;
	mrss_t *mrss;
	mrss_item_t *item;
	mrss_tag_t *othertags;
	hash_t *loadedmods;
	Module *mod;
	SET_SEGV_LOCATION();


	if (status != NS_SUCCESS) {
		nlog(LOG_WARNING, "RSS Update Feed download failed: %s", data);
		return;
	}
	mrss = NULL;
	mrss_new(&mrss);
	ret = mrss_parse_buffer(data, datasize, &mrss);
	if (ret != MRSS_OK) {
		nlog(LOG_WARNING, "RSS Update Feed Parse failed: %s", mrss_strerror(ret));
		mrss_free(mrss);
		return;
	}

	/* build a hash of modules 
	 * we do this every check rather than at Init/Sych time
	 * as we might have loaded/unloaded modules 
	 */
	loadedmods = hash_create(HASHCOUNT_T_MAX, 0, 0);
	ProcessModuleList(BuildMods, loadedmods);

	item = mrss->item;
	while (item)
	{
		othertags = item->other_tags;
		while (othertags) {
			if (!ircstrcasecmp(othertags->name, "Module")) {
				mod = (Module *)hnode_find(loadedmods, othertags->value);
				if (mod) {
					CheckModVersions(item, othertags->value, mod);
				} else if (!ircstrcasecmp(othertags->value, "NeoStats")) {
					CheckModVersions(item, "NeoStats", NULL);
				}
			}			
			othertags = othertags->next;
		}
		item = item->next;
	}
	mrss_free(mrss);
	hash_free_nodes(loadedmods);
	hash_destroy(loadedmods);
}

static int update_check_updates(void *unused)
{
	if (new_transfer(update_cfg.updateurl, NULL, NS_MEMORY, "", NULL, UpdateRSSHandler) != NS_SUCCESS) {
		nlog(LOG_WARNING, "Download Update RSS Feed Failed");
	}
	return NS_SUCCESS;
}

/** @brief ModInit
 *
 *  Init handler
 *  Loads connectserv configuration
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModInit( void )
{
	SET_SEGV_LOCATION();
	/* Load stored configuration */
	ModuleConfig( update_settings );
	return NS_SUCCESS;
}

/** @brief ModSynch
 *
 *  Startup handler
 *  Introduce bot onto network
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModSynch( void )
{
	SET_SEGV_LOCATION();

	if (add_services_set_list(update_settings) != NS_SUCCESS) {
		return NS_FAILURE;
	} 
	if (add_services_cmd_list(update_cmds) != NS_SUCCESS) {
		return NS_FAILURE;
	}
	if (update_cfg.enabled == 1) {
		/* updates are enabled. Setup Timer and load first update */
		AddTimer(TIMER_TYPE_INTERVAL, update_check_updates, "CheckUpdates", 86400, NULL);
		update_check_updates(NULL);
	}
	availableupdates = nv_hash_create(HASHCOUNT_T_MAX,0, 0,  "Update", nv_updateinfo, NV_FLAGS_RO, NULL);
	
	return NS_SUCCESS;
}

/** @brief ModFini
 *
 *  Fini handler
 *
 *  @param none
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

int ModFini( void )
{
	hscan_t scan;
	hnode_t *node;
	updateinfo *item;
	
	SET_SEGV_LOCATION();
	del_services_set_list(update_settings);
	del_services_cmd_list(update_cmds);


	hash_scan_begin(&scan, availableupdates);
	while ( ( node = hash_scan_next (&scan)) != NULL) {
		item = hnode_get(node);
		ns_free(item->mod);
		ns_free(item->title);
		ns_free(item->ver);
		ns_free(item->description);
		ns_free(item->url);
		ns_free(item);
		hash_scan_delete_destroy_node(availableupdates, node);
	}
	nv_hash_destroy(availableupdates, "Update");

	return NS_SUCCESS;
}

static int update_report_update( const CmdParams *cmdparams) {
	hscan_t scan;
	hnode_t *node;
	updateinfo *item;
	char *descline;
	char *freeme;
	char *line;
	int updates;
	
	updates = hash_count(availableupdates);
	if (updates <=  0)
		return NS_SUCCESS;
	if (cmdparams->eventid != EVENT_UMODE) 
		irc_prefmsg(ns_botptr, cmdparams->source, "There are %d updates available\n", updates);
	hash_scan_begin(&scan, availableupdates);
	while ( ( node = hash_scan_next (&scan)) != NULL) {
		item = hnode_get(node);
		irc_prefmsg(ns_botptr, cmdparams->source, "A new update for %s is available", item->title);
		irc_prefmsg(ns_botptr, cmdparams->source, "The new version is %s and can be downloaded from %s", item->ver, item->url);
		irc_prefmsg(ns_botptr, cmdparams->source, "Short ChangeLog:");
		/* we have to deal with newlines in the descrition here */
		descline = strdup(item->description);
		freeme = descline;
		while ((line = strsep(&descline, "\n"))) 
			irc_prefmsg(ns_botptr, cmdparams->source, "%s", line);
		ns_free(freeme);
		
	}
	return NS_SUCCESS;
}


static int update_checkupdate(const CmdParams *cmdparams) {

	if (new_transfer(update_cfg.updateurl, NULL, NS_MEMORY, "", NULL, UpdateRSSHandler) != NS_SUCCESS) {
		nlog(LOG_WARNING, "Download Update RSS Feed Failed");
		irc_prefmsg(ns_botptr, cmdparams->source, "Update Checking Failed");
		return NS_SUCCESS;
	}
	irc_prefmsg(ns_botptr, cmdparams->source, "Checking for new updates. If there is a update available, it will be announced in the services channel");
	return NS_SUCCESS;

}

/** @brief update_check_uoper
 *
 *  umode event handler
 *  report umode changes
 *
 *  @params cmdparams pointer to commands param struct
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int update_check_umode( const CmdParams *cmdparams )
{
	/* Mask of modes we will handle */
	static const unsigned int OperUmodes = 
		UMODE_NETADMIN |
		UMODE_TECHADMIN |
		UMODE_ADMIN |
		UMODE_COADMIN |
		UMODE_SADMIN |
		UMODE_OPER |
		UMODE_LOCOP |
		UMODE_SERVICES;
	unsigned int mask;
	int add = 1;
	const char *modes;

	SET_SEGV_LOCATION();

	/* if no updates, then ignore this */
	if (hash_count(availableupdates) == 0)
		return NS_SUCCESS;
		
	modes = cmdparams->param;
	while( *modes != '\0' )
	{
		switch( *modes ) 
		{
			case '+':
				add = 1;
				break;
			case '-':
				add = 0;
				break;
			default:
				mask = UmodeCharToMask( *modes );
				if( (OperUmodes & mask) && (add == 1))
					update_report_update(cmdparams);
				break;
		}
		modes++;
	}
	return NS_SUCCESS;
}

/** @brief update_set_updateurl_cb
 *
 *  Set callback for updateurl
 *  Check the supplied URL is valid
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int update_set_updateurl_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_VALIDATE )
	{
	}
	return NS_SUCCESS;
}

/** @brief update_set_updateenabled_cb
 *
 *  Set callback for updateenabled
 *  Turn on/off the timer
 *
 *  @params cmdparams pointer to commands param struct
 *  @params reason for SET
 *
 *  @return NS_SUCCESS if suceeds else NS_FAILURE
 */

static int update_set_updateenabled_cb( const CmdParams *cmdparams, SET_REASON reason )
{
	if( reason == SET_VALIDATE )
	{
	}
	return NS_SUCCESS;
}
