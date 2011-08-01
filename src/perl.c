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
** $Id: perl.c 3312 2008-03-03 12:42:02Z Fish $
*/

/* @file NeoStats interface to Perl Interpreter
 *  based on perl plugin from Xchat client
 */

#include "neostats.h"
#include "services.h"
#include "modules.h"
#include "nsevents.h"
#include "commands.h"
//#include <sys/types.h>
//#include <dirent.h>
#undef _
#define PERLDEFINES
#include "perlmod.h"


extern void boot_DynaLoader (pTHX_ CV * cv);
void dump_hash(HV *rethash);
/* this is defined in NV.xs to init our namedvar support for perl */
void Init_Perl_NV();

XSINIT_t extn_init;


void
dump_hash(HV *rethash) {
	char *key;
	SV *value;
	int keycount;
	I32 keylen;

	keycount = hv_iterinit(rethash);
	printf("%d items in call\n",keycount);
	while(keycount-- != 0) {
		value = hv_iternextsv(rethash, &key, &keylen);
		printf("Return (%s) -> (%s)\n",key,SvPV_nolen(value));
	}
}



/*
  this is used for autoload and shutdown callbacks
*/
int
execute_perl (const Module *mod, SV * function, int numargs, ...)
{

	int count, ret_value = 1;
	SV *sv;
	va_list args;
	char *tmpstr[10];

	if (!mod->pm) {
		nlog(LOG_WARNING, "Tried to execute a perl call when perl isn't loaded!");
		return NS_FAILURE;
	}
	{
	dSP;
	ENTER;
	SAVETMPS;
	PUSHMARK (SP);

#if 0
	SET_RUN_LEVEL(mod);
	dlog(DEBUG1, "Current Runlevel: %s", GET_CUR_MODNAME());
#endif
	dlog(DEBUG1, "Current Runlevel in call: %s: %s", GET_CUR_MODNAME(), mod->info->description);
		
	PERL_SET_CONTEXT((PMI *)mod->pm->my_perl);



	va_start(args, numargs);
	for (count = 0; count < numargs; count++) {
		tmpstr[count] = va_arg(args, char *);
		XPUSHs (sv_2mortal (newSVpv (tmpstr[count], 0)));
	}
	va_end(args);

	PUTBACK;

	count = call_sv (function, G_EVAL | G_SCALAR);
	SPAGAIN;

	sv = GvSV (gv_fetchpv ("@", TRUE, SVt_PV));
	if (SvTRUE (sv)) {
		nlog(LOG_WARNING, "Perl error: %s", SvPVX(sv)); 
		POPs;							  /* remove undef from the top of the stack */
	} else if (count != 1) {
		nlog(LOG_WARNING, "Perl error: expected 1 value from %s, "
						  "got: %d", (char *)function, count);
	} else {
		ret_value = POPi;
	}

	PUTBACK;
	FREETMPS;
	LEAVE;
	}
#if 0
	RESET_RUN_LEVEL();
#endif
	return ret_value;
}

int perl_sync_module(Module *mod)
{
	execute_perl (mod, sv_2mortal (newSVpv ("NeoStats::Embed::sync", 0)),1, mod->pm->filename);
	return NS_SUCCESS;
}

int
perl_event_cb(const Event evt, const CmdParams *cmdparams, const Module *mod_ptr) {
	int ret = NS_FAILURE;
	switch (evt) {
		case EVENT_NULL:
			nlog(LOG_WARNING, "Ehhh, PerlModule got callback for EVENT_NULL?");
			break;
		case EVENT_MODULELOAD:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 1, cmdparams->param);
			break;
		case EVENT_MODULEUNLOAD:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 1, cmdparams->param);
			break;
		case EVENT_SERVER:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_SQUIT:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_PING:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_PONG:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_SIGNON:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_QUIT:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_NICKIP:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_KILL:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->target->name, cmdparams->param);
			break;
		case EVENT_GLOBALKILL:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->target->name, cmdparams->param);
			break;
		case EVENT_LOCALKILL:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->target->name, cmdparams->param);
			break;
		case EVENT_SERVERKILL:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->target->name, cmdparams->param);
			break;
		case EVENT_BOTKILL:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->target->name, cmdparams->param);
			break;
		case EVENT_NICK:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_AWAY:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_UMODE:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_SMODE:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_NEWCHAN:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 1, cmdparams->channel->name);
			break;
		case EVENT_DELCHAN:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 1, cmdparams->channel->name);
			break;
		case EVENT_JOIN:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->channel->name, cmdparams->source->name);
			break;
		case EVENT_PART:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, (cmdparams->param == NULL ? 2 : 3), cmdparams->channel->name, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_PARTBOT:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, (cmdparams->param == NULL ? 2 : 3), cmdparams->channel->name, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_EMPTYCHAN:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->channel->name, cmdparams->bot->name);
			break;
		case EVENT_KICK:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 4, cmdparams->channel->name, cmdparams->source->name, cmdparams->target->name, cmdparams->param);
			break;
		case EVENT_KICKBOT:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 4, cmdparams->channel->name, cmdparams->source->name, cmdparams->target->name, cmdparams->param);
			break;
		case EVENT_TOPIC:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->channel->name, cmdparams->source->name);
			break;
		case EVENT_CMODE:
			dlog(DEBUG1, "EVENT_CMODE todo!");
			break;
		case EVENT_PRIVATE:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->bot->name, cmdparams->param);
			break;
		case EVENT_NOTICE:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->bot->name, cmdparams->param);
			break;
		case EVENT_CPRIVATE:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->channel->name, cmdparams->param);
			break;
		case EVENT_CNOTICE:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 3, cmdparams->source->name, cmdparams->channel->name, cmdparams->param);
			break;
		case EVENT_GLOBOPS:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_CHATOPS:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_WALLOPS:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_CTCPVERSIONRPL:
		case EVENT_CTCPVERSIONRPLBC:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_CTCPVERSIONREQ:
		case EVENT_CTCPVERSIONREQBC:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_CTCPFINGERRPL:
		case EVENT_CTCPFINGERRPLBC:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_CTCPFINGERREQ:
		case EVENT_CTCPFINGERREQBC:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_CTCPACTIONREQ:
		case EVENT_CTCPACTIONREQBC:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_CTCPTIMERPL:
		case EVENT_CTCPTIMERPLBC:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_CTCPTIMEREQ:
		case EVENT_CTCPTIMEREQBC:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_CTCPPINGRPL:
		case EVENT_CTCPPINGRPLBC:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_CTCPPINGREQ:
		case EVENT_CTCPPINGREQBC:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 1, cmdparams->source->name);
			break;
		case EVENT_DCCSEND:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_DCCCHAT:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_DCCCHATMSG:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_ADDBAN:
		case EVENT_DELBAN:
			dlog(DEBUG1, "EVENT_*BAN Todo!");
			break;
		case EVENT_CTCPUNHANDLEDRPL:
		case EVENT_CTCPUNHANDLEDRPLBC:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;
		case EVENT_CTCPUNHANDLEDREQ:
		case EVENT_CTCPUNHANDLEDREQBC:
			ret = execute_perl(mod_ptr, mod_ptr->pm->event_list[evt]->pe->callback, 2, cmdparams->source->name, cmdparams->param);
			break;

		case EVENT_COUNT:
			/* nothing */
			break;
			
	}
	return ret;
}

