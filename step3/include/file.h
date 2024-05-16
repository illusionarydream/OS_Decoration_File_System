#ifndef FILE_H
#define FILE_H
#include "inode.h"
#include "directory.h"
// --------------------------------------------------------------------------------------------
// Create a new file in the directory
// inode: the inode of the parent directory
// --------------------------------------------------------------------------------------------
int create_file(struct Inode *inode, char *name) {
    // if it is a file, return -1
    if (inode->file_type == 0) {
        return -1;
    }
    // if the name has been used
    int inode_id;
    if (find_name_id(inode, name, &inode_id) == 0) {
        return -1;
    }
    // apply for a new inode
    int new_inode_id;
    struct Inode new_inode;
    init_new_inode(&new_inode, &new_inode_id, inode->sector_id, 0);
    // add the new inode to the directory
    add_name(inode, &new_inode, name, new_inode_id);
    return 0;
}
// --------------------------------------------------------------------------------------------
// create a new directory in the directory
// inode: the inode of the parent directory
// --------------------------------------------------------------------------------------------
int create_directory(struct Inode *inode, char *name) {
    // if it is a file, return -1
    if (inode->file_type == 0) {
        return -1;
    }
    // if the name has been used
    int inode_id;
    if (find_name_id(inode, name, &inode_id) == 0) {
        return -1;
    }
    // apply for a new inode
    int new_inode_id;
    struct Inode new_inode;

    init_new_inode(&new_inode, &new_inode_id, inode->sector_id, 1);
    // add the new inode to the directory

    add_name(inode, &new_inode, name, new_inode_id);
    return 0;
}
// --------------------------------------------------------------------------------------------
// Clear the content of a file
// --------------------------------------------------------------------------------------------
int clear_file(struct Inode *inode) {
    // if it is a directory, return -1
    if (inode->file_type == 1) {
        return -1;
    }
    // clear the content of the file
    int block_num = inode->block_num;
    for (int i = 0; i < block_num; i++)
        remove_tail_block(inode);
    // update the inode
    get_inode(inode->sector_id, inode);
    return 0;
}
// --------------------------------------------------------------------------------------------
// Write to a file
// --------------------------------------------------------------------------------------------
int write_file(struct Inode *inode, int length, char *content) {
    // if it is a directory, return -1
    if (inode->file_type == 1) {
        return -1;
    }
    // clear the content of the file
    clear_file(inode);
    // printf("id: %d\n", inode->sector_id);
    // write the content to the file
    int block_num = (length + 255) / 256;
    inode->file_size = length;
    for (int i = 0; i < block_num; i++) {
        int id;
        init_new_block(inode, &id);
        write_block(id, content + i * 256);
    }
    write_inode_to_disk(inode);
    return 0;
}
// --------------------------------------------------------------------------------------------
// Read from a file
// --------------------------------------------------------------------------------------------
int read_file(struct Inode *inode, int length, char *content) {
    // if it is a directory, return -1
    if (inode->file_type == 1) {
        return -1;
    }
    // if the length is larger than the file size, return -1
    int file_size = inode->file_size;
    if (length > file_size) {
        return -1;
    }
    // read the content from the file
    int block_num = (length + 255) / 256;
    read_inode(inode, 0, content);
    for (int i = 0; i < block_num; i++) {
        read_inode(inode, i, content + i * 256);
    }

    return file_size;
}
int read_whole_file(struct Inode *inode, char *content) {
    return read_file(inode, inode->file_size, content);
}
// --------------------------------------------------------------------------------------------
// Remove a file
// inode: the inode of the parent directory
// --------------------------------------------------------------------------------------------
int remove_file(struct Inode *inode, char *name) {
    // if it is a file, return -1
    if (inode->file_type == 0) {
        return -1;
    }
    // find the inode of the file and if it does not exist, return -1
    int inode_id;
    if (find_name_id(inode, name, &inode_id) == -1) {
        return -1;
    }
    // clear the content of the file
    struct Inode file_inode;
    get_inode(inode_id, &file_inode);
    clear_file(&file_inode);
    // remove the file name from the directory
    remove_name(inode, name);
    // free the block
    block_bitmap[inode_id] = '0';
    store_bitmap();
    return 0;
}
// --------------------------------------------------------------------------------------------
// Remove a directory
// inode: the inode of the parent directory
// --------------------------------------------------------------------------------------------
int remove_directory(struct Inode *inode, char *name) {
    // if it is a file, return -1
    if (inode->file_type == 0) {
        return -1;
    }
    // find the inode of the directory and if it does not exist, return -1
    int inode_id;
    if (find_name_id(inode, name, &inode_id) == -1) {
        return -1;
    }
    // clear the content of the directory
    struct Inode directory_inode;
    get_inode(inode_id, &directory_inode);
    int block_num = directory_inode.block_num;
    for (int i = 0; i < block_num; i++) {
        // get the sub name
        int id;
        get_sector_id(&directory_inode, i, &id);
        struct Naming_block naming_block;
        read_block(id, (char *)&naming_block);
        // get the sub inode
        int sub_inode_id = naming_block.inode_sector_id;
        struct Inode sub_inode;
        get_inode(sub_inode_id, &sub_inode);
        // if it is a file, remove the file
        if (sub_inode.file_type == 0) {
            remove_file(&directory_inode, naming_block.name);
        }
        // if it is a directory, remove the directory
        else {
            remove_directory(&directory_inode, naming_block.name);
        }
    }
    // remove the directory name from the directory
    remove_name(inode, name);
    // free the block
    block_bitmap[inode_id] = '0';
    store_bitmap();
    return 0;
}
// --------------------------------------------------------------------------------------------
// change the main directory to the parent directory
// --------------------------------------------------------------------------------------------
int change_to_parentdir(struct Inode *inode) {
    if (inode->file_type == 0) {
        return -1;
    }
    if (inode->pre_inode_sector_id == -1) {
        return -1;
    }
    write_inode_to_disk(inode);
    get_inode(inode->pre_inode_sector_id, inode);
    return 0;
}
// --------------------------------------------------------------------------------------------
// change the main directory to the sub directory
// --------------------------------------------------------------------------------------------
int change_to_subdir(struct Inode *inode, char *name) {
    if (inode->file_type == 0) {
        return -1;
    }
    int inode_id;
    if (find_name_id(inode, name, &inode_id) == -1) {
        return -1;
    }
    struct Inode sub_inode;
    get_inode(inode_id, &sub_inode);
    if (sub_inode.file_type == 0) {
        return -1;
    }
    write_inode_to_disk(inode);
    get_inode(inode_id, inode);
    return 0;
}
// * --------------------------------------------------------------------------------------------
// * real operation
// --------------------------------------------------------------------------------------------
// mk f
// --------------------------------------------------------------------------------------------
int mk_f(struct Inode *parent_directory, char *name) {
    return create_file(parent_directory, name);
}
// --------------------------------------------------------------------------------------------
// mkdir
// --------------------------------------------------------------------------------------------
int mk_dir(struct Inode *parent_directory, char *name) {
    return create_directory(parent_directory, name);
}
// --------------------------------------------------------------------------------------------
// rm f
// --------------------------------------------------------------------------------------------
int rm_f(struct Inode *parent_directory, char *name) {
    return remove_file(parent_directory, name);
}
// --------------------------------------------------------------------------------------------
// rm dir
// --------------------------------------------------------------------------------------------
int rm_dir(struct Inode *parent_directory, char *name) {
    return remove_directory(parent_directory, name);
}
// --------------------------------------------------------------------------------------------
// ls
// --------------------------------------------------------------------------------------------
int ls(struct Inode *parent_directory, char name[256][252]) {
    int name_num;
    list_all_name(parent_directory, name, &name_num);
    return name_num;
}
// -------------------------------------------------------------------------------------------
// w_f
// -------------------------------------------------------------------------------------------
int w_f(struct Inode *parent_directory, char *name, int length, char *content) {
    int inode_id;
    if (find_name_id(parent_directory, name, &inode_id) == -1) {
        return -1;
    }
    struct Inode inode;
    get_inode(inode_id, &inode);
    return write_file(&inode, length, content);
}
// -------------------------------------------------------------------------------------------
// cat_f
// -------------------------------------------------------------------------------------------
int cat_f(struct Inode *parent_directory, char *name, char *content) {
    int inode_id;
    if (find_name_id(parent_directory, name, &inode_id) == -1) {
        return -1;
    }
    struct Inode inode;
    get_inode(inode_id, &inode);
    int length = read_whole_file(&inode, content);
    content[length] = '\0';
    return length;
}
// -------------------------------------------------------------------------------------------
// cd
// -------------------------------------------------------------------------------------------
int parse_cd(char *path, char token[256][256], int *token_num) {
    *token_num = 0;
    char tmp_path[256];
    strcpy(tmp_path, path);
    char *p = strtok(tmp_path, "/");
    while (p != NULL) {
        strcpy(token[*token_num], p);
        (*token_num)++;
        p = strtok(NULL, "/");
    }
    return 0;
}
int cd(struct Inode *inode, char *path) {
    char token[256][256];
    int token_num = 0;
    parse_cd(path, token, &token_num);
    write_inode_to_disk(inode);
    // trace the path
    struct Inode tmp_inode = *inode;
    for (int i = 0; i < token_num; i++) {
        if (strcmp(token[i], "..") == 0) {
            if (change_to_parentdir(&tmp_inode) == -1) {
                return -1;
            }
        } else if (strcmp(token[i], ".") == 0) {
            continue;
        } else if (change_to_subdir(&tmp_inode, token[i]) == -1) {
            return -1;
        }
    }
    // user cannot use cd to access the root directory
    if (tmp_inode.pre_inode_sector_id == -1) {
        return -1;
    }
    *inode = tmp_inode;
    return 0;
}
// --------------------------------------------------------------------------------------------
// i_f
// --------------------------------------------------------------------------------------------
int i_f(struct Inode *parent_directory, char *name, int pos, int length, char *content) {
    int inode_id;
    if (find_name_id(parent_directory, name, &inode_id) == -1) {
        return -1;
    }
    struct Inode inode;
    get_inode(inode_id, &inode);
    // if the pos is larger than the file size, set the pos as the end of file
    if (pos > inode.file_size) {
        pos = inode.file_size;
    }
    // copy the data from the file to buffer
    int new_length = inode.file_size + length;
    char buffer[new_length + 256];
    read_whole_file(&inode, buffer);
    // insert the length of data to the position
    for (int i = new_length - 1; i >= pos + length; i--) {
        buffer[i] = buffer[i - length];
    }
    for (int i = 0; i < length; i++) {
        buffer[pos + i] = content[i];
    }
    // write the data to the file
    return write_file(&inode, new_length, buffer);
}
// --------------------------------------------------------------------------------------------
// d_f
// --------------------------------------------------------------------------------------------
int d_f(struct Inode *parent_directory, char *name, int pos, int length) {
    int inode_id;
    if (find_name_id(parent_directory, name, &inode_id) == -1) {
        return -1;
    }
    struct Inode inode;
    get_inode(inode_id, &inode);
    // the length if larger than the file size
    if (length > inode.file_size) {
        length = inode.file_size - pos;
    }
    // copy the data from the file to buffer
    int pre_length = inode.file_size;
    int new_length = inode.file_size - length;
    char buffer[pre_length + 256];
    read_whole_file(&inode, buffer);
    // delete the length of data from the position
    for (int i = pos; i < new_length; i++) {
        buffer[i] = buffer[i + length];
    }
    // write the data to the file
    write_file(&inode, new_length, buffer);
    return 0;
}

