/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2008 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Portions Copyright (c) 2000-2008 ^Enigma^
**
**  Portions Copyright (c) 1999 Johnathan George net@lite.net
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
** $Id: sock.c 3294 2008-02-24 02:45:41Z Fish $
*/

#include "neostats.h"
#include "main.h"
#ifdef HAVE_FCNTL_H
#include <fcntl.h> 
#endif /* HAVE_FCNTL_H */
#include "transfer.h"
#include "services.h"
#include "ircprotocol.h"
#include "sock.h"


/* @brief Module Socket List hash */
static hash_t *sockethash;

char recbuf[BUFSIZE];

/* temp till we figure out our DelSock Issues */
void do_backtrace();

/** @brief Event Subsystem Callback
 */
static void libevent_log(int severity, const char *msg) 
{
	switch (severity) {
		case _EVENT_LOG_DEBUG:
			dlog(DEBUG10, "LibEvent: %s", msg);
			break;
		case _EVENT_LOG_MSG:
			nlog(LOG_INFO, "LibEvent: %s", msg);
			break;
		case _EVENT_LOG_WARN:
			nlog(LOG_WARNING, "LibEvent: %s", msg);
			break;
		case _EVENT_LOG_ERR:
			nlog(LOG_ERROR, "LibEvent: %s", msg);
			break;
		default:
			nlog(LOG_WARNING, "LibEvent(%d): %s", severity, msg);
			break;
	}
}      

/** @brief Called when we receive a error from the ircd socket 
  * 
  * @param what the type of error
  * @data not used.
  *
  * @return Never Returns. Just exits
  */

static int irc_sock_error( int what, void *data )
{
	nlog (LOG_CRITICAL, "Error from IRCd Socket: %s", strerror(errno));
	/* Try to close socket then reset the servsock value to avoid cyclic calls */
	DelSock(me.servsock);
	me.servsock = NULL;
	/* XXX really exit? */
	do_exit (NS_EXIT_ERROR, NULL);
	return 0;
}

/** @brief Connect to a IRC server. Only used internally. 
 *
 * @param host to connect to
 * @param port on remote host to connect to
 * 
 * @return socket connected to on success
 *         NS_FAILURE on failure 
 */
