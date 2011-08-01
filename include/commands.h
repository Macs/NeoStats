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
** $Id: commands.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _COMMANDS_H_
#define _COMMANDS_H_

void del_all_bot_cmds( Bot *bot_ptr );
int add_bot_cmd( hash_t *cmd_hash, bot_cmd *cmd_ptr );
bot_cmd *find_bot_cmd( const Bot *bot_ptr, const char *cmd);
void del_bot_cmd( hash_t *cmd_hash, const bot_cmd *cmd_ptr );
int run_bot_cmd( CmdParams * cmdparams, int ischancmd );
int getuserlevel( const CmdParams *cmdparams );
void msg_permission_denied( const CmdParams *cmdparams, const char *subcommand );
void msg_error_need_more_params( const CmdParams *cmdparams );
void msg_error_param_out_of_range( const CmdParams *cmdparams );
void msg_syntax_error( const CmdParams *cmdparams );
void msg_unknown_command( const CmdParams *cmdparams );
void msg_only_opers( const CmdParams *cmdparams );

#endif /* _COMMANDS_H_ */
