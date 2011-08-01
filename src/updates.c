
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
** $Id: updates.c 3294 2008-02-24 02:45:41Z Fish $
*/

/* @file NeoStats interface to send updates back to NeoStats/Secure.irc-chat.net
 */

#include "neostats.h"
#include "services.h"
#include "event.h"
#include "MiniMessage.h"
#include "MiniMessageGateway.h"
#include "NeoNet.h"
#include "updates.h"

static void GotUpdateAddress(void *data, adns_answer *a);
static int mqswrite(int fd, void *data);
static int mqsread(void *data, void *notused, int len);
static int mqs_login();
static char *MQGetStatus(MQS_STATE state);
static void CheckMQOut();
void MQcmd_Broadcast(MMessage *msg);
void MQcmd_Shutdown(MMessage *msg);
static void ResetMQ();
static char *MQGetConnect(MQS_CONNECT connect);

list_t *MQlist;
hash_t *MQcmds;


/* this is our standard commands we handle internally */
static NeoNetCmds stdcmds[]=
{
	{"BROADCAST",	MQcmd_Broadcast},
	{"SHUTDOWN", 	MQcmd_Shutdown},
	{NULL,		NULL}
};
	

updateserver mqs;
MMessageGateway *mqsgw;

void MQcmd_Broadcast(MMessage *msg) {
	MByteBuffer **message;
	message = MMGetStringField(msg, "message", NULL);
	if (message) {
		if (IsModuleSynched(&ns_module))
			irc_chanalert(ns_botptr, "NeoNet BroadCast Message: %s",(char *)message[0]+sizeof(uint32));
		nlog(LOG_WARNING, "NeoNet BroadCast Message: %s",(char *)message[0]+sizeof(uint32));

	}
}
void MQcmd_Shutdown(MMessage *msg) {
	MByteBuffer **message;
	message = MMGetStringField(msg, "message", NULL);
	if (message) {
		if (IsModuleSynched(&ns_module))
			irc_chanalert(ns_botptr, "NeoNet Shutdown Message: %s",(char *)message[0]+sizeof(uint32));
		nlog(LOG_WARNING, "NeoNet Shutdown Message: %s",(char *)message[0]+sizeof(uint32));
	}
	ResetMQ();
}

int MQAddcmd(NeoNetCmds *cmd) {
	while (cmd->topic) {
		if (hnode_find(MQcmds, cmd->topic)) {
			nlog(LOG_WARNING, "Can't Add MQCommand Handler %s. Exists already", cmd->topic);
		} else {
			hnode_create_insert(MQcmds, cmd, cmd->topic);
			cmd->modptr = GET_CUR_MODULE();
			dlog(LOG_WARNING, "Added %s to MQCommand Handler", cmd->topic);
		}
		cmd++;
	}
	return NS_SUCCESS;
}
int MQDelcmd(NeoNetCmds *cmd) {
	hnode_t *cmdnode;
	while (cmd->topic) {
		if ((cmdnode = hnode_find(MQcmds, cmd->topic)) != NULL) {
			dlog(DEBUG3, "Deleting %s Command from MQCommand Handlers", cmd->topic);
			hash_delete_destroy_node(MQcmds, cmdnode);
		} else {
			nlog(LOG_WARNING, "Couldn't find %s handler for MQCommand Handler Delete", cmd->topic);
		}
		cmd++;
	}
	return NS_SUCCESS;
}
int MQModuleDelcmd(Module *modptr) {
	hnode_t *cmdnode;
	hscan_t hs;
	NeoNetCmds *cmds;
	hash_scan_begin(&hs, MQcmds);
	while ( ( cmdnode = hash_scan_next(&hs)) != NULL) {
		cmds = hnode_get(cmdnode);
		if (cmds->modptr == modptr) {
			dlog(DEBUG3, "Deleting %s Command from MQCommand Module Delete", cmds->topic);
			hash_scan_delete_destroy_node(MQcmds, cmdnode);
		}
	}
	return NS_SUCCESS;
}