static OS_SOCKET ConnectTo( const char *host, int port )
{
	struct hostent *hp;
	OS_SOCKET s;

	if ((hp = gethostbyname (host)) == NULL)
	{
		return NS_FAILURE;
	}
	os_memset( &me.srvip, 0, sizeof( me.srvip ) );
	me.srvip.sin_family = AF_INET;
	me.srvip.sin_port = htons (port);
	os_memcpy( ( char * ) &me.srvip.sin_addr, hp->h_addr, hp->h_length );
	if( ( s = os_sock_socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
		return NS_FAILURE;
	}
	if( me.dobind )
	{
		os_sock_bind( s, ( struct sockaddr * ) &me.lsa, sizeof( me.lsa ) );
	}
	if( os_sock_connect( s, ( struct sockaddr * ) &me.srvip, sizeof( me.srvip ) ) != 0 ) 
	{
		os_sock_close (s);
		return NS_FAILURE;
	}
	return s;
}

/** @brief main recv loop
 *
 * @param none
 * 
 * @return none
 */
#ifndef WIN32
static
#endif
void read_loop( void )
{
	me.lastmsg = me.now;
	for( ; ; ) /* loop till we get a error */
	{
		SET_SEGV_LOCATION();
		update_time_now();
#ifdef CURLHACK
		event_loop(EVLOOP_ONCE);
		SET_SEGV_LOCATION();
		/* this is a hack till CURL gets the new socket code */
		CurlHackReadLoop();
#else /* CURLHACK */
		event_dispatch();
#endif /* CURLHACK */
	}
}

/** @brief Connects to IRC and starts the main loop
 *
 * Connects to the IRC server and attempts to login
 * If it connects and logs in, then starts the main program loop
 * if control is returned to this function, restart
 * 
 * @return Nothing
 *
 * @todo make the restart code nicer so it doesn't go mad when we can't connect
 */
void Connect( void )
{
	OS_SOCKET mysock;

	SET_SEGV_LOCATION();
	nlog (LOG_NOTICE, "Connecting to %s:%d", me.uplink, me.port);
	mysock = ConnectTo (me.uplink, me.port);
	if (mysock == INVALID_SOCKET) {
		nlog (LOG_WARNING, "Unable to connect to %s", me.uplink);
	} else {
		me.servsock=add_linemode_socket("IRCd", mysock, irc_parse, irc_sock_error, NULL);
		/* Call the IRC specific function send_server_connect to login as a server to IRC */
		irc_connect (me.name, me.numeric, me.infoline, nsconfig.pass, me.ts_boot, me.now);
#ifndef WIN32
		read_loop ();
#endif
	}
}


/** @brief connect to a socket
 *
 * @param socktype type of socket
 * @param ipaddr ip address of target
 * @param port to connect to
 * 
 * @return socket number if connect successful
 *         -1 if unsuccessful
 */
OS_SOCKET sock_connect( int socktype, struct in_addr ip, int port )
{
	struct sockaddr_in sa;
	OS_SOCKET s;
	int flags = 1;

	os_memset( &sa, 0, sizeof( sa ) );
	sa.sin_family = AF_INET;
	sa.sin_port = htons (port);
	sa.sin_addr.s_addr = ip.s_addr;
	if( ( s = os_sock_socket( AF_INET, socktype, 0 ) ) < 0 )
		return NS_FAILURE;
	/* bind to an IP address */
	if (me.dobind) 
		os_sock_bind( s, ( struct sockaddr * ) &me.lsa, sizeof( me.lsa ) );
	/* set non blocking */
	if( os_sock_set_nonblocking( s ) < 0 )
		return NS_FAILURE;
	os_sock_setsockopt( s, SOL_SOCKET, SO_REUSEADDR, (char *)&flags, sizeof( flags ) );
	if( os_sock_connect( s, ( struct sockaddr * ) &sa, sizeof( sa ) ) != 0 ) 
	{
		if( os_sock_errno != OS_SOCK_EWOULDBLOCK && os_sock_errno != OS_SOCK_EINPROGRESS )
		{
			os_sock_close (s);
			return -1;
		}
	}
	return s;
}

/** @brief send to a linemode based socket
 * 
 * @param sock the Socket to send to
 * @param buf the charactor string to send
 * @param buflen the lenght of data to send
 *
 * @return NS_SUCCESS or NS_FAILURE
 */

int send_to_sock( Sock *sock, const char *buf, size_t buflen )
{
	int sent;

	if (!sock) {
		nlog(LOG_WARNING, "Not sending to socket as we have a invalid socket");
		return NS_FAILURE;
	}
	if ((sock->socktype == SOCK_BUFFERED) || (sock->socktype == SOCK_LINEMODE)) {
    	/* the linemode socket is a buffered socket */
    	sent = bufferevent_write(sock->event.buffered, (void *)buf, buflen);
    	if (sent == -1) {
	    	nlog (LOG_CRITICAL, "Write error: %s", strerror(errno));
	    	/* Try to close socket then reset the servsock value to avoid cyclic calls */
			sock->sfunc.linemode.errcb(-1, sock->data);	
    		DelSock(sock);
	    	return NS_FAILURE;		
    	}
    } else if ((sock->socktype == SOCK_NATIVE) || (sock->socktype == SOCK_STANDARD)) {
        sent = os_sock_write(sock->sock_no, buf, buflen);
        if (sent == -1) {
			sock->sfunc.standmode.readfunc(sock->data, NULL, -1);
	    	/* Try to close socket then reset the servsock value to avoid cyclic calls */
    		DelSock(sock);
	    	return NS_FAILURE;		
        }
    } else {
        nlog(LOG_CRITICAL, "send_to_sock: ehhhh. Tried to write to a sock that doesn't support write: %s", sock->name);
        return NS_FAILURE;
    }          
	sock->smsgs++;
	sock->sbytes += buflen;
	return sent;
}


#undef SOCKDEBUG 

/** @brief This function actually handles reading data from our sockets
 *  if the socket is a Linemode socket, it reads the data from our event buffer and 
 *  places the data in a temporary buffer assocated with this socket
 *  it then checks thetemporary buffer for new line charactors and if found
 *  sends the line of text to the function specified when this socket was created
 *
 * @param bufferevent the bufferevent structure direct from the event subsystem
 * @param arg is actually the Sock structure for this socket.
 * 
 * @return Nothing
 *
 */
static void linemode_read(struct bufferevent *bufferevent, void *arg)
{
	Sock *thisock = (Sock*)arg;

	char buf[512];
	size_t len;
	int bufpos;
#ifdef SOCKDEBUG
	printf("Start Reading\n");
#endif

	while ((len = bufferevent_read(bufferevent, buf, 512)) > 0) {
#ifdef SOCKDEBUG
		printf("Buffer: %d |%s|\n\n", thisock->linemode->readbufsize, thisock->linemode->readbuf);
		printf("ReceivedSize %d |%s|\n\n", len, buf);
#endif
		if (len > 0) {
			/* firstly, if what we have read plus our existing buffer size exceeds our recvq
			 * we should drop the connection
			 */
			if ((len + thisock->sfunc.linemode.readbufsize) > thisock->sfunc.linemode.recvq) {
				nlog(LOG_ERROR, "RecvQ for %s(%s) exceeded. Dropping Connection", thisock->name, thisock->moduleptr->info->name);
				thisock->sfunc.linemode.errcb(-1, thisock->data);	
				DelSock(thisock);
				return;
			}
			thisock->rbytes += len;

			for (bufpos = 0; bufpos < len; bufpos++) {
				/* if the static buffer now contains newline chars,
				 * we are ready to parse the line 
				 * else we just keep putting the received charactors onto the buffer
				 */
				if ((buf[bufpos] == '\n') || (buf[bufpos] == '\r')) {
					/* only process if we have something to do. This might be a trailing NL or CR char*/
					if (thisock->sfunc.linemode.readbufsize > 0) {
						/* update some stats */
						thisock->rmsgs++;
#if 0
						me.lastmsg = me.now;
#endif
						/* make sure its null terminated */
						thisock->sfunc.linemode.readbuf[thisock->sfunc.linemode.readbufsize++] = '\0';
						/* copy it to our recbuf for debugging */
						strlcpy(recbuf, thisock->sfunc.linemode.readbuf, BUFSIZE);
						/* parse the buffer */
#ifdef SOCKDEBUG
						printf("line:|%s| %d %d\n", thisock->sfunc.linemode.readbuf, thisock->sfunc.linemode.readbufsize, bufpos);
#endif
						if(thisock->sfunc.linemode.funccb(thisock->data, thisock->sfunc.linemode.readbuf, thisock->sfunc.linemode.readbufsize) == NS_FAILURE) {
						    DelSock(thisock);
                        }
						/* ok, reset the recbuf */
						thisock->sfunc.linemode.readbufsize = 0;
					}
				} else {
					/* add it to our static buffer */
					thisock->sfunc.linemode.readbuf[thisock->sfunc.linemode.readbufsize] = buf[bufpos];
					thisock->sfunc.linemode.readbufsize++;
				}
			}
			thisock->sfunc.linemode.readbuf[thisock->sfunc.linemode.readbufsize] = '\0';
		}
	}
}


/** @brief This function signals that the socket has sent all available data 
 *  in the write buffer. We don't actually use it Currently, but we could
 *
 * @param bufferevent the bufferevent structure direct from the event subsystem
 * @param arg is actually the Sock structure for this socket.
 * 
 * @return Nothing
 *
 */

static void socket_linemode_write_done (struct bufferevent *bufferevent, void *arg)
{
/* NOOP - We require this otherwise the event subsystem segv's */
}


/** @brief This function signals that a error has occured on the socket
 *  such as EOF etc. We use this to detect disconnections etc rather than 
 *  having to worry about the return code from a read function call
 *
 * @param bufferevent the bufferevent structure direct from the event subsystem
 * @param what indicates what caused this error, a read or write call
 * @param arg is actually the Sock structure for this socket.
 * 
 * @return Nothing
 *
 */

static void socket_linemode_error(struct bufferevent *bufferevent, short what, void *arg)
{
	Sock *sock = (Sock*)arg;
	if (what & EVBUFFER_READ) 
		what &= ~EVBUFFER_READ;
	if (what & EVBUFFER_WRITE)
		what &= ~EVBUFFER_WRITE;
	
	if ((what & EVBUFFER_EOF) || (what & EVBUFFER_ERROR) || (what & EVBUFFER_TIMEOUT)) {
		nlog(LOG_ERROR, "LinemodeSock Error: %d (%s)", what, strerror(errno));
	} else {
		nlog(LOG_ERROR, "Unknown Error from Socket: %d (%s)", what, strerror(errno));
	}	
	sock->sfunc.linemode.errcb(what, sock->data);	
	DelSock(sock);
}
	

/** @brief Init the socket subsystem
 *
 * 
 * @return Nothing
 *
 */

int InitSocks (void)
{
	sockethash = hash_create (me.maxsocks, 0, 0);
	if(!sockethash) {
		nlog (LOG_CRITICAL, "Unable to create socks hash");
		return NS_FAILURE;
	}
	event_set_log_callback(libevent_log);
	event_init();
	return NS_SUCCESS;
}


/** @brief Finish up the socket subsystem
 *
 * 
 * @return Nothing
 *
 */

void FiniSocks (void) 
{
	Sock *sock = NULL;
	hscan_t ss;
	hnode_t *sn;

	hash_scan_begin (&ss, sockethash);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		sock = hnode_get (sn);
		DelSock(sock);
	}
	if (me.servsock) {
		me.servsock = NULL;
	}
	hash_destroy(sockethash);
	event_base_free(NULL);
}

