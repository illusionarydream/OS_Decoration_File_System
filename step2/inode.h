#ifndef INODE_H
#define INODE_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// block bitmap: '1' means the block is used, '0' means the block is free
// global variable
#define BLOCK_NUM 100000
char block_bitmap[BLOCK_NUM];
// ---------------------------------
// Inode
// size: 256 bytes
// ---------------------------------
// file size(4 byte): 4 bytes
// sector id: 4 bytes
// pre_inode sector id: 4 bytes
// directory_num: 4 bytes(if file type is directory)
// file type: 4 bytes
// direct block: 40 * 4 bytes
// indirect block: 10 * 4 bytes
// double indirect block: 9 * 4 bytes
// ---------------------------------
struct Inode {
    // ---------------------------------
    // data
    // ---------------------------------
    int file_size;                 // the stored file size
    int sector_id;                 // self sector id
    int pre_inode_sector_id;       // pre inode sector id
    int file_type;                 // file type
    int block_num;                 // directory num
    int direct_block[40];          // direct block
    int indirect_block[10];        // indirect block
    int double_indirect_block[9];  // double indirect block
};
// ---------------------------------
// write block data
// ---------------------------------
int write_block(char* disk, int sector_id, char data[256]) {
    // write data to the sector id
    memcpy(&disk[sector_id * 256], data, 256);
    return 0;
}
// ---------------------------------
// read block data
// ---------------------------------
void read_block(char* disk, int sector_id, char data[256]) {
    // read data from the sector id
    memcpy(data, &disk[sector_id * 256], 256);
}