int MQCheckGroups(char *group) {
	int i;
	for (i = 0; i < mqs.nogroups; i++) {
		if (!ircstrcasecmp(mqs.groups[i], group)) {
			return NS_SUCCESS;
		}
	}
	return NS_FAILURE;
}

MMessage *MQCreateMessage(char *topic, char *target, int flags, char *groups, int peermsg) {
	MByteBuffer **MMtopic;
	MByteBuffer **MMtarget;
	MByteBuffer **MMFrom;
	MMessage *msg;

	/* if the message is for a queue, check we are subscribed to the groups for this message
	* else, just send the message to the peer, and let the peer decide if they are authed
	* or not 
	*/ 
	if (((groups != NULL) && (peermsg > 0)) && (!MQCheckGroups(groups))) {
		dlog(DEBUG3, "Message for Group membership %s did not pass", groups);
		return NULL;
	}
	msg = MMAllocMessage(0);
	if (!msg) {
		nlog(LOG_WARNING, "Warning, Couldn't create MiniMessage in MQCreateMessage");
		return NULL;
	}
	/* not necessarly required, but for completness, so MQServer doesn't have to add it */
	MMFrom = MMPutStringField(msg, false, NNFLD_FROM, 1);
	MMFrom[0] = MBStrdupByteBuffer(mqs.username);
	
	MMtopic = MMPutStringField(msg, false, NNFLD_TOPIC, 1);
	MMtopic[0] = MBStrdupByteBuffer(topic);
	MMtarget = MMPutStringField(msg, false, NNFLD_TARGET, 1);
	MMtarget[0] = MBStrdupByteBuffer(target);
	
	/* is this a peer to peer message, or a queue message */
	if (peermsg > 0) 
		MMSetWhat(msg, NNMSG_SNDTOUSER);
	else
		MMSetWhat(msg, NNMSG_SNDTOQUEUE);
			
	/* ok, our standard fields are inserted, now return the msg */
	return msg;
}

int MQSendMessage(MMessage *msg, int canqueue) {
	if ((mqs.state != MQS_OK) && (canqueue == 0)) {
		nlog(LOG_WARNING, "NeoNet is not connected, and canqueue is false. Dropping Msg");
		return NS_FAILURE;
	}
	else if ((mqs.state != MQS_OK) && (canqueue > 0)) {
		/* Queue the message */
		lnode_create_append(MQlist, msg);
		/* if we are on demand connect, then start connect now */
		if (mqs.connect >= MQ_CONNECT_DEMAND) 
			dns_lookup( mqs.hostname,  adns_r_a, GotUpdateAddress, NULL );
			
		dlog(DEBUG3, "MQ Message is queued up for sending later");
		return NS_SUCCESS;
	}
	/* else we just send the message now */
#ifdef DEBUG
	MMPrintToStream(msg);
#endif
	MGAddOutgoingMessage(mqsgw, msg);
	MMFreeMessage(msg);

	CheckMQOut();
	return NS_SUCCESS;	
}
void MQStatusMsg(const Bot *bot, const CmdParams *cmdparams) {
	int i;
	irc_prefmsg( bot, cmdparams->source, __("NeoNet Connection Enabled: %s", cmdparams->source), MQGetConnect(mqs.connect));
	irc_prefmsg( bot, cmdparams->source, __("NeoNet Status: %s", cmdparams->source), MQGetStatus(mqs.state));
	if (mqs.state == MQS_OK) {
		irc_prefmsg( bot, cmdparams->source, __("NeoNet Group Memberships:", cmdparams->source));
		for (i = 0; i < mqs.nogroups; i++) irc_prefmsg(bot, cmdparams->source, "%d) %s", i, mqs.groups[i]);
	}
}

static char *MQGetConnect(MQS_CONNECT connect) {
	switch (connect) {
		case MQ_CONNECT_ERROR:
			return "No (NeoNet Error/Failure - Please consult Log files)";
			break;
		case MQ_CONNECT_NO:
			return "No (disabled)";
			break;
		case MQ_CONNECT_DEMAND:
			return "Yes - On Demand";
			break;
		case MQ_CONNECT_YES:
			return "Yes";
			break;
	}
	return "Uknown Connect Style";
}