/** @brief create a new socket
 *
 * For core use only, creates a new socket for a module
 *
 * @param sock_name the name of the socket to create
 * 
 * @return pointer to created socket on success, NULL on error
 */
static Sock *new_sock(const char *sock_name)
{
	Sock *sock;

	SET_SEGV_LOCATION();
	if (hash_isfull( sockethash ) ) 
	{
		nlog( LOG_CRITICAL, "new_sock: socket hash is full" );
		return NULL;
	}
	dlog( DEBUG2, "new_sock: %s", sock_name );
	sock = ns_calloc( sizeof( Sock ) );
	strlcpy( sock->name, sock_name, MAX_MOD_NAME );
	sock->moduleptr = GET_CUR_MODULE();
	hnode_create_insert( sockethash, sock, sock->name );
	me.cursocks++;
	return sock;
}

/** @brief find socket
 *
 * For core use only, finds a socket in the current list of socket
 *
 * @param sock_name the name of socket to find
 * 
 * @return pointer to socket if found, NULL if not found
 */
Sock *FindSock (const char *sock_name)
{
	Sock *sock;

	sock = (Sock *)hnode_find (sockethash, sock_name);
	if (!sock) {
		dlog (DEBUG3, "FindSock: %s not found!", sock_name);		
	}
	return sock;
}

