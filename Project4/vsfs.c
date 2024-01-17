#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "vsfs.h"
#include "util.h"

// globals  =======================================
int vs_fd; // file descriptor of the Linux file that acts as virtual disk.
              // this is not visible to an application.

Superblock superblock;
FAT fat;
Dir root_dir;
OFT oft;
// ========================================================


// read block k from disk (virtual disk) into buffer block.
// size of the block is BLOCKSIZE.
// space for block must be allocated outside of this function.
// block numbers start from 0 in the virtual disk. 
int read_block (void *block, int k)
{
    int n;
    int offset;

    offset = k * BLOCKSIZE;
    lseek(vs_fd, (off_t) offset, SEEK_SET);
    n = read (vs_fd, block, BLOCKSIZE);
    if (n != BLOCKSIZE) {
	printf ("read error\n");
	return -1;
    }
    return (0); 
}

// write block k into the virtual disk. 
int write_block (void *block, int k)
{
    int n;
    int offset;

    offset = k * BLOCKSIZE;
    lseek(vs_fd, (off_t) offset, SEEK_SET);
    n = write (vs_fd, block, BLOCKSIZE);
    if (n != BLOCKSIZE) {
	printf ("write error\n");
	return (-1);
    }
    return 0; 
}


/**********************************************************************
   The following functions are to be called by applications directly. 
***********************************************************************/

// this function is partially implemented.
int vsformat (char *vdiskname, unsigned int m)
{
    char command[1000];
    int size;
    int num = 1;
    int count;
    size  = num << m;
    count = size / BLOCKSIZE;
    //    printf ("%d %d", m, size);
    sprintf (command, "dd if=/dev/zero of=%s bs=%d count=%d",
             vdiskname, BLOCKSIZE, count);
    //printf ("executing command = %s\n", command);
    system (command);

    superblock.size = size;
    superblock.fat_offset = 1 * BLOCKSIZE;
    superblock.dir_offset = 33 * BLOCKSIZE;
    superblock.data_offset = 41 * BLOCKSIZE;
    superblock.free_blocks = count - 41;

    fat.entries = (int *) malloc (sizeof(int) * count);
    fat.size = count;
    int i;
    for (i = 0; i < count; ++i) {
        fat.entries[i] = -2;
    }

    for (i = 0; i < MAXFILES; ++i) {
        root_dir.entries[i].name[0] = '\0';
        root_dir.entries[i].size = 0;
        root_dir.entries[i].start_block = -1;
    }

    int fp = open(vdiskname, O_WRONLY);
    write(fp, &superblock, sizeof(Superblock));

    lseek(fp, superblock.fat_offset, SEEK_SET);
    write(fp, &fat.size, sizeof(int));
    for (i = 0; i < count; ++i) {
        write(fp, &fat.entries[i], sizeof(int));
    }

    lseek(fp, superblock.dir_offset, SEEK_SET);
    for (i = 0; i < MAXFILES; ++i) {
        write(fp, &root_dir.entries[i], sizeof(Dentry));
    }

    close(fp);
    free(fat.entries);

    return (0); 
}


// this function is partially implemented.
int  vsmount (char *vdiskname)
{
    // open the Linux file vdiskname and in this
    // way make it ready to be used for other operations.
    // vs_fd is global; hence other function can use it. 
    vs_fd = open(vdiskname, O_RDWR);
    // load (chache) the superblock info from disk (Linux file) into memory
    // load the FAT table from disk into memory
    // load root directory from disk into memory


    read(vs_fd, &superblock, sizeof(Superblock));
    logg(DEEP_DEBUG, "size: %d\n", superblock.size);
    logg(DEEP_DEBUG, "fat_offset: %d\n", superblock.fat_offset);
    logg(DEEP_DEBUG, "dir_offset: %d\n", superblock.dir_offset);
    logg(DEEP_DEBUG, "data_offset: %d\n", superblock.data_offset);
    logg(DEEP_DEBUG, "free_blocks: %d\n", superblock.free_blocks);

    logg(DEEP_DEBUG, "FAT\n");
    fat.entries = (int *) malloc (sizeof(int) * superblock.size / BLOCKSIZE);
    lseek(vs_fd, superblock.fat_offset, SEEK_SET);
    int i;
    read(vs_fd, &fat.size, sizeof(int));
    for (i = 0; i < superblock.size / BLOCKSIZE; ++i) {
        read(vs_fd, &fat.entries[i], sizeof(int));
    }
    logg(DEEP_DEBUG, "fatsize: %d\n", fat.size);

    logg(DEEP_DEBUG, "DIR\n");

    lseek(vs_fd, superblock.dir_offset, SEEK_SET);
    for (i = 0; i < MAXFILES; ++i) {
        read(vs_fd, &root_dir.entries[i], sizeof(Dentry));
    }


    // open file table
    for (i = 0; i < OFT_SIZE; ++i) {
        oft.entries[i].index_dir = -1;
        oft.entries[i].offset = 0;
        oft.entries[i].mode = 0;
        oft.entries[i].free = 0;
    }


    return(0);
}


