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
** $Id: dns.c 3294 2008-02-24 02:45:41Z Fish $
*/


/* this file does the dns checking for adns. it provides a callback mechinism for dns lookups
** so that DNS lookups will not block. It uses the adns libary (installed in the adns directory
*/

#include "neostats.h"
#include "dns.h"
#include "services.h"
#include "event.h"
#ifdef HAVE_POLL_H
#include <poll.h>
#endif /* HAVE_POLL_H */
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif /* HAVE_SYS_TIME_H */

#define DNS_QUEUE_SIZE  300	/* number on concurrent DNS lookups */
#define DNS_DATA_SIZE	255

typedef struct DnsLookup {
	adns_query q;	/**< the ADNS query */
	adns_answer *a;	/**< the ADNS result if we have completed */
	adns_rrtype type; /**< the type we are looking for, only populated if we add to a queue */
	void *data;
	char lookupdata[255]; /**< the look up data, only populated if we add to a queue */
	void (*callback) (void *data, adns_answer * a);
						      /**< a function pointer to call when we have a result */
	Module *modptr;
} DnsLookup;

adns_state nsads;
static struct event *dnstimeout;

static struct DNSStats {
	unsigned int totalq;
	unsigned int maxqueued;
	unsigned int totalqueued;
	unsigned int success;
	unsigned int failure;
} DNSStats;

/** @brief List of DNS queryies
 *  Contains DnsLookup entries 
 */
static list_t *dnslist;

/** @brief list of DNS queries that are queued up
 * 
 */
static list_t *dnsqueue;

/** @brief starts a DNS lookup
 *
 * starts a DNS lookup for str of type type can callback the function
 * when complete. Data is an identifier that is not modified to identify this lookup to the callback function
 *
 * @param str the record to lookup 
 * @param type The type of record to lookup. See adns.h for more details
 * @param callback the function to callback when we are complete
 * @param data a string to pass unmodified to the callback function to help identifing this lookup
 * 
 * @return returns 1 on success, 0 on failure (to add the lookup, not a successful lookup
*/
int dns_lookup (char *str, adns_rrtype type, void (*callback) (void *data, adns_answer * a), void *data)
{
	DnsLookup *dnsdata;
	int status;
	struct sockaddr_in sa;

	SET_SEGV_LOCATION();
	dnsdata = ns_calloc (sizeof (DnsLookup));
	DNSStats.totalq++;
	if (!dnsdata) {
		nlog (LOG_CRITICAL, "DNS: Out of Memory");
		DNSStats.failure++;
		return 0;
	}
	/* set the module name */
	dnsdata->modptr = GET_CUR_MODULE();
	dnsdata->data = data;
	dnsdata->callback = callback;
	dnsdata->type = type;
	strlcpy(dnsdata->lookupdata, str, 254);
	if (list_isfull (dnslist)) {
		dlog(DEBUG1, "DNS: Lookup list is full, adding to queue");
		strlcpy(dnsdata->lookupdata, str, 254);
		lnode_create_append (dnsqueue, dnsdata);
		DNSStats.totalqueued++;
		if (list_count(dnsqueue) > DNSStats.maxqueued) {
			DNSStats.maxqueued = list_count(dnsqueue);
		}
		return NS_SUCCESS;
	}
	if (type == adns_r_ptr) {
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = inet_addr (str);
		status = adns_submit_reverse (nsads, (const struct sockaddr *) &sa, type, adns_qf_owner | adns_qf_cname_loose, NULL, &dnsdata->q);
	} else {
		status = adns_submit (nsads, str, type, adns_qf_owner | adns_qf_cname_loose, NULL, &dnsdata->q);
	}
	if (status) {
		nlog (LOG_WARNING, "DNS: adns_submit error: %s", strerror (status));
		ns_free (dnsdata);
		DNSStats.failure++;
		return 0;
	}
	dlog(DEBUG1, "DNS: Added dns query %s to list", str);
	/* if we get here, then the submit was successful. Add it to the list of queryies */
	lnode_create_append (dnslist, dnsdata);
	return 1;
}

static int adns_read(void *data, void *notused, int len)
{
    Sock *sock = (Sock *)data;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    if (len > 0) {
        adns_processreadable(nsads, sock->sock_no, &tv);
    } else {
        adns_processexceptional(nsads, sock->sock_no, &tv);
    }
    do_dns(0,0,NULL);
    return NS_SUCCESS;
}

static int adns_write(int fd, void *data)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    adns_processwriteable(nsads, fd, &tv);
    do_dns(0,0,NULL);
    return NS_SUCCESS;
}

static void sock_update (int fd, short what) 
{
    static char tmpname[32];
    Sock *sock;
    int what2;

    ircsnprintf(tmpname, 32, "ADNS-%d", fd);
    sock = FindSock(tmpname);
    what2 = EV_PERSIST;
    if (what & POLLIN) {
		what2 |= EV_READ;
    } else if (what & POLLOUT) {
		what2 |= EV_WRITE;
    } else if (what == -1) {
		DelSock(sock);
		return;
    }
    if (sock) {
		UpdateSock(sock, what2, 1, NULL);
		/* just update */
    } else {
		/* its new */
		sock = AddSock( SOCK_NATIVE, tmpname, fd, adns_read, adns_write, what2, sock, NULL );
		sock->data = sock;
    }
}

