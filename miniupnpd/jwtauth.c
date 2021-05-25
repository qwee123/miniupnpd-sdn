#include "config.h"

#ifdef USE_JWT_AUTH

#include <stdbool.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <syslog.h>
#include <string.h>

#include "jwtauth.h"
#include "jwtauthutils.h"
#include "portutils.h"
#include "upnphttp.h"
#include "json/json_tokener.h"

// 5+1+5, 5 means the max number of digits of an unsigned int, then plus '-'.
#define MAX_PORTRANGE_STR_LEN 11

static int
decodeBase64(unsigned char** out, size_t* out_len, const char* b64str, size_t b64str_len);

static int 
_verifySignature(const char * payload, int payload_len,
                    const unsigned char * sig, int sig_len, EVP_PKEY * pubkey);

static int 
verifySignature(const char * payload, int payload_len,
                    const unsigned char * sig, int sig_len, const char * pubkey);
static int
verifySignatureFromKeyFile(const char * payload, size_t payload_len, 
                    const unsigned char * sig, size_t sig_len, char *pub_key_path);

static bool
retrievePortRangeFromJsonObj(struct json_object *jobj, const char *key,
                    struct PortRange ** ret, unsigned int * ret_size);

static bool
retrieveIpRangeFromJsonObj(struct json_object *jobj, const char *key,
                    struct IpRange ** ret, unsigned int * ret_size);

static bool
retrieveStringFromJsonObj(struct json_object *jobj, const char *key, const char ** ret);

static bool
retrieveJsonArrayFromJsonObj(struct json_object *jobj, const char *key, struct json_object ** ret);