static char *MQGetStatus(MQS_STATE state) {
	switch (state) {
		case MQS_DISCONNECTED:
			return "Disconnected";
			break;
		case MQS_CONNECTING:
			return "Connecting";
			break;
		case MQS_SENTAUTH:
			return "Logging In";
			break;
		case MQS_OK:
			return "Connected";
			break;
	}
	return "Unknown State";
}

static int32 MQSSendSock(const uint8 * buf, uint32 numBytes, void * arg) {
	return send_to_sock(mqs.Sockinfo, (char *)buf, numBytes); 
}

static int32 MQSRecvSock(uint8 * buf, uint32 numBytes, void * arg) {
	return  os_sock_read( mqs.sock, (char *)buf, numBytes );
}

static int MQPingSrv(void *unused) {
	MMessage *msg;
	int64 *ping;
	if (mqs.state != MQS_OK) { 
		/* just return silently cause this is only a ping*/
		return NS_SUCCESS;
	}
	msg = MMAllocMessage(0);
	if (!msg) {
		nlog(LOG_WARNING, "Warning, Couldn't create MQ Ping Packet");
		return NS_SUCCESS;
	}
	ping = MMPutInt64Field(msg, false, NNFLD_SNDTIME, 1);
	if (ping) ping[0] = time(NULL);
	
	MMSetWhat(msg, NNMSG_PING);
	MQSendMessage(msg, 0);
		
	return NS_SUCCESS;

}

int MQCredOk() 
{
	if (mqs.username[0] != '\0' && mqs.password[0] != '\0') {
		return NS_SUCCESS;
	} else {
		return NS_FAILURE;
	}
}

char *MQUsername() 
{
	return mqs.username;
}

char *MQPassword()
{
	return mqs.password;
}

int InitUpdate(void) 
{
	if (mqs.connect == MQ_CONNECT_YES) {
		if (MQCredOk() == NS_SUCCESS) {
			dns_lookup( mqs.hostname,  adns_r_a, GotUpdateAddress, NULL );
			nlog(LOG_INFO, "NeoNet Initialized successfully, connecting to %s", mqs.hostname);
		}
	} else if (mqs.connect == MQ_CONNECT_DEMAND) {
		if (MQCredOk() != NS_SUCCESS) {
			nlog(LOG_WARNING, "Can't do OnDemand Connect to NeoNet as no username/password has been set");
			mqs.connect = MQ_CONNECT_ERROR;
		}
	}
	mqs.state = MQS_DISCONNECTED;
	MQlist = list_create(LISTCOUNT_T_MAX);
	MQcmds = hash_create(HASHCOUNT_T_MAX, 0, 0);
	MQAddcmd(stdcmds);
	AddTimer(TIMER_TYPE_INTERVAL, MQPingSrv, "MQPingSrv", 300, NULL);
	return NS_SUCCESS;
}

void FiniUpdate(void) 
{
	MMessage *qmsg;
	lnode_t *node;
	ResetMQ();
	DelTimer("MQPingSrv");
	if (MQlist && (list_count(MQlist) > 0)) {
		while ((node = list_first(MQlist)) != NULL) {
			qmsg = lnode_get(node);
			MMFreeMessage(qmsg);
			list_del_first(MQlist);
		}
	}
#if 0
	MQDelcmd(stdcmds);
	list_destroy(MQlist);
	hash_destroy(MQcmds);
#endif
}

static void GotUpdateAddress(void *data, adns_answer *a) 
{
	struct timeval tv;
	struct in_addr ip;
	
	SET_SEGV_LOCATION();
	if( a && a->nrrs > 0 && a->status == adns_s_ok ) {
			ip.s_addr = a->rrs.inaddr->s_addr;
			
			mqs.sock = sock_connect(SOCK_STREAM, ip, mqs.port);
			
			if (mqs.sock > 0) {
				tv.tv_sec = 60;
				mqsgw = MGAllocMessageGateway();
				if (!mqsgw) {
					nlog(LOG_WARNING, "Couldn't allocate MiniMessageGateway Object");
					return;
				}
				mqs.Sockinfo = AddSock(SOCK_NATIVE, "MQS", mqs.sock, mqsread, mqswrite, EV_WRITE|EV_TIMEOUT|EV_READ|EV_PERSIST, NULL, &tv);
				mqs.state = MQS_CONNECTING;
			}
			nlog (LOG_NORMAL, "Got DNS for MQ Pool Server: %s", inet_ntoa(ip));
	} else {
		nlog(LOG_WARNING, "DNS error Checking for MQ Server Pool: %s", adns_strerror(a->status));
	}
}

