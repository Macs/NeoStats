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
** $Id: oscalls.c 2366 2005-03-18 12:29:37Z Fish $
*/

/* @file Portability wrapper functions
 */

#include "neostats.h"
#ifndef WIN32
#ifdef HAVE_FCNTL_H
#include <fcntl.h> 
#endif /* HAVE_FCNTL_H */
#endif /* WIN32 */

/* local errno implementation */
int os_sock_errno = 0;

#ifdef WIN32
#define OS_SOCK_SET_ERRNO()	os_sock_errno = WSAGetLastError();
#else /* WIN32 */
#define OS_SOCK_SET_ERRNO() os_sock_errno = errno;
#endif /* WIN32 */

int get_os_sock_errno() {
	return os_sock_errno;
}

char *os_sock_strerror( int sockerrno )
{
#ifdef WIN32
	static char unknown[256];

	switch( sockerrno )
	{
		case 0:
			return "No Error";
		case WSAEINTR:
			/* 10004 */
			return "Interrupted function call.";
			/* A blocking operation was interrupted by a call to WSACancelBlockingCall. */
		case WSAEACCES:
			/* 10013 */
			return "Permission denied.";
			/* An attempt was made to access a socket in a way forbidden by its access permissions. An example is using a broadcast address for sendto without broadcast permission being set using setsockopt(SO_BROADCAST). Another possible reason for the WSAEACCES error is that when the bind function is called (on Windows NT 4 SP4 or later), another application, service, or kernel mode driver is bound to the same address with exclusive access. Such exclusive access is a new feature of Windows NT 4 SP4 and later, and is implemented by using the SO_EXCLUSIVEADDRUSE option. */
		case WSAEFAULT:
			/* 10014 */
			return "Bad address.";
			/* The system detected an invalid pointer address in attempting to use a pointer argument of a call. This error occurs if an application passes an invalid pointer value, or if the length of the buffer is too small. For instance, if the length of an argument, which is a sockaddr structure, is smaller than the sizeof(sockaddr).  */
		case WSAEINVAL:
			/* 10022 */
			return "Invalid argument.";
			/* Some invalid argument was supplied (for example, specifying an invalid level to the setsockopt function). In some instances, it also refers to the current state of the socket—for instance, calling accept on a socket that is not listening.  */
		case WSAEMFILE:
			/* 10024 */
			return "Too many open files.";
			/* Too many open sockets. Each implementation may have a maximum number of socket handles available, either globally, per process, or per thread.  */
		case WSAEWOULDBLOCK:
			/* 10035 */
			return "Resource temporarily unavailable.";
			/* This error is returned from operations on nonblocking sockets that cannot be completed immediately, for example recv when no data is queued to be read from the socket. It is a nonfatal error, and the operation should be retried later. It is normal for WSAEWOULDBLOCK to be reported as the result from calling connect on a nonblocking SOCK_STREAM socket, since some time must elapse for the connection to be established.  */
		case WSAEINPROGRESS:
			/* 10036 */
			return "Operation now in progress.";
			/* A blocking operation is currently executing. Windows Sockets only allows a single blocking operation—per- task or thread—to be outstanding, and if any other function call is made (whether or not it references that or any other socket) the function fails with the WSAEINPROGRESS error.  */
		case WSAEALREADY:
			/* 10037 */
			return "Operation already in progress.";
			/* An operation was attempted on a nonblocking socket with an operation already in progress—that is, calling connect a second time on a nonblocking socket that is already connecting, or canceling an asynchronous request (WSAAsyncGetXbyY) that has already been canceled or completed.  */
		case WSAENOTSOCK:
			/* 10038 */
			return "Socket operation on nonsocket.";
			/* An operation was attempted on something that is not a socket. Either the socket handle parameter did not reference a valid socket, or for select, a member of an fd_set was not valid.  */
		case WSAEDESTADDRREQ:
			/* 10039 */
			return "Destination address required.";
			/* A required address was omitted from an operation on a socket. For example, this error is returned if sendto is called with the remote address of ADDR_ANY.  */
		case WSAEMSGSIZE:
			/* 10040 */
			return "Message too long.";
			/* A message sent on a datagram socket was larger than the internal message buffer or some other network limit, or the buffer used to receive a datagram was smaller than the datagram itself.  */
		case WSAEPROTOTYPE:
			/* 10041 */
			return "Protocol wrong type for socket.";
			/* A protocol was specified in the socket function call that does not support the semantics of the socket type requested. For example, the ARPA Internet UDP protocol cannot be specified with a socket type of SOCK_STREAM.  */
		case WSAENOPROTOOPT:
			/* 10042 */
			return "Bad protocol option.";
			/* An unknown, invalid or unsupported option or level was specified in a getsockopt or setsockopt call.  */
		case WSAEPROTONOSUPPORT:
			/* 10043 */
			return "Protocol not supported.";
			/* The requested protocol has not been configured into the system, or no implementation for it exists. For example, a socket call requests a SOCK_DGRAM socket, but specifies a stream protocol.  */
		case WSAESOCKTNOSUPPORT:
			/* 10044 */
			return "Socket type not supported.";
			/* The support for the specified socket type does not exist in this address family. For example, the optional type SOCK_RAW might be selected in a socket call, and the implementation does not support SOCK_RAW sockets at all.  */
		case WSAEOPNOTSUPP:
			/* 10045 */
			return "Operation not supported.";
			/* The attempted operation is not supported for the type of object referenced. Usually this occurs when a socket descriptor to a socket that cannot support this operation is trying to accept a connection on a datagram socket.  */
		case WSAEPFNOSUPPORT:
			/* 10046 */
			return "Protocol family not supported.";
			/* The protocol family has not been configured into the system or no implementation for it exists. This message has a slightly different meaning from WSAEAFNOSUPPORT. However, it is interchangeable in most cases, and all Windows Sockets functions that return one of these messages also specify WSAEAFNOSUPPORT.  */
		case WSAEAFNOSUPPORT:
			/* 10047 */
			return "Address family not supported by protocol family.";
			/* An address incompatible with the requested protocol was used. All sockets are created with an associated address family (that is, AF_INET for Internet Protocols) and a generic protocol type (that is, SOCK_STREAM). This error is returned if an incorrect protocol is explicitly requested in the socket call, or if an address of the wrong family is used for a socket, for example, in sendto.  */
		case WSAEADDRINUSE:
			/* 10048 */
			return "Address already in use.";
			/* Typically, only one usage of each socket address (protocol/IP address/port) is permitted. This error occurs if an application attempts to bind a socket to an IP address/port that has already been used for an existing socket, or a socket that was not closed properly, or one that is still in the process of closing. For server applications that need to bind multiple sockets to the same port number, consider using setsockopt (SO_REUSEADDR). 
			   Client applications usually need not call bind at all—connect chooses an unused port automatically. When bind is called with a wildcard address (involving ADDR_ANY), a WSAEADDRINUSE error could be delayed until the specific address is committed. This could happen with a call to another function later, including connect, listen, WSAConnect, or WSAJoinLeaf.  */
		case WSAEADDRNOTAVAIL:
			/* 10049 */
			return "Cannot assign requested address.";
			/* The requested address is not valid in its context. This normally results from an attempt to bind to an address that is not valid for the local computer. This can also result from connect, sendto, WSAConnect, WSAJoinLeaf, or WSASendTo when the remote address or port is not valid for a remote computer (for example, address or port 0).  */
		case WSAENETDOWN:
			/* 10050 */
			return "Network is down.";
			/* A socket operation encountered a dead network. This could indicate a serious failure of the network system (that is, the protocol stack that the Windows Sockets DLL runs over), the network interface, or the local network itself.  */
		case WSAENETUNREACH:
			/* 10051 */
			return "Network is unreachable.";
			/* A socket operation was attempted to an unreachable network. This usually means the local software knows no route to reach the remote host.  */
		case WSAENETRESET:
			/* 10052 */
			return "Network dropped connection on reset.";
			/* The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress. It can also be returned by setsockopt if an attempt is made to set SO_KEEPALIVE on a connection that has already failed.  */
		case WSAECONNABORTED:
			/* 10053 */
			return "Software caused connection abort.";
			/* An established connection was aborted by the software in your host computer, possibly due to a data transmission time-out or protocol error.  */
		case WSAECONNRESET:
			/* 10054 */
			return "Connection reset by peer.";
			/* An existing connection was forcibly closed by the remote host. This normally results if the peer application on the remote host is suddenly stopped, the host is rebooted, the host or remote network interface is disabled, or the remote host uses a hard close (see setsockopt for more information on the SO_LINGER option on the remote socket). 
			  This error may also result if a connection was broken due to keep-alive activity detecting a failure while one or more operations are in progress. Operations that were in progress fail with WSAENETRESET. Subsequent operations fail with WSAECONNRESET.  */
		case WSAENOBUFS:
			/* 10055 */
			return "No buffer space available.";
			/* An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full.  */
		case WSAEISCONN:
			/* 10056 */
			return "Socket is already connected.";
			/* A connect request was made on an already-connected socket. Some implementations also return this error if sendto is called on a connected SOCK_DGRAM socket (for SOCK_STREAM sockets, the to parameter in sendto is ignored) although other implementations treat this as a legal occurrence.  */
		case WSAENOTCONN:
			/* 10057 */
			return "Socket is not connected.";
			/* A request to send or receive data was disallowed because the socket is not connected and (when sending on a datagram socket using sendto) no address was supplied. Any other type of operation might also return this error—for example, setsockopt setting SO_KEEPALIVE if the connection has been reset.  */
		case WSAESHUTDOWN:
			/* 10058 */
			return "Cannot send after socket shutdown.";
			/* A request to send or receive data was disallowed because the socket had already been shut down in that direction with a previous shutdown call. By calling shutdown a partial close of a socket is requested, which is a signal that sending or receiving, or both have been discontinued.  */
		case WSAETIMEDOUT:
			/* 10060 */
			return "Connection timed out.";
			/* A connection attempt failed because the connected party did not properly respond after a period of time, or the established connection failed because the connected host has failed to respond.  */
		case WSAECONNREFUSED:
			/* 10061 */
			return "Connection refused.";
			/* No connection could be made because the target computer actively refused it. This usually results from trying to connect to a service that is inactive on the foreign host—that is, one with no server application running.  */
		case WSAEHOSTDOWN:
			/* 10064 */
			return "Host is down.";
			/* A socket operation failed because the destination host is down. A socket operation encountered a dead host. Networking activity on the local host has not been initiated. These conditions are more likely to be indicated by the error WSAETIMEDOUT.  */
		case WSAEHOSTUNREACH:
			/* 10065 */
			return "No route to host.";
			/* A socket operation was attempted to an unreachable host. See WSAENETUNREACH.  */
		case WSAEPROCLIM:
			/* 10067 */
			return "Too many processes.";
			/* A Windows Sockets implementation may have a limit on the number of applications that can use it simultaneously. WSAStartup may fail with this error if the limit has been reached.  */
		case WSASYSNOTREADY:
			/* 10091 */
			return "Network subsystem is unavailable.";
			/* This error is returned by WSAStartup if the Windows Sockets implementation cannot function at this time because the underlying system it uses to provide network services is currently unavailable. Users should check: 
			That the appropriate Windows Sockets DLL file is in the current path. 
			That they are not trying to use more than one Windows Sockets implementation simultaneously. If there is more than one Winsock DLL on your system, be sure the first one in the path is appropriate for the network subsystem currently loaded. 
			The Windows Sockets implementation documentation to be sure all necessary components are currently installed and configured correctly. 
			*/ 
		case WSAVERNOTSUPPORTED:
			/* 10092 */
			return "Winsock.dll version out of range.";
			/* The current Windows Sockets implementation does not support the Windows Sockets specification version requested by the application. Check that no old Windows Sockets DLL files are being accessed.  */
		case WSANOTINITIALISED:
			/* 10093 */
			return "Successful WSAStartup not yet performed.";
			/* Either the application has not called WSAStartup or WSAStartup failed. The application may be accessing a socket that the current active task does not own (that is, trying to share a socket between tasks), or WSACleanup has been called too many times.  */
		case WSAEDISCON:
			/* 10101 */
			return "Graceful shutdown in progress.";
			/* Returned by WSARecv and WSARecvFrom to indicate that the remote party has initiated a graceful shutdown sequence.  */
		case WSATYPE_NOT_FOUND:
			/* 10109 */
			return "Class type not found.";
			/* The specified class was not found.  */
		case WSAHOST_NOT_FOUND:
			/* 11001 */
			return "Host not found.";
			/* No such host is known. The name is not an official host name or alias, or it cannot be found in the database(s) being queried. This error may also be returned for protocol and service queries, and means that the specified name could not be found in the relevant database.  */
		case WSATRY_AGAIN:
			/* 11002 */
			return "Nonauthoritative host not found.";
			/* This is usually a temporary error during host name resolution and means that the local server did not receive a response from an authoritative server. A retry at some time later may be successful.  */
		case WSANO_RECOVERY:
			/* 11003 */
			return "This is a nonrecoverable error.";
			/* This indicates some sort of nonrecoverable error occurred during a database lookup. This may be because the database files (for example, BSD-compatible HOSTS, SERVICES, or PROTOCOLS files) could not be found, or a DNS request was returned by the server with a severe error.  */
		case WSANO_DATA:
			/* 11004 */
			return "Valid name, no data record of requested type.";
			/* The requested name is valid and was found in the database, but it does not have the correct associated data being resolved for. The usual example for this is a host name-to-address translation attempt (using gethostbyname or WSAAsyncGetHostByName) which uses the DNS (Domain Name Server). An MX record is returned but no A record—indicating the host itself exists, but is not directly reachable.  */
		case WSA_INVALID_HANDLE:
			/* OS dependent */
			return "Specified event object handle is invalid.";
			/* An application attempts to use an event object, but the specified handle is not valid.  */
		case WSA_INVALID_PARAMETER:
			/* OS dependent */
			return "One or more parameters are invalid.";
			/* An application used a Windows Sockets function which directly maps to a Windows function. The Windows function is indicating a problem with one or more parameters.  */
		case WSA_IO_INCOMPLETE:
			/* OS dependent */
			return "Overlapped I/O event object not in signaled state.";
			/* The application has tried to determine the status of an overlapped operation which is not yet completed. Applications that use WSAGetOverlappedResult (with the fWait flag set to FALSE) in a polling mode to determine when an overlapped operation has completed, get this error code until the operation is complete.  */
		case WSA_IO_PENDING:
			/* OS dependent */
			return "Overlapped operations will complete later.";
			/* The application has initiated an overlapped operation that cannot be completed immediately. A completion indication will be given later when the operation has been completed.  */
		case WSA_NOT_ENOUGH_MEMORY:
			/* OS dependent */
			return "Insufficient memory available.";
			/* An application used a Windows Sockets function that directly maps to a Windows function. The Windows function is indicating a lack of required memory resources.  */
		case WSA_OPERATION_ABORTED:
			/* OS dependent */
			return "Overlapped operation aborted.";
			/* An overlapped operation was canceled due to the closure of the socket, or the execution of the SIO_FLUSH command in WSAIoctl.  */
#ifdef WSAINVALIDPROCTABLE
		case WSAINVALIDPROCTABLE:
			/* OS dependent */
			return "Invalid procedure table from service provider.";
			/* A service provider returned a bogus procedure table to Ws2_32.dll. (Usually caused by one or more of the function pointers being null.)  */
#endif /* WSAINVALIDPROCTABLE */
#ifdef WSAINVALIDPROVIDER
		case WSAINVALIDPROVIDER:
			/* OS dependent */
			return "Invalid service provider version number.";
			/* A service provider returned a version number other than 2.0.  */
#endif /* WSAINVALIDPROVIDER */
#ifdef WSAPROVIDERFAILEDINIT
		case WSAPROVIDERFAILEDINIT:
			/* OS dependent */
			return "Unable to initialize a service provider.";
			/* Either a service provider's DLL could not be loaded (LoadLibrary failed) or the provider's WSPStartup/NSPStartup function failed.  */
#endif /* WSAPROVIDERFAILEDINIT */
		case WSASYSCALLFAILURE:
			/* OS dependent */
			return "System call failure.";
			/* Generic error code, returned under various conditions. 
			Returned when a system call that should never fail does fail. For example, if a call to WaitForMultipleEvents fails or one of the registry functions fails trying to manipulate the protocol/namespace catalogs.
			Returned when a provider does not return SUCCESS and does not provide an extended error code. Can indicate a service provider implementation error.
			*/
		default:
			break;
	}
	sprintf( unknown, "Unknown error %d", sockerrno );
	return unknown;
#else /* WIN32 */
	return strerror( sockerrno );
#endif /* WIN32 */
}

