#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include "jwtauth.h"

int main(int argc, char ** argv) {
    openlog("Test App: ", LOG_PERROR, LOG_DAEMON);
    char testcase[] = "aGVhZGVy.dGhpcyBpcyBhIGhlbGxvd29ybGQgbWVzc2FnZSEh.GbE+GkM8vkxjCVvxFuhEisWEk30bDB1B+RUiRoB8zNLl+nioRZe/QxGk57S9ibeYQL21WVPiXkHrVmfH+6O3OP0MoAl97cVIErabwV2LSiKkiAZKrguTtpmGFLnxXcKT/sfUkuvoQnipk6dwoXc2Ro1+NaAMEOz2O6QKAMN7HCwi9KcjJL7ycw5xumHuzAbhMHZvmGXMP/r4DnKPZ7BGdJMMAiX28c8yXuXrAnRII9U+RcpgF2Z/Bw+6xoqyScda+dNzRGRL7a/F12kP9dilBK1kz3EwaLI3DviO+VcI+YpLMHOwfT4+WP2MFebdoyCFWDRVgHDJAXGzNS+qL7+SNg==";

    printf("%d\n", VerifyAuth(testcase, strlen(testcase), "action"));
    closelog();
    return 0;
}