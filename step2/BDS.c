#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
// --------------------------------------------------------------------------------------------
// Decode the parameters from the command line
// --------------------------------------------------------------------------------------------
void decode_parameters(int argc,
                       char *argv[],
                       char **DiskFileName,
                       int *cylinder_num,
                       int *sector_num,
                       int *track_to_track_delay,
                       int *port,
                       int block_size,
                       long *FileSize) {
    if (argc != 6) {
        fprintf(stderr, "Usage: BDS <DiskFileName> <cylinder_num> <sector_num> <track_to_track_delay> <port>\n");
        exit(1);
    }
    *DiskFileName = argv[1];
    *cylinder_num = atoi(argv[2]);
    *sector_num = atoi(argv[3]);
    *track_to_track_delay = atoi(argv[4]);
    *port = atoi(argv[5]);

    // Check the validity of the parameters
    if (*cylinder_num < 1 || *cylinder_num > 100) {
        fprintf(stderr, "Error: cylinder_num should be between 1 and 100\n");
        exit(1);
    }
    if (*sector_num < 1 || *sector_num > 100) {
        fprintf(stderr, "Error: sector_num should be between 1 and 100\n");
        exit(1);
    }
    if (*track_to_track_delay < 1 || *track_to_track_delay > 100) {
        fprintf(stderr, "Error: track_to_track_delay should be between 1 and 100\n");
        exit(1);
    }
    if (*port < 1024 || *port > 65535) {
        fprintf(stderr, "Error: port should be between 1024 and 65535\n");
        exit(1);
    }
    // Calculate the size of the disk file
    *FileSize = (long)(*cylinder_num) * (long)(*sector_num) * (long)block_size;
}

// --------------------------------------------------------------------------------------------
// Open the disk file
// --------------------------------------------------------------------------------------------
void open_and_stretch_disk_file(char *DiskFileName,
                                int *fd,
                                long FileSize) {
    *fd = open(DiskFileName, O_RDWR | O_CREAT, 0666);
    if (*fd == -1) {
        fprintf(stderr, "Error: cannot open the disk file\n");
        exit(1);
    }
    if (lseek(*fd, FileSize - 1, SEEK_SET) == -1) {
        fprintf(stderr, "Error: cannot stretch the disk file\n");
        close(*fd);
        exit(1);
    }
    if (write(*fd, "", 1) != 1) {
        fprintf(stderr, "Error: cannot write to the disk file\n");
        close(*fd);
        exit(1);
    }
}

// --------------------------------------------------------------------------------------------
// Make the mapping file
// --------------------------------------------------------------------------------------------
void make_mapping_file(int fd,
                       int File_Size,
                       char **mapped_diskfile) {
    *mapped_diskfile = (char *)mmap(NULL, File_Size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_diskfile == MAP_FAILED) {
        fprintf(stderr, "Error: cannot map the disk file\n");
        close(fd);
        exit(1);
    }
}

// --------------------------------------------------------------------------------------------
// Read the disk file
// --------------------------------------------------------------------------------------------
int read_disk_file(char *mapped_diskfile,
                   char *buf,
                   int sector_num,
                   int block_size,
                   int File_Size,
                   int c,
                   int s) {
    int offset = (c * sector_num + s) * block_size;
    if (offset + block_size > File_Size || offset < 0) {
        fprintf(stderr, "Error: the disk file is too small\n");
        return 0;
    }
    memcpy(buf, mapped_diskfile + offset, block_size);
    return 1;
}

// --------------------------------------------------------------------------------------------
// Write the disk file
// --------------------------------------------------------------------------------------------
int write_disk_file(char *mapped_diskfile,
                    char *buf,
                    int sector_num,
                    int block_size,
                    int File_Size,
                    int c,
                    int s,
                    int l) {
    int offset = (c * sector_num + s) * block_size;
    if (offset + l > File_Size || offset < 0) {
        fprintf(stderr, "Error: the disk file is too small\n");
        return 0;
    }
    memcpy(mapped_diskfile + offset, buf, l);
    return 1;
}

