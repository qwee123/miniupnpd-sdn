#ifndef ONOSCLIENT_H
#define ONOSCLIENT_H

#include "mongoose/mongoose.h"

struct http_options {
    const char * req_url;
    const char * host;
    int host_len;
};

void
OnosCheckAlive(struct mg_connection *, int, void *, void *);

void
OnosGetExtIpAddr(struct mg_connection *, int, void *, void *);

#endif