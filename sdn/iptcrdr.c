/* $Id: iptcrdr.c,v 1.67 2020/11/11 12:09:05 nanard Exp $ */
/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2020 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <xtables.h>
#include <linux/version.h>

#include "../config.h"
#include "onosclient.h"
#include "mongoose/mongoose.h"
#include "../macros.h"
#include "iptcrdr.h"
#include "../upnpglobalvars.h"
#include "json/json_object.h"

#define DEFAULT_PORTMAPPING_CAP 64
#define ISVALID_USHORT(x) \
	x >= 0 && x <= USHRT_MAX

/* IPT_ALIGN was renamed XT_ALIGN in iptables-1.4.11 */
#ifndef IPT_ALIGN
#define IPT_ALIGN XT_ALIGN
#endif

static const char * json_tag_ext_ip = "ext_ip_addr";
static const char * json_tag_wan_conn_status = "wan_conn_status";
static const char * json_tag_iface_status = "iface_status";
static const char * json_tag_baudrate = "baudrate";
static const char * json_tag_total_bytes_sent = "total_bytes_sent";
static const char * json_tag_total_bytes_recv = "total_bytes_received";
static const char * json_tag_total_pkt_sent = "total_packets_sent";
static const char * json_tag_total_pkt_recv = "total_packets_received";
static const char * json_tag_portmapping = "portmapping";
static const char * json_tag_eport = "eport";
static const char * json_tag_proto = "proto";
static const char * json_tag_rhost = "rhost";
static const char * json_tag_iaddr = "iaddr";
static const char * json_tag_iport = "iport";
static const char * json_tag_duration = "duration";
static const char * json_tag_auto = "auto"; // To distinquish AddAny and normal Add method
static const char * json_tag_return_code = "return_code";
/* success and failure are tags for deletePortmappings_in_range method */
static const char * json_tag_success = "success";
static const char * json_tag_fail = "failure";

static struct mg_mgr mgr;

/* local functions declarations */
static bool
retrieveStringFromJsonObj(struct json_object *jobj,
			 const char *key, char ** ret);

static bool
retrieveIntFromJsonObj(struct json_object *jobj,
			 const char *key, int * ret);

static bool
retrieveUnsignedIntFromJsonObj(struct json_object *jobj,
			 const char *key, unsigned int * ret);

static bool
retrievePortNumberFromJsonObj(struct json_object *jobj,
			 const char *key, unsigned short * ret);

static bool
retrieveJsonArrayFromJsonObj(struct json_object *jobj, 
			 const char *key, struct json_object ** ret);

static bool
retrievePortNumberArrayFromJsonObj(struct json_object *jobj, 
			 const char *key, unsigned short ** ret, unsigned int * ret_size);

static bool
retrievePortmappingArrayFromJsonObj(struct json_object *jobj,
			 const char *key, struct portmapping_entry ** ret, unsigned int * cap);


/*
 * To printout a json object:
 * printf("jobj from str:\n---\n%s\n---\n", json_object_to_json_string_ext(data.payload, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
 */ 

/* dummy init and shutdown functions */
int init_redirect(void)
{
	bool done = false;
	mg_mgr_init(&mgr);
	mg_http_connect(&mgr, controller_address, OnosCheckAlive, &done);
	while(!done) mg_mgr_poll(&mgr, 1000);

	return 0;
}

void shutdown_redirect(void)
{
	syslog(LOG_INFO, "releasing mongoose manager...");
	mg_mgr_free(&mgr);
	return;
}

int get_sdn_igd_external_ip_addr(char * ret_addr, size_t max_len) 
{
	struct conn_runtime_vars data = { .done = false };
	mg_http_connect(&mgr, controller_address, OnosGetExtIpAddr, &data);
	
	while(!data.done) mg_mgr_poll(&mgr, 1000);

	char * ip;
	if (!retrieveStringFromJsonObj(data.payload, json_tag_ext_ip, &ip)) 
	{
		syslog(LOG_WARNING, "Fail to retreive external ip address from the response of onos");
		return -1;
	} else {
		if (strlen(ip) <= max_len) {
			strncpy(ret_addr, ip, max_len);
		} else {
			syslog(LOG_WARNING, "Retrieved external ip address exceeds maximun length, invalid");
			return -1;
		}
	}

	return 0;
}