int initial_inode(struct Inode* inode, uint32_t sector_id, uint32_t pre_inode_sector_id, uint32_t file_type) {
    inode->file_size = 0;
    inode->sector_id = sector_id;
    inode->pre_inode_sector_id = pre_inode_sector_id;
    inode->file_type = file_type;
    inode->block_num = 0;
    for (int i = 0; i < 40; i++) {
        inode->direct_block[i] = -1;
    }
    for (int i = 0; i < 10; i++) {
        inode->indirect_block[i] = -1;
    }
    for (int i = 0; i < 9; i++) {
        inode->double_indirect_block[i] = -1;
    }
    return 0;
}
// ---------------------------------
// get the i th sector index of the inode file
// ---------------------------------
int get_sector_id(char* disk, struct Inode* inode, int index, int* sector_id) {
    if (index >= inode->block_num) {
        return -1;
    }
    // if the index is in the direct block
    if (index < 40) {
        // read the data from the direct block
        *sector_id = inode->direct_block[index];
        return 0;
    }
    // if the index is in the first-level indirect block
    if (index < 40 + 64 * 10) {
        // read the data from the first-level indirect block
        int indirect_index = index - 40;
        int first_index = indirect_index / 64;
        int second_index = indirect_index % 64;
        char indirect_data[256];
        read_block(disk, inode->indirect_block[first_index], indirect_data);
        int* indirect_block = (int*)indirect_data;
        *sector_id = indirect_block[second_index];
        return 0;
    }
    // if the index is in the second-level indirect block
    if (index < 40 + 64 * 10 + 64 * 64 * 9) {
        // read the data from the second-level indirect block
        int indirect_index = index - 40 - 64 * 10;
        int first_index = indirect_index / 64 / 64;
        int second_index = indirect_index / 64 % 64;
        int third_index = indirect_index % 64;
        char indirect_data[256];
        read_block(disk, inode->double_indirect_block[first_index], indirect_data);
        int* double_indirect_block = (int*)indirect_data;
        read_block(disk, double_indirect_block[second_index], indirect_data);
        int* indirect_block = (int*)indirect_data;
        *sector_id = indirect_block[third_index];
        return 0;
    }
    return -1;
}
// ---------------------------------
// write inode data (256 bytes in one time)
// input: disk, index, inode, inode_data
// it can only write the data to the used block
// ---------------------------------
int write_inode(char* disk, struct Inode* inode, int index, char* inode_data) {
    // write the inode data to the disk
    int sector_id;
    int flag = get_sector_id(disk, inode, index, &sector_id);
    if (flag == -1 || sector_id < 0 || block_bitmap[sector_id] == '0') {
        return -1;
    }
    write_block(disk, sector_id, inode_data);
    return 0;
}
// ---------------------------------
// read inode data (256 bytes in one time)
// input: disk, index, inode
// output: inode_data
// it can only read the data from the used block
// ---------------------------------
int read_inode(char* disk, struct Inode* inode, int index, char* inode_data) {
    // read the inode data from the disk
    int sector_id;
    int flag = get_sector_id(disk, inode, index, &sector_id);
    if (flag == -1 || sector_id < 0 || block_bitmap[sector_id] == '0') {
        return -1;
    }
    read_block(disk, sector_id, inode_data);
    return 0;
}
// ---------------------------------
// find free block
// ---------------------------------
int find_free_block() {
    for (int i = 0; i < BLOCK_NUM; i++) {
        if (block_bitmap[i] == '0') {
            return i;
        }
    }
    return -1;
}
// ---------------------------------
// initialize new block
// ---------------------------------
int init_new_block(char* disk, struct Inode* inode, int* sector_id) {
    // if there is no free block
    if (inode->block_num >= 40 + 64 * 10 + 64 * 64 * 9) {
        return -1;
    }
    // find the first free block
    inode->block_num++;
    *sector_id = find_free_block();
    if (*sector_id == -1) {
        return -1;
    }
    // update the block bitmap
    block_bitmap[*sector_id] = '1';
    // write the sector id to the inode
    // if the index is in the direct block
    int index = inode->block_num - 1;
    if (inode->block_num <= 40) {
        inode->direct_block[index] = *sector_id;
        return 0;
    }
    // if the index is in the first-level indirect block
    if (inode->block_num <= 40 + 64 * 10) {
        int indirect_index = index - 40;
        int first_index = indirect_index / 64;
        int second_index = indirect_index % 64;
        // if the indirect block is not initialized, we need to intialize it
        if (second_index == 0) {
            inode->indirect_block[first_index] = find_free_block();
            if (inode->indirect_block[first_index] == -1) {
                return -1;
            }
            block_bitmap[inode->indirect_block[first_index]] = '1';
        }
        // write the sector id to the indirect block
        char indirect_data[256];
        read_block(disk, inode->indirect_block[first_index], indirect_data);
        int* indirect_block = (int*)indirect_data;
        indirect_block[second_index] = *sector_id;
        write_block(disk, inode->indirect_block[first_index], indirect_data);
        return 0;
    }
    // if the index is in the second-level indirect block
    if (inode->block_num <= 40 + 64 * 10 + 64 * 64 * 9) {
        int double_indirect_index = index - 40 - 64 * 10;
        int first_index = double_indirect_index / 64 / 64;
        int second_index = double_indirect_index / 64 % 64;
        int third_index = double_indirect_index % 64;
        // if the double indirect block is not initialized, we need to intialize it
        if (second_index == 0 && third_index == 0) {
            inode->double_indirect_block[first_index] = find_free_block();
            if (inode->double_indirect_block[first_index] == -1) {
                return -1;
            }
            block_bitmap[inode->double_indirect_block[first_index]] = '1';
        }
        // if the indirect block is not initialized, we need to intialize it
        if (third_index == 0) {
            char indirect_data[256];
            read_block(disk, inode->double_indirect_block[first_index], indirect_data);
            int* double_indirect_block = (int*)indirect_data;
            double_indirect_block[second_index] = find_free_block();
            if (double_indirect_block[second_index] == -1) {
                return -1;
            }
            block_bitmap[double_indirect_block[second_index]] = '1';
            write_block(disk, inode->double_indirect_block[first_index], indirect_data);
        }
        // write the sector id to the indirect block
        char indirect_data[256];
        char double_indirect_data[256];
        read_block(disk, inode->double_indirect_block[first_index], indirect_data);
        int* double_indirect_block = (int*)indirect_data;
        read_block(disk, double_indirect_block[second_index], double_indirect_data);
        int* indirect_block = (int*)double_indirect_data;
        indirect_block[third_index] = *sector_id;
        write_block(disk, double_indirect_block[second_index], double_indirect_data);
        return 0;
    }
    return -1;
}
// ---------------------------------
// remove the tail block
// ---------------------------------
int remove_tail_block(char* disk, struct Inode* inode) {
    // if there is no block
    if (inode->block_num == 0) {
        return -1;
    }
    // if the index is in the direct block
    int index = inode->block_num - 1;
    if (index < 40) {
        block_bitmap[inode->direct_block[index]] = '0';
        inode->direct_block[index] = -1;
        inode->block_num--;
        return 0;
    }
    // if the index is in the first-level indirect block
    if (index < 40 + 64 * 10) {
        int indirect_index = index - 40;
        int first_index = indirect_index / 64;
        int second_index = indirect_index % 64;
        char indirect_data[256];
        read_block(disk, inode->indirect_block[first_index], indirect_data);
        int* indirect_block = (int*)indirect_data;
        block_bitmap[indirect_block[second_index]] = '0';
        indirect_block[second_index] = -1;
        write_block(disk, inode->indirect_block[first_index], indirect_data);
        inode->block_num--;
        // remove the indirect block
        if (second_index == 0) {
            block_bitmap[inode->indirect_block[first_index]] = '0';
            inode->indirect_block[first_index] = -1;
        }
        return 0;
    }
    // if the index is in the second-level indirect block
    if (index < 40 + 64 * 10 + 64 * 64 * 9) {
        // get index
        int double_indirect_index = index - 40 - 64 * 10;
        int first_index = double_indirect_index / 64 / 64;
        int second_index = double_indirect_index / 64 % 64;
        int third_index = double_indirect_index % 64;
        // char data
        char indirect_data[256];
        char double_indirect_data[256];
        // get the first layer data
        read_block(disk, inode->double_indirect_block[first_index], indirect_data);
        int* double_indirect_block = (int*)indirect_data;
        // get the second layer data
        read_block(disk, double_indirect_block[second_index], double_indirect_data);
        int* indirect_block = (int*)double_indirect_data;
        block_bitmap[indirect_block[third_index]] = '0';
        indirect_block[third_index] = -1;
        write_block(disk, double_indirect_block[second_index], double_indirect_data);
        inode->block_num--;
        // remove the double indirect block
        if (third_index == 0) {
            block_bitmap[double_indirect_block[second_index]] = '0';
            double_indirect_block[second_index] = -1;
            write_block(disk, inode->double_indirect_block[first_index], indirect_data);
        }
        // remove the indirect block
        if (second_index == 0 && third_index == 0) {
            block_bitmap[inode->double_indirect_block[first_index]] = '0';
            inode->double_indirect_block[first_index] = -1;
        }
        return 0;
    }
    return -1;
}
#endif