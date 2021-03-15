#ifndef ONOSCLIENT_H
#define ONOSCLIENT_H

void
onos_check_alive(struct mg_connection *, int, void *, void *);

void
onos_get_ext_ip_addr(struct mg_connection *, int, void *, void *);

#endif