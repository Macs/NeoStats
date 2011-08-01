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
** $Id: updates.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _UPDATES_H_
#define _UPDATES_H_
#include "MiniMessage.h"
#include "MiniMessageGateway.h"
#include "NeoNet.h"

/* this is the NeoNet Command Handler callback prototype */
typedef void (*mq_cmd_handler) ( MMessage *msg );

typedef struct NeoNetCmds {
	const char *topic; /* topic string we are interested in */
	mq_cmd_handler handler; /* function to call when we recieve this message */
	Module *modptr;
} NeoNetCmds;



int InitUpdate( void );
void FiniUpdate( void );
void MQStatusMsg(const Bot *bot, const CmdParams *cmdparams);


EXPORTFUNC MMessage *MQCreateMessage(char *topic, char *target, int flags, char *groups, int peermsg);
EXPORTFUNC int MQSendMessage(MMessage *msg, int canqueue);
EXPORTFUNC int MQCredOk();
EXPORTFUNC char *MQUsername();
EXPORTFUNC char *MQPassword(); 
EXPORTFUNC int MQAddcmd(NeoNetCmds *cmd);
EXPORTFUNC int MQDelcmd(NeoNetCmds *cmd);
EXPORTFUNC int MQModuleDelcmd(Module *modptr);
EXPORTFUNC int MQCheckGroups(char *group);


#endif /* _UPDATES_H_ */
