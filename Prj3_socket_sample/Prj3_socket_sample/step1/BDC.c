#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "tcp_utils.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <Port>", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    tcp_client client = client_init("localhost", port);
    static char buf[4096];
    while (1) {
        fgets(buf, sizeof(buf), stdin);
        if (feof(stdin)) break;
        client_send(client, buf, strlen(buf) + 1);
        int n = client_recv(client, buf, sizeof(buf));
        buf[n] = 0;
        printf("%s\n", buf);
        if (strcmp(buf, "Bye!") == 0) break;
    }
    client_destroy(client);
}