int get_sdn_igd_wan_conn_status(void) {
	struct conn_runtime_vars data = { .done = false };
	mg_http_connect(&mgr, controller_address, OnosGetWanConnectionStatus, &data);
	
	while(!data.done) mg_mgr_poll(&mgr, 1000);

	char * wan_conn_stat;
	if (!retrieveStringFromJsonObj(data.payload, json_tag_wan_conn_status, &wan_conn_stat)) 
	{
		syslog(LOG_WARNING, "Fail to retreive WAN connection status from the response of onos");
		return -1;
	}
	
	printf("%s\n", wan_conn_stat);
	if (strncmp(wan_conn_stat, "connected", 9) != 0) {
		return -1;
	}

	return 0;
}

/*
 * This function should return -1 if there is any abnormality occured during the process.
 * Even if there is only part of data unavailable, ret_data will be entirely discarded by the outside funtions.
 */
int get_sdn_igd_iface_status(struct igd_iface_status * ret_data) 
{
	struct conn_runtime_vars data = { .done = false };
	mg_http_connect(&mgr, controller_address, OnosGetIGDRuntimeStatus, &data);
	
	while(!data.done) mg_mgr_poll(&mgr, 1000);

	char *iface_status;
	unsigned int baudrate, bytes_sent, bytes_recv, pkt_sent, pkt_recv;

	if (!retrieveStringFromJsonObj(data.payload, json_tag_iface_status, &iface_status)
		|| !retrieveUnsignedIntFromJsonObj(data.payload, json_tag_baudrate, &baudrate)
		|| !retrieveUnsignedIntFromJsonObj(data.payload, json_tag_total_bytes_sent, &bytes_sent) 
		|| !retrieveUnsignedIntFromJsonObj(data.payload, json_tag_total_bytes_recv, &bytes_recv)
		|| !retrieveUnsignedIntFromJsonObj(data.payload, json_tag_total_pkt_sent, &pkt_sent)
		|| !retrieveUnsignedIntFromJsonObj(data.payload, json_tag_total_pkt_recv, &pkt_recv)) 
	{
		syslog(LOG_WARNING, "Fail to retreive igd runtime status from the response of onos");
		return -1;
	}

	ret_data->status = iface_status;
	ret_data->baudrate = baudrate;
	ret_data->ibytes = bytes_recv;
	ret_data->obytes = bytes_sent;
	ret_data->ipackets = pkt_recv;
	ret_data->opackets = pkt_sent;

	return 0;
}

/* convert an ip address to string */
static int snprintip(char * dst, size_t size, uint32_t ip)
{
	return snprintf(dst, size,
	       "%u.%u.%u.%u", ip >> 24, (ip >> 16) & 0xff,
	       (ip >> 8) & 0xff, ip & 0xff);
}

/* netfilter cannot store redirection descriptions, so we use our
 * own structure to store them */
struct rdr_desc {
	struct rdr_desc * next;
	unsigned int timestamp;
	unsigned short eport;
	short proto;
	char str[];
};

/* pointer to the chained list where descriptions are stored */
static struct rdr_desc * rdr_desc_list = 0;

/* add a description to the list of redirection descriptions */
static void
add_redirect_desc(unsigned short eport, int proto,
                  const char * desc, unsigned int timestamp)
{
	struct rdr_desc * p;
	size_t l;
	/* set a default description if none given */
	if(!desc)
		desc = "miniupnpd";
	l = strlen(desc) + 1;
	p = malloc(sizeof(struct rdr_desc) + l);
	if(p)
	{
		p->next = rdr_desc_list;
		p->timestamp = timestamp;
		p->eport = eport;
		p->proto = (short)proto;
		memcpy(p->str, desc, l);
		rdr_desc_list = p;
	}
}

/* delete a description from the list */
static void
del_redirect_desc(unsigned short eport, int proto)
{
	struct rdr_desc * p, * last;
	p = rdr_desc_list;
	last = 0;
	while(p)
	{
		if(p->eport == eport && p->proto == proto)
		{
			if(!last)
				rdr_desc_list = p->next;
			else
				last->next = p->next;
			free(p);
			return;
		}
		last = p;
		p = p->next;
	}
}

/* go through the list to find the description */
static void
get_redirect_desc(unsigned short eport, int proto,
                  char * desc, int desclen,
                  unsigned int * timestamp)
{
	struct rdr_desc * p;
	for(p = rdr_desc_list; p; p = p->next)
	{
		if(p->eport == eport && p->proto == (short)proto)
		{
			if(desc)
				strncpy(desc, p->str, desclen);
			if(timestamp)
				*timestamp = p->timestamp;
			return;
		}
	}
	/* if no description was found, return miniupnpd as default */
	if(desc)
		strncpy(desc, "miniupnpd", desclen);
	if(timestamp)
		*timestamp = 0;
}