/*
@param http_sig represents the value of the "Signature" header, the signature of the "http request payload".
@variable sig represents the signature of the jwttoken, signed by trusted third-party.
*/
int VerifyAndExtractAuthToken(const char* auth, int auth_len,
                const char* http_sigb64, int http_sigb64_len, 
                const char* http_content, int http_content_len,
                struct Permission * out_perm) {

    char *headerb64, *payloadb64, *sigb64;
    char *header, *payload;
    unsigned char *sig, *http_sig;
    size_t header_len, payload_len, sig_len, http_sig_len;
    char *auth_copy; //init later
    const char * delim = ".\0";

    /* Deny the auth if the http header does not contain relating info. */
    if (auth == NULL || auth_len == 0) {
        return -1;
    }

    /* It's unsafe to directly use upnphttp struct since strtok() will modify the content of the string.
       So, make a copy.*/
    auth_copy = malloc(auth_len+1);
    memcpy(auth_copy, auth, auth_len);
    auth_copy[auth_len] = '\0';

    /* Token format: [header].[proof payload in json].[proof signature] */
    headerb64 = strtok(auth_copy, delim);
    payloadb64 = strtok(NULL, delim);
    sigb64 = strtok(NULL, delim);
    if (payloadb64 == NULL || sigb64 == NULL) { //Incorrect format of auth token.
        syslog(LOG_ERR, "Incorrect format of token: %.*s\n", auth_len, auth);
        free(auth_copy);
        return -1;
    }

    /* decode each part of the token through base64. */
    if (1 != decodeBase64((unsigned char**)&header, &header_len, headerb64, strlen(headerb64)) ||
        1 != decodeBase64((unsigned char**)&payload, &payload_len, payloadb64, strlen(payloadb64)) ||
        1 != decodeBase64(&sig, &sig_len, sigb64, strlen(sigb64))) 
    {
        syslog(LOG_ERR, "Fail to decode token\n");
        free(auth_copy);
		return -1;
    }

    free(auth_copy);

    /* compose payload of the token siganature */
    int token_payload_len = header_len + payload_len;
    char * token_payload = malloc(token_payload_len + 1);
    memcpy(token_payload, header, header_len);
    memcpy(token_payload + header_len, payload, payload_len);
    token_payload[token_payload_len] = '\0';

    char *ca_pub_key_path = "ca_rs256.key.pub";
    if (1 != verifySignatureFromKeyFile(token_payload, token_payload_len, sig, sig_len, ca_pub_key_path)) {
        syslog(LOG_ERR, "Fail to verify digest and signature\n");
        free(token_payload);
        free(header);
        free(payload);
        free(sig);
        return -1;
    }

    free(token_payload);
    free(header);
    free(sig);

    /* extract client's public key */
    struct json_tokener * tokener = json_tokener_new();
    struct json_object * payload_json = json_tokener_parse_ex(tokener, payload, payload_len);
    json_tokener_free(tokener);
    if (payload_json == NULL) {
        syslog(LOG_ERR, "The payload of the jwt token is not in json.");
        free(payload);
        return -1;
    }
    free(payload);

    const char * client_pubkey;
    if (!retrieveStringFromJsonObj(payload_json, "public_key", &client_pubkey)) {
        syslog(LOG_ERR, "Fail to extract client's public key from the jwt token.");
        json_object_put(payload_json);
        return -1;
    }

    /* Decode the http signature through base64 */ 
    if (1 != decodeBase64(&http_sig, &http_sig_len, http_sigb64, http_sigb64_len))
    {
        syslog(LOG_ERR, "Fail to decode http signature\n");
        json_object_put(payload_json);
        free(http_sig);
		return -1;
    }

    /* verify http payload */
    if (1 != verifySignature(http_content, http_content_len, http_sig, http_sig_len, client_pubkey)) {
        syslog(LOG_ERR, "The content of the http request does not match the signature.");
        json_object_put(payload_json);
        free(http_sig);
        return -1;
    }
    free(http_sig);

    struct PortRange * pub_port_range;
    unsigned int pub_port_range_len;
    if (!retrievePortRangeFromJsonObj(payload_json, "pub_port_range", &pub_port_range, &pub_port_range_len)) {
        syslog(LOG_ERR, "Fail to extract pub_port_range from the token payload.");
        json_object_put(payload_json);
        return -1;
    }
    out_perm->pub_port_range = pub_port_range;
    out_perm->pub_port_range_len = pub_port_range_len;

    struct IpRange * int_ip_range;
    unsigned int int_ip_range_len;
    if (!retrieveIpRangeFromJsonObj(payload_json, "int_ip_range", &int_ip_range, &int_ip_range_len)) {
        syslog(LOG_ERR, "Fail to extract int_ip_range from the token payload.");
        json_object_put(payload_json);
        return -1;
    }
    out_perm->int_ip_range = int_ip_range;
    out_perm->int_ip_range_len = int_ip_range_len;

    json_object_put(payload_json); //this also free client_pubkey

    return 1;
}

struct Permission *
CreatePermissionObject() {
    struct Permission *perm = malloc(sizeof(struct Permission));
    perm->pub_port_range = NULL;
    perm->pub_port_range_len = 0;
    perm->int_ip_range = NULL;
    perm->int_ip_range_len = 0;
    return perm;
}

void
DestroyPermissionObject(struct Permission * perm) {
    if (perm == NULL) {
        return;
    }

    if (perm->pub_port_range != NULL) {
        free(perm->pub_port_range);
        perm->pub_port_range = NULL;
    }
    perm->pub_port_range_len = 0;

    if (perm->int_ip_range != NULL) {
        free(perm->int_ip_range);
        perm->int_ip_range = NULL;
    }
    perm->int_ip_range_len = 0;

    free(perm);

    return;
}

/*
    Return 1 if success, 0 or other negative value if failure.
*/
static int 
_verifySignature(const char * payload, int payload_len,
                    const unsigned char * sig, int sig_len, EVP_PKEY * pkey) {
    EVP_MD_CTX *mdctx = NULL;
    int r;

	mdctx = EVP_MD_CTX_create();
	if (mdctx == NULL) {
		syslog(LOG_ERR, "fail to create EVP_MD_CTX during verification\n");
		return -1;
	}

	if (1 != EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, pkey)) {
		syslog(LOG_ERR, "fail to init verification process\n");
		return -1;
	}
    
	if (1 != EVP_DigestVerifyUpdate(mdctx, payload, payload_len)) {
        syslog(LOG_ERR, "fail to update verification digest\n");
		return -1;
    }

    r = EVP_DigestVerifyFinal(mdctx, sig, sig_len);
    EVP_MD_CTX_destroy(mdctx);
	return r;
}

