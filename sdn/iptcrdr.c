/* $Id: iptcrdr.c,v 1.67 2020/11/11 12:09:05 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2020 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <xtables.h>
#include <linux/version.h>

#include "config.h"
#include "onosclient.h"
#include "mongoose/mongoose.h"
#include "../macros.h"
#include "iptcrdr.h"
#include "../upnpglobalvars.h"

/* IPT_ALIGN was renamed XT_ALIGN in iptables-1.4.11 */
#ifndef IPT_ALIGN
#define IPT_ALIGN XT_ALIGN
#endif

static struct mg_mgr mgr;

/* local functions declarations */
static int
addnatrule(int proto, unsigned short eport,
           const char * iaddr, unsigned short iport,
           const char * rhost);

static int
add_filter_rule(int proto, const char * rhost,
                const char * iaddr, unsigned short iport);

static int
addpeernatrule(int proto,
           const char * eaddr, unsigned short eport,
           const char * iaddr, unsigned short iport,
           const char * rhost, unsigned short rport);

static int
addpeerdscprule(int proto, unsigned char dscp,
           const char * iaddr, unsigned short iport,
           const char * rhost, unsigned short rport);

/* dummy init and shutdown functions */
int init_redirect(void)
{
	bool done = false;
	mg_mgr_init(&mgr);
	mg_http_connect(&mgr, controller_address, OnosCheckAlive, &done);
	while(!done) mg_mgr_poll(&mgr, 1000);

	return 0;
}

void shutdown_redirect(void)
{
	syslog(LOG_INFO, "releasing mongoose manager...");
	mg_mgr_free(&mgr);
	return;
}

int get_sdn_igd_external_ip_addr(char * ret_addr, int max_len) 
{
	bool done = false;
	mg_http_connect(&mgr, controller_address, OnosGetExtIpAddr, &done);
	while(!done) mg_mgr_poll(&mgr, 1000);
	return 0;
}

int get_sdn_igd_wan_conn_status(void) {
	return 0;
}

int get_sdn_igd_runtime_status(struct igd_runtime_status * data) {
	return 0;
}

/* convert an ip address to string */
static int snprintip(char * dst, size_t size, uint32_t ip)
{
	return snprintf(dst, size,
	       "%u.%u.%u.%u", ip >> 24, (ip >> 16) & 0xff,
	       (ip >> 8) & 0xff, ip & 0xff);
}

/* netfilter cannot store redirection descriptions, so we use our
 * own structure to store them */
struct rdr_desc {
	struct rdr_desc * next;
	unsigned int timestamp;
	unsigned short eport;
	short proto;
	char str[];
};

/* pointer to the chained list where descriptions are stored */
static struct rdr_desc * rdr_desc_list = 0;

/* add a description to the list of redirection descriptions */
static void
add_redirect_desc(unsigned short eport, int proto,
                  const char * desc, unsigned int timestamp)
{
	struct rdr_desc * p;
	size_t l;
	/* set a default description if none given */
	if(!desc)
		desc = "miniupnpd";
	l = strlen(desc) + 1;
	p = malloc(sizeof(struct rdr_desc) + l);
	if(p)
	{
		p->next = rdr_desc_list;
		p->timestamp = timestamp;
		p->eport = eport;
		p->proto = (short)proto;
		memcpy(p->str, desc, l);
		rdr_desc_list = p;
	}
}

/* delete a description from the list */
static void
del_redirect_desc(unsigned short eport, int proto)
{
	struct rdr_desc * p, * last;
	p = rdr_desc_list;
	last = 0;
	while(p)
	{
		if(p->eport == eport && p->proto == proto)
		{
			if(!last)
				rdr_desc_list = p->next;
			else
				last->next = p->next;
			free(p);
			return;
		}
		last = p;
		p = p->next;
	}
}

/* go through the list to find the description */
static void
get_redirect_desc(unsigned short eport, int proto,
                  char * desc, int desclen,
                  unsigned int * timestamp)
{
	struct rdr_desc * p;
	for(p = rdr_desc_list; p; p = p->next)
	{
		if(p->eport == eport && p->proto == (short)proto)
		{
			if(desc)
				strncpy(desc, p->str, desclen);
			if(timestamp)
				*timestamp = p->timestamp;
			return;
		}
	}
	/* if no description was found, return miniupnpd as default */
	if(desc)
		strncpy(desc, "miniupnpd", desclen);
	if(timestamp)
		*timestamp = 0;
}

/* add nat rule
 * iptables -t nat -A MINIUPNPD -p <proto> [-s <rhost>] --dport <eport> -j DNAT --to <iaddr>:<iport>
 * */
static int
addnatrule(int proto, unsigned short eport,
           const char * iaddr, unsigned short iport,
           const char * rhost)
{
	return -1;
}

/* add_redirect_rule2() */
int
add_redirect_rule2(const char * rhost, unsigned short eport,
                   const char * iaddr, unsigned short iport, int proto,
				   const char * desc, unsigned int timestamp)
{
	int r;

	r = addnatrule(proto, eport, iaddr, iport, rhost);
	if(r >= 0) {
		add_redirect_desc(eport, proto, desc, timestamp);
	}
	return r;
}

