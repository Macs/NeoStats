/*  LibLang - Message Translation and Retrival Library 
**  Copyright (c) 2004 Justin Hammond
** 
**  Portions Copyright Mark Hetherington, NeoStats Software
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
** LibLang CVS Identification
** $Id: lang.h 11 2004-08-10 12:34:28Z Fish $
*/
#ifndef LANG_H
#define LANG_H

#ifndef MAXLANG
#define MAXLANG 10
#endif

#ifndef STRSIZE
#define STRSIZE 512
#endif

#ifndef KEYSIZE
/* #define KEYSIZE sizeof(unsigned long) */
#define KEYSIZE 12
#endif

#ifndef LANGNAMESIZE
#define LANGNAMESIZE 64
#endif

#ifndef DEFDATABASE
#define DEFDATABASE "Default"
#endif


struct langdump_struct {
	char key[KEYSIZE];
	char string[STRSIZE];
	char realstring[STRSIZE];
};

typedef struct langdump_struct langdump;

extern struct lang_stats {
	char *lang_list[MAXLANG];
	int noofloadedlanguages;
	int dbhits[MAXLANG];
	int dbfails[MAXLANG];
	int dbupdates[MAXLANG];
	int cachehits[MAXLANG];
	int transfailed[MAXLANG];
} lang_stats;
	

typedef void (*LANGDebugFunc) (const char *fmt, ...);


void LANGSetData( char *key, void *data, int size, char *lang, int keydone );
int LANGGotData( char *key, char *lang );
int LANGDumpDB( char *lang, int missing, void *mylist );
int LANGNewLang( char *lang );
void LANGinit( int debug, char *dbpath, LANGDebugFunc debugfunc );
void LANGfini( void );
int LANGfindlang( char * string);

#ifdef USEGETTEXT
#define _(x) LANGgettext(x, lang)
#define __(x, y) LANGgettext(x, y)
#endif

#endif