/*
 *  
 */

char *os_sock_getlasterrorstring( void )
{
	return os_sock_strerror( os_sock_errno );
}

/*
 *  Wrapper function for close
 */

int os_sock_close( OS_SOCKET sock )
{
#ifdef WIN32
	return closesocket( sock );
#else
	return close( sock );
#endif      			
}

/*
 *  Wrapper function for write
 */

int os_sock_write( OS_SOCKET s, const char *buf, int len )
{
	int ret;

	/* reset local errno implementation */
	os_sock_errno = 0;
#ifdef WIN32
	ret = send( s, buf, len, 0 );
#else
	ret = write( s, buf, len );
#endif
	if( ret == SOCKET_ERROR )
	{
		OS_SOCK_SET_ERRNO();
		nlog( LOG_ERROR, "os_sock_write: failed for socket %d with error %s", s, os_sock_strerror( os_sock_errno ) );
	}
	return ret;
}

/*
 *  Wrapper function for sendto
 */

int os_sock_sendto( OS_SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen )
{
	int ret;

	/* reset local errno implementation */
	os_sock_errno = 0;
	ret = sendto( s, buf, len, flags, to, tolen );
	if( ret == SOCKET_ERROR )
	{
		OS_SOCK_SET_ERRNO();
		nlog( LOG_ERROR, "os_sock_sendto: failed for socket %d with error %s", s, os_sock_strerror( os_sock_errno ) );
	}
	return ret;
}

