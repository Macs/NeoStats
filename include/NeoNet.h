#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
namespace NeoNetMQ {
#endif

/* message types (what codes) */

#define NNMSG_PING  2000
#define NNMSG_PONG  2001
#define NNMSG_LOGIN 2002
#define NNMSG_ERROR 2003
#define NNMSG_LOGINOK 2004
#define NNMSG_SNDTOUSER 2005
#define NNMSG_MSGFROMUSER 2006
#define NNMSG_SNDTOQUEUE 2007
#define NNMSG_MSGFROMQUEUE 2008
#define NNMSG_GETMSGFROMSTORE 2009

/* field names */
/* NNMSG_PING */
#define NNFLD_SNDTIME "sndtime" /* int64 */
/* NNMSG_PONG */
#define NNFLD_RCVDTIME "rcvdtime" /* int64 */
/* NNMSG_LOGIN */
#define NNFLD_USERNAME "username" /* string */
#define NNFLD_PASSWORD "password" /* string */
#define NNFLD_VERSION "version" /* string (optional) */
#define NNFLD_FLAGS "flags" /* int64 */
#define NNFLD_STOREMSG "storemsg" /* array of stored messages */
#define NNFLD_MSGCOUNT "msgcount" /* int32 */
/* NNMSG_ERROR */
#define NNFLD_STATUS "status" /* int32 */
#define NNFLD_ERROR "error" /* string */
#define NNFLD_FATAL "fatal" /* bool */
#define NNFLD_REF "reference" /* msg (optional) */
/* NNMSG_LOGINOK */
#define NNFLD_GROUP "group" /* string */
/* NNMSG_SNDTOUSER */
#define NNFLD_TARGET "target" /* string */
#define NNFLD_FROM "from" /* string */
#define NNFLD_TOPIC "topic" /* string */
/* NNMSG_MSGFROMUSER */
/* NNMSG_GETMSGFROMSTORE */
#define NNFLD_MAXITEMS "maxitems" /* int32 */


/* NNFLD_STATUS types */
#define NNSTATUS_BADLOGIN -1
#define NNSTATUS_DBERROR -10
#define NNSTATUS_NOUSER -11

/* NNFLD_FLAG types */
#define NNFLAG_DONTSENDSTOREMSG	0x1 /* dont send any stored messages on login, only counts, use NNMSG_GETSTORE instead */

#ifdef __cplusplus
}
#endif



#endif
