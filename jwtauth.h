#ifndef JWTAUTH_H
#define JWTAUTH_H

#include "upnphttp.h"

#ifdef USE_JWT_AUTH

int
VerifyAuth(char* auth, int auth_len, const char * action);

#endif

#endif