// this function is partially implemented.
int vsumount ()
{
    // write superblock to virtual disk file
    // write FAT to virtual disk file
    // write root directory to virtual disk file
    lseek(vs_fd, 0, SEEK_SET);

    write(vs_fd, &superblock, sizeof(Superblock));

    lseek(vs_fd, superblock.fat_offset, SEEK_SET);
    write(vs_fd, &fat.size, sizeof(int));
    for (int i = 0; i < fat.size; ++i) {
        write(vs_fd, &fat.entries[i], sizeof(int));
    }

    lseek(vs_fd, superblock.dir_offset, SEEK_SET);
    for (int i = 0; i < MAXFILES; ++i) {
        write(vs_fd, &root_dir.entries[i], sizeof(Dentry));
    }

    fsync (vs_fd); // synchronize kernel file cache with the disk
    close (vs_fd);
    //free(fat.entries);
    return (0); 
}


int vscreate(char *filename)
{
    // Check if the file already exists in the root directory
    for (int i = 0; i < MAXFILES; i++) {
        if (strcmp(root_dir.entries[i].name, filename) == 0) {
            // File already exists
            logg(DEEP_DEBUG,"File %s already exists.\n", filename);
            return -1;
        }
    }

    // Find an empty slot in the root directory
    int empty_slot = -1;
    for (int i = 0; i < MAXFILES; i++) {
        if (root_dir.entries[i].name[0] == '\0') {
            empty_slot = i;
            break;
        }
    }

    if (empty_slot == -1) {
        // No empty slot available
        logg(DEEP_DEBUG,"No space available in the root directory.\n");
        return -1;
    }
    printf("empty slot: %d\n", empty_slot);
    // Initialize the directory entry for the new file
    strncpy(root_dir.entries[empty_slot].name, filename, sizeof(root_dir.entries[empty_slot].name) - 1);
    root_dir.entries[empty_slot].size = 0;
    root_dir.entries[empty_slot].start_block = -1; // No data block allocated yet

    // Optionally, write the updated root directory back to the disk
    // This can be done immediately or deferred until vsumount()
    logg(DEEP_DEBUG,"File %s created successfully.\n", filename);
    return 0;
}


int vsopen(char *file, int mode)
{
    // Check if the file exists in the root directory
    int file_index = -1;
    for (int i = 0; i < MAXFILES; i++) {
        if (strcmp(root_dir.entries[i].name, file) == 0) {
            file_index = i;
            break;
        }
    }
    if (file_index == -1) {
        // File does not exist
        logg(INFO,"File %s does not exist.\n", file);
        return -1;
    }

    for (int i = 0; i < OFT_SIZE; ++i) {

        if (oft.entries[i].index_dir == file_index && oft.entries[i].free == 1) {
            logg(DEEP_DEBUG, "File %s is already open.\n", file);
            return i;
        }

        if (oft.entries[i].free == 0) {
            oft.entries[i].free = 1;
            oft.entries[i].index_dir = file_index;
            oft.entries[i].offset = 0;
            oft.entries[i].mode = mode;
            return i;
        }

    }
    logg(INFO, "No space available in the open file table. "
               "Try closing some file descriptors with vsclose\n");
    return (-1);
}

int vsclose(int fd){
    if (fd < 0 || fd >= OFT_SIZE) {
        logg(INFO, "Invalid file descriptor %d\n", fd);
        return -1;
    }

    if (oft.entries[fd].free == 0) {
        logg(INFO, "File descriptor %d is already closed\n", fd);
        return -1;
    }

    oft.entries[fd].free = 0;
    oft.entries[fd].index_dir = -1;
    oft.entries[fd].offset = 0;
    oft.entries[fd].mode = 0;

    return (0); 
}

int vssize (int  fd)
{
    if(!oft.entries[fd].free) {
        return -1;
    }
    int index_dir = oft.entries[fd].index_dir;
    return root_dir.entries[index_dir].size;
}

