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
int find_name_id(struct Inode* inode, char* name, int* inode_id) {
    // if it is a file, return -1
    if (inode->file_type == 0) {
        return -1;
    }
    // if it is a directory
    int directory_num = inode->block_num;
    for (int i = 0; i < directory_num; i++) {
        struct Naming_block naming_block;
        int id;
        get_sector_id(inode, i, &id);
        read_block(id, (char*)&naming_block);
        if (strcmp(naming_block.name, name) == 0) {
            *inode_id = naming_block.inode_sector_id;
            return 0;
        }
    }
    return -1;
}
// ---------------------------------
// add a new name to the directory
// input: inode, name, inode_id
// ---------------------------------
int add_name(struct Inode* inode, struct Inode* new_inode, char* name, int inode_id) {
    // if it is a file, return -1
    if (inode->file_type == 0) {
        return -1;
    }
    // if the name has been used
    if (find_name_id(inode, name, &inode_id) == 0) {
        return -1;
    }
    // apply for a new block
    int new_block_id;
    init_new_block(inode, &new_block_id);
    // if it is a directory
    struct Naming_block naming_block;
    strcpy(naming_block.name, name);
    naming_block.inode_sector_id = inode_id;
    // write the new name to the new block
    write_block(new_block_id, (char*)&naming_block);
    new_inode->name_inode = new_block_id;
    write_inode_to_disk(new_inode);
    write_inode_to_disk(inode);
    return 0;
}
// ---------------------------------
// remove a name from the directory
// input: inode, name
// ---------------------------------
int remove_name(struct Inode* inode, char* name) {
    // if it is a file, return -1
    if (inode->file_type == 0) {
        return -1;
    }
    // if it is a directory
    // insert the tail name into the removed place
    int directory_num = inode->block_num;
    int tail_id = directory_num - 1;
    struct Naming_block tail_naming_block;
    int tail_block_id;
    get_sector_id(inode, tail_id, &tail_block_id);
    read_block(tail_block_id, (char*)&tail_naming_block);
    // remove the name
    for (int i = 0; i < directory_num; i++) {
        struct Naming_block naming_block;
        int id;
        get_sector_id(inode, i, &id);
        read_block(id, (char*)&naming_block);
        if (strcmp(naming_block.name, name) == 0 && i != tail_id) {
            write_block(id, (char*)&tail_naming_block);
            remove_tail_block(inode);
            return 0;
        } else if (strcmp(naming_block.name, name) == 0 && i == tail_id) {
            remove_tail_block(inode);
            return 0;
        }
    }
    return -1;  // if not found
}
// ---------------------------------
// list all the names in the directory
// ---------------------------------
int list_all_name(struct Inode* inode, char name[256][252], int* name_num) {
    // if it is a file, return -1
    if (inode->file_type == 0) {
        return -1;
    }
    // if it is a directory
    int directory_num = inode->block_num;
    for (int i = 0; i < directory_num; i++) {
        struct Naming_block naming_block;
        int id;
        get_sector_id(inode, i, &id);
        read_block(id, (char*)&naming_block);
        strcpy(name[i], naming_block.name);
    }
    *name_num = directory_num;
    return 0;
}
// ---------------------------------
// get current directory
// ---------------------------------
int get_current_directory(struct Inode* inode, char* name) {
    struct Inode tmp_inode = *inode;

    // if it is a file, return -1
    if (inode->file_type == 0) {
        return -1;
    }
    while (tmp_inode.pre_inode_sector_id != -1) {
        char tmp_name[256];
        struct Naming_block tmp_naming_block;
        read_block(tmp_inode.name_inode, tmp_name);
        memcpy(&tmp_naming_block, tmp_name, sizeof(struct Naming_block));
        char buffer[4096];
        sprintf(buffer, "/%s%s", tmp_naming_block.name, name);
        strcpy(name, buffer);
        get_inode(tmp_inode.pre_inode_sector_id, &tmp_inode);
    }
    printf("name: %s\n", name);
    return 0;
}
#endif