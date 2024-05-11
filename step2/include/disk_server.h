#ifndef DISK_SERVER_H
#define DISK_SERVER_H
#include "file.h"
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
struct Inode ROOT;
int root_sector_id;

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
              char command_array[10][1024]) {
    int i = 0;
    char *token;

    printf("line: %s\n", line);

    // first token
    token = strtok(line, " ");
    printf("token: %s\n", token);

    // f
    if (strcmp(token, "f") == 0) {
        strcpy(command_array[i++], "f");

        return i;
    }

    // e
    if (strcmp(token, "e") == 0) {
        strcpy(command_array[i++], "e");

        return i;
    }

    // mk f
    if (strcmp(token, "mk") == 0) {
        strcpy(command_array[i++], "mk");

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        return i;
    }

    // mkdir d
    if (strcmp(token, "mkdir") == 0) {
        strcpy(command_array[i++], "mkdir");

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        return i;
    }

    // rm f
    if (strcmp(token, "rm") == 0) {
        strcpy(command_array[i++], "rm");

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        return i;
    }

    // cd path
    if (strcmp(token, "cd") == 0) {
        strcpy(command_array[i++], "cd");

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        return i;
    }

    // rmdir d
    if (strcmp(token, "rmdir") == 0) {
        strcpy(command_array[i++], "rmdir");

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        return i;
    }

    // ls
    if (strcmp(token, "ls") == 0) {
        strcpy(command_array[i++], "ls");

        return i;
    }

    // cat f
    if (strcmp(token, "cat") == 0) {
        strcpy(command_array[i++], "cat");

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);
        return i;
    }

    // d f pos l
    if (strcmp(token, "d") == 0) {
        strcpy(command_array[i++], "d");

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        return i;
    }

    // w f l data
    if (strcmp(token, "w") == 0) {
        strcpy(command_array[i++], "w");

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        int len = atoi(command_array[2]);
        if (len <= 0)
            return -1;

        token = strtok(NULL, "");
        if (token == NULL)
            return -1;
        strncpy(command_array[i++], token, len);

        return i;
    }

    // i f pos l data
    if (strcmp(token, "i") == 0) {
        strcpy(command_array[i++], "i");

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        int len = atoi(command_array[3]);
        if (len <= 0)
            return -1;

        token = strtok(NULL, "");
        if (token == NULL)
            return -1;
        strncpy(command_array[i++], token, len);

        return i;
    }

    // adduser username password
    // the password should not contain space
    if (strcmp(token, "adduser") == 0) {
        strcpy(command_array[i++], "adduser");

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        return i;
    }

    // su username password
    // the password should not contain space
    if (strcmp(token, "su") == 0) {
        strcpy(command_array[i++], "su");

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        token = strtok(NULL, " ");
        if (token == NULL)
            return -1;
        strcpy(command_array[i++], token);

        return i;
    }
    return -1;
}

// --------------------------------------------------------------------------------------------
// Read the command from the client
// --------------------------------------------------------------------------------------------
int read_command_from_client(int client_sockfd,
                             char *buf) {
    int len = read(client_sockfd, buf, 1024);
    if (len < 2) {
        fprintf(stderr, "Error: cannot read the command from the client\n");
        return -1;
    }
    buf[len] = '\0';
    if (buf[len - 1] == '\n')
        buf[len - 1] = '\0';
    return len;
}

// --------------------------------------------------------------------------------------------
// get the current directory
// --------------------------------------------------------------------------------------------
void get_current_path(struct Inode *inode, char *name) {
    name[0] = '/';
    g_dir(inode, name);
    char buffer[4096];
    sprintf(buffer, "~%s", name);
    strcpy(name, buffer);
}

