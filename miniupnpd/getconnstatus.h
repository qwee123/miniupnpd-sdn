/* $Id: getconnstatus.h,v 1.2 2011/05/23 20:22:41 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2011 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef GETCONNSTATUS_H_INCLUDED
#define GETCONNSTATUS_H_INCLUDED

#include "config.h"

/**
 * get the connection status
 * return values :
 *  0 - Unconfigured
 *  1 - Connecting
 *  2 - Connected
 *  3 - PendingDisconnect
 *  4 - Disconnecting
 *  5 - Disconnected */

#ifdef USE_SDN

int
get_wan_connection_status(void);

/**
 * return the same value as get_wan_connection_status()
 * as a C string */
const char *
get_wan_connection_status_str(void);

#else

int
get_wan_connection_status(const char * ifname);

/**
 * return the same value as get_wan_connection_status()
 * as a C string */
const char *
get_wan_connection_status_str(const char * ifname);

#endif

#endif