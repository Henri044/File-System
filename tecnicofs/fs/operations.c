#include "operations.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int tfs_init() {
    state_init();
    int root = inode_create(T_DIRECTORY);
    if (root != ROOT_DIR_INUM) {
        return -1;
    }
    
    return 0;
}

int tfs_destroy() {
    state_destroy();
    return 0;
}

static bool valid_pathname(char const *name) {
    return name != NULL && strlen(name) > 1 && name[0] == '/';
}


int tfs_lookup(char const *name) {
    if (!valid_pathname(name)) {
        return -1;
    }

    // skip the initial '/' character
    name++;

    return find_in_dir(ROOT_DIR_INUM, name);
}

int tfs_open(char const *name, int flags) {
    int inum;
    size_t offset;
    /* Checks if the path name is valid */
    if (!valid_pathname(name)) {
        return -1;
    }
    inum = tfs_lookup(name);
    if (inum >= 0) {
        /* The file already exists */
        inode_t *inode = inode_get(inum);
        if (inode == NULL) {
            return -1;
        }
        /* Trucate (if requested) */
        if (flags & TFS_O_TRUNC) {
            if (inode->i_size > 0) {
                int number_data_block = 0;                
                while (number_data_block<266){
                    if (number_data_block<10){
                        if (data_block_free(inode->i_data_block[number_data_block]) == -1) {
                            return -1;
                        }
                    }
                    else{
                        int* indirect_block = data_block_get(inode->i_x_data_block);
                        int index = number_data_block - 10;
                        if (data_block_free(indirect_block[index]) == -1) {
                            return -1;
                        }
                    }
                    number_data_block++;
                }
                inode->i_size = 0;
            }
        }
        /* Determine initial offset */
        if (flags & TFS_O_APPEND) {
            offset = inode->i_size;
        } else {
            offset = 0;
        }
    } else if (flags & TFS_O_CREAT) {
        /* The file doesn't exist; the flags specify that it should be created*/
        /* Create inode */
        inum = inode_create(T_FILE);
        if (inum == -1) {
            return -1;
        }
        /* Add entry in the root directory */
        if (add_dir_entry(ROOT_DIR_INUM, inum, name + 1) == -1) {
            inode_delete(inum);
            return -1;
        }
        offset = 0;
    } else {
        return -1;
    }

    /* Finally, add entry to the open file table and
     * return the corresponding handle */
    return add_to_open_file_table(inum, offset);

    /* Note: for simplification, if file was created with TFS_O_CREAT and there
     * is an error adding an entry to the open file table, the file is not
     * opened but it remains created */
}


