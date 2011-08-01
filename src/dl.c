/* NeoStats - IRC Statistical Services
** Copyright (c) 1999-2008 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000 - 2001 ^Enigma^
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
** $Id: dl.c 3294 2008-02-24 02:45:41Z Fish $
*/

/** @file dl.c 
 *  @brief module functions
 */ 

/*  TODO:
 *  - Better Win32 error processing for failed modules
 */

#include "neostats.h"
#include "dl.h"

char *ns_dlerrormsg;

void *ns_dlsym (void *handle, const char *name)
{
#ifdef WIN32
	void *ret;

	ret = ( void * )GetProcAddress((HMODULE)handle, name);
	return ret;
#else /* WIN32 */
#ifdef NEED_UNDERSCORE_PREFIX
	static char sym[128];
	void *ret;

	/* reset error */
	ns_dlerrormsg = 0;
	ret = dlsym ((int *) handle, name);
	/* Check with underscore prefix */
	if (ret == NULL) {
		ircsnprintf(sym, 128, "_%s", name);
		ret = dlsym ((int *) handle, sym);
	}
	/* Check for error */
#ifndef HAVE_LIBDL
	if(ret == NULL) {
		ns_dlerrormsg = ns_dlerror ();
#else /* HAVE_LIBDL */
	if ((ns_dlerrormsg = ns_dlerror ()) != NULL) {
#endif /* HAVE_LIBDL */
		nlog (LOG_ERROR, "dl error %s", ns_dlerrormsg);
		return NULL;
	}
	return ret;
#else /* NEED_UNDERSCORE_PREFIX */
	return (dlsym ((int *) handle, name));
#endif /* NEED_UNDERSCORE_PREFIX */
#endif /* WIN32 */
}

void *ns_dlopen (const char *file, int mode)
{
#ifdef WIN32
	void *ret;

	mode = mode; /* supress warning */
	/* reset error */
	ns_dlerrormsg = 0;
	ret = ( void * )LoadLibrary(file);
	/* Check for error */
	if(ret == NULL)
	{
	}
	return ret;
#else /* WIN32 */
	void *ret;

	/* reset error */
	ns_dlerrormsg = 0;
	ret = dlopen (file, mode);
	/* Check for error */
#ifndef HAVE_LIBDL
	if(ret == NULL) {
		ns_dlerrormsg = ns_dlerror ();
#else /* HAVE_LIBDL */
	if ((ns_dlerrormsg = ns_dlerror ()) != NULL) {
#endif /* HAVE_LIBDL */
		nlog (LOG_ERROR, "dl error %s", ns_dlerrormsg);
		return NULL;
	}
	return ret;
#endif /* WIN32 */
}

int ns_dlclose (void *handle)
{
#ifdef WIN32
	FreeLibrary((HMODULE)handle);
	return 0;
#else /* WIN32 */
#ifndef VALGRIND
	return dlclose( handle );
#else /* VALGRIND */
	return NS_SUCCESS;
#endif /* VALGRIND */
#endif /* WIN32 */
}

char *ns_dlerror (void)
{
#ifdef WIN32
	return NULL;
#else /* WIN32 */
	return ((char *)dlerror ());
#endif /* WIN32 */
}