static int
_add_redirect_and_filter_rules(const char * rhost, unsigned short eport, 
					const char * iaddr, unsigned short iport,
                    const char * proto, const char * desc,
                    unsigned int duration, bool automode, 
					unsigned short * final_eport)
{
	struct json_object *jobj = json_object_new_object();
	
	if (json_object_object_add(jobj, json_tag_rhost, json_object_new_string(rhost) < 0)
		|| json_object_object_add(jobj, json_tag_eport, json_object_new_int(eport)) < 0
	    || json_object_object_add(jobj, json_tag_proto, json_object_new_string(proto)) < 0
		|| json_object_object_add(jobj, json_tag_iport, json_object_new_int(iport)) < 0
		|| json_object_object_add(jobj, json_tag_iaddr, json_object_new_string(iaddr)) < 0
		|| json_object_object_add(jobj, json_tag_duration, json_object_new_int(duration)) < 0
		|| json_object_object_add(jobj, json_tag_auto, json_object_new_boolean(automode)) < 0)
	{
		syslog(LOG_WARNING, "Fail to retrieve inputs of add_redirect_and_filter_rules method.");
		return -1;
	}

	struct conn_runtime_vars data = { 
		.request_params = jobj,
		.done = false 
	};

	mg_http_connect(&mgr, controller_address, OnosAddIGDPortMapping, &data);
	
	while(!data.done) mg_mgr_poll(&mgr, 1000);

	int ret_code;
	if(!retrieveIntFromJsonObj(data.payload, json_tag_return_code, &ret_code)){
		syslog(LOG_WARNING, "Fail to retrieve return code from reponses of onos.");
		return -1;
	}

	if(ret_code == 0 && !retrievePortNumberFromJsonObj(data.payload, json_tag_eport, final_eport)){
		syslog(LOG_WARNING, "Fail to retrieve portnumber from reponses of onos.");
		return 1;
	}
	
	return ret_code;
}

/* 
 * OnosSuccessButFailToGetResult: 1
 * Success: 0
 * Failure: -1
 * Conflicted: -2
 * Existed: -3
 * NoAvailable: -4
 */ 
int
add_redirect_and_filter_rules(const char * rhost, unsigned short eport, 
					const char * iaddr, unsigned short iport,
                    const char * proto, const char * desc,
                    unsigned int duration)
{
	unsigned short final_eport;
	int ret_code = _add_redirect_and_filter_rules(rhost, eport, iaddr, 
						iport, proto, desc, duration, false, &final_eport);
	switch (ret_code) {
		case 1:
			syslog(LOG_WARNING, "Add rule succesfully but fail to retrieve portnumber from reponses of onos.");
			return 0;
		case 0:
			return 0;
		case -2: //Conflicted -> ConflictWithOtherMechanisms
			return -4;
		case -3: //Existed -> ConflictInMappingEntry
			return -2;
		case -4: //NoAvailable -> ActionFailed
			syslog(LOG_WARNING, "Normal AddRedirect method received error code: NoAvailable, but it shouldn't.");
			return -1;
		default: //Failure and other -> ActionFailed
			return -1;
	}
}

int
add_any_redirect_and_filter_rules(const char * rhost, unsigned short eport, 
					const char * iaddr, unsigned short iport,
                    const char * proto, const char * desc,
                    unsigned int duration, unsigned short * ret)
{
	unsigned short final_eport;
	int ret_code = _add_redirect_and_filter_rules(rhost, eport, iaddr, 
						iport, proto, desc, duration, true, &final_eport);
	switch (ret_code) {
		case 1:  // ActionFailed?
			syslog(LOG_WARNING, "Add rule succesfully but fail to retrieve portnumber from reponses of onos.");
			return -1;
		case 0:  
			*ret = final_eport;
			return 0;
		case -2: //Conflicted -> ActionFailed
			syslog(LOG_WARNING, "AddAnyRedirect method received error code: Conflicted, but it shouldn't.");
			return -1;
		case -3: //Existed -> ActionFailed
			syslog(LOG_WARNING, "AddAnyRedirect method received error code: Existing, but it shouldn't.");
			return -1;
		case -4: //NoAvailable -> NoPortMapsAvailable
			return 1;
		default: //Failure and other -> ActionFailed
			return -1;
	}
}