int
perl_command_cb(const CmdParams *cmdparams) {
	return execute_perl(GET_CUR_MODULE(), sv_2mortal (newSVpv (cmdparams->cmd_ptr->moddata, 0)), 3, cmdparams->cmd_ptr->cmd, cmdparams->source->name, cmdparams->param);
}

int
perl_timer_cb( void *userptr) {
	Module *mod;
	mod = GET_CUR_MODULE();

	return execute_perl(mod, sv_2mortal(newSVpv((char *)userptr, 0)), 0);
}

/* encode a Client Structure into a perl Hash */
HV *perl_encode_client(Client *u) {
	HV *client;
	
	client = newHV();
	
	hv_store(client, "nick", 4, 
		newSVpv(u->name, strlen(u->name)), 0);
	hv_store(client, "username", 8, 
		newSVpv(u->user->username, strlen(u->user->username)), 0);
	hv_store(client, "hostname", 8, 
		newSVpv(u->user->hostname, strlen(u->user->hostname)), 0);
	hv_store(client, "vhost", 5, 
		newSVpv(u->user->hostname, strlen(u->user->hostname)), 0);
	hv_store(client, "awaymsg", 7, 
		newSVpv(u->user->awaymsg, strlen(u->user->awaymsg)), 0);
	hv_store(client, "swhois", 6, 
		newSVpv(u->user->swhois, strlen(u->user->swhois)), 0);
	hv_store(client, "userhostmask", 12, 
		newSVpv(u->user->userhostmask, strlen(u->user->userhostmask)), 0);
	hv_store(client, "uservhostmask", 13, 
		newSVpv(u->user->uservhostmask, strlen(u->user->uservhostmask)), 0);
	hv_store(client, "server", 6, 
		newSVpv(u->uplink->name, strlen(u->uplink->name)), 0);
	hv_store(client, "is_away", 7, 
		newSViv(u->user->is_away), 0);
	hv_store(client, "umodes", 6, 
		newSVpv(u->user->modes, strlen(u->user->modes)), 0);
	hv_store(client, "Umode", 5, 
		newSViv(u->user->Umode), 0);
	hv_store(client, "smodes", 6, 
		newSVpv(u->user->smodes, strlen(u->user->smodes)), 0);
	hv_store(client, "Smode", 5, 
		newSViv(u->user->Smode), 0);
	hv_store(client, "Ulevel", 6, 
		newSViv(u->user->ulevel), 0);
	return (client);
}

/* encode a Server Structure into a perl Hash */
HV *perl_encode_server(Client *u) {
	HV *client;
	
	client = newHV();
	
	hv_store(client, "name", 4, 
		newSVpv(u->name, strlen(u->name)), 0);
	hv_store(client, "uplink", 6, 
		newSVpv(u->uplinkname, strlen(u->uplinkname)), 0);
	hv_store(client, "users", 5, 
		newSViv(u->server->users), 0);
	hv_store(client, "awaycount", 9, 
		newSViv(u->server->awaycount), 0);
	hv_store(client, "hops", 4, 
		newSViv(u->server->hops), 0);
	hv_store(client, "ping", 4, 
		newSViv(u->server->ping), 0);
	hv_store(client, "uptime", 6, 
		newSViv(u->server->uptime), 0);
	return (client);
}

/* encode a Client Structure into a perl Hash */
HV *perl_encode_channel(Channel *c) {
	HV *client;
	
	client = newHV();
	
	hv_store(client, "name", 4, 
		newSVpv(c->name, strlen(c->name)), 0);
	hv_store(client, "topic", 5, 
		newSVpv(c->topic, strlen(c->topic)), 0);
	hv_store(client, "topicowner", 10, 
		newSVpv(c->topicowner, strlen(c->topicowner)), 0);
	hv_store(client, "topictime", 9, 
		newSViv(c->topictime), 0);
	hv_store(client, "users", 5, 
		newSViv(c->users), 0);
	hv_store(client, "modes", 5, 
		newSViv(c->modes), 0);
	hv_store(client, "limit", 5, 
		newSViv(c->limit), 0);
	hv_store(client, "key", 3, 
		newSVpv(c->key, strlen(c->key)), 0);
	hv_store(client, "createtime", 10, 
		newSViv(c->creationtime), 0);
	/* XXX todo: Encode Mode Params */
	return (client);
}



/* NeoStats::Internal::register (scriptname, version, desc)
 *
 */

static
XS (XS_NeoStats_register)
{
	Module *mod;
	dXSARGS;


	if (items != 3) {
		nlog(LOG_WARNING, "Usage: NeoStats::Internal::register(scriptname, version, desc)");
	} else {
		mod = GET_CUR_MODULE();
		if (!mod) {
			nlog(LOG_WARNING, "Current Mod Stack for Perl Mods is screwed");
			XSRETURN_EMPTY;
		}
		/* because this is currently a temp holding "NeoStats" */
		ns_free(mod->info->name);
		
		mod->info->name = strndup(SvPV_nolen (ST (0)), sv_len(ST (0)));
		mod->info->version = strndup(SvPV_nolen (ST (1)), sv_len(ST (1)));
		mod->info->description = strndup(SvPV_nolen (ST (2)), sv_len(ST(2)));
		mod->pm->registered = 1;
		mod->pm->type = TYPE_MODULE;
		XSRETURN_UV (PTR2UV (mod));

	}
}

static
XS (XS_NeoStats_registerextension)
{
	Module *mod;
	dXSARGS;
	if (items != 2) {
		nlog(LOG_WARNING, "Usage: NeoStats::Internal::registerextension(scriptname, version)");
	} else {
		mod = GET_CUR_MODULE();
		if (!mod) {
			nlog(LOG_WARNING, "Current Mod Stack for Perl Mods is screwed");
			XSRETURN_EMPTY;
		}
		mod->pm->extname = strndup(SvPV_nolen(ST(0)), sv_len(ST (0)));
		mod->pm->extversion = strndup(SvPV_nolen (ST (1)), sv_len(ST (1)));
		mod->pm->registered = 1;
		mod->pm->type = TYPE_EXTENSION;
		XSRETURN_UV (PTR2UV (mod));
	}
}

/* NeoStats::debug(output) */
static
XS (XS_NeoStats_debug)
{

	char *text = NULL;

	dXSARGS;
	if (items != 1) {
		nlog(LOG_WARNING, "Usage: NeoStats::Internal::debug(text)");
	} else {
		text = SvPV_nolen (ST (0));
#if 0
		strip(text);
#endif
		nlog(LOG_WARNING, "%s", text);
	}
	XSRETURN_EMPTY;
}

