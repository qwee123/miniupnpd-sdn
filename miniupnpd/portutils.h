#ifndef PORTUTILS_H
#define PORTUTILS_H

#if defined(USE_JWT_AUTH) || defined(USE_SDN)

struct PortRange {
    unsigned short start;
    unsigned short end;
};

#endif

#endif