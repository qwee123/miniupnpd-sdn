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
    json_object * request_params;
    json_object * payload;
    bool done;
};

/* Handlers for mongoose, may condense to one or less methods */
void
OnosCheckAlive(struct mg_connection *, int, void *, void *);

void
OnosAddIGDPortMapping(struct mg_connection *, int, void *, void *);

void
OnosGetExtIpAddr(struct mg_connection *, int, void *, void *);

void
OnosGetIGDRuntimeStatus(struct mg_connection *, int, void *, void *);

void
OnosGetWanConnectionStatus(struct mg_connection *, int, void *, void *);

void
OnosGetIGDPortMappingByIndex(struct mg_connection *, int, void *, void *);

void
OnosGetIGDPortMapping(struct mg_connection *, int, void *, void *);

void
OnosGetIGDPortMappingRange(struct mg_connection *, int, void *, void *);

void
OnosDeleteIGDPortMapping(struct mg_connection *, int, void *, void *);

void 
OnosDeleteIGDPortMappingRange(struct mg_connection *, int, void *, void *);

#endif