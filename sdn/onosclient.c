#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include "mongoose/mongoose.h"

#define MAX_REMOTE_ADDR_LEN 22 //3*4+3(ip and dots) + 1(:) + 5(port) + 1('\0' not sure if this is needed)

static const char *uri_checkalive = "checkalive";
static const char *uri_getportmapping = "portmapping";

void onos_check_alive(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_CONNECT) {
        // Connected to server. Extract host name from URL

        char remote_addr[MAX_REMOTE_ADDR_LEN];
        struct mg_str host;

        syslog(LOG_INFO, "Successfully connect to ONOS!");
        
        snprintf(remote_addr, sizeof(remote_addr), "%u", ntohl(c->peer.ip));
        remote_addr[MAX_REMOTE_ADDR_LEN-1] = '\0';
        host = mg_url_host(remote_addr);
        
        // Send request
        mg_printf(c,
                "GET %s HTTP/1.1\r\n"
                "Host: %.*s\r\n"
                "\r\n",
                uri_checkalive, (int) host.len, host.ptr);
    } else if (ev == MG_EV_HTTP_MSG) {
        // Response is received. Print it
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        printf("***********Alive Message************");
        printf("%.*s", (int) hm->message.len, hm->message.ptr);
        printf("************************************");
        
        c->is_closing = 1;         // Tell mongoose to close this connection
        *(bool *) fn_data = true;  // Tell event loop to stop
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        *(bool *) fn_data = true;  // Error, tell event loop to stop
    }
}