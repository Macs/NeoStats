/* NeoStats - IRC Statistical Services 
** Copyright (c) 1999-2008 Adam Rutter, Justin Hammond, Mark Hetherington
** http://www.neostats.net/
**
**  Based on adns, which is
**    Copyright (C) 1997-2008 Ian Jackson <ian@davenant.greenend.org.uk>
**    Copyright (C) 1999-2008 Tony Finch <dot@dotat.at>
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
** $Id: adnstransmit.c 3294 2008-02-24 02:45:41Z Fish $
*/
/*
 * transmit.c
 * - construct queries
 * - send queries
 */

#include "adnsinternal.h"
#ifndef WIN32
#include <sys/uio.h>
#endif
#include "adnstvarith.h"

#define MKQUERY_START(vb) (rqp= (vb)->buf+(vb)->used)
#define MKQUERY_ADDB(b) *rqp++= (b)
#define MKQUERY_ADDW(w) (MKQUERY_ADDB(((w)>>8)&0x0ff), MKQUERY_ADDB((w)&0x0ff))
#define MKQUERY_STOP(vb) ((vb)->used= rqp-(vb)->buf)

static adns_status mkquery_header(adns_state ads, vbuf * vb, int *id_r,
				  int qdlen)
{
	int id;
	byte *rqp;

	if (!adns__vbuf_ensure(vb, DNS_HDRSIZE + qdlen + 4))
		return adns_s_nomemory;

	vb->used = 0;
	MKQUERY_START(vb);

	*id_r = id = (ads->nextid++) & 0x0ffff;
	MKQUERY_ADDW(id);
	MKQUERY_ADDB(0x01);	/* QR=Q(0), OPCODE=QUERY(0000), !AA, !TC, RD */
	MKQUERY_ADDB(0x00);	/* !RA, Z=000, RCODE=NOERROR(0000) */
	MKQUERY_ADDW(1);	/* QDCOUNT=1 */
	MKQUERY_ADDW(0);	/* ANCOUNT=0 */
	MKQUERY_ADDW(0);	/* NSCOUNT=0 */
	MKQUERY_ADDW(0);	/* ARCOUNT=0 */

	MKQUERY_STOP(vb);

	return adns_s_ok;
}

static adns_status mkquery_footer(vbuf * vb, adns_rrtype type)
{
	byte *rqp;

	MKQUERY_START(vb);
	MKQUERY_ADDW(type & adns__rrt_typemask);	/* QTYPE */
	MKQUERY_ADDW(DNS_CLASS_IN);	/* QCLASS=IN */
	MKQUERY_STOP(vb);
	assert(vb->used <= vb->avail);

	return adns_s_ok;
}

adns_status adns__mkquery(adns_state ads, vbuf * vb, int *id_r,
			  const char *owner, int ol,
			  const typeinfo * typei, adns_queryflags flags)
{
	int ll, c, nbytes;
	byte label[255], *rqp;
	const char *p, *pe;
	adns_status st;

	st = mkquery_header(ads, vb, id_r, ol + 2);
	if (st)
		return st;

	MKQUERY_START(vb);

	p = owner;
	pe = owner + ol;
	nbytes = 0;
	while (p != pe) {
		ll = 0;
		while (p != pe && (c = *p++) != '.') {
			if (c == '\\') {
				if (!(flags & adns_qf_quoteok_query))
					return adns_s_querydomaininvalid;
				if (ctype_digit(p[0])) {
					if (ctype_digit(p[1])
					    && ctype_digit(p[2])) {
						c = (p[0] - '0') * 100 +
						    (p[1] - '0') * 10 +
						    (p[2] - '0');
						p += 3;
						if (c >= 256)
							return adns_s_querydomaininvalid;
					} else {
						return adns_s_querydomaininvalid;
					}
				} else if (!(c = *p++)) {
					return adns_s_querydomaininvalid;
				}
			}
			if (!(flags & adns_qf_quoteok_query)) {
				if (c == '-') {
					if (!ll)
						return
						    adns_s_querydomaininvalid;
				} else if (!ctype_alpha(c)
					   && !ctype_digit(c)) {
					return adns_s_querydomaininvalid;
				}
			}
			if (ll == sizeof(label))
				return adns_s_querydomaininvalid;
			label[ll++] = c;
		}
		if (!ll)
			return adns_s_querydomaininvalid;
		if (ll > DNS_MAXLABEL)
			return adns_s_querydomaintoolong;
		nbytes += ll + 1;
		if (nbytes >= DNS_MAXDOMAIN)
			return adns_s_querydomaintoolong;
		MKQUERY_ADDB(ll);
		os_memcpy(rqp, label, ll);
		rqp += ll;
	}
	MKQUERY_ADDB(0);

	MKQUERY_STOP(vb);

	st = mkquery_footer(vb, typei->type);

	return adns_s_ok;
}

