#include "inode.h"
#include "file.h"
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
    open_and_stretch_disk_file("disk", &fd, 1024 * 1024);
    char *mapped_diskfile;
    make_mapping_file(fd, 1024 * 1024, &mapped_diskfile);

    for (int i = 0; i < BLOCK_NUM; i++) {
        block_bitmap[i] = '0';
    }

    struct Inode root;
    initial_inode(&root, 0, -1, 1);
    block_bitmap[0] = '1';

    mk_f(mapped_diskfile, &root, "file1");
    mk_f(mapped_diskfile, &root, "file2");
    mk_f(mapped_diskfile, &root, "file3");
    mk_dir(mapped_diskfile, &root, "dir1");
    w_f(mapped_diskfile, &root, "file1", 10, "abcdefghij");
    w_f(mapped_diskfile, &root, "file2", 10, "klmnopqrst");
    w_f(mapped_diskfile, &root, "file3", 10, "uvwxyzABCD");
    char output[256];
    cat_f(mapped_diskfile, &root, "file1", output);
    printf("%s\n", output);
    cat_f(mapped_diskfile, &root, "file2", output);
    printf("%s\n", output);
    cat_f(mapped_diskfile, &root, "file3", output);
    printf("%s\n", output);
    char name[256][252];
    ls(mapped_diskfile, &root, name);
    // for (int i = 0; i < 4; i++) {
    //     printf("%s\n", name[i]);
    // }
    change_to_subdir(mapped_diskfile, &root, "dir1");
    mk_f(mapped_diskfile, &root, "file4");
    mk_f(mapped_diskfile, &root, "file5");
    mk_f(mapped_diskfile, &root, "file6");
    mk_dir(mapped_diskfile, &root, "dir2");
    change_to_subdir(mapped_diskfile, &root, "dir2");
    mk_f(mapped_diskfile, &root, "file7");
    w_f(mapped_diskfile, &root, "file7", 10, "1234567890");
    cat_f(mapped_diskfile, &root, "file7", output);
    cd(mapped_diskfile, &root, "../../dir1");
    ls(mapped_diskfile, &root, name);
    for (int i = 0; i < 4; i++) {
        printf("%s\n", name[i]);
    }
    return 0;
}