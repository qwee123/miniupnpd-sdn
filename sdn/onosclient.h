#ifndef ONOSCLIENT_H
#define ONOSCLIENT_H

#include "mongoose/mongoose.h"
#include "json/json_object.h"

enum http_response_code {
    OK_200,
    BADREQUEST_400,
    NOTFOUND_404,
    METHODNOTALLOWED_405,
    CONFLICT_409,
    INTERNALSERVERERROR_500,
    UNKNOWNERRORCODE
};

extern char *http_response_code_to_string[];

struct http_options {
    const char * req_url;
    const char * host;
    int host_len;
};

struct conn_runtime_vars {
    json_object * request;
    json_object * response;
    enum http_response_code response_code;
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