/* NeoStats::Internal::hook_event(event, flags, callback, userdata) */
static
XS (XS_NeoStats_hook_event)
{

	ModuleEvent *evt;

	dXSARGS;
	if (items != 4) {
		nlog(LOG_WARNING, "Usage: NeoStats::Internal::hook_event(event, flags, callback, userdata)");
	} else {
		evt = ns_calloc(sizeof(ModuleEvent));
		evt->pe = ns_calloc(sizeof(PerlEvent));
		evt->event = (int) SvIV (ST (0));
		evt->flags = (int) SvIV (ST (1));
		/* its a perl callback */
		evt->flags |= EVENT_FLAG_PERLCALL; 
		/* null, because its a perl event, which will execute via dedicated perl event handler */
		evt->handler = NULL; 
		evt->pe->callback = sv_mortalcopy(ST (2));
		SvREFCNT_inc (evt->pe->callback);
		evt->pe->userdata = sv_mortalcopy(ST (3));
		SvREFCNT_inc (evt->pe->userdata);
		/* add it as a event */
		AddEvent(evt);

		XSRETURN_UV (PTR2UV (evt));
	}
}

static
XS (XS_NeoStats_unhook_event)
{
	Module *mod;
	Event evt = -1;
	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items != 1) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:unhook_event(hook)");
	} else {
		evt = (int) SvIV (ST(0));
		if (mod->pm->event_list && mod->pm->event_list[evt]->pe) {
			SvREFCNT_dec(mod->pm->event_list[evt]->pe->callback);
			SvREFCNT_dec(mod->pm->event_list[evt]->pe->userdata);
			ns_free(mod->pm->event_list[evt]->pe);
			ns_free(mod->pm->event_list[evt]);
			mod->pm->event_list[evt] = NULL;
		}
	}
	XSRETURN_EMPTY;
}


static
XS (XS_NeoStats_AddBot)
{
	Module *mod;
	HV * rethash;
	SV *ret;
	int flags;
	SV *value;
	BotInfo *bi;
	Bot *bot;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 2) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:AddBot(botinfo, flags, data)");
	} else {
		ret = ST(0);
		flags = (int) SvIV (ST (1));
		if(SvTYPE(SvRV(ret))!=SVt_PVHV) {
			 dlog(DEBUG1, "XS_NeoStats_AddBot: unsuported input %lu, must %i", SvTYPE(SvRV(ret)), SVt_PVHV);
			 XSRETURN_EMPTY;
		}
		rethash = (HV*)SvRV(ret);
#ifdef DEBUG
		dump_hash(rethash);
#endif
		bi = ns_malloc(sizeof(BotInfo));
		value = *hv_fetch(rethash, "nick", strlen("nick"), FALSE);
		strncpy(bi->nick, SvPV_nolen(value), MAXNICK);
		value = *hv_fetch(rethash, "altnick", strlen("altnick"), FALSE);
		strncpy(bi->altnick, SvPV_nolen(value), MAXNICK);
		value = *hv_fetch(rethash, "ident", strlen("ident"), FALSE);
		strncpy(bi->user, SvPV_nolen(value), MAXUSER);
		value = *hv_fetch(rethash, "host", strlen("host"), FALSE);
		strncpy(bi->host, SvPV_nolen(value), MAXHOST);
		value = *hv_fetch(rethash, "gecos", strlen("gecos"), FALSE);
		strncpy(bi->realname, SvPV_nolen(value), MAXREALNAME);
		bi->flags = (int) SvIV (ST (1));
		bi->bot_cmd_list = NULL;
		bi->bot_setting_list = NULL;
		if ((bot = AddBot(bi)) == NULL) {
			free(bi);
			XSRETURN_EMPTY;
		}
		bot->botinfo = bi;
		XSRETURN_PV (bot->name);
	}
	XSRETURN_EMPTY;
}




static
XS (XS_NeoStats_DelBot)
{
	Module *mod;
	dXSARGS;
	Bot *bot;


	if (items != 2) {
		nlog(LOG_WARNING, "Usage: NeoStats::Internal::DelBot(botname, quitreason)");
	} else {
		mod = GET_CUR_MODULE();
		if (!mod) {
			nlog(LOG_WARNING, "Current Mod Stack for Perl Mods is screwed");
			XSRETURN_EMPTY;
		}
		bot = FindBot(SvPV_nolen(ST(0)));
		ns_free(bot->botinfo);
		irc_quit(bot, SvPV_nolen(ST(1)));
		XSRETURN_UV( NS_SUCCESS);
	}
}


static
XS (XS_NeoStats_FindUser)
{
	Module *mod;
	Client *u;
	SV *client;

	dXSARGS;

	if (items != 1) {
		nlog(LOG_WARNING, "Usage: NeoStats::Internal::FindUser(nick)");
	} else {
		SP -= items; /* remove args from the stack */
		mod = GET_CUR_MODULE();
		if (!mod) {
			nlog(LOG_WARNING, "Current Mod Stack for Perl Mods is screwed");
			XSRETURN_EMPTY;
		}
		u = FindUser(SvPV_nolen(ST(0)));
		if (!u) {
			XSRETURN_EMPTY;
		}
		/* create a hash with the users details filled in */
		client = (SV *)perl_encode_client(u);
		sv_2mortal(client);
		XPUSHs(newRV_noinc((SV *)client));
		PUTBACK;
		XSRETURN(1);
	}
}

static
XS (XS_NeoStats_FindServer)
{
	Module *mod;
	Client *u;
	SV *client;

	dXSARGS;

	if (items != 1) {
		nlog(LOG_WARNING, "Usage: NeoStats::Internal::FindServer(name)");
	} else {
		SP -= items; /* remove args from the stack */
		mod = GET_CUR_MODULE();
		if (!mod) {
			nlog(LOG_WARNING, "Current Mod Stack for Perl Mods is screwed");
			XSRETURN_EMPTY;
		}
		u = FindServer(SvPV_nolen(ST(0)));
		if (!u) {
			XSRETURN_EMPTY;
		}
		/* create a hash with the users details filled in */
		client = (SV *)perl_encode_server(u);
		sv_2mortal(client);
		XPUSHs(newRV_noinc((SV *)client));
		PUTBACK;
		XSRETURN(1);
	}
}


static
XS (XS_NeoStats_FindChannel)
{
	Module *mod;
	Channel *c;
	SV *client;

	dXSARGS;

	if (items != 1) {
		nlog(LOG_WARNING, "Usage: NeoStats::Internal::FindChannel(name)");
	} else {
		SP -= items; /* remove args from the stack */
		mod = GET_CUR_MODULE();
		if (!mod) {
			nlog(LOG_WARNING, "Current Mod Stack for Perl Mods is screwed");
			XSRETURN_EMPTY;
		}
		c = FindChannel(SvPV_nolen(ST(0)));
		if (!c) {
			XSRETURN_EMPTY;
		}
		/* create a hash with the users details filled in */
		client = (SV *)perl_encode_channel(c);
		sv_2mortal(client);
		XPUSHs(newRV_noinc((SV *)client));
		PUTBACK;
		XSRETURN(1);
	}
}

