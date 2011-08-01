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
** $Id: dl.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef _DL_H_
#define _DL_H_

/*
 * dl.h
 * dynamic runtime library loading routines
 */

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif /* HAVE_DLFCN_H */

/* 
 *   Ensure RTLD flags correctly defined
 */
#ifndef RTLD_NOW
#define RTLD_NOW  1 
#endif /* RTLD_NOW */
#ifndef RTLD_NOW
#define RTLD_NOW RTLD_LAZY	/* openbsd deficiency */
#endif /* RTLD_NOW */
#ifndef RTLD_GLOBAL
#define RTLD_GLOBAL 0
#endif /* RTLD_GLOBAL */

#ifdef WIN32
#ifndef NDEBUG
/* The extra d in debug mode uses debug versions of DLLs */
#define MOD_STDEXT	"d.dll"
#else /* NDEBUG */
#define MOD_STDEXT	".dll"
#endif /* NDEBUG */
#else /* WIN32 */
#define MOD_STDEXT	".so"
#endif /* WIN32 */

/* 
 * Prototypes
 */
void *ns_dlsym( void *handle, const char *name );
void *ns_dlopen( const char *file, int mode );
int ns_dlclose( void *handle );
char *ns_dlerror( void );

extern char *ns_dlerrormsg;

#endif /* _DL_H_ */