/** @brief create a new socket that's protocol is based on lines
 *
 * This sets up the core to create a buffered, newline terminated communication. 
 * The socket connection should have already been established, and socknum should be a valid
 * socket. 
 *
 * @param sock_name the name of the new socket
 * @param socknum the Socket number, from the socket function call
 * @param readcb function prototype that is called when a new "newline" terminated string is received.
 * @param errcb function to call when we get a error, such as EOF 
 * 
 * @return pointer to socket structure for reference to future calls
 */

Sock *
add_linemode_socket(const char *sock_name, OS_SOCKET socknum, sockfunccb readcb, sockcb errcb, void *arg) 
{
	Sock *sock;

	sock = add_buffered_socket(sock_name, socknum, linemode_read, socket_linemode_write_done, socket_linemode_error, arg);
	if (sock) {
		sock->sfunc.linemode.readbuf = ns_malloc(nsconfig.recvq);
		sock->sfunc.linemode.recvq = nsconfig.recvq;
		sock->sfunc.linemode.funccb = readcb;
		sock->sfunc.linemode.errcb = errcb;
		sock->sfunc.linemode.readbufsize = 0;
		sock->socktype = SOCK_LINEMODE;
		dlog(DEBUG3, "add_linemode_socket: Added a new Linemode Socket called %s (%s)", sock_name, sock->moduleptr->info->name);
	}
	return sock;
}


/** @brief create a new socket that's input and output are buffered.
 *
 * This sets up the core to create a buffered communication. 
 * The socket connection should have already been established, and socknum should be a valid
 * socket. 
 *
 * @param sock_name the name of the new socket
 * @param socknum the Socket number, from the socket function call
 * @param readcb function prototype that is called when a new buffered data is ready.
 * @param writecb function to call that indicates we have sent all buffered data
 * @param errcb function to call when we get a error, such as EOF 
 * 
 * @return pointer to socket structure for reference to future calls
 */