/*
 *  Wrapper function for read
 */

int os_sock_read( OS_SOCKET s, char *buf, int len )
{
	int ret;

	/* reset local errno implementation */
	os_sock_errno = 0;
#ifdef WIN32
	ret = recv( s, buf, len, 0 );
#else
	ret = read( s, buf, len );
#endif
	if( ret == SOCKET_ERROR )
	{
		OS_SOCK_SET_ERRNO();
		switch( os_sock_errno )
		{
			case OS_SOCK_EWOULDBLOCK:
			case OS_SOCK_EINPROGRESS:
				/* non-fatal errors */
				return 0;
			default:
				nlog( LOG_ERROR, "os_sock_read: failed for socket %d with error %s", s, os_sock_strerror( os_sock_errno ) );
		}	
	}
	return ret;
}

/*
 *  Wrapper function for recvfrom
 */

int os_sock_recvfrom( OS_SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen )
{
	int ret;

	/* reset local errno implementation */
	os_sock_errno = 0;
	ret = recvfrom( s, buf, len, flags, from, (socklen_t *)fromlen );
	if( ret == SOCKET_ERROR )
	{
		OS_SOCK_SET_ERRNO();
		switch( os_sock_errno )
		{
			case OS_SOCK_EWOULDBLOCK:
			case OS_SOCK_EINPROGRESS:
				/* don't log */
				break;
			default:
				nlog( LOG_ERROR, "os_sock_recvfrom: failed for socket %d with error %s", s, os_sock_strerror( os_sock_errno ) );
		}	
	}
	return ret;
}