/* add_peer_redirect_rule2() */
int
add_peer_redirect_rule2(const char * rhost, unsigned short rport,
                   const char * eaddr, unsigned short eport,
                   const char * iaddr, unsigned short iport, int proto,
                   const char * desc, unsigned int timestamp)
{
	int r;

	return -1;
}

int
add_peer_dscp_rule2(const char * rhost, unsigned short rport,
                    unsigned char dscp, const char * iaddr, 
					unsigned short iport, int proto,
                    const char * desc, unsigned int timestamp)
{
	return -1;
}

/* get_redirect_rule()
 * returns -1 if the rule is not found */
int
get_redirect_rule(unsigned short eport, const char * proto,
                  char * iaddr, int iaddrlen, unsigned short * iport,
                  char * desc, int desclen,
                  char * rhost, int rhostlen,
                  unsigned int * leaseduration,
                  u_int64_t * packets, u_int64_t * bytes)
{
	struct json_object *jobj = json_object_new_object();

	//use json_int seems to be safe to convert an unsigned short after testing.
	if (json_object_object_add(jobj, json_tag_rhost, json_object_new_string(rhost) < 0)
		|| json_object_object_add(jobj, json_tag_eport, json_object_new_int(eport)) < 0
	    || json_object_object_add(jobj, json_tag_proto, json_object_new_string(proto)) < 0)
	{
		syslog(LOG_WARNING, "Fail to retrieve inputs of get_portmapping method.");
		return -1;
	}

	struct conn_runtime_vars data = { 
		.request_params = jobj,
		.done = false 
	};

	mg_http_connect(&mgr, controller_address, OnosGetIGDPortMapping, &data);
	
	while(!data.done) mg_mgr_poll(&mgr, 1000);

	const char * rhost_new, * proto_new, * iaddr_new;
	unsigned short eport_new, iport_new;
	if (!retrieveStringFromJsonObj(data.payload, json_tag_rhost, &rhost_new))
	{
		syslog(LOG_WARNING, "Fail to retrieve remote host from response of onos.");
		return -1;
	}
		
	if (!retrievePortNumberFromJsonObj(data.payload, json_tag_eport, &eport_new)
		|| eport_new != eport) 
	{
		syslog(LOG_WARNING, "Fail to retrieve external port from response of onos."
			"Or the received external port is not equeal to the requested one.");
		return -1;
	}
	
	if (!retrieveStringFromJsonObj(data.payload, json_tag_proto, &proto_new)
		|| strcmp(proto_new, proto) != 0) 
	{
		syslog(LOG_WARNING, "Fail to retrieve protocol from response of onos."
			"Or the received protocol is not equeal to the requested one.");
		return -1;	
	}
			
	if (!retrieveStringFromJsonObj(data.payload, json_tag_iaddr, &iaddr_new)) {
		syslog(LOG_WARNING, "Fail to retrieve internal address from response of onos.");
		return -1;	
	}
			
	if (!retrievePortNumberFromJsonObj(data.payload, json_tag_iport, &iport)) {
		syslog(LOG_WARNING, "Fail to retrieve internal portnumber from response of onos.");
		return -1;
	}

	if (!retrieveUnsignedIntFromJsonObj(data.payload, json_tag_duration, leaseduration)) {
		syslog(LOG_WARNING, "Fail to retrieve leaseduration from response of onos.");
		return -1;
	}

	strncpy(iaddr, iaddr_new, iaddrlen);
	strncpy(desc, "empty", desclen);

	return 0;
}

/* get_redirect_rule_by_index()
 * return -1 when the rule was not found */
