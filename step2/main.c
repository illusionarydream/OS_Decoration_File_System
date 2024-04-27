#include "inode.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
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
int main() {
    int fd;
    open_and_stretch_disk_file("disk", &fd, 1024 * 1024 * 16);
    char *mapped_diskfile;
    make_mapping_file(fd, 1024 * 1024 * 16, &mapped_diskfile);

    for (int i = 0; i < BLOCK_NUM; i++) {
        block_bitmap[i] = '0';
    }

    struct Inode inode;
    initial_inode(&inode, 0, -1, 1);
    block_bitmap[0] = '1';

    for (int i = 0; i < 10000; i++) {
        int id;
        init_new_block(mapped_diskfile, &inode, &id);
    }

    for (int i = 0; i < 10000; i++) {
        printf("%d\n", inode.block_num);
        remove_tail_block(mapped_diskfile, &inode);
    }

    for (int i = 0; i < 10000; i++) {
        int id;
        init_new_block(mapped_diskfile, &inode, &id);
    }
    for (int i = 0; i < 10000; i++) {
        printf("%d\n", inode.block_num);
        remove_tail_block(mapped_diskfile, &inode);
    }

    for (int i = 0; i < 10000; i++) {
        int id;
        init_new_block(mapped_diskfile, &inode, &id);
    }
    for (int i = 0; i < 10000; i++) {
        int id;
        get_sector_id(mapped_diskfile, &inode, i, &id);
        printf("id: %d\n", id);
    }
    return 0;
}