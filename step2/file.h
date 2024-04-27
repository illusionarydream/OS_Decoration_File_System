#ifndef FILE_H
#define FILE_H
#include "inode.h"
#include "directory.h"
// --------------------------------------------------------------------------------------------
// Create a new file in the directory
// inode: the inode of the parent directory
// --------------------------------------------------------------------------------------------
int create_file(char *disk, struct Inode *inode, char *name) {
    // if it is a file, return -1
    if (inode->file_type == 0) {
        return -1;
    }
    // if the name has been used
    int inode_id;
    if (find_name_id(disk, inode, name, &inode_id) == 0) {
        return -1;
    }
    // apply for a new inode
    int new_inode_id;
    struct Inode new_inode;
    init_new_inode(disk, &new_inode, &new_inode_id, inode->sector_id, 0);
    // add the new inode to the directory
    add_name(disk, inode, name, new_inode_id);
    return 0;
}
// --------------------------------------------------------------------------------------------
// create a new directory in the directory
// inode: the inode of the parent directory
// --------------------------------------------------------------------------------------------
int create_directory(char *disk, struct Inode *inode, char *name) {
    // if it is a file, return -1
    if (inode->file_type == 0) {
        return -1;
    }
    // if the name has been used
    int inode_id;
    if (find_name_id(disk, inode, name, &inode_id) == 0) {
        return -1;
    }
    // apply for a new inode
    int new_inode_id;
    struct Inode new_inode;
    init_new_inode(disk, &new_inode, &new_inode_id, inode->sector_id, 1);
    // add the new inode to the directory
    add_name(disk, inode, name, new_inode_id);
    return 0;
}
// --------------------------------------------------------------------------------------------
// Clear the content of a file
// --------------------------------------------------------------------------------------------
int clear_file(char *disk, struct Inode *inode) {
    // if it is a directory, return -1
    if (inode->file_type == 1) {
        return -1;
    }
    // clear the content of the file
    int block_num = inode->block_num;
    for (int i = 0; i < block_num; i++)
        remove_tail_block(disk, inode);
    return 0;
}
// --------------------------------------------------------------------------------------------
// Write to a file
// --------------------------------------------------------------------------------------------
int write_file(char *disk, struct Inode *inode, int length, char *content) {
    // if it is a directory, return -1
    if (inode->file_type == 1) {
        return -1;
    }
    // clear the content of the file
    clear_file(disk, inode);
    // printf("id: %d\n", inode->sector_id);
    // write the content to the file
    int block_num = (length + 255) / 256;
    inode->file_size = length;
    for (int i = 0; i < block_num; i++) {
        int id;
        init_new_block(disk, inode, &id);
        write_block(disk, id, content + i * 256);
    }
    write_inode_to_disk(disk, inode);
    return 0;
}
// --------------------------------------------------------------------------------------------
// Read from a file
// --------------------------------------------------------------------------------------------
int read_file(char *disk, struct Inode *inode, int length, char *content) {
    // if it is a directory, return -1
    if (inode->file_type == 1) {
        return -1;
    }
    // read the content from the file
    int block_num = (length + 255) / 256;
    read_inode(disk, inode, 0, content);
    for (int i = 0; i < block_num; i++) {
        read_inode(disk, inode, i, content + i * 256);
    }
    return 0;
}
int read_whole_file(char *disk, struct Inode *inode, char *content) {
    return read_file(disk, inode, inode->file_size, content);
}
// --------------------------------------------------------------------------------------------
// Remove a file
// inode: the inode of the parent directory
// --------------------------------------------------------------------------------------------
int remove_file(char *disk, struct Inode *inode, char *name) {
    // if it is a file, return -1
    if (inode->file_type == 0) {
        return -1;
    }
    // find the inode of the file and if it does not exist, return -1
    int inode_id;
    if (find_name_id(disk, inode, name, &inode_id) == -1) {
        return -1;
    }
    // clear the content of the file
    struct Inode file_inode;
    get_inode(disk, inode_id, &file_inode);
    clear_file(disk, &file_inode);
    // remove the file name from the directory
    remove_name(disk, inode, name);
    // free the block
    block_bitmap[inode_id] = '0';
    return 0;
}
// --------------------------------------------------------------------------------------------
// Remove a directory
// inode: the inode of the parent directory
// --------------------------------------------------------------------------------------------
int remove_directory(char *disk, struct Inode *inode, char *name) {
    // if it is a file, return -1
    if (inode->file_type == 0) {
        return -1;
    }
    // find the inode of the directory and if it does not exist, return -1
    int inode_id;
    if (find_name_id(disk, inode, name, &inode_id) == -1) {
        return -1;
    }
    // clear the content of the directory
    struct Inode directory_inode;
    get_inode(disk, inode_id, &directory_inode);
    int block_num = directory_inode.block_num;
    for (int i = 0; i < block_num; i++) {
        // get the sub name
        int id;
        get_sector_id(disk, &directory_inode, i, &id);
        struct Naming_block naming_block;
        read_block(disk, id, (char *)&naming_block);
        // get the sub inode
        int sub_inode_id = naming_block.inode_sector_id;
        struct Inode sub_inode;
        get_inode(disk, sub_inode_id, &sub_inode);
        // if it is a file, remove the file
        if (sub_inode.file_type == 0) {
            remove_file(disk, &directory_inode, naming_block.name);
        }
        // if it is a directory, remove the directory
        else {
            remove_directory(disk, &directory_inode, naming_block.name);
        }
    }
    // remove the directory name from the directory
    remove_name(disk, inode, name);
    // free the block
    block_bitmap[inode_id] = '0';
    return 0;
}
// --------------------------------------------------------------------------------------------
// change the main directory to the parent directory
// --------------------------------------------------------------------------------------------
int change_to_parentdir(char *disk, struct Inode *inode) {
    if (inode->file_type == 0) {
        return -1;
    }
    if (inode->pre_inode_sector_id == -1) {
        return -1;
    }
    write_inode_to_disk(disk, inode);
    get_inode(disk, inode->pre_inode_sector_id, inode);
    return 0;
}
// --------------------------------------------------------------------------------------------
// change the main directory to the sub directory
// --------------------------------------------------------------------------------------------
int change_to_subdir(char *disk, struct Inode *inode, char *name) {
    if (inode->file_type == 0) {
        return -1;
    }
    int inode_id;
    if (find_name_id(disk, inode, name, &inode_id) == -1) {
        return -1;
    }
    struct Inode sub_inode;
    get_inode(disk, inode_id, &sub_inode);
    if (sub_inode.file_type == 0) {
        return -1;
    }
    write_inode_to_disk(disk, inode);
    get_inode(disk, inode_id, inode);
    return 0;
}
// * --------------------------------------------------------------------------------------------
// * real operation
// --------------------------------------------------------------------------------------------
// mk f
// --------------------------------------------------------------------------------------------
int mk_f(char *disk, struct Inode *parent_directory, char *name) {
    return create_file(disk, parent_directory, name);
}
// --------------------------------------------------------------------------------------------
// mkdir
// --------------------------------------------------------------------------------------------
int mk_dir(char *disk, struct Inode *parent_directory, char *name) {
    return create_directory(disk, parent_directory, name);
}
// --------------------------------------------------------------------------------------------
// rm f
// --------------------------------------------------------------------------------------------
int rm_f(char *disk, struct Inode *parent_directory, char *name) {
    return remove_file(disk, parent_directory, name);
}
// --------------------------------------------------------------------------------------------
// rm dir
// --------------------------------------------------------------------------------------------
int rm_dir(char *disk, struct Inode *parent_directory, char *name) {
    return remove_directory(disk, parent_directory, name);
}
// --------------------------------------------------------------------------------------------
// ls
// --------------------------------------------------------------------------------------------
int ls(char *disk, struct Inode *parent_directory, char name[256][252]) {
    int name_num;
    list_all_name(disk, parent_directory, name, &name_num);
    return name_num;
}
// -------------------------------------------------------------------------------------------
// w_f
// -------------------------------------------------------------------------------------------
int w_f(char *disk, struct Inode *parent_directory, char *name, int length, char *content) {
    int inode_id;
    if (find_name_id(disk, parent_directory, name, &inode_id) == -1) {
        return -1;
    }
    struct Inode inode;
    get_inode(disk, inode_id, &inode);
    return write_file(disk, &inode, length, content);
}
// -------------------------------------------------------------------------------------------
// cat_f
// -------------------------------------------------------------------------------------------
int cat_f(char *disk, struct Inode *parent_directory, char *name, char *content) {
    int inode_id;
    if (find_name_id(disk, parent_directory, name, &inode_id) == -1) {
        return -1;
    }
    struct Inode inode;
    get_inode(disk, inode_id, &inode);
    return read_whole_file(disk, &inode, content);
}
// -------------------------------------------------------------------------------------------
// cd
// -------------------------------------------------------------------------------------------
int parse_cd(char *command, char *token[256], int *token_num) {
    *token_num = 0;
    char *p = strtok(command, "/");
    while (p != NULL) {
        token[*token_num++] = p;
        p = strtok(NULL, "/");
    }
    return 0;
}
int cd(char *disk, struct Inode *inode, char *path) {
    char *token[256];
    int token_num = 0;
    parse_cd(path, token, &token_num);
    // ?debug
    for (int i = 0; i < token_num; i++) {
        printf("%s\n", token[i]);
    }
    write_inode_to_disk(disk, inode);
    for (int i = 0; i < token_num; i++) {
        if (strcmp(token[i], "..") == 0) {
            if (change_to_parentdir(disk, inode) == -1) {
                return -1;
            }
        } else if (strcmp(token[i], ".") == 0) {
            continue;
        } else if (change_to_subdir(disk, inode, token[i]) == -1) {
            return -1;
        }
    }
    return 0;
}
#endif