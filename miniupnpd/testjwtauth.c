#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include "jwtauth.h"

int main(int argc, char ** argv) {
    openlog("Test App: ", LOG_PERROR, LOG_DAEMON);
    char header[] = "eyJhbGciOiAiUlMyNTYifQ==";
    int header_len = strlen(header);
    char payload[] = "CnsKICAgICJhdXRob3JpemVkX3BvcnRfcmFuZ2UiOiAiMTAyNC02MDAwMCIsCiAgICAicHVibGljX2tleSI6ICItLS0tLUJFR0lOIFBVQkxJQyBLRVktLS0tLQpNSUlCSWpBTkJna3Foa2lHOXcwQkFRRUZBQU9DQVE4QU1JSUJDZ0tDQVFFQXIyS2dlZCtoU0Y1OVBPb2ZmdTZ2ClVpNTNTOE5IU1dzbGlQTHQ3WTBVTFZFWi9lNUtpNCsvVGRib25ydTlWeGpHKzlDbUVIcCtOY0NzNS9aTllicUEKRFlJY1Zmc2NYbEZDSHhjYWtweldkcVIxblpLbTBnMWtFM2UzN24zR2NhM2FkTUhyUTlDL09FdTdqZnkyZmxmSAo5Z3ZFMEZXMnFhY2xQY0wyUUQrcEpDbjFzZjBtaUk2SmtMS2J6bTFUeExvejVzNXA2NzFSNkhpWm1oYTNqNi9aCldpS2I5L2tFeGpvOHlUdjhxWGlOdGZnUjlxQVdiNmZvZm5WK1BFbTE2cCtNUTU4MlIxQTVHcVR0bXBqd2hZbnAKSGpYUEhrWTFGOXI4YlVzQzBjZnpLS1cwUFNjRFZQdmdZQnRJR0R4WFlWOVRXUzJWQWZzTUJtYWZCVXRaWTliQgpRUUlEQVFBQgotLS0tLUVORCBQVUJMSUMgS0VZLS0tLS0KIgp9Cg==";
    int payload_len = strlen(payload);
    char signature[] = "nI0xWUJHE7KBx008r8QXLwU8To+zmLNMcbE5Ak2oFNJgNzwMz6AQ7OcK0wI1Ist3vmb5s1PTwhjnlU+kNTOzoNz/A1+nh/qbB6v72pdlPbcFe3Sc9YanvXQn62GVBGwajKEgdAikeUvmLxHMY9fNAIWpQq1z7GxOR2lw3x+q5Teyio2yLPwMKNuD8Coa3oxSc5XOsrXDu2IGDFJeN04QZo6nBOft4ksJdxgfgThhgyFDGpy19hRoOYguIEG2q+5+tu8OY5eVkA0CjbvEMvN2Fe4/MOo/eqP0eHYp/1+61WYHoRXXQmZF3PZPEhntKNC0t1iE2VIMO5XV66PFSCSuQw==";
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

    printf("%d\n", VerifyAuthTokenAndSignature(testcase, strlen(testcase), http_sig, strlen(http_sig), content, strlen(content), "action"));
    free(testcase);
    closelog();
    return 0;
}