int tfs_close(int fhandle) {return remove_from_open_file_table(fhandle);
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t to_write) {
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        return -1;
    }
    pthread_rwlock_wrlock(&file->trinco);
    /* From the open file table entry, we get the inode */
    inode_t *inode = inode_get(file->of_inumber);
    if (inode == NULL) {
        return -1;
    }
    pthread_rwlock_wrlock(&inode->trinco);
    /* Determine how many bytes to write */
    if (to_write + file->of_offset > BLOCK_SIZE*266) {
        to_write = BLOCK_SIZE*266 - file->of_offset;
    }

    if (to_write > 0) {
        long unsigned int position_in_block = inode->i_size;
        long unsigned int last_block_allocced;
        while(position_in_block>BLOCK_SIZE){
            position_in_block-=BLOCK_SIZE;
        }
        long unsigned int position_in_block_offset = file->of_offset;
        long unsigned int block_where_to_write = file->of_offset/BLOCK_SIZE;
        while(position_in_block_offset>=BLOCK_SIZE){
            position_in_block_offset-=BLOCK_SIZE;
        }
        long unsigned int memory_free_in_block_to_write = BLOCK_SIZE-position_in_block;
        long unsigned int memory_left_in_block_to_write = BLOCK_SIZE-position_in_block_offset;
        long unsigned int blocks_to_allock_total;
        long unsigned int position_of_block_that_im_allocating;
        long unsigned int memory_yet_to_allock;
        void* block;
        long unsigned int index;
        int* indirect_block = data_block_get(inode->i_x_data_block);
        if (inode->i_size == 0) {
            /* If empty file, allocate new block */
            blocks_to_allock_total = to_write / BLOCK_SIZE;
            if ((to_write % BLOCK_SIZE)>0){
                blocks_to_allock_total++;
            }
            position_of_block_that_im_allocating = 0;
            while(blocks_to_allock_total>0){
                if (position_of_block_that_im_allocating<10){              
                    inode->i_data_block[position_of_block_that_im_allocating] = data_block_alloc();
                    blocks_to_allock_total--;
                    position_of_block_that_im_allocating++;
                }
                else if (position_of_block_that_im_allocating>=10){
                    if (position_of_block_that_im_allocating == 10){
                        inode->i_x_data_block = data_block_alloc();
                    }
                    index = position_of_block_that_im_allocating-10;
                    if (index>=256){
                        break;
                    }
                    indirect_block = data_block_get(inode->i_x_data_block);
                    indirect_block[index] = data_block_alloc();
                    blocks_to_allock_total--;
                    position_of_block_that_im_allocating++;
                }
            }
            
        }
        else if ((inode->i_size- file->of_offset)>to_write){

        }
        else{
            /*if file not empty and needed to add new blocks*/
            /*checks to see if new memory allocation necessary*/
            if (memory_free_in_block_to_write<to_write){
                memory_yet_to_allock = to_write-memory_free_in_block_to_write;
                blocks_to_allock_total = memory_yet_to_allock / BLOCK_SIZE;
                if ((memory_yet_to_allock % BLOCK_SIZE)!=0){
                    blocks_to_allock_total++;
                }
                last_block_allocced = inode->i_size / BLOCK_SIZE;
                if ((inode->i_size % BLOCK_SIZE)==0){
                    last_block_allocced-= 1;
                }
                position_of_block_that_im_allocating = last_block_allocced+1;
                for (int i = 0; blocks_to_allock_total > i;i++){
                    if (position_of_block_that_im_allocating<10){
                        inode->i_data_block[position_of_block_that_im_allocating] = data_block_alloc();
                        position_of_block_that_im_allocating++;
                    }
                    else if (position_of_block_that_im_allocating>=10){
                        if (position_of_block_that_im_allocating == 10){
                            inode->i_x_data_block = data_block_alloc();
                        }
                        index = position_of_block_that_im_allocating-10;
                        if (index>=256){
                            break;
                        }
                        indirect_block = data_block_get(inode->i_x_data_block);
                        if (indirect_block == NULL)
                            break;
                        indirect_block[index] = data_block_alloc();
                        position_of_block_that_im_allocating++;
                    }
                }
            }

        }
        if (block_where_to_write<10){
            block = data_block_get(inode->i_data_block[block_where_to_write]);
        }
        else{
            index = block_where_to_write-10;
            block = data_block_get(indirect_block[index]);
        }
        if (block == NULL) {
            return -1;
        }
        /* Perform the actual write */
        if (memory_left_in_block_to_write >= to_write){
            memcpy(block + position_in_block_offset, buffer, to_write);
        }
        else{
            memcpy(block + position_in_block_offset, buffer, memory_left_in_block_to_write);
            long unsigned int memory_yet_to_write = to_write - memory_left_in_block_to_write;
            long unsigned int memory_already_written = memory_left_in_block_to_write;
            int stopped = 0;
            while(memory_yet_to_write>BLOCK_SIZE){ 
                block_where_to_write+=1;
                if (block_where_to_write<10){
                    block = data_block_get(inode->i_data_block[block_where_to_write]);
                }
                else{
                    index = block_where_to_write-10;
                    if (index>=256){
                        stopped = 1;
                        break;
                    }
                    block = data_block_get(indirect_block[index]);
                }
                if (block == NULL) {
                    return -1;
                }
                memcpy(block, buffer + memory_already_written, BLOCK_SIZE);
                memory_already_written+=BLOCK_SIZE;
                memory_yet_to_write-=BLOCK_SIZE;
            }
                
            if (memory_yet_to_write<=BLOCK_SIZE && stopped == 0){
                block_where_to_write+=1;
                index = block_where_to_write-10;
                if (block_where_to_write<10)
                    block = data_block_get(inode->i_data_block[block_where_to_write]);
                else{
                    block = data_block_get(indirect_block[index]);
                }
                if (block == NULL) {
                    return -1;
                }
                memcpy(block, buffer + memory_already_written, memory_yet_to_write);
            }         
        }

        /* The offset associated with the file handle is
         * incremented accordingly */
        file->of_offset += to_write;
        if (file->of_offset > inode->i_size) {
            inode->i_size = file->of_offset;
        }
    }
    pthread_rwlock_unlock(&inode->trinco);
    pthread_rwlock_unlock(&file->trinco);

    return (ssize_t)to_write;
}