static int 
verifySignature(const char * payload, int payload_len,
                    const unsigned char * sig, int sig_len, const char * pubkey) {
    BIO *pubkey_bio;
	EVP_PKEY *pkey = NULL;
    int r;

    pubkey_bio = BIO_new_mem_buf((void *)pubkey, strlen(pubkey));
    if (pubkey_bio == NULL) {
        syslog(LOG_ERR, "fail to allocate memory from BIO.");
        return -1;
    }

    pkey = PEM_read_bio_PUBKEY(pubkey_bio, NULL, NULL, NULL);
    if (pkey == NULL) {
		syslog(LOG_ERR, "fail to load public key\n");
		return -1;
	}

    BIO_free_all(pubkey_bio);
	r = _verifySignature(payload, payload_len, sig, sig_len, pkey);
    EVP_PKEY_free(pkey);
    return r;
}

static int 
verifySignatureFromKeyFile(const char * payload, size_t payload_len, const unsigned char * sig, size_t sig_len, char * pub_key_path) {
	FILE *pub_key_file;
	pub_key_file = fopen(pub_key_path, "r");
    EVP_PKEY * pkey;
    int r;

    if (pub_key_file == NULL) {
        syslog(LOG_ERR, "No such file %s\n", pub_key_path);
        return -1;
    }

    pkey = PEM_read_PUBKEY(pub_key_file, NULL, NULL, NULL);
    if (pkey == NULL) {
		syslog(LOG_ERR, "fail to load public key\n");
		return -1;
	}
    fclose(pub_key_file);

	r = _verifySignature(payload, payload_len, sig, sig_len, pkey);
    EVP_PKEY_free(pkey);
    return r;
}

static int 
decodeBase64(unsigned char** out, size_t* out_len, const char* b64str, size_t b64str_len) {
    EVP_ENCODE_CTX *b64_ctx = calloc(1, sizeof(EVP_ENCODE_CTX));
	int ret = 0;
    int len = 0;

	EVP_DecodeInit(b64_ctx);

	*out = malloc((int)(b64str_len*3/4)+2);
	ret = EVP_DecodeUpdate(b64_ctx, *out, &len, (unsigned char*)b64str, b64str_len);
	if (0 != ret && 1 != ret) {
		syslog(LOG_ERR, "Fail to update decoding process\n");
        free(*out);
        free(b64_ctx);
		return -1;
	}

	int final_len = 0;
	if (1 != EVP_DecodeFinal(b64_ctx, *out, &final_len)) {
		syslog(LOG_ERR, "Fail to finalize decoding process\n");
        free(*out);
        free(b64_ctx);
		return -1;
	}

	len += final_len;
    *out_len = (size_t)len;
    free(b64_ctx);
    return 1;
}

