#ifndef JWTAUTH_H
#define JWTAUTH_H

#include "upnphttp.h"

#ifdef USE_JWT_AUTH

int
VerifyAuthTokenAndSignature(const char* auth, int auth_len,
                const char* sig, int sig_len,
                const char* http_content, int http_content_len, const char * action);

#endif

#endif