/* called by add_peer_redirect_rule2()
 *
 * iptables -t nat -A MINIUPNPD-POSTROUTING -s <iaddr> -d <rhost>
 *    -p <proto> --sport <iport> --dport <rport> -j SNAT
 *    --to-source <eaddr>:<eport> */
static int
addpeernatrule(int proto,
           const char * eaddr, unsigned short eport,
           const char * iaddr, unsigned short iport,
           const char * rhost, unsigned short rport)
{
	return -1;
}

/* add_peer_redirect_rule2() */
int
add_peer_redirect_rule2(const char * rhost, unsigned short rport,
                   const char * eaddr, unsigned short eport,
                   const char * iaddr, unsigned short iport, int proto,
                   const char * desc, unsigned int timestamp)
{
	int r;

	r = addpeernatrule(proto, eaddr, eport, iaddr, iport, rhost, rport);
	if(r >= 0)
		add_redirect_desc(eport, proto, desc, timestamp);
	return r;
}

/* called by add_peer_dscp_rule2()
 * iptables -t mangle -A MINIUPNPD -s <iaddr> -d <rhost>
 *    -p <proto> --sport <iport> --dport <rport> -j DSCP
 *    --set-dscp 0xXXXX                   */
static int
addpeerdscprule(int proto, unsigned char dscp,
           const char * iaddr, unsigned short iport,
           const char * rhost, unsigned short rport)
{
	return -1;
}

int
add_peer_dscp_rule2(const char * rhost, unsigned short rport,
                    unsigned char dscp, const char * iaddr, 
					unsigned short iport, int proto,
                    const char * desc, unsigned int timestamp)
{
	int r;
	UNUSED(desc);
	UNUSED(timestamp);

	r = addpeerdscprule(proto, dscp, iaddr, iport, rhost, rport);
/*	if(r >= 0)
		add_redirect_desc(dscp, proto, desc, timestamp); */
	return r;
}

/* add_filter_rule()
 * iptables -t filter -A MINIUPNPD [-s <rhost>] -p <proto> -d <iaddr> --dport <iport> -j ACCEPT */
static int
add_filter_rule(int proto, const char * rhost,
                const char * iaddr, unsigned short iport)
{
	return -1;
}

int
add_filter_rule2(const char * rhost, const char * iaddr,
                 unsigned short eport, unsigned short iport,
                 int proto, const char * desc)
{
	UNUSED(eport);
	UNUSED(desc);

	return add_filter_rule(proto, rhost, iaddr, iport);
}

/* get_redirect_rule()
 * returns -1 if the rule is not found */
int
get_redirect_rule(unsigned short eport, int proto,
                  char * iaddr, int iaddrlen, unsigned short * iport,
                  char * desc, int desclen,
                  char * rhost, int rhostlen,
                  unsigned int * timestamp,
                  u_int64_t * packets, u_int64_t * bytes)
{
	return get_nat_redirect_rule(eport, proto,
	                             iaddr, iaddrlen, iport,
	                             desc, desclen,
	                             rhost, rhostlen,
	                             timestamp, packets, bytes);
}

int
get_nat_redirect_rule(unsigned short eport, int proto,
                  char * iaddr, int iaddrlen, unsigned short * iport,
                  char * desc, int desclen,
                  char * rhost, int rhostlen,
                  unsigned int * timestamp,
                  u_int64_t * packets, u_int64_t * bytes)
{
	return -1;
}

/* get_redirect_rule_by_index()
 * return -1 when the rule was not found */
int
get_redirect_rule_by_index(int index, unsigned short * eport,
                           char * iaddr, int iaddrlen, unsigned short * iport,
                           int * proto, char * desc, int desclen,
                           char * rhost, int rhostlen,
                           unsigned int * timestamp,
                           u_int64_t * packets, u_int64_t * bytes)
{
	return -1;
}

/* get_peer_rule_by_index()
 * return -1 when the rule was not found */
int
get_peer_rule_by_index(int index, unsigned short * eport,
						char * iaddr, int iaddrlen, unsigned short * iport,
						int * proto, char * desc, int desclen,
						char * rhost, int rhostlen, unsigned short * rport,
						unsigned int * timestamp,
						u_int64_t * packets, u_int64_t * bytes)
{
	return -1;
}

/* delete_rule_and_commit() :
 * subfunction used in delete_redirect_and_filter_rules() */
static int
delete_rule_and_commit(unsigned int index,
                       const char * miniupnpd_chain,
                       const char * logcaller)
{
	return -1;
}

/* delete_filter_rule()
 */
int
delete_filter_rule(unsigned short port, int proto)
{
	return -1;
}

/* delete_redirect_and_filter_rules()
 */
int
delete_redirect_and_filter_rules(unsigned short eport, int proto)
{
	return -1;
}

/* return an (malloc'ed) array of "external" port for which there is
 * a port mapping. number is the size of the array */
unsigned short *
get_portmappings_in_range(unsigned short startport, unsigned short endport,
                          int proto, unsigned int * number)
{
	return NULL;
}

int
update_portmapping(unsigned short eport, int proto,
                   unsigned short iport, const char * desc,
                   unsigned int timestamp)
{
	return -1;
}

int
update_portmapping_desc_timestamp(unsigned short eport,
					int proto, const char * desc,
					unsigned int timestamp)
{
	return -1;
}