#ifndef UPNPSOAPAUTH_H_INCLUDED
#define UPNPSOAPAUTH_H_INCLUDED

#include "config.h"

#ifdef USE_JWT_AUTH

#include <stdint.h>
#include "jwtauth.h"

int VeridyAddPortMappingAuth(struct Permission *perm, uint32_t int_ip_addr, unsigned short eport);

int VeridyAddAnyPortMappingAuth(struct Permission *perm, uint32_t int_ip_addr, unsigned short eport);

#endif
#endif
