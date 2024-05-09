#include "include/inode.h"
#include "include/disk_server.h"
#include "include/disk_client.h"

// ------------------------------------------------
// Parse the parameters
// ------------------------------------------------
void parse_parameters(int argc,
                      char *argv[],
                      char **DSK_server_address,
                      int *BDS_port,
                      int *FS_port) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <Disk_server_address> <BDS_port><FSport>\n", argv[0]);
        exit(1);
    }
    *DSK_server_address = argv[1];
    *BDS_port = atoi(argv[2]);
    *FS_port = atoi(argv[3]);

    if (*BDS_port <= 1000 || *FS_port <= 1000 || *BDS_port > 65535 || *FS_port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(1);
    }
}

// ------------------------------------------------
// Initial the bitmap
// ------------------------------------------------
void init_bitmap() {
    int i;
    for (i = 0; i < BLOCK_NUM; i++) {
        block_bitmap[i] = '0';
    }
}

int main(int argc, char *argv[]) {
    char *Disk_server_address;
    int BDS_port;
    int FS_port;

    // * Parse the parameters
    parse_parameters(argc, argv, &Disk_server_address, &BDS_port, &FS_port);

    // ?debug
    printf("Disk server address: %s\n", Disk_server_address);
    printf("BDS port: %d\n", BDS_port);
    printf("FS port: %d\n", FS_port);

    // * Initial the disk client
    create_client(Disk_server_address, BDS_port, &SOCKET_FD);

    // * initial the bitmap
    init_bitmap();

    // * initial the root directory
    create_root_directory(&ROOT);

    // * Initial the file server
    create_disk_server(FS_port);
    return 0;
}