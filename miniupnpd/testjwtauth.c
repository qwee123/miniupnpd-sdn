#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include "jwtauth.h"

int main(int argc, char ** argv) {
    openlog("Test App: ", LOG_PERROR, LOG_DAEMON);
    char header[] = "aGVhZGVy";
    int header_len = strlen(header);
    char payload[] = "eyJwdWJsaWNfa2V5IjoiLS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUlJQklqQU5CZ2txaGtpRzl3MEJBUUVGQUFPQ0FROEFNSUlCQ2dLQ0FRRUFyMktnZWQraFNGNTlQT29mZnU2dgpVaTUzUzhOSFNXc2xpUEx0N1kwVUxWRVovZTVLaTQrL1RkYm9ucnU5VnhqRys5Q21FSHArTmNDczUvWk5ZYnFBCkRZSWNWZnNjWGxGQ0h4Y2FrcHpXZHFSMW5aS20wZzFrRTNlMzduM0djYTNhZE1IclE5Qy9PRXU3amZ5MmZsZkgKOWd2RTBGVzJxYWNsUGNMMlFEK3BKQ24xc2YwbWlJNkprTEtiem0xVHhMb3o1czVwNjcxUjZIaVptaGEzajYvWgpXaUtiOS9rRXhqbzh5VHY4cVhpTnRmZ1I5cUFXYjZmb2ZuVitQRW0xNnArTVE1ODJSMUE1R3FUdG1wandoWW5wCkhqWFBIa1kxRjlyOGJVc0MwY2Z6S0tXMFBTY0RWUHZnWUJ0SUdEeFhZVjlUV1MyVkFmc01CbWFmQlV0Wlk5YkIKUVFJREFRQUIKLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tIn0=";
    int payload_len = strlen(payload);
    char signature[] = "L7sRedmHtwKwBiVbAsiSaP5zn6wvNoqsxyVjosDc4WfojtrmJIzp58FBJ+0KP2K5tUNr7rv6TXnKvQ8EDlY+2pajHRY/04wcIPZ+B8AMI7ta/IMQvtmingBjKskKUoPS+OMWn6zn5fvirOmpxpkxzODBrK3pocbc4f69kuEDy55ZkXBjiHh/40dAAShte1/Du+P5q6JfCZYs3QJx3iSA/xwgtQO1sz2qtcQwmnY1EHf0zDe4XM5WXRJ+T985d0bllMdvPfyNP5fOmn13rvX0rVyBiRIlux3sS/un62VAqf6rExdi4U0s/dKL8f34gK4Rv3XMOzDzs9j1Fh3kHRDGZg==";
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