static bool
retrievePortRangeFromJsonObj(struct json_object *jobj, const char *key, struct PortRange ** ret, unsigned int * ret_size) {
    struct json_object *tmp;

	if (!retrieveJsonArrayFromJsonObj(jobj, key, &tmp)) {
		syslog(LOG_WARNING, "Retrieved value of field %s is not an json array\n", key);
		return false;
	}

	size_t arr_len = json_object_array_length(tmp);
	struct PortRange * array = calloc(arr_len, sizeof(struct PortRange));
	unsigned int index = 0;

    static const char * delim_dash = "-\0";
    struct json_object * entry;
    char *portrange_copy = malloc(MAX_PORTRANGE_STR_LEN + 1);
    int portrange_copy_len;
	for (size_t i = 0;i < arr_len; i++) {
		entry = json_object_array_get_idx(tmp, i);

		if (json_object_get_type(entry) != json_type_string) {
            syslog(LOG_ERR, "Provided portrange is not a string.\n");
            break;
        }

        portrange_copy_len = json_object_get_string_len(entry);
        if (portrange_copy_len > MAX_PORTRANGE_STR_LEN) {
            syslog(LOG_ERR, "Length of the portrange string exceeds maximun value. [%s]\n", json_object_get_string(entry));
            break;
        }

        strncpy(portrange_copy, json_object_get_string(entry), portrange_copy_len);
        portrange_copy[portrange_copy_len] = '\0';
        
        char *start_str, *end_str;
        start_str = strtok(portrange_copy, delim_dash);
        end_str = strtok(NULL, delim_dash);
        if (start_str == NULL || end_str == NULL) {
            syslog(LOG_ERR, "Invalid format of portrange value. [%s]\n", json_object_get_string(entry));
            break;
        }

        int start = atoi(start_str);
        int end = atoi(end_str);
        if (start <= 0 || end <= 0) {
            syslog(LOG_ERR, "Unrecognized value of portnumber. [start: %s, end: %s]\n", start_str, end_str);
            break;
        }

        array[index].start = (unsigned int)start;
        array[index].end = (unsigned int)end;
        index++;
	}

    free(portrange_copy);
    if (index != (unsigned int)arr_len) {
        free(array);
        return false;
    }

	*ret = array;
	*ret_size = index;
	return true;
}

static bool
retrieveIpRangeFromJsonObj(struct json_object *jobj, const char *key, struct IpRange ** ret, unsigned int * ret_size) {
    struct json_object *tmp;

	if (!retrieveJsonArrayFromJsonObj(jobj, key, &tmp)) {
		syslog(LOG_WARNING, "Retrieved value of field %s is not an json array\n", key);
		return false;
	}

	size_t arr_len = json_object_array_length(tmp);
	struct IpRange * array = calloc(arr_len, sizeof(struct IpRange));
	unsigned int index = 0;

    struct json_object * entry;
    char *iprange_copy = malloc(MAX_IPRANGE_STR_LEN + 1);
    int iprange_copy_len;
    for (size_t i = 0;i < arr_len; i++) {
		entry = json_object_array_get_idx(tmp, i);

		if (json_object_get_type(entry) != json_type_string) {
            syslog(LOG_ERR, "Provided iprange is not a string.\n");
            break;
        }

        iprange_copy_len = json_object_get_string_len(entry);
        if (iprange_copy_len > MAX_IPRANGE_STR_LEN) {
            syslog(LOG_ERR, "Length of the portrange string exceeds maximun value. [%s]\n", json_object_get_string(entry));
            break;
        }

        strncpy(iprange_copy, json_object_get_string(entry), iprange_copy_len);
        iprange_copy[iprange_copy_len] = '\0';
        uint32_t address, mask;
        if (!ParseIpMaskString(iprange_copy, &address, &mask)) {
            syslog(LOG_ERR, "Provided iprange has invalid format. [%s]\n", json_object_get_string(entry));
            break;
        }
       
        array[index].address = address;
        array[index].mask = mask;
        index++;
	}

    free(iprange_copy);
    if (index != (unsigned int)arr_len) {
        free(array);
        return false;
    }

	*ret = array;
	*ret_size = index;
	return true;
}

static bool
retrieveStringFromJsonObj(struct json_object *jobj, const char *key, const char ** ret) {
	struct json_object *tmp;
	if (!json_object_object_get_ex(jobj, key, &tmp)) {
		syslog(LOG_WARNING, "Field %s does not exist in the json object\n", key);
		return false;
	}

	if (json_object_get_type(tmp) != json_type_string) {
		syslog(LOG_WARNING, "Retrieved %s value is not a string\n", key);
		return false;
	}

    //the string extracted from json-c is guranteed to be null-terminated after code tracing.
    //See json_tokener.c: _json_object_new_string()
	*ret = json_object_get_string(tmp);
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

#endif