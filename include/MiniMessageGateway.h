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
** $Id: MiniMessageGateway.h 3294 2008-02-24 02:45:41Z Fish $
*/

#ifndef MiniMessageGateway_h
#define MiniMessageGateway_h

#include "MiniMessage.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Definition of our opaque handle to a MMessageGateway object.  Your
  * code doesn't know what a (MMessageGateway *) points to, and it doesn't care,
  * because all operations on it should happen via calls to the functions
  * that are defined below.
  */
struct _MMessageGateway;
typedef struct _MMessageGateway MMessageGateway;

/** Typedef for a callback function that knows how to read data from a buffer and send it
  * out to (a file, the network, a serial line, wherever).
  * @param buf The buffer to read bytes from.
  * @param numBytes The number of bytes available for reading at (buf)
  * @param arg This is a user-specified value; it will be the same as the value passed in to MMDoOutput().
  * @returns The number of bytes actually read from (buf), or a negative value if there was a critical error (e.g. disconnected socket).
  */
typedef int32 (*MGSendFunc)(const uint8 * buf, uint32 numBytes, void * arg);

/** Typedef for a callback function that knows how to read data from 
  * (a file, the network, a serial line, wherever) and write it into a supplied buffer.
  * @param buf The buffer to write bytes to.
  * @param numBytes The number of bytes available for writing at (buf)
  * @param arg This is a user-specified value; it will be the same as the value passed in to MMDoInput().
  * @returns The number of bytes actually written into (buf), or a negative value if there was a critical error (e.g. disconnected socket).
  */
typedef int32 (*MGReceiveFunc)(uint8 * buf, uint32 numBytes, void * arg);

/** Allocates and initializes a new MMessageGateway.
  * @returns a newly allocated MMessageGateway, or NULL on failure.  If non-NULL, it becomes the
  *          the responsibility of the calling code to call MMFreeMessageGateway() on the 
  *          MMessageGateway when it is done using it. 
  */
MMessageGateway * MGAllocMessageGateway();

/** Frees a previously created MMessageGateway and all the data that it holds.
  * @param msg The MMessageGateway to free.  If NULL, no action will be taken.
  */
void MGFreeMessageGateway(MMessageGateway * gw);

/** Flattens the given MMessage into bytes and adds the result to our queue of outgoing data.
  * @param gw The Gateway to add the message data to.
  * @param msg The MMessage object to flatten.  Note that the gateway DOES NOT assume ownership of this MMessage!
  *            You are still responsible for freeing it, and may do so immediately on return of this function, if you wish.
  * @returns B_NO_ERROR on success, or B_ERROR on error (out of memory?)
  */
status_t MGAddOutgoingMessage(MMessageGateway * gw, const MMessage * msg);

/** Returns MTrue iff the given gateway has any output bytes queued up, that it wants to send.
  * @param gw The Gateway to query.
  * @returns MTrue if there are bytes queued up to send, or MFalse otherwise.
  */
MBool MGHasBytesToOutput(const MMessageGateway * gw);

/** Writes out as many queued bytes as possible (up to maxBytes).
  * @param gw The Gateway that should do the outputting.
  * @param maxBytes The maximum number of bytes that should be sent by this function call.  Pass in ~0 to write without limit.
  * @param sendFunc The function that the gateway way will call to actually do the write operation.
  * @param arg The argument to pass to the write function.
  * @returns The number of bytes written, or a negative number if there was an error.
  */
int32 MGDoOutput(MMessageGateway * gw, uint32 maxBytes, MGSendFunc sendFunc, void * arg);

/** Reads in as many queued bytes as possible (up to maxBytes, or until a full MMessage is read).
  * @param gw The Gateway that should do the inputting.
  * @param maxBytes The maximum number of bytes that should be read by this function call.  Pass in ~0 to read without limit.
  * @param recvFunc The function that the gateway way will call to actually do the read operation.
  * @param arg The argument to pass to the read function.
  * @param optRetMsg If non-NULL, the pointer this argument points to will be set to a returned MMessage object
  *                  if there is one ready, or NULL otherwise.  NOTE:  If the pointer is set non-NULL, it becomes
  *                  the calling code's responsibility to call MMFreeMessage() on the pointer when you are done with it!
  *                  Failure to do so will result in a memory leak.
  * @returns The number of bytes read, or a negative number if there was an error.
  */
int32 MGDoInput(MMessageGateway * gw, uint32 maxBytes, MGReceiveFunc recvFunc, void * arg, MMessage ** optRetMsg);

#ifdef __cplusplus
};
#endif

#endif
