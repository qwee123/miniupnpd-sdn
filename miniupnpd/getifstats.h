/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2008 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#ifndef GETIFSTATS_H_INCLUDED
#define GETIFSTATS_H_INCLUDED

#include "config.h"
//declared during sdn mode for now
struct ifdata {
	unsigned long opackets;
	unsigned long ipackets;
	unsigned long obytes;
	unsigned long ibytes;
	unsigned long baudrate;
};

#ifdef USE_SDN

struct igd_iface_status {
    const char * status;
    unsigned long opackets;
	unsigned long ipackets;
	unsigned long obytes;
	unsigned long ibytes;
	unsigned long baudrate;
};

int
get_sdn_igd_iface_status(struct igd_iface_status * data);

#else

/* getifstats()
 * Fill the ifdata structure with statistics for network interface ifname.
 * Return 0 in case of success, -1 for bad arguments or any error */
int
getifstats(const char * ifname, struct ifdata * data);
#endif
#endif