int
get_redirect_rule_by_index(int index, unsigned short * eport,
                           char * iaddr, int iaddrlen, unsigned short * iport,
                           const char * proto, char * desc, int desclen,
                           char * rhost, int rhostlen,
                           unsigned int * leaseduration,
                           u_int64_t * packets, u_int64_t * bytes)
{
	struct json_object *jobj = json_object_new_object();

	if (json_object_object_add(jobj, "index", json_object_new_int(index) < 0)) {
		syslog(LOG_WARNING, "Fail to retrieve inputs of get_portmapping_index method.");
		return -1;
	}

	struct conn_runtime_vars data = { 
		.request_params = jobj,
		.done = false 
	};

	mg_http_connect(&mgr, controller_address, OnosGetIGDPortMappingByIndex, &data);
	
	while(!data.done) mg_mgr_poll(&mgr, 1000);

	const char * rhost_new, * proto_new, * iaddr_new;
	if (!retrieveStringFromJsonObj(data.payload, json_tag_rhost, &rhost_new))
	{
		syslog(LOG_WARNING, "Fail to retrieve remote host from response of onos.");
		return -1;
	}
		
	if (!retrievePortNumberFromJsonObj(data.payload, json_tag_eport, eport)) 
	{
		syslog(LOG_WARNING, "Fail to retrieve external port from response of onos.");
		return -1;
	}
	
	if (!retrieveStringFromJsonObj(data.payload, json_tag_proto, &proto_new)) 
	{
		syslog(LOG_WARNING, "Fail to retrieve protocol from response of onos.");
		return -1;	
	}
			
	if (!retrieveStringFromJsonObj(data.payload, json_tag_iaddr, &iaddr_new)) {
		syslog(LOG_WARNING, "Fail to retrieve internal address from response of onos.");
		return -1;	
	}
			
	if (!retrievePortNumberFromJsonObj(data.payload, json_tag_iport, iport)) {
		syslog(LOG_WARNING, "Fail to retrieve internal portnumber from response of onos.");
		return -1;
	}

	if (!retrieveUnsignedIntFromJsonObj(data.payload, json_tag_duration, leaseduration)) {
		syslog(LOG_WARNING, "Fail to retrieve leaseduration from response of onos.");
		return -1;
	}

	strncpy(proto, proto_new, strlen(proto_new) + 1); // plus one to include the '\0' symbol
	strncpy(rhost, rhost_new, rhostlen);
	strncpy(iaddr, iaddr_new, iaddrlen);
	strncpy(desc, "empty", desclen);
	
	return 0;
}

/* get_peer_rule_by_index()
 * return -1 when the rule was not found */
int
get_peer_rule_by_index(int index, unsigned short * eport,
						char * iaddr, int iaddrlen, unsigned short * iport,
						int * proto, char * desc, int desclen,
						char * rhost, int rhostlen, unsigned short * rport,
						unsigned int * timestamp,
						u_int64_t * packets, u_int64_t * bytes)
{
	return -1;
}

/* delete_rule_and_commit() :
 * subfunction used in delete_redirect_and_filter_rules() */
static int
delete_rule_and_commit(unsigned int index,
                       const char * miniupnpd_chain,
                       const char * logcaller)
{
	return -1;
}

/* delete_redirect_and_filter_rules() */
int
delete_redirect_and_filter_rules(const char * rhost, unsigned short eport, const char * proto)
{
	struct json_object *jobj = json_object_new_object();

	//use json_int seems to be safe to convert an unsigned short after testing.
	if (json_object_object_add(jobj, json_tag_rhost, json_object_new_string(rhost) < 0)
		|| json_object_object_add(jobj, json_tag_eport, json_object_new_int(eport)) < 0
	    || json_object_object_add(jobj, json_tag_proto, json_object_new_string(proto)) < 0)
	{
		syslog(LOG_WARNING, "Fail to retrieve inputs of delete_portmapping method.");
		return -1;
	}

	struct conn_runtime_vars data = { 
		.request_params = jobj,
		.done = false 
	};

	mg_http_connect(&mgr, controller_address, OnosDeleteIGDPortMapping, &data);
	
	while(!data.done) mg_mgr_poll(&mgr, 1000);

	int ret_code = 0;
	if(!retrieveIntFromJsonObj(data.payload, json_tag_return_code, &ret_code)){
		syslog(LOG_WARNING, "Fail to retrieve return code from reponses of onos.");
		return -1;
	}

	return ret_code;
}

/* return an (malloc'ed) array of selected portmapping entries
 * return NULL if failed.
 * @var number should have been initialized before function call 
 */