adns_status adns__mkquery_frdgram(adns_state ads, vbuf * vb, int *id_r,
				  const byte * qd_dgram, int qd_dglen,
				  int qd_begin, adns_rrtype type,
				  adns_queryflags flags)
{
	byte *rqp;
	findlabel_state fls;
	int lablen, labstart;
	adns_status st;

	st = mkquery_header(ads, vb, id_r, qd_dglen);
	if (st)
		return st;

	MKQUERY_START(vb);

	adns__findlabel_start(&fls, ads, -1, 0, qd_dgram, qd_dglen,
			      qd_dglen, qd_begin, 0);
	for (;;) {
		st = adns__findlabel_next(&fls, &lablen, &labstart);
		assert(!st);
		if (!lablen)
			break;
		assert(lablen < 255);
		MKQUERY_ADDB(lablen);
		os_memcpy(rqp, qd_dgram + labstart, lablen);
		rqp += lablen;
	}
	MKQUERY_ADDB(0);

	MKQUERY_STOP(vb);

	st = mkquery_footer(vb, type);

	return adns_s_ok;
}

void adns__querysend_tcp(adns_query qu, struct timeval now)
{
	byte length[2];
#ifndef WIN32
	struct iovec iov[2];
#endif
	int wr, r;
	adns_state ads;

	if (qu->ads->tcpstate != server_ok)
		return;

	assert(qu->state == query_tcpw);

	length[0] = (qu->query_dglen & 0x0ff00U) >> 8;
	length[1] = (qu->query_dglen & 0x0ff);

	ads = qu->ads;
	if( !adns__vbuf_ensure( &ads->tcpsend, ads->tcpsend.used + qu->query_dglen + 2 ) )
		return;

	qu->retries++;

	/* Reset idle timeout. */
	ads->tcptimeout.tv_sec = ads->tcptimeout.tv_usec = 0;

	if (ads->tcpsend.used) {
		wr = 0;
	} else {
#ifdef WIN32
		char *buf = NULL;
		
		buf = (char *)ns_malloc((2 + qu->query_dglen));
		os_memcpy(buf, length, 2);
		os_memcpy((buf + 2), qu->query_dgram, qu->query_dglen);
		wr = os_sock_write(qu->ads->tcpsocket, buf, (2 + qu->query_dglen));
		errno = os_sock_errno;
		ns_free(buf);
#else
		iov[0].iov_base = length;
		iov[0].iov_len = 2;
		iov[1].iov_base = qu->query_dgram;
		iov[1].iov_len = qu->query_dglen;
		adns__sigpipe_protect(qu->ads);
		wr = writev(qu->ads->tcpsocket, iov, 2);
#endif
		adns__sigpipe_unprotect(qu->ads);
		if (wr < 0) {
			if (!
			    (errno == EAGAIN || errno == OS_SOCK_EINTR
			     || errno == ENOSPC || errno == OS_SOCK_ENOBUFS
			     || errno == ENOMEM)) {
				adns__tcp_broken(ads, "write",
						 strerror(errno));
				return;
			}
			wr = 0;
		}
	}

	if( wr < 2 )
	{
		r = adns__vbuf_append( &ads->tcpsend, length, 2 - wr );
		assert( r );
		wr = 0;
	} 
	else 
	{
		wr -= 2;
	}
	if ( wr < qu->query_dglen ) 
	{
		r = adns__vbuf_append( &ads->tcpsend, qu->query_dgram + wr,
				      qu->query_dglen - wr );
		assert( r );
	}
}

static void query_usetcp( adns_query qu, struct timeval now )
{
	qu->state = query_tcpw;
	qu->timeout = now;
	timevaladd( &qu->timeout, TCPWAITMS );
	ALIST_LINK_TAIL( qu->ads->tcpw, qu );
	adns__querysend_tcp( qu, now );
	adns__tcp_tryconnect( qu->ads, now );
}

void adns__query_send( adns_query qu, struct timeval now )
{
	struct sockaddr_in servaddr;
	int serv, r;
	adns_state ads;

	assert( qu->state == query_tosend );
	if( ( qu->flags & adns_qf_usevc ) || ( qu->query_dglen > DNS_MAXUDP ) ) 
	{
		query_usetcp( qu, now );
		return;
	}
	if( qu->retries >= UDPMAXRETRIES ) 
	{
		adns__query_fail( qu, adns_s_timeout );
		return;
	}

	serv = qu->udpnextserver;
	os_memset( &servaddr, 0, sizeof( servaddr ) );

	ads = qu->ads;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr = ads->servers[serv].addr;
	servaddr.sin_port = htons(DNS_PORT);

	r = os_sock_sendto( ads->udpsocket, (char *)qu->query_dgram, qu->query_dglen, 0,
		   ( const struct sockaddr * ) &servaddr, sizeof( servaddr ) );
	if( r < 0 && os_sock_errno == OS_SOCK_EMSGSIZE ) 
	{
		qu->retries = 0;
		query_usetcp( qu, now );
		return;
	}
	if( r < 0 && os_sock_errno!= OS_SOCK_EAGAIN )
		adns__warn( ads, serv, 0, "sendto failed: %s", os_sock_getlasterrorstring() );

	qu->timeout = now;
	timevaladd( &qu->timeout, UDPRETRYMS );
	qu->udpsent |= ( 1 << serv );
	qu->udpnextserver = ( serv + 1 ) % ads->nservers;
	qu->retries++;
	ALIST_LINK_TAIL( ads->udpw, qu );
}
