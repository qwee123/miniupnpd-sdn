#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include "jwtauth.h"

int main(int argc, char ** argv) {
    openlog("Test App: ", LOG_PERROR, LOG_DAEMON);
    char header[] = "eyAgICAiYWxnIjogIlJTMjU2IiAgICAgfQ==";
    int header_len = strlen(header);
    char payload[] = "eyJhdXRob3JpemVkX3BvcnRfcmFuZ2UiOiIwLTY2NjYiLCJwdWJsaWNfa2V5IjoiLS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUlJQklqQU5CZ2txaGtpRzl3MEJBUUVGQUFPQ0FROEFNSUlCQ2dLQ0FRRUFyMktnZWQraFNGNTlQT29mZnU2dgpVaTUzUzhOSFNXc2xpUEx0N1kwVUxWRVovZTVLaTQrL1RkYm9ucnU5VnhqRys5Q21FSHArTmNDczUvWk5ZYnFBCkRZSWNWZnNjWGxGQ0h4Y2FrcHpXZHFSMW5aS20wZzFrRTNlMzduM0djYTNhZE1IclE5Qy9PRXU3amZ5MmZsZkgKOWd2RTBGVzJxYWNsUGNMMlFEK3BKQ24xc2YwbWlJNkprTEtiem0xVHhMb3o1czVwNjcxUjZIaVptaGEzajYvWgpXaUtiOS9rRXhqbzh5VHY4cVhpTnRmZ1I5cUFXYjZmb2ZuVitQRW0xNnArTVE1ODJSMUE1R3FUdG1wandoWW5wCkhqWFBIa1kxRjlyOGJVc0MwY2Z6S0tXMFBTY0RWUHZnWUJ0SUdEeFhZVjlUV1MyVkFmc01CbWFmQlV0Wlk5YkIKUVFJREFRQUIKLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tIn0=";
    int payload_len = strlen(payload);
    char signature[] = "UU0KJ/lx7k48PMDxoDDpriPxEBuAbAqc7NAA3fZLIdW58zgE48ZO1RP6eKLRMxmyxCHPU8m1yz5yRZs/nIdll7ppvyNsIw9oBS6iQ/dhNXsGAPnqB/Cx+4I1opQ8AEvaQ65CZot4VCOVcbGPFiDVkTKGIdNoFppKUMQgu2rddBea2oqXOybwL6igJcwh71U/Ml71Zav0iynp+Nmk/b5bIqWqGe2xVSamOcRjA7r73YnUwHgZAG0/0ow5i3iOMyJhQyOaDuufnh/0Pluu6m7fcDypXtlcfd8SINVwryD38qPRcVtB+LdGbJ5SeRdHbo0/9OjmswyW6VX7K5ErvDOwjQ==";
    int signature_len = strlen(signature);

    char *testcase = malloc(header_len + payload_len + signature_len + 2);
    int write_offset = 0;
    memcpy(testcase, header, header_len);
    testcase[header_len] = '.';
    write_offset = header_len + 1;
    memcpy(testcase + write_offset, payload, payload_len);
    testcase[write_offset + payload_len] = '.';
    write_offset += (payload_len + 1);
    memcpy(testcase + write_offset, signature, signature_len);
    write_offset += signature_len;

    char content[] = "Too bad, We got 29 positive cases today.";
    char http_sig[] = "QOL74h5h0X6r1onnWfbMutUt+lLO/yPbhNmgwG5vWq7dbwBGub2x3a3Gt1Uyt7B7iB0M1NaPFz0YRl7Uva1FWJYyl4bGgfz7n1qLd+cTOEii+IEy9UlnjwVlLeB88FJx+FcgI88qXJaagks9qbIWxr3a0l5IFhPPOiipeneddez1IEtpIVb1XqMzeBRssvPYeFTIfZBcNxvOhX22VzMnYskyGJhJDprxZQd8MdbYB4V/RJ5/iZnDGiNBJZIYJzh9MpdnXrrKEQDnK9eHVYFKkmhLSUlbhBxQA97xmFlQXEvt1pPKaHnAAfNtH5wQDAG6ozghb+Lfh7nqC5IzFh7P8g==";
    
    printf("%d\n", VerifyAuthTokenAndSignature(testcase, strlen(testcase), http_sig, strlen(http_sig), content, strlen(content), "action"));
    free(testcase);
    closelog();
    return 0;
}