struct portmapping_entry *
get_portmappings_in_range(unsigned short startport, unsigned short endport,
                          const char * proto, unsigned int * number)
{
	struct portmapping_entry * array;
	unsigned int capacity = DEFAULT_PORTMAPPING_CAP;
	struct json_object *jobj = json_object_new_object();

	//use json_int seems to be safe to convert an unsigned short after testing.
	if (json_object_object_add(jobj, "proto", json_object_new_string(proto) < 0)
		|| json_object_object_add(jobj, "start_port_num", json_object_new_int(startport)) < 0
	    || json_object_object_add(jobj, "end_port_num", json_object_new_int(endport)) < 0
		|| json_object_object_add(jobj, "max_entry_number", json_object_new_uint64(capacity)) < 0)
	{
		syslog(LOG_WARNING, "Fail to retrieve inputs of get_portmappings_in_range method.");
		return NULL;
	}

	struct conn_runtime_vars data = { 
		.request_params = jobj,
		.done = false 
	};

	mg_http_connect(&mgr, controller_address, OnosGetIGDPortMappingRange, &data);
	
	while(!data.done) mg_mgr_poll(&mgr, 1000);

	if(!retrievePortmappingArrayFromJsonObj(data.payload, json_tag_portmapping, &array, number))
	{
		syslog(LOG_WARNING, "Fail to retrieve portmappings from reponses of onos.");
		return NULL;
	}

	return array;
}

/*
 * @var slist_number should have been initialized before function call 
 * @var flist_nubmer should have been initialized before function call
 */
int
delete_portmappings_in_range(unsigned short startport, 
                    unsigned short endport, const char * proto,	
                    unsigned short ** success_list, unsigned int * slist_number,
					unsigned short ** fail_list, unsigned int * flist_number)
{
	struct json_object *jobj = json_object_new_object();

	//use json_int seems to be safe to convert an unsigned short after testing.
	if (json_object_object_add(jobj, "proto", json_object_new_string(proto) < 0)
		|| json_object_object_add(jobj, "start_port_num", json_object_new_int(startport)) < 0
	    || json_object_object_add(jobj, "end_port_num", json_object_new_int(endport)) < 0)
	{
		syslog(LOG_WARNING, "Fail to retrieve inputs of delete_portmappings_in_range method.");
		return -1;
	}

	struct conn_runtime_vars data = { 
		.request_params = jobj,
		.done = false 
	};

	mg_http_connect(&mgr, controller_address, OnosDeleteIGDPortMappingRange, &data);

	while(!data.done) mg_mgr_poll(&mgr, 1000);

	if (!retrievePortNumberArrayFromJsonObj(data.payload, json_tag_success, success_list, slist_number)) {
		syslog(LOG_WARNING, "Fail to retrieve portnumbers from reponses of onos.");
		return -1;
	}

	if (!retrievePortNumberArrayFromJsonObj(data.payload, json_tag_fail, fail_list, flist_number)) {
		syslog(LOG_WARNING, "Fail to retrieve portnumbers from reponses of onos.");
		return -1;
	}

	return 0;
}


int
update_portmapping(unsigned short eport, int proto,
                   unsigned short iport, const char * desc,
                   unsigned int timestamp)
{
	return -1;
}

int
update_portmapping_desc_timestamp(unsigned short eport,
					int proto, const char * desc,
					unsigned int timestamp)
{
	return -1;
}

static bool
retrieveStringFromJsonObj(struct json_object *jobj, const char *key, char ** ret) {
	struct json_object *tmp;
	if (!json_object_object_get_ex(jobj, key, &tmp)) {
		syslog(LOG_WARNING, "Fail to extract %s from json object\n", key);
		return false;
	}

	if (json_object_get_type(tmp) != json_type_string) {
		syslog(LOG_WARNING, "Retrieved %s value is not a string\n", key);
		return false;
	}

	*ret = json_object_get_string(tmp);
	(*ret)[json_object_get_string_len(tmp)] = '\0';
    return true;
}

static bool
retrieveIntFromJsonObj(struct json_object *jobj, const char *key, int * ret) {
	struct json_object *tmp;
	if (!json_object_object_get_ex(jobj, key, &tmp)) {
		syslog(LOG_WARNING, "Fail to extract %s from json object\n", key);
		return false;
	}

	if (json_object_get_type(tmp) != json_type_int) {
		syslog(LOG_WARNING, "Retrieved %s value is not an int\n", key);
		return false;
	}

	*ret = json_object_get_int(tmp);
    return true;
}

static bool
retrieveUnsignedIntFromJsonObj(struct json_object *jobj, const char *key, unsigned int * ret) {
	struct json_object *tmp;
	if (!json_object_object_get_ex(jobj, key, &tmp)) {
		syslog(LOG_WARNING, "Fail to extract %s from json object\n", key);
		return false;
	}
	if (json_object_get_type(tmp) != json_type_int) {
		syslog(LOG_WARNING, "Retrieved %s value is not an int of json\n", key);
		return false;
	}

	uint64_t ret_64 = json_object_get_uint64(tmp);
	if (ret_64 > UINT_MAX) {
		syslog(LOG_WARNING, "Retrieved %s value is exceeds uint32 range.\n", key);
		return false;
	}

	*ret = (unsigned int)ret_64;
    return true;
}