static void ResetMQ() {
	if (mqs.state >= MQS_CONNECTING) {
		mqs.state = MQS_DISCONNECTED;
		DelSock(mqs.Sockinfo);
		mqs.Sockinfo = NULL;
		MGFreeMessageGateway(mqsgw);
	}
}

static void CheckMQOut() {
	int i;
	if (mqs.state < MQS_SENTAUTH) {
			nlog(LOG_WARNING, "MQ Server Not Connected....");
			/* do something */
			return;
	}
	if (MGHasBytesToOutput(mqsgw)) {
		i = MGDoOutput(mqsgw, ~0, MQSSendSock, NULL);
		if (i < 0) {
			nlog(LOG_WARNING, "MQ Server Connection Lost.... ");
			/* we lost our connection... */
			ResetMQ();
		}
	}
}

static void ProcessMQError(MMessage *msg) {
	MBool * fatal;
	MByteBuffer **error;
	int32 * status;
	fatal = MMGetBoolField(msg, NNFLD_FATAL, NULL);
	error = MMGetStringField(msg, NNFLD_ERROR, NULL);
	status = MMGetInt32Field(msg, NNFLD_STATUS, NULL);
	if (error && status) {
		if (IsModuleSynched(&ns_module))
			irc_chanalert(ns_botptr, "NeoNet Error %lld: %s", (long long int)status[0], (char *)error[0]+sizeof(uint32));
		nlog(LOG_WARNING, "NeoNet Error %lld: %s", (long long int)status[0], (char *)error[0]+sizeof(uint32));

	}
	if (fatal[0] > 0) {
		if (IsModuleSynched(&ns_module))
			irc_chanalert(ns_botptr, "Fatal NeoNet Error. Disconnecting From NeoNet");
		nlog(LOG_WARNING, "Fatal NeoNet Error. Disconnecting from NeoNet");
		ResetMQ();
	}
}

static void ProcessMessage(MMessage *msg) {
	unsigned long what;
	MByteBuffer **groups;
	uint32 nogroups;
	int i;
	lnode_t *node;
	MMessage *qmsg;
	int64 *sndtime;
	MByteBuffer **topic;
	NeoNetCmds *nncmd;
	hnode_t *cmdnode;

	what = MMGetWhat(msg);
	switch (what) {
		case NNMSG_LOGINOK:
			dlog(DEBUG1, "Login Ok");
			mqs.state = MQS_OK;
			groups = MMGetStringField(msg, "group", &nogroups);
			if (nogroups == 0 || groups == NULL) {
				/* NeoNetMQ sent a funky loginok message... */
				nlog(LOG_WARNING, "NeoNetMQ LoginOK message did not contain groups... ");
				return;
			}
			for (i = 0; i < nogroups; i++) {
				int len;
				/* max groups so far */
				if (i > 8) break;
				len = strlen((char *)groups[i]+sizeof(uint32));
				if (len > MAXUSER) len = MAXUSER;
				strncpy(mqs.groups[i], (char *)groups[i]+sizeof(uint32), len);
			}
			mqs.nogroups = i;
			/* if there are any queued messages, send them now */
			if (list_count(MQlist) > 0) {
				while ((node = list_first(MQlist)) != NULL) {
					qmsg = lnode_get(node);
					MGAddOutgoingMessage(mqsgw, qmsg);
					MMFreeMessage(qmsg);
					list_del_first(MQlist);
				}
				/* send them out */
				CheckMQOut();
			}				
			break;
		case NNMSG_ERROR:
			ProcessMQError(msg);
			break;
		case NNMSG_PONG:
			sndtime = MMGetInt64Field(msg, NNFLD_SNDTIME, NULL);
			if (sndtime) 
				dlog(DEBUG3, "MQ Ping Time Took %lld Seconds", time(NULL) - sndtime[0]);
			break;
		case NNMSG_PING:
			sndtime = MMPutInt64Field(msg, false, NNFLD_RCVDTIME, 1);
			if (sndtime) sndtime[0] = time(NULL);
			MMSetWhat(msg, NNMSG_PONG);
			MQSendMessage(msg, 0);
			break;			
		case NNMSG_MSGFROMUSER:
		case NNMSG_MSGFROMQUEUE:
			topic = MMGetStringField(msg, NNFLD_TOPIC, NULL);
			if (topic) {
				dlog(DEBUG3, "MQ Message Topic %s recieved", (char *)topic[0]+sizeof(uint32));
				cmdnode = hash_lookup(MQcmds, (char *)topic[0]+sizeof(uint32));
				if (!cmdnode) {
					nlog(LOG_INFO, "No Handler registered for NeoNet Message %s", (char *)topic[0]+sizeof(uint32));
				} else {
					nncmd = hnode_get(cmdnode);
					dlog(DEBUG3, "Running Handler for %s NeoNet Message", nncmd->topic);
					nncmd->handler(msg);
				}
			}	
			break;
		default:
			dlog(DEBUG1, "Got message type %lu", what);
			break;
	}

}