/** @brief sets up DNS subsystem
 *
 * configures ADNS for use with NeoStats.
 *
 * @return returns 1 on success, 0 on failure
*/
int InitDns (void)
{
	int adnsstart;
	struct timeval tv;
	SET_SEGV_LOCATION();
	dnslist = list_create (DNS_QUEUE_SIZE);
	if (!dnslist) {
		nlog (LOG_CRITICAL, "Unable to create DNS list");
		return NS_FAILURE;
	}
	/* dnsqueue is unlimited. */
	dnsqueue = list_create(LISTCOUNT_T_MAX);
	if (!dnsqueue)
	{
		nlog (LOG_CRITICAL, "Unable to create DNS queue");
		return NS_FAILURE;
	}
#ifndef DEBUG
	adnsstart = adns_init (&nsads, adns_if_noerrprint | adns_if_noautosys, 0, sock_update);
#else
	adnsstart = adns_init (&nsads, adns_if_debug | adns_if_noautosys, 0, sock_update);
#endif
	if (adnsstart) {
		nlog (LOG_CRITICAL, "ADNS init failed: %s", strerror (adnsstart));
		return NS_FAILURE;
	}

	dnstimeout = ns_malloc(sizeof(struct event));
	timerclear(&tv);
	tv.tv_sec = 1;
	event_set(dnstimeout, 0, EV_TIMEOUT|EV_PERSIST, do_dns, NULL);
	event_add(dnstimeout, &tv);

	return NS_SUCCESS;
}

/* @brief Clean up ADNS data when we shutdown 
 *
 */
void FiniDns (void) 
{
	lnode_t *dnsnode;
	DnsLookup *dnsdata;

	SET_SEGV_LOCATION();
	dnsnode = list_first (dnslist);
	while (dnsnode != NULL) {
		dnsdata = lnode_get(dnsnode);
		adns_cancel(dnsdata->q);
		ns_free (dnsdata->a);
		ns_free (dnsdata);
		dnsnode = list_next(dnslist, dnsnode);
	}
	list_destroy_nodes (dnslist);
	list_destroy (dnslist);
	dnsnode = list_first(dnsqueue);
	while (dnsnode != NULL) {
		dnsdata = lnode_get(dnsnode);
		ns_free(dnsdata);
		dnsnode = list_next(dnsqueue, dnsnode);
	}
	list_destroy_nodes (dnsqueue);
	list_destroy (dnsqueue);
	event_del(dnstimeout);
	free(dnstimeout);
	adns_finish(nsads);
}

/** @brief Checks the DNS queue and if we can
 * add new queries to the active DNS queries and remove from Queue 
*/
static void dns_check_queue(void)
{
	lnode_t *dnsnode, *dnsnode2;
	DnsLookup *dnsdata;
	struct sockaddr_in sa;
	int status;
	
	/* first, if the DNSLIST is full, just exit straight away */
	if (list_isfull(dnslist)) {
		dlog(DEBUG2, "DNS list is still full. Can't work on queue");
		return;
	}
	/* if the dnsqueue isn't empty, then lets process some more till we are full again */
	if (!list_isempty(dnsqueue)) {
		dnsnode = list_first(dnsqueue);
		while ((dnsnode != NULL) && (!list_isfull(dnslist))) {
			dnsdata = lnode_get(dnsnode);	
			dlog(DEBUG2, "Moving DNS query from queue to active");
			if (dnsdata->type == adns_r_ptr) {
				sa.sin_family = AF_INET;
				sa.sin_addr.s_addr = inet_addr (dnsdata->lookupdata);
				status = adns_submit_reverse (nsads, (const struct sockaddr *) &sa, dnsdata->type, adns_qf_owner | adns_qf_cname_loose, NULL, &dnsdata->q);
			} else {
				status = adns_submit (nsads, dnsdata->lookupdata, dnsdata->type, adns_qf_owner | adns_qf_cname_loose, NULL, &dnsdata->q);
			}
			if (status) {
				/* delete from queue and delete node */
				nlog (LOG_WARNING, "DNS: adns_submit error: %s", strerror (status));
				ns_free (dnsdata);
				dnsnode2 = dnsnode;
				dnsnode = list_next(dnsqueue, dnsnode);
				list_delete_destroy_node( dnsqueue, dnsnode2 );
				continue;
			}
			/* move from queue to active list */
			dnsnode2 = dnsnode;
			dnsnode = list_next(dnsqueue, dnsnode);
			list_delete(dnsqueue, dnsnode2);
			list_append(dnslist, dnsnode2);
			dlog(DEBUG1, "DNS: Added dns query to list");
		/* while loop */
		}
	/* isempty */
	}
}