static bool
retrievePortNumberFromJsonObj(struct json_object *jobj, const char *key, unsigned short * ret) {
	int json_int;
	
	if (!retrieveIntFromJsonObj(jobj, key, &json_int) 
		&& ISVALID_USHORT(json_int)) {
		syslog(LOG_WARNING, "Retrieved %s value is not an port number\n", key);
		return false;
	}

	*ret = (unsigned short)json_int;

	return true;
}

static bool
retrieveJsonArrayFromJsonObj(struct json_object *jobj, const char *key, struct json_object ** ret)
{
	struct json_object *tmp;
	if(!json_object_object_get_ex(jobj, key, &tmp)) {
		syslog(LOG_WARNING, "Fail to extract %s from json object\n", key);
		return false;
	}

	if (json_object_get_type(tmp) != json_type_array) {
		syslog(LOG_WARNING, "Retrieved %s value is not an array\n", key);
		return false;
	}

	*ret = tmp;
	return true;
}

static bool
retrievePortNumberArrayFromJsonObj(struct json_object *jobj, const char *key,
							unsigned short ** ret, unsigned int * ret_size) 
{
	struct json_object *tmp;

	if (!retrieveJsonArrayFromJsonObj(jobj, key, &tmp)) {
		syslog(LOG_WARNING, "Retrieved %s value is not an array of json\n", key);
		return false;
	}

	size_t arr_len = json_object_array_length(tmp);
	unsigned short * array = calloc(arr_len, sizeof(unsigned short));
	unsigned int index = 0;

	int json_int;
	struct json_object * entry;
	for (int i = 0;i < arr_len; i++) {
		entry = json_object_array_get_idx(tmp, i);

		if (json_object_get_type(entry) != json_type_int) {
			syslog(LOG_WARNING, "Retrieved %s value is not an int of json\n", key);
			continue;
		} 
		
		json_int = json_object_get_int(entry);
		if (ISVALID_USHORT(json_int)) {
			array[index++] = (unsigned short)json_int;
		} else {
			syslog(LOG_WARNING, "Retrieved %s value %d is not a port nubmer\n", key, json_int);
		}
	}

	*ret = array;
	*ret_size = index;
	return true;
}

static bool
retrievePortmappingArrayFromJsonObj(struct json_object *jobj, const char *key, 
							struct portmapping_entry ** ret, unsigned int * ret_size) 
{
	struct json_object *tmp;

	if (!retrieveJsonArrayFromJsonObj(jobj, key, &tmp)) {
		syslog(LOG_WARNING, "Retrieved %s value is not an array of json\n", key);
		return false;
	}

	size_t arr_len = json_object_array_length(tmp);
	struct portmapping_entry * array = calloc(arr_len, sizeof(struct portmapping_entry));
	unsigned int index = 0;

	struct json_object * entry;
	for (int i = 0;i < arr_len; i++) {
		entry = json_object_array_get_idx(tmp, i);

		const char *rhost, *iaddr, *proto;
		unsigned short eport, iport;
		unsigned int leaseduration;
		if (retrieveStringFromJsonObj(entry, json_tag_rhost, &rhost)
			&& retrievePortNumberFromJsonObj(entry, json_tag_eport, &eport)
			&& retrieveStringFromJsonObj(entry, json_tag_proto, &proto)
			&& retrieveStringFromJsonObj(entry, json_tag_iaddr, &iaddr)
			&& retrievePortNumberFromJsonObj(entry, json_tag_iport, &iport)
			&& retrieveUnsignedIntFromJsonObj(entry, json_tag_duration, &leaseduration))
		{
			array[index].rhost = rhost;
			array[index].eport = eport;
			array[index].proto = proto;
			array[index].iaddr = iaddr;
			array[index].iport = iport;
			array[index].leaseduration = leaseduration;
			index++;
		} else {
			syslog(LOG_WARNING, "Fail to retrieve portmapping entry from: \n%s\n",
					json_object_to_json_string(entry));
		}
	}

	*ret = array;
	*ret_size = index;
	return true;
}