/*
 *  Wrapper function for set_nonblocking
 */

int os_sock_set_nonblocking( OS_SOCKET s )
{
#ifdef WIN32
	int ret;
	unsigned long flags;

	/* reset local errno implementation */
	os_sock_errno = 0;
	flags = 1;
	ret = ioctlsocket( s, FIONBIO, &flags );
#else
	int ret;
	int flags;

	/* reset local errno implementation */
	os_sock_errno = 0;
	flags = fcntl( s, F_GETFL, 0 );
	flags |= O_NONBLOCK;
	ret = fcntl( s, F_SETFL, flags );
#endif
	if( ret == SOCKET_ERROR )
	{
		OS_SOCK_SET_ERRNO();
		nlog( LOG_ERROR, "os_sock_set_nonblocking: unable to set socket %d for %s non-blocking %s", s, GET_CUR_MODNAME(), os_sock_strerror( os_sock_errno ) );
	}
	return ret;
}

/*
 *  Wrapper function for connect
 */

int os_sock_connect( OS_SOCKET s, const struct sockaddr *name, int namelen )
{
	int ret;

	/* reset local errno implementation */
	os_sock_errno = 0;
	ret = connect( s, name, namelen );
	if( ret == SOCKET_ERROR )
	{
		OS_SOCK_SET_ERRNO();
		switch( os_sock_errno )
		{
			case OS_SOCK_EWOULDBLOCK:
			case OS_SOCK_EINPROGRESS:
				/* don't log */
				break;
			default:
				nlog( LOG_ERROR, "os_sock_connect: %s failed for socket %d with error %s", GET_CUR_MODNAME(), s, os_sock_strerror( os_sock_errno ) );
		}	
	}
	return ret;
}