Sock *
add_buffered_socket(const char *sock_name, OS_SOCKET socknum, evbuffercb readcb, evbuffercb writecb, everrorcb errcb, void *arg) {
	Sock *sock;

	SET_SEGV_LOCATION();
	if (!readcb) {
		nlog(LOG_WARNING, "add_buffered_socket: read buffer function doesn't exist = %s (%s)", sock_name, GET_CUR_MODNAME() );
		return NULL;
	}
	if (!errcb) {
		nlog(LOG_WARNING, "add_buffered_socket: error function doesn't exist = %s (%s)", sock_name, GET_CUR_MODNAME() );
		return NULL;
	}
	if (!writecb) {
		nlog(LOG_WARNING, "add_buffered_socket: write function doesn't exist = %s (%s)", sock_name, GET_CUR_MODNAME() );
		return NULL;
    }  
	
	sock = new_sock(sock_name);
	sock->sock_no = socknum;
	sock->socktype = SOCK_BUFFERED;
	sock->data = arg;
	
	/* add the buffered socket to the core event subsystem */
	sock->event.buffered = bufferevent_new(sock->sock_no, readcb, writecb, errcb, sock);
	if (!sock->event.buffered) {
		nlog(LOG_WARNING, "bufferevent_new() failed");
		DelSock(sock);
		return NULL;
	}
	bufferevent_enable(sock->event.buffered, EV_READ|EV_WRITE);	
	bufferevent_setwatermark(sock->event.buffered, EV_READ|EV_WRITE, 0, 0);
	dlog(DEBUG3, "add_buffered_sock: Registered Module %s with Standard Socket functions %s", GET_CUR_MODNAME(), sock->name);
	return sock;
}

/** @brief call the function to accept a new connection and handle it gracefully!
 *
 * This is for use only in the core, and not by modules
 *
 * @param fd the socket number
 * @param what ignored
 * @param arg is a pointer to the Sock structure for the listening socket
 * 
 * @return nothing
 */

static void
listen_accept_sock(int fd, short what, void *arg) {
	Sock *sock = (Sock *)arg;
	
	SET_SEGV_LOCATION();
	dlog(DEBUG1, "Got Activity on Listen Socket %d port %d (%s)", sock->sock_no, sock->sfunc.listenmode.port, sock->name);
	if (sock->sfunc.listenmode.funccb(sock->sock_no, sock->data) == NS_SUCCESS) {
		/* re-add this listen socket if the acceptcb succeeds */
		event_add(sock->event.event, NULL);
		sock->rmsgs++;
	} else {
		dlog(DEBUG1, "Deleting Listen Socket %d port %d (%s)", sock->sock_no, sock->sfunc.listenmode.port, sock->name);
		DelSock(sock);
	}
}

/** @brief create a new socket to listen on a port (with standard bindings)
 *
 * This creates a new socket that will listen on a standard port and will automatically 
 * bind to the local host if specified in the configuration
 *
 * @param sock_name the socket name
 * @param port a integer of the port to listen on.
 * @param acceptcb the function to call to accept a new connection. Responsible for actually accepting the connection
 * and createing the new Socket interface
 * @param data A void pointer that is passed back to the user via the acceptcb callback
 * 
 * @return A socket struct that represents the new Listening Socket
 */

Sock *
add_listen_sock(const char *sock_name, const int port, int type, sockcb acceptcb, void *data) {
	OS_SOCKET s;
	struct sockaddr_in srvskt;
	Sock *sock;
	
	SET_SEGV_LOCATION();
	os_memset( ( void * ) &srvskt, 0, sizeof( struct sockaddr_in ) );
	srvskt.sin_family = AF_INET;
	/* bind to the local IP */
	if (me.dobind)
		srvskt.sin_addr = me.lsa.sin_addr;
	else
		srvskt.sin_addr.s_addr = INADDR_ANY;
	srvskt.sin_port = htons( port );
	if( ( s = os_sock_socket( AF_INET, type, 0 ) ) < 0 )
	{
		return NULL;
	}
	os_sock_set_nonblocking( s );
	if( os_sock_bind( s, ( struct sockaddr * ) &srvskt, sizeof( struct sockaddr_in ) ) < 0 )
	{
		return NULL;
	}
	if( os_sock_listen( s, 1 ) < 0 )
	{
		return NULL;
	}
	sock = new_sock(sock_name);
	sock->sock_no = s;
	sock->socktype = SOCK_LISTEN;
	sock->data = data;
	sock->sfunc.listenmode.port = port;
	sock->sfunc.listenmode.funccb = acceptcb;
	sock->event.event = ns_malloc(sizeof(struct event));	
	event_set(sock->event.event, sock->sock_no, EV_READ, listen_accept_sock, (void*) sock);
	event_add(sock->event.event, NULL);	
	return sock;
}