static int mqswrite(int fd, void *data) {
	switch (mqs.state) {
		case MQS_DISCONNECTED:
			nlog(LOG_CRITICAL, "Trying to send a message to MQ while disconnected?");
			break;
		/* we are connected */
		case MQS_CONNECTING:
			/* once I'm connected, update the sock flags */
			UpdateSock(mqs.Sockinfo, EV_READ|EV_PERSIST, 1, NULL);			
			return mqs_login();
			break;
		case MQS_SENTAUTH:
		case MQS_OK:
			/* ask MiniMessageGateway to write any buffer out */
			CheckMQOut();
			break;
	}
	return NS_SUCCESS;
}

static int mqsread(void *data, void *notused, int len) {
	MMessage *msg;
	
	if (len == -2 && mqs.state < MQS_OK) {
		/* timeout */
		nlog(LOG_WARNING, "Timeout Connecting to MQ Server");
		ResetMQ();
		return NS_SUCCESS;
	}
	if (len <= 0) {
		/* EOF etc */
		nlog(LOG_WARNING, "Lost Connection to MQ Server %s", os_sock_getlasterrorstring());
		ResetMQ();
		return NS_SUCCESS;
	}
	MGDoInput(mqsgw, ~0, MQSRecvSock, NULL, &msg);
	if (msg) {
#ifdef DEBUG
		MMPrintToStream(msg);
#endif
		/* do something */
		ProcessMessage(msg);
		if (msg) MMFreeMessage(msg);
	}
	return NS_SUCCESS;
}

static int mqs_login() 
{
	MMessage *msg = MMAllocMessage(0);
	MByteBuffer **username;
	MByteBuffer **password;
	MByteBuffer **version;
	if (!msg) {
		nlog(LOG_WARNING, "Warning, Couldn't create MiniMessage");
		return NS_FAILURE;
	}
	nlog(LOG_INFO, "Connnected to MQ Server, Logging in");

	username = MMPutStringField(msg, false, "username", 1);
	username[0] = MBStrdupByteBuffer(mqs.username);
	password = MMPutStringField(msg, false, "password", 1);
	password[0] = MBStrdupByteBuffer(mqs.password);
	version = MMPutStringField(msg, false, "version", 1);
	version[0] = MBStrdupByteBuffer(me.version);
	MMSetWhat(msg, NNMSG_LOGIN);
#ifdef DEBUG
	MMPrintToStream(msg);
#endif
	MGAddOutgoingMessage(mqsgw, msg);
	MMFreeMessage(msg);

	mqs.state = MQS_SENTAUTH;

	CheckMQOut();
		
	return NS_SUCCESS;
}
