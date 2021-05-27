#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include "jwtauth.h"
#include "jwtauthutils.h"
#include "config.h"

int main(int argc, char ** argv) {
#ifdef USE_JWT_AUTH
    openlog("Test App: ", LOG_PERROR, LOG_DAEMON);
    char header[] = "eyAgICAiYWxnIjogIlJTMjU2IiAgICAgfQ==";
    int header_len = strlen(header);
    char payload[] = "eyJwdWJfcG9ydF9yYW5nZSI6WyIxMDI0LTIwMDAiLCI1MDAwMC02MDAwMCJdLCJpbnRfaXBfcmFuZ2UiOlsiMTcyLjE3LjAuMC8yNCIsICIxNzIuMjIuMTI4LjAvMjUiXSwicHVibGljX2tleSI6Ii0tLS0tQkVHSU4gUFVCTElDIEtFWS0tLS0tCk1JSUJJakFOQmdrcWhraUc5dzBCQVFFRkFBT0NBUThBTUlJQkNnS0NBUUVBcjJLZ2VkK2hTRjU5UE9vZmZ1NnYKVWk1M1M4TkhTV3NsaVBMdDdZMFVMVkVaL2U1S2k0Ky9UZGJvbnJ1OVZ4akcrOUNtRUhwK05jQ3M1L1pOWWJxQQpEWUljVmZzY1hsRkNIeGNha3B6V2RxUjFuWkttMGcxa0UzZTM3bjNHY2EzYWRNSHJROUMvT0V1N2pmeTJmbGZICjlndkUwRlcycWFjbFBjTDJRRCtwSkNuMXNmMG1pSTZKa0xLYnptMVR4TG96NXM1cDY3MVI2SGlabWhhM2o2L1oKV2lLYjkva0V4am84eVR2OHFYaU50ZmdSOXFBV2I2Zm9mblYrUEVtMTZwK01RNTgyUjFBNUdxVHRtcGp3aFlucApIalhQSGtZMUY5cjhiVXNDMGNmektLVzBQU2NEVlB2Z1lCdElHRHhYWVY5VFdTMlZBZnNNQm1hZkJVdFpZOWJCClFRSURBUUFCCi0tLS0tRU5EIFBVQkxJQyBLRVktLS0tLSJ9";
    int payload_len = strlen(payload);
    char signature[] = "LRLNKKMzACOmZXNhcBsWcaaPF6W328/Ak750BCeyiJSSdXDtY7EFY2hVKkMrQZrTt+Dec40/fw4ZK2k81t2XmJcSQlp7PSNrUDegkjnlchAsLMERdBVPmZPM1SDdy0oNs3xr0Kis5GKFM/0vRYQJone1QtXg0GTQo2AuePxGT8aRRmhLi7DtdwynktefUMGR6fL2JAB0KdhLG/b5FVFWLrh4PTlYgMd9K8nplefbeyVYAhVsJH4C/5KhnHLHWrqjPO7E0ewSAiAeoTikoEl/gtwLcYN6DYMpBm8yNKbTRayafeXoOgoS5j422/0CqqtAhiVDsH/wtq/MRh+byveYbA==";
    int signature_len = strlen(signature);

    int testcase_len = header_len + payload_len + signature_len + 2; //2 dot
    char *testcase = malloc(testcase_len + 1);
    int write_offset = 0;
    memcpy(testcase, header, header_len);
    testcase[header_len] = '.';
    write_offset = header_len + 1;
    memcpy(testcase + write_offset, payload, payload_len);
    testcase[write_offset + payload_len] = '.';
    write_offset += (payload_len + 1);
    memcpy(testcase + write_offset, signature, signature_len);
    write_offset += signature_len;
    testcase[write_offset] = '\0';

    char content[] = "Too bad, We got 29 positive cases today.";
    char http_sig[] = "QOL74h5h0X6r1onnWfbMutUt+lLO/yPbhNmgwG5vWq7dbwBGub2x3a3Gt1Uyt7B7iB0M1NaPFz0YRl7Uva1FWJYyl4bGgfz7n1qLd+cTOEii+IEy9UlnjwVlLeB88FJx+FcgI88qXJaagks9qbIWxr3a0l5IFhPPOiipeneddez1IEtpIVb1XqMzeBRssvPYeFTIfZBcNxvOhX22VzMnYskyGJhJDprxZQd8MdbYB4V/RJ5/iZnDGiNBJZIYJzh9MpdnXrrKEQDnK9eHVYFKkmhLSUlbhBxQA97xmFlQXEvt1pPKaHnAAfNtH5wQDAG6ozghb+Lfh7nqC5IzFh7P8g==";

    struct Permission * perm = CreatePermissionObject();
    printf("%d\n", VerifyAndExtractAuthToken(testcase, strlen(testcase), http_sig, strlen(http_sig), content, strlen(content), perm));
    for(unsigned int i = 0;i < perm->pub_port_range_len;i++) {
        printf("start: %d, end:%d\n", perm->pub_port_range[i].start, perm->pub_port_range[i].end);
    }

    for(unsigned int i = 0;i < perm->int_ip_range_len;i++) {
        printf("address: %d.%d.%d.%d, mask:%d\n", 
            (perm->int_ip_range[i].address >> 24)&0xff,
            (perm->int_ip_range[i].address >> 16)&0xff,
            (perm->int_ip_range[i].address >> 8)&0xff,
            (perm->int_ip_range[i].address)&0xff,
            perm->int_ip_range[i].mask);
    }

    DestroyPermissionObject(perm);
    free(testcase);
    closelog();
    return 0;
#endif
}