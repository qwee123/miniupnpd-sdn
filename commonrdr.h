/* $Id: commonrdr.h,v 1.11 2019/10/02 22:12:02 nanard Exp $ */
/* MiniUPnP project
 * (c) 2006-2020 Thomas Bernard
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#ifndef COMMONRDR_H_INCLUDED
#define COMMONRDR_H_INCLUDED

#include "config.h"
#if defined(USE_SDN)
#include "sdn/iptcrdr.h"
#endif

/* init and shutdown functions */
/* init_redirect() return values :
 *  0 : OK
 * -1 : error */
int
init_redirect(void);

void
shutdown_redirect(void);

/* get_redirect_rule_count()
 * return value : -1 for error or the number of redirection rules */
int
get_redirect_rule_count(
#ifdef USE_SDN
                        void
#else
                        const char * ifname
#endif
                        );

/* get_redirect_rule() gets internal IP and port from
 * interface, external port and protocol
 * return value :
 *  0 success (rule found)
 * -1 error or rule not found
 */
/* get_redirect_rule_by_index()
 * return values :
 *  0 success (rule found)
 * -1 error or rule not found */
#ifdef USE_SDN
int
get_redirect_rule(unsigned short eport, const char * proto,
                  char * iaddr, int iaddrlen, unsigned short * iport,
                  char * desc, int desclen,
                  char * rhost, int rhostlen,
                  unsigned int * leaseduration,
                  u_int64_t * packets, u_int64_t * bytes);

int
get_redirect_rule_by_index(int index, unsigned short * eport,
                           char * iaddr, int iaddrlen, unsigned short * iport,
                           const char * proto, char * desc, int desclen,
                           char * rhost, int rhostlen,
                           unsigned int * timestamp,
                           u_int64_t * packets, u_int64_t * bytes);
#else
int
get_redirect_rule(const char * ifname,
                unsigned short eport, int proto,
                char * iaddr, int iaddrlen, unsigned short * iport,
                char * desc, int desclen,
                char * rhost, int rhostlen,
                unsigned int * timestamp,
                u_int64_t * packets, u_int64_t * bytes);

int
get_redirect_rule_by_index(int index, char * ifname, 
                           unsigned short * eport,
                           char * iaddr, int iaddrlen, unsigned short * iport,
                           int * proto, char * desc, int desclen,
                           char * rhost, int rhostlen,
                           unsigned int * timestamp,
                           u_int64_t * packets, u_int64_t * bytes);
#endif

/* return an (malloc'ed) array of "external" port for which there is
 * a port mapping. number is the size of the array */

#ifdef USE_SDN
struct portmapping_entry *
get_portmappings_in_range(unsigned short startport, unsigned short endport,
                          const char * proto, unsigned int * number);

int
delete_portmappings_in_range(unsigned short startport, 
                    unsigned short endport, const char * proto,	
                    unsigned short ** entry_list, unsigned int * list_number);
#else
unsigned short *
get_portmappings_in_range(unsigned short startport, unsigned short endport,
                          const char * proto, unsigned int * number);
#endif

/* update the port mapping internal port, description and timestamp */
int
update_portmapping(
#ifndef USE_SDN
                   const char * ifname, 
#endif
                   unsigned short eport, int proto,
                   unsigned short iport, const char * desc,
                   unsigned int timestamp);

/* update the port mapping description and timestamp */
int
update_portmapping_desc_timestamp(
#ifndef USE_SDN
                    const char * ifname,
#endif
                   unsigned short eport, int proto,
                   const char * desc, unsigned int timestamp);

#if defined(USE_NETFILTER)
/*
 * only provided by nftables implementation at the moment.
 * Should be implemented for iptables too, for consistency
 */

typedef enum {
	RDR_TABLE_NAME,
	RDR_NAT_PREROUTING_CHAIN_NAME,
	RDR_NAT_POSTROUTING_CHAIN_NAME,
	RDR_FORWARD_CHAIN_NAME,
} rdr_name_type;

/*
 * used by the config file parsing in the core
 * to set
 */

int set_rdr_name( rdr_name_type param, const char * string );

#endif

#endif
