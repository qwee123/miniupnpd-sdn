#include "config.h"

#ifdef USE_JWT_AUTH

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <syslog.h>
#include <string.h>

#include "upnphttp.h"

static int
decodeBase64(unsigned char** out, size_t* out_len, char* b64str, size_t b64str_len);

static int
verifyJWTSignature(unsigned char * payload, size_t payload_len, unsigned char * sig, size_t sig_len, FILE *fp_pub_key);

/*
@param http_sig represents the value of the "Signature" header, the signature of the "http request payload".
@variable sig represents the signature of the jwttoken, signed by trusted third-party.
*/
int VerifyAuthTokenAndSignature(const char* auth, int auth_len,
                const char* http_sig, int http_sig_len, 
                const char* http_content, int http_content_len, const char * action) {
    
    char *headerb64, *payloadb64, *sigb64;
    unsigned char *header, *payload, *sig;
    size_t header_len, payload_len, sig_len;
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
    if (1 != decodeBase64(&header, &header_len, headerb64, strlen(headerb64)) ||
        1 != decodeBase64(&payload, &payload_len, payloadb64, strlen(payloadb64)) ||
        1 != decodeBase64(&sig, &sig_len, sigb64, strlen(sigb64))) 
    {
        syslog(LOG_ERR, "Fail to decode token\n");
        free(auth_copy);
		return -1;
    }

    free(auth_copy);

    char *ca_pub_key_name = "rs256.key.pub";
	FILE *ca_pub_key_file;
	ca_pub_key_file = fopen(ca_pub_key_name, "r");
    if (1 != verifyJWTSignature(payload, payload_len, sig, sig_len, ca_pub_key_file)) {
        syslog(LOG_ERR, "Fail to verify digest and signature\n");
        fclose(ca_pub_key_file);
        free(header);
        free(payload);
        free(sig);
        return -1;
    }
    fclose(ca_pub_key_file);

    

    /* verify http payload */
    verifyHttpPayload(http_content, http_content_len);

    free(header);
    free(payload);
    free(sig);
    return 1;
}

int verifyHttpPayload(const char * payload, int payload_len) {
    return 1;
}

int decodeBase64(unsigned char** out, size_t* out_len, char* b64str, size_t b64str_len) {
    EVP_ENCODE_CTX *b64_ctx = calloc(1, sizeof(EVP_ENCODE_CTX));
	int ret = 0;
    int len = 0;

	EVP_DecodeInit(b64_ctx);

	*out = malloc((int)(b64str_len*6/8)+2);
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

/*
    Return 1 if success, 0 or other negative value if failure.
*/
int verifyJWTSignature(unsigned char * payload, size_t payload_len, unsigned char * sig, size_t sig_len, FILE *fp_pub_key) {
	EVP_MD_CTX *mdctx = NULL;
	EVP_PKEY *pkey = NULL;

	pkey = PEM_read_PUBKEY(fp_pub_key, NULL, NULL, NULL);
	if (pkey == NULL) {
		syslog(LOG_ERR, "fail to load public key\n");
		return -1;
	}
	fclose(fp_pub_key);

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

	return EVP_DigestVerifyFinal(mdctx, sig, sig_len);
}

#endif