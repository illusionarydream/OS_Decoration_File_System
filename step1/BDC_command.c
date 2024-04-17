#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// ------------------------------------------------
// Parse the parameters
// ------------------------------------------------
void parse_parameters(int argc,
                      char **argv,
                      char **server_address,
                      int *port) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_address> <port>\n", argv[0]);
        exit(1);
    }

    *server_address = argv[1];
    *port = atoi(argv[2]);
}

// ------------------------------------------------
// Create a client
// ------------------------------------------------
void create_client(char *server_address,
                   int port,
                   int *sockfd) {
    struct sockaddr_in server_addr;

    // * Create a socket
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
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
// Interact with the server
// ------------------------------------------------
void interact_with_server(int sockfd) {
    char buffer[1024];
    int n;

    while (1) {
        // * Read the command from the user
        printf("Enter the command: ");
        bzero(buffer, 1024);
        fgets(buffer, 1024, stdin);

        // * Send the command to the server
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) {
            perror("write");
            exit(1);
        }

        // * Read the response from the server
        bzero(buffer, 1024);
        n = read(sockfd, buffer, 1024);
        if (n < 0) {
            perror("read");
            exit(1);
        }

        // * Check if the server closed the connection
        if (n == 0) {
            printf("Server closed the connection\n");
            break;
        }

        // * Print the response
        printf("Response: %s\n", buffer);
    }
}

// ------------------------------------------------
// Main function
// ------------------------------------------------
int main(int argc, char **argv) {
    int port;
    char *server_address;

    // * Parse the parameters
    parse_parameters(argc, argv, &server_address, &port);

    // * Create a client
    int sockfd;
    create_client(server_address, port, &sockfd);

    // * interact with the server
    interact_with_server(sockfd);

    // * Close the socket
    close(sockfd);
}