/*
 *  Wrapper function for socket
 */

OS_SOCKET os_sock_socket(int socket_family, int socket_type, int protocol )
{
	OS_SOCKET s;

	/* reset local errno implementation */
	os_sock_errno = 0;
	s = socket( socket_family, socket_type, protocol );
	if( s == INVALID_SOCKET )
	{
		OS_SOCK_SET_ERRNO();
		nlog( LOG_ERROR, "os_sock_socket: %s failed with error %s", GET_CUR_MODNAME(), os_sock_strerror( os_sock_errno ) );
	}
	return s;
}

/*
 *  Wrapper function for bind
 */

int os_sock_bind( OS_SOCKET s, const struct sockaddr *name, int namelen )
{
	int ret;

	/* reset local errno implementation */
	os_sock_errno = 0;
	ret = bind( s, name, namelen );
	if( ret == SOCKET_ERROR )
	{
		OS_SOCK_SET_ERRNO();
		nlog( LOG_WARNING, "os_sock_bind: Unable to bind to IP address %s", os_sock_strerror( os_sock_errno ) );
	}
	return ret;
}

/*
 *  Wrapper function for listen
 */

int os_sock_listen( OS_SOCKET s, int backlog )
{
	int ret;

	/* reset local errno implementation */
	os_sock_errno = 0;
	ret = listen( s, backlog );
	if( ret == SOCKET_ERROR )
	{
		OS_SOCK_SET_ERRNO();
		nlog( LOG_WARNING, "os_sock_listen: Unable to listen %s", os_sock_strerror( os_sock_errno ) );
	}
	return ret;
}

