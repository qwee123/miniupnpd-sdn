#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include "mongoose/mongoose.h"
#include "onosclient.h"
#include "json/json_tokener.h"

#define MAX_REMOTE_ADDR_LEN 22 //3*4+3(ip and dots) + 1(:) + 5(port) + 1('\0')

static const char *uri_checkalive = "/checkalive";
static const char *uri_extipaddr = "/stats/extipaddr";
static const char *uri_igdiface_status = "/stats/iface";
static const char *uri_wanconnstatus = "/stats/wanconnstatus";
static const char *uri_portmapping = "/portmapping";
static const char *uri_portmapping_portrange = "/portmapping/portrange";
static const char *uri_portmapping_index = "/portmapping/index";

void sendGetRequest(struct mg_connection *c, struct http_options *options) {

    static const char resp[] =
        "GET %s HTTP/1.1\r\n"
        "Host: %.*s\r\n"        
        "\r\n";
    
    mg_printf(c, resp, options->req_url, options->host_len, options->host);
}

void sendPostRequest(struct mg_connection *c, struct http_options *options) {

    static const char resp[] =
        "POST %s HTTP/1.1\r\n"
        "Host: %.*s\r\n"        
        "\r\n";
    
    mg_printf(c, resp, options->req_url, options->host_len, options->host);
}

void sendDeleteRequest(struct mg_connection *c, struct http_options *options) {

    static const char resp[] =
        "DELETE %s HTTP/1.1\r\n"
        "Host: %.*s\r\n"        
        "\r\n";
    
    mg_printf(c, resp, options->req_url, options->host_len, options->host);
}

/*
 * Extract Json payload from the http request. The extracted json payload is then assigned
 * to data.
 */ 
void retrieveJsonPayload(void *ev_data, struct conn_runtime_vars *data) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    struct json_tokener * tokener = json_tokener_new();

    data->payload = json_tokener_parse_ex(tokener, hm->body.ptr, hm->body.len);

    json_tokener_free(tokener);
}

struct mg_str getHttpRemoteHostInfo(struct mg_connection *c) {
    char remote_addr[MAX_REMOTE_ADDR_LEN];

    snprintf(remote_addr, sizeof(remote_addr), "%u", ntohl(c->peer.ip));
    remote_addr[MAX_REMOTE_ADDR_LEN-1] = '\0';
    return mg_url_host(remote_addr);
}

void OnosCheckAlive(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_CONNECT) {

        struct mg_str host;
        struct http_options options; 

        syslog(LOG_INFO, "Successfully connect to ONOS!");
        
        host = getHttpRemoteHostInfo(c);

        //Different action may have different options.       
        options.req_url = uri_checkalive;
        options.host = host.ptr;
        options.host_len = (int)host.len;

        sendGetRequest(c, &options);
    } else if (ev == MG_EV_HTTP_MSG) {
        // Response is received. Print it
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        printf("***********Alive Message************\n");
        printf("%.*s", (int) hm->message.len, hm->message.ptr);
        printf("************************************\n");

        fflush(stdout);
        c->is_closing = 1;         // Tell mongoose to close this connection
        *(bool *) fn_data = true;  // Tell event loop to stop
    } else if (ev == MG_EV_ERROR) {
        
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        *(bool *) fn_data = true;  // Error, tell event loop to stop
    }
}

void OnosAddIGDPortMapping(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct conn_runtime_vars * data = (struct conn_runtime_vars *) fn_data;

    if (ev == MG_EV_CONNECT) {

        struct mg_str host;
        struct http_options options; 
        
        host = getHttpRemoteHostInfo(c);
        
        options.req_url = uri_portmapping;
        options.host = host.ptr;
        options.host_len = host.len;

        sendPostRequest(c, &options);
    } else if (ev == MG_EV_HTTP_MSG) {
        retrieveJsonPayload(ev_data, data);         
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    }
    return ;
}

void OnosGetExtIpAddr(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct conn_runtime_vars * data = (struct conn_runtime_vars *) fn_data;

    if (ev == MG_EV_CONNECT) {

        struct mg_str host;
        struct http_options options; 
        
        host = getHttpRemoteHostInfo(c);
        
        options.req_url = uri_extipaddr;
        options.host = host.ptr;
        options.host_len = host.len;

        sendGetRequest(c, &options);
    } else if (ev == MG_EV_HTTP_MSG) {
        retrieveJsonPayload(ev_data, data);         
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    }
    return ;
}

