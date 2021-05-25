#ifndef JWTAUTHUTILS_H
#define JWTAUTHUTILS_H

#include <stdbool.h>
#include <stdint.h>

// 3*4 + 3(dot)
#define MAX_IP_STR_LEN 15
// IP + 1(slash) + 2(mask)
#define MAX_IPRANGE_STR_LEN (MAX_IP_STR_LEN + 3)

bool
ParseIpAddress(char * address_str, uint32_t *address);

bool
ParseIpMaskString(char * range_str, uint32_t *address, uint32_t *mask);

int
ContainIp(uint32_t  range_addr, uint32_t range_mask, uint32_t target);

#endif