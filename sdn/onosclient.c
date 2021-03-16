#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include "mongoose/mongoose.h"
#include "onosclient.h"

#define MAX_REMOTE_ADDR_LEN 22 //3*4+3(ip and dots) + 1(:) + 5(port) + 1('\0' not sure if this is needed)

static const char *uri_checkalive = "checkalive";
static const char *uri_getextipaddr = "extipaddr";
static const char *uri_getportmapping = "portmapping";

void sendGetRequest(struct mg_connection *c, struct http_options *options) {
    static const char resp[] =
        "GET %s HTTP/1.1\r\n   \
         Host: %.*s\r\n        \
         \r\n";
    
    mg_printf(c, resp, options->req_url, options->host_len, options->host);
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

        options.req_url = uri_checkalive;
        options.host = host.ptr;
        options.host_len = host.len;

        sendGetRequest(c, &options);
    } else if (ev == MG_EV_HTTP_MSG) {
        // Response is received. Print it
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        printf("***********Alive Message************");
        printf("%.*s", (int) hm->message.len, hm->message.ptr);
        printf("************************************");
        fflush(stdout);
        c->is_closing = 1;         // Tell mongoose to close this connection
        *(bool *) fn_data = true;  // Tell event loop to stop
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        *(bool *) fn_data = true;  // Error, tell event loop to stop
    }
}

void OnosGetExtIpAddr(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_CONNECT) {

        struct mg_str host;
        struct http_options options; 
        
        host = getHttpRemoteHostInfo(c);
        
        options.req_url = uri_getextipaddr;
        options.host = host.ptr;
        options.host_len = host.len;

        sendGetRequest(c, &options);
    } else if (ev == MG_EV_HTTP_MSG) {
        // Response is received. Print it
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        printf("***********Alive Message************");
        printf("%.*s", (int) hm->message.len, hm->message.ptr);
        printf("************************************");
        fflush(stdout);
        c->is_closing = 1;         // Tell mongoose to close this connection
        *(bool *) fn_data = true;  // Tell event loop to stop
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        *(bool *) fn_data = true;  // Error, tell event loop to stop
    }
    return ;
}