#include "config.h"

#ifdef USE_JWT_AUTH

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "jwtauthutils.h"

static char * delim_dot = ".\0";
static char * delim_slash = "/\0";

bool
_parseIpAddress(char * address_str, uint32_t *address) {
    char * first, * second, * third, * fourth; //four segments of an address
    *address = 0;

    first = strtok(address_str, delim_dot);
    second = strtok(NULL, delim_dot);
    third = strtok(NULL, delim_dot);
    fourth = strtok(NULL, delim_dot);
    if (first == NULL || second == NULL || third == NULL || fourth == NULL) {
        return false;
    }

    int tmp = atoi(first);
    if (tmp > 256 || tmp < 0 || (tmp == 0 && (strlen(first) != 1 || first[0] != '0'))) {
       return false;
    }
    *address += tmp << 24;

    tmp = atoi(second);
    if (tmp > 256 || tmp < 0 || (tmp == 0 && (strlen(second) != 1 || second[0] != '0'))) {
       return false;
    }
    *address += tmp << 16;

    tmp = atoi(third);
    if (tmp > 256 || tmp < 0 || (tmp == 0 && (strlen(third) != 1 || third[0] != '0'))) {
       return false;
    }
    *address += tmp << 8;

    tmp = atoi(fourth);
    if (tmp > 256 || tmp < 0 || (tmp == 0 && (strlen(fourth) != 1 || fourth[0] != '0'))) {
       return false;
    }
    *address += tmp;

    return true;
}

bool
ParseIpAddress(char * address_str, uint32_t *address) {
    
    int addr_len = strlen(address_str);
    if (addr_len > MAX_IP_STR_LEN) {
        return false;
    }

    char *copy = malloc(addr_len);
    strncpy(copy, address_str, addr_len);
    bool r = _parseIpAddress(copy, address);
    free(copy);
    return r;
}

bool
ParseIpMaskString(char * range_str, uint32_t *address, uint32_t *mask) {
    char * addr_seg, * mask_seg;
    *address = 0;
    *mask = 0;

    addr_seg = strtok(range_str, delim_slash);
    mask_seg = strtok(NULL, delim_slash);
    if (addr_seg == NULL || mask_seg == NULL) {
        return false;
    }

    if (!_parseIpAddress(addr_seg, address)) {
        return false;
    }

    int tmp = atoi(mask_seg);
    if (tmp > 32 || tmp < 0 || (tmp == 0 && (strlen(mask_seg) != 1 || mask_seg[0] != '0'))) {
        return false;
    }
    *mask = tmp;
    return true;
}

/**
 * return 1 if true, 0 if false, -1 if passed ip_range is not valid
 */
int
ContainIp(uint32_t  range_addr, uint32_t range_mask, uint32_t target) {
    if (range_mask == 0) {
        return 1;
    }

    if (range_mask < 1 || range_mask > 32) {
        return -1;
    }
    
    uint32_t mask = 0xffffffff;
    mask -= ((1 << (32 - range_mask)) - 1);
    return (range_addr&mask) == (target&mask);
}

#endif