ssize_t tfs_read(int fhandle, void *buffer, size_t len) {
    open_file_entry_t *file = get_open_file_entry(fhandle);
    if (file == NULL) {
        return -1;
    }
    pthread_rwlock_rdlock(&file->trinco);
    /* From the open file table entry, we get the inode */
    inode_t *inode = inode_get(file->of_inumber);
    if (inode == NULL) {
        return -1;
    }
    pthread_rwlock_rdlock(&inode->trinco);
    /* Determine how many bytes to read */
    size_t to_read = inode->i_size - file->of_offset;
    if (to_read > len) {
        to_read = len;
    }
    void* block;
    long unsigned int index;
    int* indirect_block = data_block_get(inode->i_x_data_block);;
    if (to_read > 0) {
        long unsigned int position_in_block_to_read = file->of_offset;
        long unsigned int block_to_read = file->of_offset / BLOCK_SIZE;
        while(position_in_block_to_read>=BLOCK_SIZE){
            position_in_block_to_read-=BLOCK_SIZE;
        }

        if (block_to_read<10){
            block = data_block_get(inode->i_data_block[block_to_read]);
        }
        else{
            index = block_to_read-10;
            block = data_block_get(indirect_block[index]);
        }
        if (block == NULL) {
            return -1;
        }
        /* Perform the actual read */
        
        long unsigned int memory_left_in_block = BLOCK_SIZE - position_in_block_to_read;
        if (to_read<=memory_left_in_block){
            memcpy(buffer, block + position_in_block_to_read, to_read);
        }
        else{
            memcpy(buffer, block + position_in_block_to_read, memory_left_in_block);
            long unsigned int memory_left_to_read_in_all_blocks = to_read-memory_left_in_block;
            long unsigned int memory_already_read = memory_left_in_block;
            if (memory_left_to_read_in_all_blocks>=BLOCK_SIZE){
                while(memory_left_to_read_in_all_blocks >= BLOCK_SIZE){
                    block_to_read+=1;
                    if (block_to_read<10){
                        block = data_block_get(inode->i_data_block[block_to_read]);
                    }
                    else{
                        index = block_to_read-10;
                        block = data_block_get(indirect_block[index]);
                    }
                    if (block == NULL) {
                        return -1;
                    }
                    memcpy(buffer+memory_already_read, block, BLOCK_SIZE);
                    memory_already_read += BLOCK_SIZE;
                    memory_left_to_read_in_all_blocks-=BLOCK_SIZE;
                }
                if (memory_left_to_read_in_all_blocks!=0){
                    block_to_read+=1;
                    if (block_to_read<10)
                        block = data_block_get(inode->i_data_block[block_to_read]);
                    else{
                        index = block_to_read-10;
                        block = data_block_get(indirect_block[index]);
                    }
                    if (block == NULL) {
                        return -1;
                    }
                    memcpy(buffer+memory_already_read, block, memory_left_to_read_in_all_blocks);
                }

            }
            else if (memory_left_to_read_in_all_blocks<BLOCK_SIZE){
                block_to_read+=1;
                if (block_to_read<10)
                    block = data_block_get(inode->i_data_block[block_to_read]);
                else{
                    index = block_to_read-10;
                    block = data_block_get(indirect_block[index]);
                }
                if (block == NULL) {
                    return -1;
                }
                memcpy(buffer + memory_already_read, block, memory_left_to_read_in_all_blocks);
            }   

        }       
        /* The offset associated with the file handle is
        * incremented accordingly */
        file->of_offset += to_read;
    }
    pthread_rwlock_unlock(&inode->trinco);
    pthread_rwlock_unlock(&file->trinco);
    return (ssize_t)to_read;
}
int tfs_copy_to_external_fs(char const *source_path, char const *dest_path){
    int f1handle;
    FILE *f2;
    char buffer[BLOCK_SIZE];
    f1handle = tfs_open(source_path,0);
    open_file_entry_t *file = get_open_file_entry(f1handle);
    if (file == NULL) {
        return -1;
    }
    f2 = fopen(dest_path, "w");
    if (f2 == NULL){
        return -1;
    }
    ssize_t read_file = tfs_read(f1handle, buffer, BLOCK_SIZE);
    long unsigned int read_file2 = (long unsigned int)read_file;
    while (read_file2 == BLOCK_SIZE){
        fwrite(buffer,1,BLOCK_SIZE,f2);
        read_file = tfs_read(f1handle, buffer, BLOCK_SIZE);
        read_file2 = (long unsigned int)read_file;
    }
    if (read_file2>0){
        fwrite(buffer,1,read_file2,f2);
    }
    tfs_close(f1handle);
    fclose(f2);
    return 0;
}