static
XS (XS_NeoStats_AddCommand)
{
	Module *mod;
	HV * rethash;
	SV *ret;
	SV *value;
	AV *helptext;
	bot_cmd *bc;
	Bot *bot;
	int i, j;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:AddCommand(bot, botcmd, callback)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot) {
			if (bot->botcmds == NULL) {
				bot->botcmds = hash_create(HASHCOUNT_T_MAX, 0, 0);
			}
			ret = ST(1);
			if(SvTYPE(SvRV(ret))!=SVt_PVHV) {
				 dlog(DEBUG1, "XS_NeoStats_AddCommand: unsuported input %lu, must %i", SvTYPE(SvRV(ret)), SVt_PVHV);
				 XSRETURN_EMPTY;
			}
			rethash = (HV*)SvRV(ret);
#ifdef DEBUG
			dump_hash(rethash);
#endif
			bc = ns_malloc(sizeof(bot_cmd));
			value = *hv_fetch(rethash, "cmd", strlen("cmd"), FALSE);
			bc->cmd = ns_malloc(SvLEN(value)+1);
			strlcpy((char *)bc->cmd, SvPV_nolen(value), SvLEN(value)+1);
			value = *hv_fetch(rethash, "minparams", strlen("minparams"), FALSE);
			bc->minparams = SvIV(value);
			value = *hv_fetch(rethash, "ulevel", strlen("ulevel"), FALSE);
			bc->ulevel = SvIV(value);

			value = *hv_fetch(rethash, "helptext", strlen("helptext"), FALSE);
			/* make sure its a array */
			if (SvTYPE(SvRV(value)) != SVt_PVAV) {
				dlog(DEBUG1, "XS_NeoStats_AddCommand: Helptext field is not a array");
				ns_free(bc->cmd);
				ns_free(bc);
				XSRETURN_EMPTY;
			}
			/* ok, lets setup the array */
			helptext = (AV*)SvRV(value);
			j = 0;
			for (i =0; i <= av_len(helptext); i++) {
				/* ok, try to follow me here:
				 * we extract each member of the array (av_fetch)
				 * and convert it to a string (SvPV_nolen)
				 * then make a copy (malloc'ed) of it (strdup)
				 * and put it on the end of the array of helptext strings (AddStringToList)!
				 */
				 AddStringToList((char ***)&bc->helptext, strdup(SvPV_nolen(*av_fetch(helptext, i, FALSE))), &j);
			}
			value = *hv_fetch(rethash, "flags", strlen("flags"), FALSE);
			bc->flags = SvIV(value);

			bc->moddata = malloc(SvLEN(ST(2))+1);
			strlcpy(bc->moddata, SvPV_nolen(ST(2)), SvLEN(ST(2))+1);
			bc->modptr = mod;
			bc->handler = perl_command_cb;
	
			XSRETURN_UV(add_bot_cmd(bot->botcmds, bc));
		} else {
			nlog(LOG_WARNING, "XS_NeoStats_AddBot: Bot %s is not valid", SvPV_nolen(ST(0)));
		}
			
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_DelCommand)
{
	Module *mod;
	Bot *bot;
	bot_cmd *cmd_ptr = NULL;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 2) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:DelCommand(bot, botcmd)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot)
			cmd_ptr = find_bot_cmd(bot, SvPV_nolen(ST(1)));
		if (cmd_ptr) 
		{
			del_bot_cmd(bot->botcmds, cmd_ptr);
			XSRETURN_UV( NS_SUCCESS);
		}
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_Prefmsg)
{
	Module *mod;
	Bot *bot;
	Client *u;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:Prefmsg(from, to, message)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		u = FindUser(SvPV_nolen(ST(1)));
		if (bot && u) {
			XSRETURN_UV(irc_prefmsg(bot, u, "%s", SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_ChanAlert)
{
	Module *mod;
	Bot *bot;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 2) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:ChanAlert(from, message)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot) {
			XSRETURN_UV(irc_chanalert(bot, "%s", SvPV_nolen(ST(1))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_PrivMsg)
{
	Module *mod;
	Bot *bot;
	Client *u;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:PrivMsg(from, to, message)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		u = FindUser(SvPV_nolen(ST(1)));
		if (bot && u) {
			XSRETURN_UV(irc_privmsg(bot, u, "%s", SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_Notice)
{
	Module *mod;
	Bot *bot;
	Client *u;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:Notice(from, to, message)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		u = FindUser(SvPV_nolen(ST(1)));
		if (bot && u) {
			XSRETURN_UV(irc_notice(bot, u, "%s", SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_ChanPrivMsg)
{
	Module *mod;
	Bot *bot;


	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:ChanPrivMsg(from, to, message)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot && FindChannel(SvPV_nolen(ST(1)))) {
			XSRETURN_UV(irc_chanprivmsg(bot, SvPV_nolen(ST(1)), "%s", SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_ChanNotice)
{
	Module *mod;
	Bot *bot;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:ChanNotice(from, to, message)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot && FindChannel(SvPV_nolen(ST(1)))) {
			XSRETURN_UV(irc_channotice(bot, SvPV_nolen(ST(1)), "%s", SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_Globops)
{
	Module *mod;
	Bot *bot;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 2) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:Globops(from, message)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot) {
			XSRETURN_UV(irc_globops(bot, "%s", SvPV_nolen(ST(1))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_Wallops)
{
	Module *mod;
	Bot *bot;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 2) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:Wallops(from, message)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot) {
			XSRETURN_UV(irc_wallops(bot, "%s", SvPV_nolen(ST(1))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_Numeric)
{
	Module *mod;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:Numeric(numeric, to, message)");
	} else {
		XSRETURN_UV(irc_numeric(SvIV(ST(0)), SvPV_nolen(ST(1)), "%s", SvPV_nolen(ST(2))));
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_Umode)
{
	Module *mod;
	Bot *bot;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:Umode(bot, target, umode)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot) {
			XSRETURN_UV(irc_umode(bot, SvPV_nolen(ST(1)), SvIV(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_Join)
{
	Module *mod;
	Bot *bot;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:Join(bot, channel, cmode)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot) {
			XSRETURN_UV(irc_join(bot, SvPV_nolen(ST(1)), SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_Part)
{
	Module *mod;
	Bot *bot;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 2) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:Part(bot, channel, <message>)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot) {
			XSRETURN_UV(irc_part(bot, SvPV_nolen(ST(1)), items == 3 ? SvPV_nolen(ST(2)) : ""));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_NickChange)
{
	Module *mod;
	Bot *bot;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 2) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:NickChange(bot, newnick)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot) {
			XSRETURN_UV(irc_nickchange(bot, SvPV_nolen(ST(1))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_CMode)
{
	Module *mod;
	Bot *bot;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:CMode(bot, chan, mode, <args>)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot && FindChannel(SvPV_nolen(ST(1)))) {
			XSRETURN_UV(irc_cmode(bot, SvPV_nolen(ST(1)), SvPV_nolen(ST(2)), items == 4 ? SvPV_nolen(ST(3)) : ""));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_ChanUserMode)
{
	Module *mod;
	Bot *bot;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 4) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:ChanUserMode(bot, chan, mode, target)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot && FindChannel(SvPV_nolen(ST(1)))) {
			XSRETURN_UV(irc_cmode(bot, SvPV_nolen(ST(1)), SvPV_nolen(ST(2)), SvPV_nolen(ST(3))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_Kill)
{
	Module *mod;
	Bot *bot;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:Kill(bot, target, reason)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot) {
			XSRETURN_UV(irc_kill(bot, SvPV_nolen(ST(1)), SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_Kick)
{
	Module *mod;
	Bot *bot;

	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:Kill(bot, chan, target, <reason>)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot && FindChannel(SvPV_nolen(ST(1)))) {
			XSRETURN_UV(irc_kick(bot, SvPV_nolen(ST(1)), SvPV_nolen(ST(2)), items == 4 ? SvPV_nolen(ST(3)) : "" ));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_Invite)
{
	Module *mod;
	Bot *bot;
	Client *u;
	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:Invite(bot, target, chan)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		u = FindUser(SvPV_nolen(ST(1)));
		if (bot && u && FindChannel(SvPV_nolen(ST(2)))) {
			XSRETURN_UV(irc_invite(bot, u, SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_Topic)
{
	Module *mod;
	Bot *bot;
	Channel *c;
	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:Topic(bot, chan, topic)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		c = FindChannel(SvPV_nolen(ST(1)));
		if (bot && c) {
			XSRETURN_UV(irc_topic(bot, c, SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_SvsKill)
{
	Module *mod;
	Bot *bot;
	Client *u;
	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:SvsKill(bot, target, reason)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		u = FindUser(SvPV_nolen(ST(1)));
		if (bot && u) {
			XSRETURN_UV(irc_svskill(bot, u, SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_SvsMode)
{
	Module *mod;
	Bot *bot;
	Client *u;
	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:SvsMode(bot, target, mode)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		u = FindUser(SvPV_nolen(ST(1)));
		if (bot && u) {
			XSRETURN_UV(irc_svsmode(bot, u, SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_SvsHost)
{
	Module *mod;
	Bot *bot;
	Client *u;
	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:SvsHost(bot, target, host)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		u = FindUser(SvPV_nolen(ST(1)));
		if (bot && u) {
			XSRETURN_UV(irc_svshost(bot, u, SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_SvsJoin)
{
	Module *mod;
	Bot *bot;
	Client *u;
	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:SvsJoin(bot, target, chan)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		u = FindUser(SvPV_nolen(ST(1)));
		if (bot && u) {
			XSRETURN_UV(irc_svsjoin(bot, u, SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_SvsPart)
{
	Module *mod;
	Bot *bot;
	Client *u;
	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:SvsPart(bot, target, chan)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		u = FindUser(SvPV_nolen(ST(1)));
		if (bot && u) {
			XSRETURN_UV(irc_svspart(bot, u, SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_Swhois)
{
	Module *mod;
	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 2) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:Swhois(target, swhois)");
	} else {
		XSRETURN_UV(irc_swhois(SvPV_nolen(ST(0)), SvPV_nolen(ST(1))));
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_SvsNick)
{
	Module *mod;
	Bot *bot;
	Client *u;
	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:SvsNick(bot, target, newnick)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		u = FindUser(SvPV_nolen(ST(1)));
		if (bot && u) {
			XSRETURN_UV(irc_svsnick(bot, u, SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_SMO)
{
	Module *mod;
	Bot *bot;
	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:SMO(bot, umodetarget, message)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot) {
			XSRETURN_UV(irc_smo(bot->name, SvPV_nolen(ST(1)), SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_Akill)
{
	Module *mod;
	Bot *bot;
	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 5) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:Akill(bot, host, ident, length, reason)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot) {
			XSRETURN_UV(irc_akill(bot, SvPV_nolen(ST(1)), SvPV_nolen(ST(2)), SvIV(ST(3)), SvPV_nolen(ST(4))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_Rakill)
{
	Module *mod;
	Bot *bot;
	dXSARGS;
	mod = GET_CUR_MODULE();
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:Rakill(bot, host, ident)");
	} else {
		bot = FindBot(SvPV_nolen(ST(0)));
		if (bot) {
			XSRETURN_UV(irc_rakill(bot, SvPV_nolen(ST(1)), SvPV_nolen(ST(2))));
		}
		
	}
	XSRETURN_UV(NS_FAILURE);
}



static
XS (XS_NeoStats_AddTimer)
{
	Module *mod;
	dXSARGS;
	
	mod = GET_CUR_MODULE();
	if (items < 4) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:AddTimer(type, name, interval, callback)");
	} else {
		XSRETURN_UV(AddTimer(SvIV(ST(0)), perl_timer_cb, SvPV_nolen(ST(1)), SvIV(ST(2)), strndup(SvPV_nolen(ST(3)), sv_len(ST(3)))));
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS (XS_NeoStats_DelTimer)
{
	Module *mod;
	dXSARGS;
	
	mod = GET_CUR_MODULE();
	if (items < 1) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:DelTimer(name)");
	} else {
		XSRETURN_UV(DelTimer(SvPV_nolen(ST(0))));
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS(XS_NeoStats_DBAStore)
{
	dXSARGS;
	size_t mysize;
	if (items < 3) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:DBAStore(table, name, data)");
	} else {
		/* our DBA layer expects us to be able to know the size of the data when we fetch, which in the
		 * case of perl, we have no idea, so this is a workaround to store the size in a seperate entry
		 * and we can retrive that in a Fetch Call before actually fetching the data
		 */
		mysize = SvCUR(ST(2));
		DBAStoreInt("PerlSizes", SvPV_nolen(ST(1)), &mysize);
		XSRETURN_UV(DBAStore(SvPV_nolen(ST(0)), SvPV_nolen(ST(1)), SvPVbyte_nolen(ST(2)), SvCUR(ST(2))));
	}
	XSRETURN_UV(NS_FAILURE);
}

static
XS(XS_NeoStats_DBAFetch)
{
	dXSARGS;
	char *data;
	SV *ret;
	size_t mysize;
	
	if (items < 2) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:DBAFetch(table, name)");
	} else {
		SP -= items;
		/* first get the size */
		DBAFetchInt("PerlSizes", SvPV_nolen(ST(1)), &mysize);
		data = ns_malloc((int)mysize);
		DBAFetch(SvPV_nolen(ST(0)), SvPV_nolen(ST(1)), data, (int)mysize);
		data[mysize] = '\0';
		ret = newSVpv(data, mysize);
		free(data);
		sv_2mortal(ret);
		XPUSHs(ret);
		PUTBACK;
		XSRETURN(1);
	}
	XSRETURN_EMPTY;
}	

static
XS(XS_NeoStats_DBADelete)
{
	dXSARGS;
	if (items < 2) {
		nlog(LOG_WARNING, "Usage: NeoStats:Internal:DBADelete(table, name)");
	} else {
		/* our DBA layer expects us to be able to know the size of the data when we fetch, which in the
		 * case of perl, we have no idea, so this is a workaround to store the size in a seperate entry
		 * and we can retrive that in a Fetch Call before actually fetching the data
		 */
		DBADelete("PerlSizes", SvPV_nolen(ST(1)));
		XSRETURN_UV(DBADelete(SvPV_nolen(ST(0)), SvPV_nolen(ST(1))));
	}
	XSRETURN_UV(NS_FAILURE);
}

/* I hate Global Variables, but there is no user cookie var to pass around */
HV *perlFetchRows;
int gotitems = 0;

static int
load_perl_rows(char *key, void *data, int size)
{
	gotitems = 1;
	hv_store(perlFetchRows, key, strlen(key),
		newSVpv(data, size), 0);
	return NS_FALSE;
}

static
XS(XS_NeoStats_DBAFetchRows)
{
	dXSARGS;
	if (items < 1) {
		nlog(LOG_WARNING, "Useage: NeoStats::Internal::DBAFetchRows(table)");
	} else {
		SP -= items;
		if (gotitems > 0) {
			hv_clear(perlFetchRows);
			gotitems = 0;
		}
		DBAFetchRows2(SvPV_nolen(ST(0)), load_perl_rows);
		XPUSHs(newRV_noinc((SV *)perlFetchRows));
		PUTBACK;
		XSRETURN(1);
	}
	XSRETURN_UV(NS_FAILURE);
}

/* xs_init is the second argument perl_parse. As the name hints, it
   initializes XS subroutines (see the perlembed manpage) */
static void
xs_init (pTHX)
{
	HV *stash;
	Module *mod;
	mod = GET_CUR_MODULE();
	/* This one allows dynamic loading of perl modules in perl
	   scripts by the 'use perlmod;' construction */
	newXS ("DynaLoader::boot_DynaLoader", boot_DynaLoader, __FILE__);
	/* load up all the custom IRC perl functions */
	newXS ("NeoStats::Internal::debug", XS_NeoStats_debug, __FILE__);
	newXS ("NeoStats::Internal::register", XS_NeoStats_register, __FILE__);
	newXS ("NeoStats::Internal::hook_event", XS_NeoStats_hook_event, __FILE__);
	newXS ("NeoStats::Internal::unhook_event", XS_NeoStats_unhook_event, __FILE__);
	newXS ("NeoStats::Internal::AddBot", XS_NeoStats_AddBot, __FILE__);
	newXS ("NeoStats::Internal::DelBot", XS_NeoStats_DelBot, __FILE__);
	newXS ("NeoStats::Internal::FindUser", XS_NeoStats_FindUser, __FILE__);
	newXS ("NeoStats::Internal::FindServer", XS_NeoStats_FindServer, __FILE__);
	newXS ("NeoStats::Internal::FindChannel", XS_NeoStats_FindChannel, __FILE__);
	newXS ("NeoStats::Internal::AddCommand", XS_NeoStats_AddCommand, __FILE__);
	newXS ("NeoStats::Internal::DelCommand", XS_NeoStats_DelCommand, __FILE__);
	newXS ("NeoStats::Internal::Prefmsg", XS_NeoStats_Prefmsg, __FILE__);
	newXS ("NeoStats::Internal::ChanAlert", XS_NeoStats_ChanAlert, __FILE__);
	newXS ("NeoStats::Internal::PrivMsg", XS_NeoStats_PrivMsg, __FILE__);
	newXS ("NeoStats::Internal::Notice", XS_NeoStats_Notice, __FILE__);
	newXS ("NeoStats::Internal::ChanPrivMsg", XS_NeoStats_ChanPrivMsg, __FILE__);
	newXS ("NeoStats::Internal::ChanNotice", XS_NeoStats_ChanNotice, __FILE__);
	newXS ("NeoStats::Internal::Globops", XS_NeoStats_Globops, __FILE__);
	newXS ("NeoStats::Internal::Wallops", XS_NeoStats_Wallops, __FILE__);
	newXS ("NeoStats::Internal::Numeric", XS_NeoStats_Numeric, __FILE__);
	newXS ("NeoStats::Internal::Umode", XS_NeoStats_Umode, __FILE__);
	newXS ("NeoStats::Internal::Join", XS_NeoStats_Join, __FILE__);
	newXS ("NeoStats::Internal::Part", XS_NeoStats_Part, __FILE__);
	newXS ("NeoStats::Internal::NickChange", XS_NeoStats_NickChange, __FILE__);
	newXS ("NeoStats::Internal::CMode", XS_NeoStats_CMode, __FILE__);
	newXS ("NeoStats::Internal::ChanUserMode", XS_NeoStats_ChanUserMode, __FILE__);
	newXS ("NeoStats::Internal::Kill", XS_NeoStats_Kill, __FILE__);
	newXS ("NeoStats::Internal::Kick", XS_NeoStats_Kick, __FILE__);
	newXS ("NeoStats::Internal::Invite", XS_NeoStats_Invite, __FILE__);
	newXS ("NeoStats::Internal::Topic", XS_NeoStats_Topic, __FILE__);
	newXS ("NeoStats::Internal::SvsKill", XS_NeoStats_SvsKill, __FILE__);
	newXS ("NeoStats::Internal::SvsMode", XS_NeoStats_SvsMode, __FILE__);
	newXS ("NeoStats::Internal::SvsHost", XS_NeoStats_SvsHost, __FILE__);
	newXS ("NeoStats::Internal::SvsJoin", XS_NeoStats_SvsJoin, __FILE__);
	newXS ("NeoStats::Internal::SvsPart", XS_NeoStats_SvsPart, __FILE__);
	newXS ("NeoStats::Internal::Swhois", XS_NeoStats_Swhois, __FILE__);
	newXS ("NeoStats::Internal::SvsNick", XS_NeoStats_SvsNick, __FILE__);
	newXS ("NeoStats::Internal::SMO", XS_NeoStats_SMO, __FILE__);
	newXS ("NeoStats::Internal::Akill", XS_NeoStats_Akill, __FILE__);
	newXS ("NeoStats::Internal::Rakill", XS_NeoStats_Rakill, __FILE__);
	newXS ("NeoStats::Internal::AddTimer", XS_NeoStats_AddTimer, __FILE__);
	newXS ("NeoStats::Internal::DelTimer", XS_NeoStats_DelTimer, __FILE__);
	newXS ("NeoStats::Internal::DBAFetch", XS_NeoStats_DBAFetch, __FILE__);
	newXS ("NeoStats::Internal::DBAStore", XS_NeoStats_DBAStore, __FILE__);
	newXS ("NeoStats::Internal::DBADelete", XS_NeoStats_DBADelete, __FILE__);
	newXS ("NeoStats::Internal::DBAFetchRows", XS_NeoStats_DBAFetchRows, __FILE__);
	
	stash = get_hv ("NeoStats::", TRUE);
	if (stash == NULL) {
		exit (1);
	}
	newCONSTSUB (stash, "EVENT_NULL", newSViv (EVENT_NULL));
	newCONSTSUB (stash, "EVENT_MODULELOAD", newSViv (EVENT_MODULELOAD));
	newCONSTSUB (stash, "EVENT_MODULEUNLOAD", newSViv (EVENT_MODULEUNLOAD));
	newCONSTSUB (stash, "EVENT_SERVER", newSViv (EVENT_SERVER));
	newCONSTSUB (stash, "EVENT_SQUIT", newSViv (EVENT_SQUIT));
	newCONSTSUB (stash, "EVENT_PING", newSViv (EVENT_PING));
	newCONSTSUB (stash, "EVENT_PONG", newSViv (EVENT_PONG));
	newCONSTSUB (stash, "EVENT_SIGNON", newSViv (EVENT_SIGNON));
	newCONSTSUB (stash, "EVENT_QUIT", newSViv (EVENT_QUIT));
	newCONSTSUB (stash, "EVENT_NICKIP", newSViv (EVENT_NICKIP));
	newCONSTSUB (stash, "EVENT_KILL", newSViv (EVENT_KILL));
	newCONSTSUB (stash, "EVENT_GLOBALKILL", newSViv (EVENT_GLOBALKILL));
	newCONSTSUB (stash, "EVENT_LOCALKILL", newSViv (EVENT_LOCALKILL));
	newCONSTSUB (stash, "EVENT_SERVERKILL", newSViv (EVENT_SERVERKILL));
	newCONSTSUB (stash, "EVENT_BOTKILL", newSViv (EVENT_BOTKILL));
	newCONSTSUB (stash, "EVENT_NICK", newSViv (EVENT_NICK));
	newCONSTSUB (stash, "EVENT_AWAY", newSViv (EVENT_AWAY));
	newCONSTSUB (stash, "EVENT_UMODE", newSViv (EVENT_UMODE));
	newCONSTSUB (stash, "EVENT_SMODE", newSViv (EVENT_SMODE));
	newCONSTSUB (stash, "EVENT_NEWCHAN", newSViv (EVENT_NEWCHAN));
	newCONSTSUB (stash, "EVENT_DELCHAN", newSViv (EVENT_DELCHAN));
	newCONSTSUB (stash, "EVENT_JOIN", newSViv (EVENT_JOIN));
	newCONSTSUB (stash, "EVENT_PART", newSViv (EVENT_PART));
	newCONSTSUB (stash, "EVENT_PARTBOT", newSViv (EVENT_PARTBOT));
	newCONSTSUB (stash, "EVENT_EMPTYCHAN", newSViv (EVENT_EMPTYCHAN));
	newCONSTSUB (stash, "EVENT_KICK", newSViv (EVENT_KICK));
	newCONSTSUB (stash, "EVENT_KICKBOT", newSViv (EVENT_KICKBOT));
	newCONSTSUB (stash, "EVENT_TOPIC", newSViv (EVENT_TOPIC));
	newCONSTSUB (stash, "EVENT_CMODE", newSViv (EVENT_CMODE));
	newCONSTSUB (stash, "EVENT_PRIVATE", newSViv (EVENT_PRIVATE));
	newCONSTSUB (stash, "EVENT_NOTICE", newSViv (EVENT_NOTICE));
	newCONSTSUB (stash, "EVENT_CPRIVATE", newSViv (EVENT_CPRIVATE));
	newCONSTSUB (stash, "EVENT_CNOTICE", newSViv (EVENT_CNOTICE));
	newCONSTSUB (stash, "EVENT_GLOBOPS", newSViv (EVENT_GLOBOPS));
	newCONSTSUB (stash, "EVENT_CHATOPS", newSViv (EVENT_CHATOPS));
	newCONSTSUB (stash, "EVENT_WALLOPS", newSViv (EVENT_WALLOPS));
	newCONSTSUB (stash, "EVENT_CTCPVERSIONRPL", newSViv (EVENT_CTCPVERSIONRPL));
	newCONSTSUB (stash, "EVENT_CTCPVERSIONREQ", newSViv (EVENT_CTCPVERSIONREQ));
	newCONSTSUB (stash, "EVENT_CTCPFINGERRPL", newSViv (EVENT_CTCPFINGERRPL));
	newCONSTSUB (stash, "EVENT_CTCPFINGERREQ", newSViv (EVENT_CTCPFINGERREQ));
	newCONSTSUB (stash, "EVENT_CTCPACTIONREQ", newSViv (EVENT_CTCPACTIONREQ));
	newCONSTSUB (stash, "EVENT_CTCPTIMERPL", newSViv (EVENT_CTCPTIMERPL));
	newCONSTSUB (stash, "EVENT_CTCPTIMEREQ", newSViv (EVENT_CTCPTIMEREQ));
	newCONSTSUB (stash, "EVENT_CTCPPINGRPL", newSViv (EVENT_CTCPPINGRPL));
	newCONSTSUB (stash, "EVENT_CTCPPINGREQ", newSViv (EVENT_CTCPPINGREQ));
	newCONSTSUB (stash, "EVENT_DCCSEND", newSViv (EVENT_DCCSEND));
	newCONSTSUB (stash, "EVENT_DCCCHAT", newSViv (EVENT_DCCCHAT));
	newCONSTSUB (stash, "EVENT_DCCCHATMSG", newSViv (EVENT_DCCCHATMSG));
	newCONSTSUB (stash, "EVENT_ADDBAN", newSViv (EVENT_ADDBAN));
	newCONSTSUB (stash, "EVENT_DELBAN", newSViv (EVENT_DELBAN));
	
	newCONSTSUB (stash, "EVENT_FLAG_DISABLED", newSViv (EVENT_FLAG_DISABLED));
	newCONSTSUB (stash, "EVENT_FLAG_IGNORE_SYNCH", newSViv (EVENT_FLAG_IGNORE_SYNCH));
	newCONSTSUB (stash, "EVENT_FLAG_USE_EXCLUDE", newSViv (EVENT_FLAG_USE_EXCLUDE));
	newCONSTSUB (stash, "EVENT_FLAG_EXCLUDE_ME", newSViv (EVENT_FLAG_EXCLUDE_ME));
	newCONSTSUB (stash, "EVENT_FLAG_EXCLUDE_MODME", newSViv (EVENT_FLAG_EXCLUDE_MODME));


	newCONSTSUB (stash, "BOT_FLAG_ONLY_OPERS", newSViv (BOT_FLAG_ONLY_OPERS));
	newCONSTSUB (stash, "BOT_FLAG_RESTRICT_OPERS", newSViv (BOT_FLAG_RESTRICT_OPERS));
	newCONSTSUB (stash, "BOT_FLAG_DEAF", newSViv (BOT_FLAG_DEAF));
	newCONSTSUB (stash, "BOT_FLAG_ROOT", newSViv (BOT_FLAG_ROOT));
	newCONSTSUB (stash, "BOT_FLAG_PERSIST", newSViv (BOT_FLAG_PERSIST));
	newCONSTSUB (stash, "BOT_COMMON_HOST", newSVpv (BOT_COMMON_HOST, strlen(BOT_COMMON_HOST)));

	newCONSTSUB (stash, "NS_ERR_SYNTAX_ERROR", newSViv (NS_ERR_SYNTAX_ERROR));
	newCONSTSUB (stash, "NS_ERR_NEED_MORE_PARAMS", newSViv(NS_ERR_NEED_MORE_PARAMS));
	newCONSTSUB (stash, "NS_ERR_PARAM_OUT_OF_RANGE", newSViv(NS_ERR_PARAM_OUT_OF_RANGE));
	newCONSTSUB (stash, "NS_ERR_UNKNOWN_COMMAND", newSViv(NS_ERR_UNKNOWN_COMMAND));
	newCONSTSUB (stash, "NS_ERR_NO_PERMISSION", newSViv(NS_ERR_NO_PERMISSION));

	newCONSTSUB (stash, "TIMER_TYPE_INTERVAL", newSViv(TIMER_TYPE_INTERVAL));	
	newCONSTSUB (stash, "TIMER_TYPE_DAILY", newSViv(TIMER_TYPE_DAILY));	
	newCONSTSUB (stash, "TIMER_TYPE_WEEKLY", newSViv(TIMER_TYPE_WEEKLY));	
	newCONSTSUB (stash, "TIMER_TYPE_MONTHLY", newSViv(TIMER_TYPE_MONTHLY));	
	newCONSTSUB (stash, "TIMER_TYPE_COUNTDOWN", newSViv(TIMER_TYPE_COUNTDOWN));	



	newCONSTSUB (stash, "NS_SUCCESS", newSViv (NS_SUCCESS));
	newCONSTSUB (stash, "NS_FAILURE", newSViv (NS_FAILURE));

	if (mod->pm->extninit) {
		newXS ("NeoStats::Internal::registerextension", XS_NeoStats_registerextension, __FILE__);
		mod->pm->extninit();
	}
	Init_Perl_NV();
	perlFetchRows = newHV();

}

int
Init_Perl (void)
{

	return NS_SUCCESS;
}

static Module *load_perlfiles (const char *filename, Module *mod, perl_xs_init init_func) 
{
	char *perl_args[] = { "", "-e", "0", "-w" };
	const char perl_definitions[] = {
#include "neostats.pm.h" 
	};


	mod->pm = ns_calloc(sizeof(PerlModInfo));
	mod->pm->registered = 0;
	mod->pm->extninit = init_func;

	strlcpy(mod->pm->filename, filename, MAXPATH);


/*	PL_perl_destruct_level = 1; */
	mod->pm->my_perl = perl_alloc ();
	PL_perl_destruct_level = 1;
	PERL_SET_CONTEXT((PMI *)mod->pm->my_perl);
	perl_construct (mod->pm->my_perl);
	PL_perl_destruct_level = 1;
	SET_RUN_LEVEL(mod);
	perl_parse (mod->pm->my_perl, xs_init, 4, perl_args, NULL);
	/*
	   Now initialising the perl interpreter by loading the
	   perl_definition array.
	 */
	eval_pv (perl_definitions, TRUE);
	RESET_RUN_LEVEL();
	return mod;
}

int load_perlextension(const char *filename, perl_xs_init init_func, Client *u)
{
	Module *mod;
	char filebuf[BUFSIZE];
	
	mod = GET_CUR_MODULE();
	if (!mod) {
		nlog(LOG_WARNING, "Trying to laod a Perl Extension %s in the core? No No", filename);
		return NS_FAILURE;
	}	
	ircsnprintf(filebuf, BUFSIZE, "modules/%s%s", filename, MOD_EXTEXT);

	mod = load_perlfiles((const char *)filebuf, mod, init_func);
	
	SET_RUN_LEVEL(mod);
	if (!execute_perl (mod, sv_2mortal (newSVpv ("NeoStats::Embed::loadextension", 0)),
								1, (char *)filebuf)) {
		/* if we are here, check that pm->mod->description has something, otherwise the script didnt register */
		if (!mod->pm->registered) {
			load_module_error(u, filebuf, __("Perl extension didn't register.", u));
			unload_perlextension(mod);
			return NS_FAILURE;
		}		
		/* it loaded ok */
	} else {
		load_module_error(u, filebuf, __("Errors in Perl extension", u));
		unload_perlextension(mod);
		return NS_FAILURE;	
	}
	return NS_SUCCESS;
	
	
}
Module *load_perlmodule (const char *filename, Client *u)
{
	Module *mod;
	CmdParams *cmd;
	
	mod = ns_calloc(sizeof(Module));
	mod->info = ns_calloc(sizeof(ModuleInfo));
	/* this is a temp solution till we get fully loaded. Its Bad */
	mod->info->name = ns_malloc(strlen("NeoStats")+1);
	ircsnprintf((char *)mod->info->name, strlen("NeoStats")+1, "NeoStats");
	
	mod = load_perlfiles((const char *)filename, mod, NULL);

	mod->type = MOD_TYPE_PERL;
	SET_RUN_LEVEL(mod);
	if (!execute_perl (mod, sv_2mortal (newSVpv ("NeoStats::Embed::load", 0)),
								1, (char *)filename)) {
		/* if we are here, check that pm->mod->description has something, otherwise the script didnt register */
		if (!mod->pm->registered) {
			load_module_error(u, filename, __("Perl Module didn't register.", u));
			unload_perlmod(mod);
			RESET_RUN_LEVEL();
			free(mod);
			return NULL;
		}		
		/* it loaded ok */
	} else {
		load_module_error(u, filename, __("Errors in Perl Module", u));
		unload_perlmod(mod);
		RESET_RUN_LEVEL();
		free(mod);
		return NULL;	
	}
	assign_mod_number(mod);

	DBAOpenDatabase();
	RESET_RUN_LEVEL();

	insert_module(mod);


	if (IsNeoStatsSynched()) {
		SynchModule(mod);
	}		

	cmd = ns_calloc (sizeof(CmdParams));
	cmd->param = (char*)mod->info->name;
	SendAllModuleEvent(EVENT_MODULELOAD, cmd);
	ns_free(cmd);

	nlog(LOG_NORMAL, "Loaded Perl Module %s (%s)", mod->info->name, mod->info->version);
         if (u) {
	         irc_prefmsg (ns_botptr, u, __("Perl Module %s loaded, %s",u), mod->info->name, mod->info->description);
                  irc_globops (NULL, _("Perl Module %s loaded"), mod->info->name);
         }
	return mod;
}

void PerlModFini(Module *mod)
{
	if( IsModuleSynched( mod ) && mod->pm->type == TYPE_MODULE )
	{
		/* only execute unload if synced */
		execute_perl (mod, sv_2mortal (newSVpv ("NeoStats::Embed::unload", 0)), 1, mod->pm->filename);
	}
}

void PerlExtensionFini(Module *mod)
{
	if (IsModuleSynched(mod) && mod->pm->type == TYPE_EXTENSION) {
		execute_perl(mod, sv_2mortal (newSVpv ("NeoStats::Embed::unload", 0)), 1, mod->pm->filename);
	}
}

void unload_perlmod(Module *mod)
{
	PERL_SET_CONTEXT((PMI *)mod->pm->my_perl);
	/* because segv handler doesn't handle perl well yet */
//		RESET_RUN_LEVEL()
	PL_perl_destruct_level = 1;
	perl_destruct ((PMI *)mod->pm->my_perl);

	perl_free ((PMI *)mod->pm->my_perl);

	free((void *)mod->info->name);

	free((void *)mod->info->description);

	free((void *)mod->info->version);
	
	free((void *)mod->info->build_date);
	
	free((void *)mod->info->build_time);

	free(mod->info);
	
	free(mod->pm);
}
void unload_perlextension(Module *mod)
{
	PERL_SET_CONTEXT((PMI *)mod->pm->my_perl);
	/* because segv handler doesn't handle perl well yet */
//		RESET_RUN_LEVEL()
	PL_perl_destruct_level = 1;
	perl_destruct ((PMI *)mod->pm->my_perl);

	perl_free ((PMI *)mod->pm->my_perl);
	
	if (mod->pm->event_list) {
		free(mod->pm->event_list);
		mod->pm->event_list = NULL;
	}
	free(mod->pm);
}
