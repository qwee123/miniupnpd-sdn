#ifndef ONOSCLIENT_H
#define ONOSCLIENT_H

#include "mongoose/mongoose.h"
#include "json/json_object.h"

struct http_options {
    const char * req_url;
    const char * host;
    int host_len;
};

struct conn_runtime_vars {
    json_object * payload;
    bool done;
};

void
OnosCheckAlive(struct mg_connection *, int, void *, void *);

void
OnosGetExtIpAddr(struct mg_connection *, int, void *, void *);

void
OnosGetIGDRuntimeStatus(struct mg_connection *, int , void *, void *);

#endif