void OnosGetIGDRuntimeStatus(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct conn_runtime_vars * data = (struct conn_runtime_vars *) fn_data;

    if (ev == MG_EV_CONNECT) {

        struct mg_str host;
        struct http_options options;
        
        host = getHttpRemoteHostInfo(c);
        
        options.req_url = uri_igdiface_status;
        options.host = host.ptr;
        options.host_len = host.len;

        sendGetRequest(c, &options);
    } else if (ev == MG_EV_HTTP_MSG) {
        retrieveJsonPayload(ev_data, data);        
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    }
    return ;
}

void OnosGetWanConnectionStatus(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct conn_runtime_vars * data = (struct conn_runtime_vars *) fn_data;

    if (ev == MG_EV_CONNECT) {

        struct mg_str host;
        struct http_options options;
        
        host = getHttpRemoteHostInfo(c);
        
        options.req_url = uri_wanconnstatus;
        options.host = host.ptr;
        options.host_len = host.len;

        sendGetRequest(c, &options);
    } else if (ev == MG_EV_HTTP_MSG) {
        retrieveJsonPayload(ev_data, data);        
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    }
    return ;
}

void OnosGetIGDPortMappingByIndex(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct conn_runtime_vars * data = (struct conn_runtime_vars *) fn_data;

    if (ev == MG_EV_CONNECT) {

        struct mg_str host;
        struct http_options options;
        
        host = getHttpRemoteHostInfo(c);
        
        options.req_url = uri_portmapping_index;
        options.host = host.ptr;
        options.host_len = host.len;
        printf("send\n");

        sendGetRequest(c, &options);
    } else if (ev == MG_EV_HTTP_MSG) {
        retrieveJsonPayload(ev_data, data);        
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    }
    return ;
}

void OnosGetIGDPortMapping(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct conn_runtime_vars * data = (struct conn_runtime_vars *) fn_data;

    if (ev == MG_EV_CONNECT) {

        struct mg_str host;
        struct http_options options;
        
        host = getHttpRemoteHostInfo(c);
        
        options.req_url = uri_portmapping;
        options.host = host.ptr;
        options.host_len = host.len;

        sendGetRequest(c, &options);
    } else if (ev == MG_EV_HTTP_MSG) {
        retrieveJsonPayload(ev_data, data);        
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    }
    return ;
}

void OnosGetIGDPortMappingRange(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct conn_runtime_vars * data = (struct conn_runtime_vars *) fn_data;

    if (ev == MG_EV_CONNECT) {

        struct mg_str host;
        struct http_options options;
        
        host = getHttpRemoteHostInfo(c);
        
        options.req_url = uri_portmapping_portrange;
        options.host = host.ptr;
        options.host_len = host.len;

        sendGetRequest(c, &options);
    } else if (ev == MG_EV_HTTP_MSG) {
        retrieveJsonPayload(ev_data, data);        
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    }
    return ;
}

void OnosDeleteIGDPortMapping(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct conn_runtime_vars * data = (struct conn_runtime_vars *) fn_data;

    if (ev == MG_EV_CONNECT) {

        struct mg_str host;
        struct http_options options;
        
        host = getHttpRemoteHostInfo(c);
        
        options.req_url = uri_portmapping;
        options.host = host.ptr;
        options.host_len = host.len;

        sendDeleteRequest(c, &options);
    } else if (ev == MG_EV_HTTP_MSG) {
        retrieveJsonPayload(ev_data, data);        
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    }
    return ;
}

void OnosDeleteIGDPortMappingRange(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct conn_runtime_vars * data = (struct conn_runtime_vars *) fn_data;

    if (ev == MG_EV_CONNECT) {

        struct mg_str host;
        struct http_options options;
        
        host = getHttpRemoteHostInfo(c);
        
        options.req_url = uri_portmapping_portrange;
        options.host = host.ptr;
        options.host_len = host.len;

        sendDeleteRequest(c, &options);
    } else if (ev == MG_EV_HTTP_MSG) {
        retrieveJsonPayload(ev_data, data);        
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    }
    return ;
}