// --------------------------------------------------------------------------------------------
// g_dir
// --------------------------------------------------------------------------------------------
int g_dir(struct Inode *parent_directory, char *content) {
    get_current_directory(parent_directory, content);
    return 0;
}
// *--------------------------------------------------------------------------------------------
// * User information operation
// *--------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------
// Create the root directory
// --------------------------------------------------------------------------------------------
void create_root_directory(struct Inode *root) {
    // *create the root directory
    int sector_id;
    init_new_inode(root, &sector_id, -1, 1);
}
// --------------------------------------------------------------------------------------------
// Create the public directory
// --------------------------------------------------------------------------------------------
void create_public_directory(struct Inode *root) {
    // *create the public directory
    mk_dir(root, "public");
}
// --------------------------------------------------------------------------------------------
// Create a new user
// --------------------------------------------------------------------------------------------
int create_user(struct Inode *root, char *name, char *password) {
    // create the user directory
    int flag = mk_dir(root, name);
    if (flag == -1) {
        return -1;
    }

    // create the user file
    char user_info_name[256];

    sprintf(user_info_name, "%s_%s", name, "user_info");
    mk_f(root, user_info_name);

    // write the password to the user file
    w_f(root, user_info_name, strlen(password), password);

    return 0;
}
// --------------------------------------------------------------------------------------------
// Change the user
// --------------------------------------------------------------------------------------------
int change_user(struct Inode *root, struct Inode *user_inode, char *name, char *password) {
    int inode_id;
    // if the user want to access the public directory
    if (strcmp(name, "public") == 0) {
        find_name_id(root, name, &inode_id);
        get_inode(inode_id, user_inode);
        return 0;
    }
    // find the user directory
    if (find_name_id(root, name, &inode_id) == -1) {
        return -1;
    }
    get_inode(inode_id, user_inode);
    // find the user file
    char user_info_name[256];
    sprintf(user_info_name, "%s_%s", name, "user_info");
    if (find_name_id(root, user_info_name, &inode_id) == -1) {
        return -1;
    }
    // read the password from the user file
    char buffer[4096];
    cat_f(root, user_info_name, buffer);
    // check the password
    if (strcmp(buffer, password) != 0) {
        return -1;
    }
    return 0;
}
#endif