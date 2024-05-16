#ifndef DISK_CLIENT_H
#define DISK_CLIENT_H
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// ------------------------------------------------
// Create a client
// ------------------------------------------------
void create_client(char *server_address,
                   int port,
                   int *sockfd) {
    struct sockaddr_in server_addr;

    // * Create a socket
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // * Set the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(1);
    }

    // * Connect to the server
    if (connect(*sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    // * Print the connection message
    printf("Connected to the server\n");
}
// ------------------------------------------------
// Read
// ------------------------------------------------
void read_disk_client(int sockfd, char *buffer) {
    int n;
    n = read(sockfd, buffer, 256);
    if (n < 0) {
        perror("read");
        exit(1);
    }
}
// ------------------------------------------------
// Write
// ------------------------------------------------
void write_disk_client(int sockfd, char *buffer) {
    int n;
    n = write(sockfd, buffer, 512);
    if (n < 0) {
        perror("write");
        exit(1);
    }
}

// ------------------------------------------------
// Close
// ------------------------------------------------
void close_disk_client(int sockfd) {
    close(sockfd);
}
#endif