int vsread(int fd, void *buf, int n){

    if (fd < 0 || fd >= OFT_SIZE) {
        logg(INFO, "Invalid file descriptor %d\n", fd);
        return -1;
    }
    if (oft.entries[fd].free == 0) {
        logg(INFO, "File descriptor %d is closed\n", fd);
        return -1;
    }
    if (oft.entries[fd].mode == MODE_APPEND) {
        logg(INFO, "File descriptor %d is in append mode\n", fd);
        return -1;
    }
    int index_dir = oft.entries[fd].index_dir;
    int offset = oft.entries[fd].offset;

    if (offset >= root_dir.entries[index_dir].size) {
        logg(INFO, "Offset %d is larger than file size %d\n", offset, root_dir.entries[index_dir].size);
        return -1;
    }

    int start_block = root_dir.entries[index_dir].start_block;
    int num_of_blocks_to_go = offset / BLOCKSIZE;

    int current_block = start_block;
    for (int i = 0; i < num_of_blocks_to_go; ++i) {
        current_block = fat.entries[current_block];
    }
    int offset_in_block = offset % BLOCKSIZE;
    int bytes_read = 0;
    int bytes_to_read = n;

    while (bytes_to_read > 0) {
        char block[BLOCKSIZE];

        read_block(block, current_block);

        int bytes_to_read_from_block = BLOCKSIZE - offset_in_block;
        if (bytes_to_read_from_block > bytes_to_read) {
            bytes_to_read_from_block = bytes_to_read;
        }

        memcpy(buf + bytes_read, block + offset_in_block, bytes_to_read_from_block);

        bytes_read += bytes_to_read_from_block;
        bytes_to_read -= bytes_to_read_from_block;
        offset_in_block = 0;
        if (current_block != -1) {
            current_block = fat.entries[current_block];
        }
        if (current_block == -1) {
            break;
        }

    }

    oft.entries[fd].offset += bytes_read;
    return bytes_read;

}


int vsappend(int fd, void *buf, int n)
{
    if (fd < 0 || fd >= OFT_SIZE) {
        logg(INFO, "Invalid file descriptor %d\n", fd);
        return -1;
    }
    if (oft.entries[fd].free == 0) {
        logg(INFO, "File descriptor %d is closed\n", fd);
        return -1;
    }
    if (oft.entries[fd].mode == MODE_READ) {
        logg(INFO, "File descriptor %d is in read mode\n", fd);
        return -1;
    }

    int index_dir = oft.entries[fd].index_dir;
    int offset = oft.entries[fd].offset;
    int start_block = root_dir.entries[index_dir].start_block;
    int num_of_blocks_to_go = offset / BLOCKSIZE;

    int current_block = start_block;
    int index_of_last_block = -1;
    for (int i = 0; i < num_of_blocks_to_go; i++) {
        index_of_last_block = current_block;
        current_block = fat.entries[current_block];

    }
    int offset_in_block = offset % BLOCKSIZE;
    int bytes_written = 0;
    int bytes_to_write = n;


    while (bytes_to_write > 0) {
        char block[BLOCKSIZE];
        if (current_block == -1) {
            // Allocate a new block
            int new_block = -1;
            for (int i = 0; i < fat.size; ++i) {
                if (fat.entries[i] == -2) {
                    new_block = i;
                    break;
                }
            }
            if (new_block == -1) {
                logg(INFO, "No free blocks available\n");
                return -1;
            }
            fat.entries[index_of_last_block] = new_block;
            fat.entries[new_block] = -1;
            current_block = new_block;
            superblock.free_blocks--;

            if (start_block == -1) {
                root_dir.entries[index_dir].start_block = new_block;
            }

        }
        else {
            read_block(block, current_block);
        }

        int bytes_to_write_to_block = BLOCKSIZE - offset_in_block;

        if (bytes_to_write_to_block > bytes_to_write) {
            bytes_to_write_to_block = bytes_to_write;
        }

        memcpy(block + offset_in_block, buf + bytes_written, bytes_to_write_to_block);

        bytes_written += bytes_to_write_to_block;
        bytes_to_write -= bytes_to_write_to_block;
        write_block(block, current_block);

        /*logg(DEBUG, "%d bytes written to file %s starting at file offset %d \n"
                    "and block offset %d at block %d\n",
             bytes_to_write_to_block, root_dir.entries[index_dir].name, offset, offset_in_block, current_block);*/
        offset_in_block = 0;
        index_of_last_block = current_block;
        current_block = fat.entries[current_block];

    }
    oft.entries[fd].offset += bytes_written;
    if (oft.entries[fd].offset > root_dir.entries[index_dir].size){
        root_dir.entries[index_dir].size = oft.entries[fd].offset;
    }

    return bytes_written;
}

int vsdelete(char *filename)
{
    int start_block = -1;
    int directory_index = -1;
    for(int i = 0; i< MAXFILES; i++) {
        if(strcmp(root_dir.entries[i].name, filename) == 0) {
            start_block = root_dir.entries[i].start_block;
            directory_index = i;
            break;
        }
    }
    //There is a block
    if (start_block != -1 ) {
        // -2 means free   -1 means end
        int block_hand = start_block;
        int next_block;
        while ( fat.entries[block_hand] != -1 ) {
            next_block = fat.entries[block_hand];
            fat.entries[block_hand] = -2;
            block_hand = next_block;
        }
        fat.entries[block_hand] = -2;

        root_dir.entries[directory_index].start_block = -1;
        root_dir.entries[directory_index].size = 0;
        root_dir.entries[directory_index].name[0] = '\0';

        return 0;
    }
    return (-1);
}

