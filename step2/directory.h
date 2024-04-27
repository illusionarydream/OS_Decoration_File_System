#ifndef DIRECTORY_H
#define DIRECTORY_H
#include "inode.h"
// ---------------------------------
// Naming block
// ---------------------------------
struct Naming_block {
    char name[252];
    uint32_t inode_sector_id;
};
// renaming basic Inode class to Directory
typedef struct Inode Directory;
// ---------------------------------
// find the inode id of the name
// ---------------------------------
int find_name_id(char* disk, struct Inode* inode, char* name, int* inode_id) {
    // if it is a file, return -1
    if (inode->file_type == 0) {
        return -1;
    }
    // if it is a directory
    int directory_num = inode->block_num;
    for (int i = 0; i < directory_num; i++) {
        struct Naming_block naming_block;
        read_block(disk, inode->direct_block[i], (char*)&naming_block);
        if (strcmp(naming_block.name, name) == 0) {
            *inode_id = naming_block.inode_sector_id;
            return 0;
        }
    }
    return -1;
}
// ---------------------------------
// add a new name to the directory
// input: disk, inode, name, inode_id
// ---------------------------------
int add_name(char* disk, struct Inode* inode, char* name, int inode_id) {
    // if it is a file, return -1
    if (inode->file_type == 0) {
        return -1;
    }
    // apply for a new block
    int new_block_id;
    init_new_block(disk, inode, &new_block_id);
    // if it is a directory
    struct Naming_block naming_block;
    strcpy(naming_block.name, name);
    naming_block.inode_sector_id = inode_id;
    // write the new name to the new block
    write_block(disk, new_block_id, (char*)&naming_block);
    return 0;
}
#endif