// --------------------------------------------------------------------------------------------
// Importantly, the following function is the key function in this snippet
// Execution for one client in the child process
// --------------------------------------------------------------------------------------------
void Execution_for_one_client_in_child_process(int client_sockfd) {
    // *initial the current directory in
    get_inode(ROOT.sector_id, &ROOT);
    struct Inode cur_directory = ROOT;
    cd(&cur_directory, "public");

    // *main loop
    while (1) {
        // *store the bitmap
        store_bitmap();

        // *print the current directory
        char cur_name[4096];
        bzero(cur_name, 4096);
        get_current_path(&cur_directory, cur_name);
        write(client_sockfd, cur_name, 1024);

        // *read the command from the client
        char buf[1024];
        int len = read_command_from_client(client_sockfd, buf);
        if (len == -1) {
            fprintf(stderr, "Error: cannot read the command from the client\n");
            char output[1024];
            sprintf(output, "Error: cannot read the command from the client\n");
            write(client_sockfd, output, 1024);
            continue;
        }

        // *updata the bitmap
        load_bitmap();

        // *update the current directory information
        int current_sector_id = cur_directory.sector_id;
        get_inode(current_sector_id, &cur_directory);
        get_inode(root_sector_id, &ROOT);

        // *parse the command
        char command_array[10][1024];
        int command_num = parseLine(buf, command_array);
        if (command_num == -1) {
            fprintf(stderr, "Error: cannot parse the command\n");
            char output[1024];
            sprintf(output, "Error: cannot parse the command\n");
            write(client_sockfd, output, 1024);
            continue;
        }

        // ?debug
        // printf("command_num: %d\n", command_num);
        // for (int i = 0; i < command_num; i++) {
        //     printf("command_array[%d]: %s\n", i, command_array[i]);
        // }

        // *f
        if (strcmp(command_array[0], "f") == 0) {
            for (int i = 0; i < BLOCK_NUM; i++) {
                block_bitmap[i] = '0';
            }
            create_root_directory(&ROOT);
            create_public_directory(&ROOT);
            cur_directory = ROOT;
            char output[1024];
            sprintf(output, "Successfully!\n");
            write(client_sockfd, output, 1024);
            continue;
        }

        // *e
        if (strcmp(command_array[0], "e") == 0) {
            char output[1024];
            sprintf(output, "EXIT\n");
            write(client_sockfd, output, 1024);
            break;
        }

        // *mk f
        if (strcmp(command_array[0], "mk") == 0) {
            int flag = mk_f(&cur_directory, command_array[1]);
            char output[1024];
            if (flag == -1) {
                sprintf(output, "Error: cannot create the file\n");
                write(client_sockfd, output, 1024);
                continue;
            }
            sprintf(output, "Successfully!\n");
            write(client_sockfd, output, 1024);
        }

        // *mkdir d
        if (strcmp(command_array[0], "mkdir") == 0) {
            int flag = mk_dir(&cur_directory, command_array[1]);
            char output[1024];
            if (flag == -1) {
                sprintf(output, "Error: cannot create the directory\n");
                write(client_sockfd, output, 1024);
                continue;
            }
            sprintf(output, "Successfully!\n");
            write(client_sockfd, output, 1024);
        }

        // *rm f
        if (strcmp(command_array[0], "rm") == 0) {
            int flag = rm_f(&cur_directory, command_array[1]);
            char output[1024];
            if (flag == -1) {
                sprintf(output, "Error: cannot remove the file\n");
                write(client_sockfd, output, 1024);
                continue;
            }
            sprintf(output, "Successfully!\n");
            write(client_sockfd, output, 1024);
        }

        // *cd path
        if (strcmp(command_array[0], "cd") == 0) {
            int flag = cd(&cur_directory, command_array[1]);
            char output[1024];
            if (flag == -1) {
                sprintf(output, "Error: cannot change the directory\n");
                write(client_sockfd, output, 1024);
                continue;
            }
            sprintf(output, "Successfully!\n");
            write(client_sockfd, output, 1024);
        }

        // *rmdir d
        if (strcmp(command_array[0], "rmdir") == 0) {
            int flag = rm_dir(&cur_directory, command_array[1]);
            char output[1024];
            if (flag == -1) {
                sprintf(output, "Error: cannot remove the directory\n");
                write(client_sockfd, output, 1024);
                continue;
            }
            sprintf(output, "Successfully!\n");
            write(client_sockfd, output, 1024);
        }

        // *ls
        if (strcmp(command_array[0], "ls") == 0) {
            char name[256][252];
            int name_num = ls(&cur_directory, name);
            char output[1024];
            printf("name_num: %d\n", name_num);
            if (name_num < 0) {
                sprintf(output, "Error: cannot list the directory\n");
                write(client_sockfd, output, 1024);
                continue;
            }
            if (name_num == 0) {
                sprintf(output, "Successfully!\n");
                write(client_sockfd, output, 1024);
                continue;
            }
            // write the answer to the client
            bzero(output, 1024);
            for (int i = 0; i < name_num; i++) {
                strcat(output, "\n");
                strcat(output, name[i]);
            }
            write(client_sockfd, output, 1024);
        }

        // *cat f
        if (strcmp(command_array[0], "cat") == 0) {
            char content[4096];
            int flag = cat_f(&cur_directory, command_array[1], content);
            char output[1024];
            if (flag == -1) {
                sprintf(output, "Error: cannot read the file\n");
                write(client_sockfd, output, 1024);
                continue;
            }
            // write the answer to the client
            bzero(output, 1024);
            strcat(output, content);
            write(client_sockfd, output, 1024);
        }

        // *d f pos l
        if (strcmp(command_array[0], "d") == 0) {
            int flag = d_f(&cur_directory, command_array[1], atoi(command_array[2]), atoi(command_array[3]));
            char output[1024];
            if (flag == -1) {
                sprintf(output, "Error: cannot delete the data\n");
                write(client_sockfd, output, 1024);
                continue;
            }
            sprintf(output, "Successfully!\n");
            write(client_sockfd, output, 1024);
        }

        // *w f l data
        if (strcmp(command_array[0], "w") == 0) {
            int flag = w_f(&cur_directory, command_array[1], atoi(command_array[2]), command_array[3]);
            char output[1024];
            if (flag == -1) {
                sprintf(output, "Error: cannot write the data\n");
                write(client_sockfd, output, 1024);
                continue;
            }
            sprintf(output, "Successfully!\n");
            write(client_sockfd, output, 1024);
        }

        // *i f pos l data
        if (strcmp(command_array[0], "i") == 0) {
            int flag = i_f(&cur_directory, command_array[1], atoi(command_array[2]), atoi(command_array[3]), command_array[4]);
            char output[1024];
            if (flag == -1) {
                sprintf(output, "Error: cannot insert the data\n");
                write(client_sockfd, output, 1024);
                continue;
            }
            sprintf(output, "Successfully!\n");
            write(client_sockfd, output, 1024);
        }

        // *adduser username password
        if (strcmp(command_array[0], "adduser") == 0) {
            get_inode(ROOT.sector_id, &ROOT);
            int flag = create_user(&ROOT, command_array[1], command_array[2]);
            char output[1024];
            if (flag == -1) {
                sprintf(output, "Error: cannot add the user\n");
                write(client_sockfd, output, 1024);
                continue;
            }
            sprintf(output, "Successfully!\n");
            write(client_sockfd, output, 1024);
        }

        // *su username password
        if (strcmp(command_array[0], "su") == 0) {
            get_inode(ROOT.sector_id, &ROOT);
            struct Inode user_inode;
            int flag = change_user(&ROOT, &user_inode, command_array[1], command_array[2]);
            char output[1024];
            if (flag == -1) {
                sprintf(output, "Error: cannot switch the user\n");
                write(client_sockfd, output, 1024);
                continue;
            }
            write_inode_to_disk(&cur_directory);
            cur_directory = user_inode;
            sprintf(output, "Successfully!\n");
            write(client_sockfd, output, 1024);
        }
    }
}

// --------------------------------------------------------------------------------------------
// Execution for one client
// --------------------------------------------------------------------------------------------
void Execution_for_one_client(int client_sockfd) {
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
        Execution_for_one_client_in_child_process(client_sockfd);
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
void create_disk_server(int port) {
    // *create FS server
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
        Execution_for_one_client(client_sockfd);
    }
}

#endif