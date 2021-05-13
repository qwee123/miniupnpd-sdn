#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include "json/json_tokener.h"
#include "mongoose/mongoose.h"
#include "onosclient.h"

#define MAX_REMOTE_ADDR_LEN 22 //3*4+3(ip and dots) + 1(:) + 5(port) + 1('\0')

static const char *uri_checkalive = "/checkalive";
static const char *uri_extipaddr = "/stats/extipaddr";
static const char *uri_igdiface_status = "/stats/iface";
static const char *uri_wanconnstatus = "/stats/wanconnstatus";
static const char *uri_portmapping = "/portmapping";
static const char *uri_portmapping_portrange = "/portmapping/range";
static const char *uri_portmapping_index = "/portmapping/index";

char *http_response_code_to_string[] =
    { "200", "400", "404", "405", "409," "500", "UnknownCode" };

void sendGetRequest(struct mg_connection *c, struct http_options *options) {

    static const char resp[] =
        "GET %s HTTP/1.1\r\n"
        "Host: %.*s\r\n"        
        "\r\n";
    
    mg_printf(c, resp, options->req_url, options->host_len, options->host);
}

void sendGetRequestWithPayload(struct mg_connection *c, struct http_options *options, json_object * payload) {

   static const char resp[] =
        "GET %s HTTP/1.1\r\n"
        "Host: %.*s\r\n"
        "Content-length: %d\r\n"        
        "\r\n"
        "%.*s\r\n";
        
    size_t payload_len;
    const char * post_payload =
        json_object_to_json_string_length(payload, JSON_C_TO_STRING_PLAIN, &payload_len);

    mg_printf(c, resp, options->req_url,
        options->host_len, options->host, payload_len,
        payload_len, post_payload);
}

void sendPostRequest(struct mg_connection *c, struct http_options *options, json_object * payload) {

    static const char resp[] =
        "POST %s HTTP/1.1\r\n"
        "Host: %.*s\r\n"
        "Content-length: %d\r\n"        
        "\r\n"
        "%.*s\r\n";
        
    size_t payload_len;
    const char * post_payload =
        json_object_to_json_string_length(payload, JSON_C_TO_STRING_PLAIN, &payload_len);

    mg_printf(c, resp, options->req_url,
        options->host_len, options->host, payload_len,
        payload_len, post_payload);
}

void sendDeleteRequest(struct mg_connection *c, struct http_options *options, json_object * payload) {

    static const char resp[] =
        "DELETE %s HTTP/1.1\r\n"
        "Host: %.*s\r\n"        
        "Content-length: %d\r\n"        
        "\r\n"
        "%.*s\r\n";
    
    size_t payload_len;
    const char * post_payload =
        json_object_to_json_string_length(payload, JSON_C_TO_STRING_PLAIN, &payload_len);

    mg_printf(c, resp, options->req_url,
        options->host_len, options->host, payload_len,
        payload_len, post_payload);
}

void retrieveResponseStatus(void *ev_data, struct conn_runtime_vars *data) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    int status_code_len = 3;
    char status_code_str[status_code_len+1];
    strncpy(status_code_str, hm->uri.ptr, status_code_len);
    status_code_str[status_code_len] = '\0';

    int status_code = atoi(status_code_str);
    switch (status_code) {
        case 200:
            data->response_code = OK_200;
            break;
        case 400:
            data->response_code = BADREQUEST_400;
            break;
        case 404:
            data->response_code = NOTFOUND_404;
            break;
        case 405:
            data->response_code = METHODNOTALLOWED_405;
            break;
        case 409:
            data->response_code = CONFLICT_409;
            break;
        case 500:
            data->response_code = INTERNALSERVERERROR_500;
            break;
        default:
            data->response_code = UNKNOWNERRORCODE;
            syslog(LOG_WARNING, "Got unknown error code: %s\n", status_code_str);
            break;
    }
}

/*
 * Extract Json payload from the http request. The extracted json payload is then assigned
 * to data.
 */ 
void retrieveJsonPayload(void *ev_data, struct conn_runtime_vars *data) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    struct json_tokener * tokener = json_tokener_new();

    data->response = json_tokener_parse_ex(tokener, hm->body.ptr, hm->body.len);

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
    } else if (ev == MG_EV_CLOSE) {
        syslog(LOG_WARNING, "Connection to onos is closed.");
        *(bool *) fn_data = true;
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

        sendPostRequest(c, &options, data->request);
    } else if (ev == MG_EV_HTTP_MSG) {
        retrieveResponseStatus(ev_data, data);
        retrieveJsonPayload(ev_data, data);         
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    } else if (ev == MG_EV_CLOSE) {
        syslog(LOG_WARNING, "Connection to onos is closed.");
        data->done = true;
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
        retrieveResponseStatus(ev_data, data);
        retrieveJsonPayload(ev_data, data);         
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    }  else if (ev == MG_EV_CLOSE) {
        syslog(LOG_WARNING, "Connection to onos is closed.");
        data->done = true;
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
        retrieveResponseStatus(ev_data, data);
        retrieveJsonPayload(ev_data, data);        
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    }  else if (ev == MG_EV_CLOSE) {
        syslog(LOG_WARNING, "Connection to onos is closed.");
        data->done = true;
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
        retrieveResponseStatus(ev_data, data);
        retrieveJsonPayload(ev_data, data);        
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    }  else if (ev == MG_EV_CLOSE) {
        syslog(LOG_WARNING, "Connection to onos is closed.");
        data->done = true;
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

        sendGetRequestWithPayload(c, &options, data->request);
    } else if (ev == MG_EV_HTTP_MSG) {
        retrieveResponseStatus(ev_data, data);
        retrieveJsonPayload(ev_data, data);        
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    } else if (ev == MG_EV_CLOSE) {
        syslog(LOG_WARNING, "Connection to onos is closed.");
        data->done = true;
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

        sendGetRequestWithPayload(c, &options, data->request);
    } else if (ev == MG_EV_HTTP_MSG) {
        retrieveResponseStatus(ev_data, data);
        retrieveJsonPayload(ev_data, data);        
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    } else if (ev == MG_EV_CLOSE) {
        syslog(LOG_WARNING, "Connection to onos is closed.");
        data->done = true;
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

        sendGetRequestWithPayload(c, &options, data->request);
    } else if (ev == MG_EV_HTTP_MSG) {
        retrieveResponseStatus(ev_data, data);
        retrieveJsonPayload(ev_data, data);        
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    }  else if (ev == MG_EV_CLOSE) {
        syslog(LOG_WARNING, "Connection to onos is closed.");
        data->done = true;
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

        sendDeleteRequest(c, &options, data->request);
    } else if (ev == MG_EV_HTTP_MSG) {
        retrieveResponseStatus(ev_data, data);
        retrieveJsonPayload(ev_data, data);        
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    } else if (ev == MG_EV_CLOSE) {
        syslog(LOG_WARNING, "Connection to onos is closed.");
        data->done = true;
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

        sendDeleteRequest(c, &options, data->request);
    } else if (ev == MG_EV_HTTP_MSG) {
        retrieveResponseStatus(ev_data, data);
        retrieveJsonPayload(ev_data, data);        
        c->is_closing = 1;
        data->done = true;
    } else if (ev == MG_EV_ERROR) {
        syslog(LOG_WARNING, "Fail to connect to ONOS");
        data->done = true;  // Error, tell event loop to stop
    } else if (ev == MG_EV_CLOSE) {
        syslog(LOG_WARNING, "Connection to onos is closed.");
        data->done = true;
    }

    return ;
}