/** @brief Canx any DNS queries for modules we might be unloading
 * 
 * @param module name
 * @return Nothing
 */
void canx_dns(Module *modptr) 
{
	lnode_t *dnsnode, *lnode2;
	DnsLookup *dnsdata;

	SET_SEGV_LOCATION();
	dnsnode = list_first (dnslist);
	while (dnsnode != NULL) {
		dnsdata = lnode_get(dnsnode);
		if (dnsdata->modptr == modptr) {
			adns_cancel(dnsdata->q);
			ns_free (dnsdata->a);
			ns_free (dnsdata);
			lnode2 = list_next(dnslist, dnsnode);
			list_delete_destroy_node( dnslist, dnsnode );
			dnsnode = lnode2;
		}
		if (dnsnode == NULL) 
			continue;
		dnsnode = list_next(dnslist, dnsnode);
	}
	dnsnode = list_first(dnsqueue);
	while (dnsnode != NULL) {
		dnsdata = lnode_get(dnsnode);
		if (dnsdata->modptr == modptr) {
			ns_free(dnsdata);
			lnode2 = list_next(dnsqueue, dnsnode);
			list_delete_destroy_node( dnsqueue, dnsnode );
			dnsnode = lnode2;
		}
		dnsnode = list_next(dnsqueue, dnsnode);
	}
	dns_check_queue();
}

/** @brief Checks for Completed DNS queries
 *
 *  Goes through the dnslist of pending queries and calls the callback function for each lookup
 *  with the adns_answer set. Always calls the callback function even if the lookup was unsuccessful
*  its upto the callback function to make check the answer struct to see if it failed or not
 *
 * @return Nothing
*/
void do_dns (int notused, short event, void *arg)
{
	lnode_t *dnsnode, *dnsnode1;
	int status;
	DnsLookup *dnsdata;
	struct timeval tv;
	SET_SEGV_LOCATION();


	/* process timeouts for ADNS */
	gettimeofday(&tv,NULL);
	adns_processtimeouts(nsads, &tv);
    
	/* if the list is empty, no use doing anything */
	if (list_isempty (dnslist)) {
		dns_check_queue();
		return;
	}
	dnsnode = list_first (dnslist);
	while (dnsnode != NULL) {
		/* loop through the list */
		dnsdata = lnode_get (dnsnode);
		status = adns_check (nsads, &dnsdata->q, &dnsdata->a, NULL);
		/* if status == eagain, the lookup hasn't completed yet */
		if (status == EAGAIN) {
			dlog(DEBUG2, "DNS: Lookup hasn't completed for %s", (char *) &dnsdata->lookupdata);
			dnsnode = list_next (dnslist, dnsnode);
		} else if (status) {
			nlog (LOG_CRITICAL, "DNS: Bad error on adns_check: %s. Please report to NeoStats", strerror (status));
			irc_chanalert (ns_botptr, "Bad Error on DNS lookup. Please check logfile");
			DNSStats.failure++;
			/* call the callback function with answer set to NULL */
			SET_RUN_LEVEL(dnsdata->modptr);
			dnsdata->callback (dnsdata->data, NULL);
			RESET_RUN_LEVEL();
			/* delete from list */
			dnsnode1 = dnsnode;
			dnsnode = list_next (dnslist, dnsnode);
			ns_free (dnsdata->a);
			ns_free (dnsdata);
			list_delete_destroy_node( dnslist, dnsnode1 );
		} else {
			dlog(DEBUG1, "DNS: Calling callback function for lookup %s", dnsdata->lookupdata);
			DNSStats.success++;
			/* call the callback function */
			SET_RUN_LEVEL(dnsdata->modptr);
			dnsdata->callback (dnsdata->data, dnsdata->a);
			RESET_RUN_LEVEL();
			/* delete from list */
			dnsnode1 = dnsnode;
			dnsnode = list_next (dnslist, dnsnode);
			ns_free (dnsdata->a);
			ns_free (dnsdata);
			list_delete_destroy_node( dnslist, dnsnode1 );
		}
	}
	dns_check_queue();
}

void do_dns_stats_Z( const Client *u ) 
{
	irc_numeric( RPL_MEMSTATS, u->name, "Active DNS queries: %d", ( int ) list_count( dnslist ) );
	irc_numeric( RPL_MEMSTATS, u->name, "Queued DNS Queries: %d", ( int ) list_count( dnsqueue ) );
	irc_numeric( RPL_MEMSTATS, u->name, "Max Queued Queries: %d", DNSStats.maxqueued );
	irc_numeric( RPL_MEMSTATS, u->name, "Total DNS Queries: %d", DNSStats.totalq );
	irc_numeric( RPL_MEMSTATS, u->name, "Successful Lookups: %d", DNSStats.success );
	irc_numeric( RPL_MEMSTATS, u->name, "Unsuccessful Lookups: %d", DNSStats.failure );
}
