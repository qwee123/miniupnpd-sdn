/* $Id: iptcrdr.h,v 1.22 2018/07/06 12:00:10 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2018 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef IPTCRDR_H_INCLUDED
#define IPTCRDR_H_INCLUDED

#include "../commonrdr.h"
#include "../config.h"

/* Explanation of the abv.
 * rhost: remoteHost, the (allowed) remote host(from wan) of the rule, usually is set to be a wildcard(an empty string)
 * eport: externalPort, port bond on the igd, exposed to wan.
 * iaddr: internal client addr, an address the rule will redirect the inbound traffic to.
 * iport: internal client port, the usage is the same as the iaddr, instead it's not an address but port nubmer
 * proto: protocol of the traffic filtered by the rule, it could either be tcp or udp
 * desc: description, just some memo.
 */

struct portmapping_entry {
    const char * rhost;
    unsigned short eport;
    const char * proto;
    const char * iaddr;
    unsigned short iport;
    unsigned int leaseduration;
};

int 
add_redirect_and_filter_rules(const char * rhost, unsigned short eport, 
					const char * iaddr, unsigned short iport,
                    const char * proto, const char * desc,
                    unsigned int duration);

int
add_any_redirect_and_filter_rules(const char * rhost, unsigned short eport, 
					const char * iaddr, unsigned short iport,
                    const char * proto, const char * desc,
                    unsigned int duration, unsigned short * ret);

int
add_peer_redirect_rule2(const char * rhost, unsigned short rport,
                   const char * eaddr, unsigned short eport,
                   const char * iaddr, unsigned short iport, int proto,
                   const char * desc, unsigned int timestamp);

int
delete_redirect_and_filter_rules(const char * rhost, unsigned short eport,
                   const char * proto);

int
add_peer_dscp_rule2(const char * rhost, unsigned short rport,
                   unsigned char dscp, const char * iaddr,
                   unsigned short iport, int proto,
                   const char * desc, unsigned int timestamp);

int get_nat_ext_addr(struct sockaddr* src, struct sockaddr *dst, uint8_t proto,
                     struct sockaddr* ret_ext);
int
get_peer_rule_by_index(int index, unsigned short * eport,
                        char * iaddr, int iaddrlen, unsigned short * iport,
                        int * proto, char * desc, int desclen,
                        char * rhost, int rhostlen, unsigned short * rport,
                        unsigned int * timestamp,
                        u_int64_t * packets, u_int64_t * bytes);

int
get_sdn_igd_external_ip_addr(char *ret_addr, size_t max_len);

int
get_sdn_igd_wan_conn_status(void);

/* for debug */
int
list_redirect_rule(void);

#endif

