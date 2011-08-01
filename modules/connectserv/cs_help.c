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
** $Id: cs_help.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"

const char *cs_about[] = 
{
	"\2ConnectServ\2 tracks users joining and leaving the network,",
	"kills, operator modes, nickname changes, server connects and",
	"server quits. These events can be reported to the services",
	"channel and logged.",
	NULL
};

const char *cs_help_set_exclusions[] = 
{
	"EXCLUSIONS <ON|OFF>",
	"Use the global exclusions before reporting events",
	NULL
};

const char *cs_help_set_logging[] = 
{
	"LOGGING <ON|OFF>",
	"Log connectserv event reports",
	NULL
};

const char *cs_help_set_signwatch[] = 
{
	"SIGNWATCH <ON|OFF>",
	"Report users joining and leaving the network.",
	NULL
};

const char *cs_help_set_killwatch[] = 
{
	"KILLWATCH <ON|OFF>",
	"Report kills.",
	NULL
};

const char *cs_help_set_modewatch[] = 
{
	"MODEWATCH <ON|OFF>",
	"Report operator mode changes.",
	NULL
};

const char *cs_help_set_nickwatch[] = 
{
	"NICKWATCH <ON|OFF>",
	"Report nick changes.",
	NULL
};

const char *cs_help_set_awaywatch[] = 
{
	"AWAYWATCH <ON|OFF>",
	"Report away events.",
	NULL
};

const char *cs_help_set_servwatch[] = 
{
	"SERVWATCH <ON|OFF>",
	"Report server joins and quits.",
	NULL
};

const char *cs_help_set_colour[] = 
{
	"COLOUR <ON|OFF>",
	"Turn on or off Colour reporting.",
	NULL
};
