#ifndef JWTAUTH_H
#define JWTAUTH_H

#include "upnphttp.h"

#ifdef USE_JWT_AUTH

struct PortRange {
    unsigned short start;
    unsigned short end;
};

struct IpRange {
    uint32_t address;
    uint32_t mask;
};

struct Permission {
    struct PortRange * pub_port_range;
    unsigned int pub_port_range_len;
    struct IpRange * int_ip_range;
    unsigned int int_ip_range_len;
};

int
VerifyAuthTokenAndSignature(const char* auth, int auth_len,
                const char* sig, int sig_len,
                const char* http_content, int http_content_len,
                struct Permission * perm);

struct Permission *
CreatePermissionObject(void);

void
DestroyPermissionObject(struct Permission * perm);

#endif

#endif