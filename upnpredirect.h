/* $Id: upnpredirect.h,v 1.36 2018/01/16 00:50:49 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2018 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef UPNPREDIRECT_H_INCLUDED
#define UPNPREDIRECT_H_INCLUDED

/* for u_int64_t */
#include <sys/types.h>

#include "config.h"

#ifdef USE_SDN
#include <stdbool.h>
#endif

#ifdef ENABLE_LEASEFILE
int reload_from_lease_file(void);
#ifdef LEASEFILE_USE_REMAINING_TIME
void lease_file_rewrite(void);
#endif
#endif

/* upnp_redirect()
 * calls OS/fw dependent implementation of the redirection.
 * protocol should be the string "TCP" or "UDP"
 * returns: 0 on success
 *          -1 failed to redirect
 *          -2 already redirected
 *          -3 permission check failed
 */
int
upnp_redirect(const char * rhost, unsigned short eport,
              const char * iaddr, unsigned short iport,
              const char * protocol, const char * desc,
              unsigned int leaseduration
#ifdef USE_SDN
			  , bool automode, unsigned short * ret_eport
#endif
            );

/* upnp_redirect_internal()
 * same as upnp_redirect() without any check */
int
upnp_redirect_internal(const char * rhost, unsigned short eport,
                       const char * iaddr, unsigned short iport,
#ifdef USE_SDN
					   const char * proto,
#else
                       int proto,
#endif 
                        const char * desc,
#ifdef USE_SDN
					   unsigned int leaseduration, bool automode,
                       unsigned short * ret_eport
#else
                       unsigned int timestamp
#endif
                    );

/* upnp_get_redirection_infos()
 * returns : 0 on success
 *           -1 failed to get the port mapping entry or no entry exists */
int
upnp_get_redirection_infos(unsigned short eport, const char * protocol,
                           unsigned short * iport, char * iaddr, int iaddrlen,
                           char * desc, int desclen,
                           char * rhost, int rhostlen,
                           unsigned int * leaseduration);

/* upnp_get_redirection_infos_by_index()
 * returns : 0 on success
 *           -1 failed to get the port mapping or index out of range */
int
upnp_get_redirection_infos_by_index(int index,
                                    unsigned short * eport, char * protocol,
                                    unsigned short * iport,
                                    char * iaddr, int iaddrlen,
                                    char * desc, int desclen,
                                    char * rhost, int rhostlen,
                                    unsigned int * leaseduration);

/* upnp_delete_redirection()
 * returns: 0 on success
 *          -1 on failure*/
int
upnp_delete_redirection(
#ifdef USE_SDN
    const char * rhost,
#endif
    unsigned short eport, const char * protocol);

/* _upnp_delete_redir()
 * same as above */
int
_upnp_delete_redir(unsigned short eport, 
#ifdef USE_SDN
                const char * rhost, const char * proto
#else
                int proto
#endif
                );

/* Periodic cleanup functions
 */
struct rule_state
{
	u_int64_t packets;
	u_int64_t bytes;
	struct rule_state * next;
	unsigned short eport;
	unsigned char proto;
	unsigned char to_remove;
};

/* return a linked list of all rules
 * or an empty list if there are not enough
 * As a "side effect", delete rules which are expired */
struct rule_state *
get_upnp_rules_state_list(int max_rules_number_target);

/* return the number of port mapping entries */
int
upnp_get_portmapping_number_of_entries(void);

/* remove_unused_rules() :
 * also free the list */
void
remove_unused_rules(struct rule_state * list);

/* upnp_get_portmappings_in_range()
 * return a list of all "external" ports for which a port
 * mapping exists */
#ifdef USE_SDN
struct portmapping_entry *
#else
unsigned short *
#endif
upnp_get_portmappings_in_range(unsigned short startport,
                               unsigned short endport,
                               const char * protocol,
                               unsigned int * number);

#ifdef USE_SDN
int
upnp_delete_portmappings_in_range(unsigned short startport,
                               unsigned short endport,
                               const char * protocol,
                               unsigned short ** success_list, unsigned int * slist_number,
							   unsigned short ** fail_list, unsigned int * flist_number);
#endif

/* stuff for responding to miniupnpdctl */
#ifdef USE_MINIUPNPDCTL
void
write_ruleset_details(int s);
#endif

#endif