/** @brief Read Data from a "STANDARD" Socket
 *
 * This function reads data from a standard socket and will handle all necessary errors
 * When it has read the data, it calls the callback with the data that has been read from the socket 
 *
 * @param fd the socket number
 * @param what ignored
 * @param data pointer to the Sock Structure for the the socket thats being read currently
 * 
 * @return Nothing
 */

static void read_sock_activity(int fd, short what, void *data)
{
	Sock *sock = (Sock *)data;
	char *p = NULL;
	int n;
	size_t howmuch = READBUFSIZE;

	if (what & EV_READ) { 	
		if( os_sock_ioctl( sock->sock_no, FIONREAD, &howmuch ) == -1 )
			howmuch = READBUFSIZE;
#if SOCKDEBUG
		printf("read called with %d bytes %d\n", howmuch, what);
#endif
		/* rmsgs is just a counter for how many times we read in the standard sockets */
		sock->rbytes += howmuch;
		sock->rmsgs++;
		if (sock->socktype == SOCK_STANDARD) {
   			if (howmuch > 0) {
    				p = ns_malloc(howmuch);
	    		}
		    	n = os_sock_read(sock->sock_no, p, howmuch);
   			if (n == -1 || n == 0) {
    				dlog(DEBUG1, "sock_read: failed %s", sock->name);
	    			sock->sfunc.standmode.readfunc(sock->data, NULL, -1);
		    		DelSock(sock);
				return;
   			}
			dlog(DEBUG1, "sock_read: Read %d bytes from fd %d (%s)", n, sock->sock_no, sock->name);
		} else {
			p = NULL;
			n = howmuch;
		}
		if (sock->sfunc.standmode.readfunc(sock->data, p, n) == NS_FAILURE) {
			dlog(DEBUG1, "sock_read_activity: Read Callback failed, Closing Socket on fd %d (%s)", sock->sock_no, sock->name);
			DelSock(sock);
			return;
		}
	} else if (what & EV_WRITE) {
		if ((sock->sfunc.standmode.writefunc(fd, sock->data)) == NS_FAILURE) {
			dlog(DEBUG1, "sock_read: Write Callback Failed, Closing Socket on fd %d (%s)", sock->sock_no, sock->name);
			DelSock(sock);
			return;
		}
	} else if (what & EV_TIMEOUT) {
		dlog(DEBUG1, "sock_read_activity: Timeout on %d (%s)", sock->sock_no, sock->name);
		if (sock->sfunc.standmode.readfunc(sock->data, NULL, -2) == NS_FAILURE) {
			dlog(DEBUG1, "sock_read: Timeout Read Callback Failed, Closing Socket on fd %d (%s)", sock->sock_no, sock->name);
			DelSock(sock);
			return;
		}
	} else {
		nlog(LOG_WARNING, "Error, Unknown State in sock_read_activity %d", what);
		sock->sfunc.standmode.readfunc(sock->data, NULL, -1);
		DelSock(sock);
	}
}