/*
 *  Wrapper function for setsockopt
 */

int os_sock_setsockopt( OS_SOCKET s, int level, int optname, const char *optval, int optlen )
{
	int ret;

	/* reset local errno implementation */
	os_sock_errno = 0;
	ret = setsockopt( s, level, optname, optval, optlen );
	if( ret == SOCKET_ERROR )
	{
		OS_SOCK_SET_ERRNO();
		nlog( LOG_WARNING, "setsockopt: failed %s", os_sock_strerror( os_sock_errno ) );
	}
	return ret;	
}

/*
 *  Wrapper function for ioctl
 */

int os_sock_ioctl( OS_SOCKET s, int cmd, void *argp )
{
#ifdef WIN32
	return ioctlsocket( s, cmd, argp );
#else
	return ioctl( s, cmd, argp );
#endif
}

/*
 *  Wrapper function for select
 */

int os_sock_select( int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout )
{
	int ret;

	/* reset local errno implementation */
	os_sock_errno = 0;
	ret = select( nfds, readfds, writefds, exceptfds, timeout );
	if( ret == SOCKET_ERROR )
	{
		OS_SOCK_SET_ERRNO();
		nlog( LOG_WARNING, "os_sock_select: select failed %s", os_sock_strerror( os_sock_errno ) );
	}
	return ret;
}

