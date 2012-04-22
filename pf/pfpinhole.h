/* $Id: pfpinhole.h,v 1.5 2012/04/21 23:25:18 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2012 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef __PFPINHOLE_H__
#define __PFPINHOLE_H__

int add_pinhole(const char * ifname,
                const char * rem_host, unsigned short rem_port,
                const char * int_client, unsigned short int_port,
                int proto, unsigned int timestamp);

int delete_pinhole(unsigned short uid);

int get_pinhole(unsigned short uid,
                char * rem_host, int rem_hostlen, unsigned short * rem_port,
                char * int_client, int int_clientlen, unsigned short * int_port,
                int * proto, unsigned int * timestamp);

int clean_pinehole_list(unsigned int * next_timestamp);

#endif