/** @brief update the read/write calls for this socket
 *
 * Updates the event subsystem if we want to signal we are now interested
 * in new events 
 *
 * @param sock The socket structure we want to update
 * @param what the new events we want to monitor 
 * 
 * @return success or failure
*/
int 
UpdateSock(Sock *sock, short what, short reset, struct timeval *tv) {

	if (reset) {
		dlog(DEBUG1, "Event for fd %d (%s) has been reset", sock->sock_no, sock->name);
		event_del(sock->event.event);
	}
	if ((!sock->sfunc.standmode.readfunc) && (what & EV_READ)) {
		nlog (LOG_WARNING, "UpdateSock: read socket function doesn't exist = %s (%s)", sock->name, sock->moduleptr->info->name);
		return NS_FAILURE;
	}
	if ((!sock->sfunc.standmode.writefunc) && (what & EV_WRITE)) {
		nlog (LOG_WARNING, "UpdateSock: write socket function doesn't exist = %s (%s)", sock->name, sock->moduleptr->info->name);
		return NS_FAILURE;
	}
	if ((!tv) && (what & EV_TIMEOUT)) {
		nlog(LOG_WARNING, "UpdateSock: Timeout wanted but not specified on %s (%s)", sock->name, sock->moduleptr->info->name);
		return NS_FAILURE;
	}
	event_set(sock->event.event, sock->sock_no, what, read_sock_activity, (void*) sock);
	event_add(sock->event.event, tv);
	dlog(DEBUG1, "Updated Socket Info %s with new event %d", sock->name, what);
	return NS_SUCCESS;
}
/** @brief add a socket to the socket list as a "Standard", or native socket
 *
 * For core use. Adds a socket with the given functions to the socket list
 *
 * @param readfunc the name of read function to register with this socket
 * @param writefunc the name of write function to register with this socket
 * @param sock_name the name of socket to register
 * @param socknum the socket number to register with this socket
 * @param data User supplied data for tracking purposes.
 * 
 * @return pointer to socket if found, NULL if not found
*/
Sock *AddSock( SOCK_TYPE type, const char *sock_name, int socknum, sockfunccb readfunc, sockcb writefunc, short what, void *data, struct timeval *tv )
{
	Sock *sock;

	SET_SEGV_LOCATION();
	if (type != SOCK_STANDARD && type != SOCK_NATIVE) {
		nlog(LOG_ERROR, "AddSock: incorrect socket type. Module: %s, socket name: %s", GET_CUR_MODNAME(), sock_name);
		return NULL;
    }
	if ((!readfunc) && (what & EV_READ)) {
		nlog (LOG_WARNING, "AddSock: read socket function doesn't exist = %s (%s)", sock_name, GET_CUR_MODNAME());
		return NULL;
	}
	if ((!writefunc) && (what & EV_WRITE)) {
		nlog (LOG_WARNING, "AddSock: write socket function doesn't exist = %s (%s)", sock_name, GET_CUR_MODNAME());
		return NULL;
	}
	if ((!tv) && (what & EV_TIMEOUT)) {
		nlog(LOG_WARNING, "AddSock: Timeout wanted but not specified on %s (%s)", sock_name, GET_CUR_MODNAME());
		return NULL;
	}
	sock = new_sock (sock_name);
	if (!sock)
		return NULL;
	sock->socktype = type;
	sock->sock_no = socknum;
	sock->data = data;
	sock->sfunc.standmode.readfunc = readfunc;
	sock->sfunc.standmode.writefunc = writefunc;
	sock->event.event = ns_malloc(sizeof(struct event));
	event_set(sock->event.event, sock->sock_no, what, read_sock_activity, (void*) sock);
	event_add(sock->event.event, tv);
	dlog(DEBUG2, "AddSock: Registered Module %s with Standard Socket functions %s", GET_CUR_MODNAME(), sock->name);
	return sock;
}

/** @brief CloseSock
 *
 *  Close socket and free event information
 *
 *  @param sock
 * 
 *  @return none
 */

static void CloseSock( const Sock *sock )
{
	switch (sock->socktype) {
		case SOCK_STANDARD:
		case SOCK_LISTEN:
		case SOCK_NATIVE:
			event_del(sock->event.event);
			/* needed? */
			os_free(sock->event.event);
			os_sock_close (sock->sock_no);
			break;
		case SOCK_LINEMODE:
			os_free(sock->sfunc.linemode.readbuf);
            /* fallthrough */
		case SOCK_BUFFERED:
			bufferevent_free(sock->event.buffered);
			os_sock_close (sock->sock_no);
			break;
	}
}

/** @brief delete a socket from the socket list
 *
 * For module use. Deletes a socket with the given name from the socket list
 *
 * @param socket_name the name of socket to delete
 * 
 * @return NS_SUCCESS if deleted, NS_FAILURE if not found
*/
int DelSock( Sock *sock )
{
	hnode_t *sn;

	SET_SEGV_LOCATION();
	if (sock == NULL) {
		/* its a double Del! Oh Oh. Print a BackTrace! */
		nlog(LOG_WARNING, "DelSock: Double Delete?");
		CaptureBackTrace(__FILE__, __LINE__, __FUNCTION__);
		return NS_SUCCESS;
	}
	CloseSock( sock );
	if( ( sn = hash_lookup( sockethash, sock->name ) ) != NULL ) {
		sock = hnode_get( sn );
		dlog( DEBUG2, "DelSock: deleting socket %s from module %s", sock->name, sock->moduleptr->info->name );
		hash_delete_destroy_node( sockethash, sn );
		ns_free( sock );
		sock = NULL;
		me.cursocks--;
		return NS_SUCCESS;
	}
	return NS_FAILURE;
}