// --------------------------------------------------------------------------------------------
// Create the server to the port
// --------------------------------------------------------------------------------------------
void create_server(int *sockfd,
                   int port) {
    struct sockaddr_in server_addr;
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd == -1) {
        fprintf(stderr, "Error: cannot create the socket\n");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    if (bind(*sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        fprintf(stderr, "Error: cannot bind the server to the port\n");
        close(*sockfd);
        exit(1);
    }
    listen(*sockfd, 5);
    printf("Success: create the server\n");
    printf("Server is listening on port %d\n", port);
}

// --------------------------------------------------------------------------------------------
// Parse the command line
// --------------------------------------------------------------------------------------------
int parseLine(char *line,
              int *id,
              char *data) {
    // Read
    if (line[0] == 'R') {
        // the id
        sscanf(line, "R %d ", id);
        return 2;
    }
    // Write
    if (line[0] == 'W') {
        // the id
        sscanf(line, "W %d ", id);
        // the data
        memcpy(data, line + 256, 256);
        return 3;
    }
    return -1;
}

// --------------------------------------------------------------------------------------------
// Read the command from the client
// --------------------------------------------------------------------------------------------
int read_command_from_client(int client_sockfd,
                             char *buf) {
    // * For this particular job, we read the command from the client for 512 bytes each time
    int n = read(client_sockfd, buf, 512);
    if (n < 0) {
        fprintf(stderr, "Error: cannot read the command from the client\n");
        return 0;
    }
    printf("Received command: %s\n", buf);
    for (int i = 0; i < 512; i++) {
        printf("%d", (int)buf[i]);
    }
    printf("\n");
    return 1;
}

// --------------------------------------------------------------------------------------------
// Importantly, the following function is the key function in this snippet
// Execution for one client in the child process
// --------------------------------------------------------------------------------------------
void Execution_for_one_client_in_child_process(
    int client_sockfd,
    char *mapped_diskfile,
    int sector_num,
    int block_size,
    int File_Size,
    int track_to_track_delay) {
    // ?for the following code, we can see that it is the same as the code in the main function
    // track_to_track_delay = 0;
    while (1) {
        printf("Waiting for the command from the client\n");
        // *read the command from the client
        char buf[512];
        if (!read_command_from_client(client_sockfd, buf)) {
            break;
        }

        // *parse the command
        int id;
        char data[256];
        int cmd_num = parseLine(buf, &id, data);

        // ?debug
        // printf("cmd_num: %d\n", cmd_num);
        // printf("id: %d\n", id);

        // *handle the read command
        if (cmd_num == 2) {
            int c = id / sector_num;
            int s = id % sector_num;
            char buffer[256];
            if (!read_disk_file(mapped_diskfile,
                                buffer,
                                sector_num,
                                block_size,
                                File_Size,
                                c,
                                s)) {
                break;
            }
            write(client_sockfd, buffer, 256);
        }

        // *handle the write command
        else if (cmd_num == 3) {
            int c = id / sector_num;
            int s = id % sector_num;
            char *buffer = data;
            if (!write_disk_file(mapped_diskfile,
                                 buffer,
                                 sector_num,
                                 block_size,
                                 File_Size,
                                 c,
                                 s,
                                 256)) {
                break;
            }
        }
    }
}

// --------------------------------------------------------------------------------------------
// Execution for one client
// --------------------------------------------------------------------------------------------
void Execution_for_one_client(int client_sockfd,
                              char *mapped_diskfile,
                              int sector_num,
                              int block_size,
                              int File_Size,
                              int track_to_track_delay) {
    // *fork a child process
    int pid = fork();
    if (pid == -1) {
        fprintf(stderr, "Error: cannot fork a child process\n");
        close(client_sockfd);
        return;
    }

    // *child process
    if (pid == 0) {
        // *handle one client's commands in the child process
        Execution_for_one_client_in_child_process(client_sockfd,
                                                  mapped_diskfile,
                                                  sector_num,
                                                  block_size,
                                                  File_Size,
                                                  track_to_track_delay);
    }

    // *parent process
    else {
        // *close the client socket
        close(client_sockfd);
    }
}

// --------------------------------------------------------------------------------------------
// Build the server
// Bind the server to the port
// Accept the client
// Execute the client's commands
// --------------------------------------------------------------------------------------------
void interaction_between_server_and_clients(char *mapped_diskfile,
                                            int sector_num,
                                            int block_size,
                                            int File_Size,
                                            int track_to_track_delay,
                                            int port) {
    // *build server
    int sockfd;
    create_server(&sockfd, port);

    // *client execution
    while (1) {
        // *accept the client
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);

        // *handle error or disconnection
        if (client_sockfd == -1) {
            fprintf(stderr, "Error: cannot accept the client\n");
            continue;
        }

        // *print the client's IP address and port
        printf("Client %s:%d connected\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        // *execute the client's commands
        Execution_for_one_client(client_sockfd,
                                 mapped_diskfile,
                                 sector_num,
                                 block_size,
                                 File_Size,
                                 track_to_track_delay);
    }
}

// --------------------------------------------------------------------------------------------
// Main function
// --------------------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    // *Initialize the parameters
    char *DiskFileName;
    int cylinder_num;
    int sector_num;
    int block_size = 256;
    int track_to_track_delay;
    int port;
    long FileSize;

    // *Decode the parameters from the command line
    decode_parameters(argc,
                      argv,
                      &DiskFileName,
                      &cylinder_num,
                      &sector_num,
                      &track_to_track_delay,
                      &port,
                      block_size,
                      &FileSize);

    // *Open the disk file and stretch it to the size of the disk
    int fd;  // file descriptor
    open_and_stretch_disk_file(DiskFileName,
                               &fd,
                               FileSize);

    // *Make the mapping file
    char *mapped_diskfile;  // the true file in inner memory
    make_mapping_file(fd,
                      FileSize,
                      &mapped_diskfile);

    // *Execute the server and clients
    interaction_between_server_and_clients(mapped_diskfile,
                                           sector_num,
                                           block_size,
                                           FileSize,
                                           track_to_track_delay,
                                           port);

    // *Close the disk file
    close(fd);

    return 0;
}