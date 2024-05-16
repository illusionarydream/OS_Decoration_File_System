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
// Write to the server
// ------------------------------------------------
int write_to_server(int sockfd,
                    char *buffer) {
    int n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        perror("write");
        exit(1);
    }
    return n;
}

// ------------------------------------------------
// Read from the server
// ------------------------------------------------
int read_from_server(int sockfd,
                     char *buffer) {
    int n = read(sockfd, buffer, 1024);
    if (n < 0) {
        perror("read");
        exit(1);
    }
    return n;
}

// ------------------------------------------------
// Randomly generate a command
// ------------------------------------------------
char generate_command(char *buffer,
                      int cylinder_num,
                      int sector_num) {
    // Generate a random number between 0 and 1
    char random_R_or_W = rand() % 2 == 0 ? 'R' : 'W';

    if (random_R_or_W == 'R') {
        // Generate c
        int random_c = rand() % cylinder_num;
        // Generate s
        int random_s = rand() % sector_num;

        sprintf(buffer, "%c %d %d\n", random_R_or_W, random_c, random_s);
    }

    if (random_R_or_W == 'W') {
        // Generate c
        int random_c = rand() % cylinder_num;
        // Generate s
        int random_s = rand() % sector_num;
        // Generate constant l
        int constant_l = 256;
        // Generate a random string of length l
        char random_string[constant_l + 1];
        for (int i = 0; i < constant_l; i++) {
            random_string[i] = 'a' + rand() % 26;
        }
        random_string[constant_l] = '\0';

        sprintf(buffer, "%c %d %d %d %s\n", random_R_or_W, random_c, random_s, constant_l, random_string);
    }
    return random_R_or_W;
}

// ------------------------------------------------
// Interact with the server for N times
// ------------------------------------------------
void interact_with_server_for_N_times(int sockfd,
                                      int N) {
    char buffer[1024];

    // Information
    printf("Enter the command: I\n");
    write_to_server(sockfd, "I\n");
    bzero(buffer, 1024);
    read_from_server(sockfd, buffer);
    printf("Information: %s\n", buffer);

    // Sparse the Cylinder_num and Sector_num from the information
    int Cylinder_num, Sector_num;
    sscanf(buffer, "Cylinder number: %d\nSector number: %d\n", &Cylinder_num, &Sector_num);

    // randomly send N massages to the server
    for (int i = 0; i < N; i++) {
        usleep(1000 * 50);
        // Send the command to the server
        char ans = generate_command(buffer, Cylinder_num, Sector_num);
        // printf("The %d random command: %s", i, buffer);
        printf("%c\n", ans);
        write_to_server(sockfd, buffer);

        // Read the response from the server
        bzero(buffer, 1024);
        read_from_server(sockfd, buffer);

        // Print the response
        // printf("Response: %s\n", buffer);
    }
}

// ------------------------------------------------
// Interact with the server
// ------------------------------------------------
void interact_with_server(int sockfd) {
    char buffer[1024];
    int N;

    while (1) {
        // * Read the command from the user
        printf("Enter the number of the random commands: ");
        bzero(buffer, 1024);
        fgets(buffer, 1024, stdin);

        // * Check if the user wants to exit
        if (strncmp(buffer, "exit", 4) == 0) {
            write_to_server(sockfd, "exit\n");
            break;
        }

        // * Sparse the number of the random commands
        sscanf(buffer, "%d", &N);
        if (N > 1000 || N < 0) {
            printf("The number of the random commands should be in the range of [0, 1000]\n");
            continue;
        }

        // * Interact with the server for N times
        interact_with_server_for_N_times(sockfd, N);
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