/** @brief delete a socket from the socket list for a module
 *
 * For module use. Deletes a socket with the given name from the socket list
 *
 * @param socket_name the name of socket to delete
 * 
 * @return NS_SUCCESS if deleted, NS_FAILURE if not found
*/
int del_sockets( const Module *mod_ptr )
{
	Sock *sock;
	hnode_t *socknode;
	hscan_t hscan;

	hash_scan_begin( &hscan, sockethash );
	while( ( socknode = hash_scan_next( &hscan ) ) != NULL ) 
	{
		sock = hnode_get( socknode );
		if( sock->moduleptr == mod_ptr ) 
		{
			dlog( DEBUG1, "del_sockets: deleting socket %s from module %s", sock->name, mod_ptr->info->name );
   			CloseSock( sock );
			hash_scan_delete_destroy_node( sockethash, socknode );
			ns_free( sock );
		}
	}
	return NS_SUCCESS;
}

/** @brief list sockets in use
 *
 * NeoStats command to list the current sockets from IRC
 *
 * @param u pointer to user structure of the user issuing the request
 * 
 * @return none
*/
int ns_cmd_socklist (const CmdParams *cmdparams)
{
	Sock *sock = NULL;
	hscan_t ss;
	hnode_t *sn;
	int size;
	struct sockaddr_in add;
	
	SET_SEGV_LOCATION();
	irc_prefmsg (ns_botptr, cmdparams->source, __("Sockets List: (%d)", cmdparams->source), (int)hash_count (sockethash));
	hash_scan_begin (&ss, sockethash);
	while ((sn = hash_scan_next (&ss)) != NULL) {
		sock = hnode_get (sn);
		irc_prefmsg (ns_botptr, cmdparams->source, "%s:--------------------------------", sock->moduleptr->info->name);
		irc_prefmsg (ns_botptr, cmdparams->source, __("Socket Name: %s", cmdparams->source), sock->name);
		size = sizeof(struct sockaddr_in);
		if(getsockname(sock->sock_no, (struct sockaddr *) &add, (socklen_t *)&size) > -1) {
			irc_prefmsg (ns_botptr, cmdparams->source, "Local Socket: %s:%d", inet_ntoa(add.sin_addr), ntohs(add.sin_port));
		}
		size = sizeof(struct sockaddr_in);
		/* this will not print anything if the socket is not connected */
		if (getpeername(sock->sock_no, (struct sockaddr *)&add, (socklen_t *)&size) > -1) {
			irc_prefmsg (ns_botptr, cmdparams->source, "Remote Socket: %s:%hu", inet_ntoa(add.sin_addr), ntohs(add.sin_port));
		}

		switch (sock->socktype) {
			case SOCK_STANDARD:
				irc_prefmsg (ns_botptr, cmdparams->source, __("Standard Socket - fd: %d", cmdparams->source), sock->sock_no);
				break;
			case SOCK_NATIVE:
				irc_prefmsg (ns_botptr, cmdparams->source, __("Native Socket - fd: %d", cmdparams->source), sock->sock_no);
				break;
			case SOCK_BUFFERED:
				irc_prefmsg (ns_botptr, cmdparams->source, __("Buffered Socket - fd: %d", cmdparams->source), sock->sock_no);
				break;
			case SOCK_LINEMODE:
				irc_prefmsg (ns_botptr, cmdparams->source, __("LineMode Socket - fd: %d", cmdparams->source), sock->sock_no);
				irc_prefmsg (ns_botptr, cmdparams->source, __("ReceiveQ Set: %d Current: %d", cmdparams->source), (int)sock->sfunc.linemode.recvq, (int)sock->sfunc.linemode.readbufsize);	
				break;
			case SOCK_LISTEN:
				irc_prefmsg (ns_botptr, cmdparams->source, __("Listen Socket - fd %d Port %d", cmdparams->source), sock->sock_no, sock->sfunc.listenmode.port);
				irc_prefmsg (ns_botptr, cmdparams->source, __("Accepted Connections: %ld", cmdparams->source), sock->rmsgs);
				break;
		}
		irc_prefmsg (ns_botptr, cmdparams->source, __("Received Bytes: %ld, Received Messages: %ld", cmdparams->source), sock->rbytes, sock->rmsgs);
		irc_prefmsg (ns_botptr, cmdparams->source, __("Sent Bytes: %ld, Sent Messages: %ld", cmdparams->source), sock->sbytes, sock->smsgs);
	}
	irc_prefmsg (ns_botptr, cmdparams->source, __("End of Socket List", cmdparams->source));
	return NS_SUCCESS;
}
