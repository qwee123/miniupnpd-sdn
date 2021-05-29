#include "config.h"

#ifdef USE_JWT_AUTH

#include <stdbool.h>
#include <stdint.h>
#include <syslog.h>
#include "jwtauth.h"
#include "jwtauthutils.h"

int VeridyAddPortMappingAuth(struct Permission *perm, uint32_t int_ip_addr, unsigned short eport) {

    bool contain = false;
	for (unsigned int i = 0; i < perm->int_ip_range_len ; i++) {
		if (1 == ContainIp(perm->int_ip_range[i].address, perm->int_ip_range[i].mask, int_ip_addr)) {
			contain = true;
			break;
		}
	}
	if (!contain) {
        syslog(LOG_WARNING, "int_ip invalid!");
		return 0;
	}

	contain = false;
	for (unsigned int i = 0; i < perm->pub_port_range_len; i++) {
		if (perm->pub_port_range[i].start <= eport && perm->pub_port_range[i].end >= eport) {
			contain = true;
			break;
		}
	}
	if (!contain) {
		syslog(LOG_WARNING, "ext_port invalid!");
		return 0;
	}

    return 1;
}

int VeridyAddAnyPortMappingAuth(struct Permission *perm, uint32_t int_ip_addr, unsigned short eport) {

    bool contain = false;
	for (unsigned int i = 0; i < perm->int_ip_range_len ; i++) {
		if (1 == ContainIp(perm->int_ip_range[i].address, perm->int_ip_range[i].mask, int_ip_addr)) {
			contain = true;
			break;
		}
	}
	if (!contain) {
        syslog(LOG_WARNING, "int_ip invalid!");
		return 0;
	}

	if (eport == 0 && perm->pub_port_range_len > 0) { //wildcard
		return 1;
	}

	contain = false;
	for (unsigned int i = 0; i < perm->pub_port_range_len; i++) {
		if (perm->pub_port_range[i].start <= eport && perm->pub_port_range[i].end >= eport) {
			contain = true;
			break;
		}
	}
	if (!contain) {
		syslog(LOG_WARNING, "ext_port invalid!");
		return 0;
	}

    return 1;
}

#endif