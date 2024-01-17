//
// Created by Mehmet Eren Balasar on 12.12.2023.
//

#ifndef VSFS_UTIL_H
#define VSFS_UTIL_H
#include "vsfs.h"
#define MAXFILES 128
#define OFT_SIZE 16

#define DEEP_DEBUG 2
#define DEBUG 1
#define INFO 0

extern int dlevel;
extern struct timespec program_start;

void logg(int log_level, const char *format, ...);

typedef struct superblock {
    int size; // in bytes
    int fat_offset; // in bytes
    int dir_offset; // in bytes
    int data_offset; // in bytes
    int free_blocks; // number of free blocks
} Superblock;

// -1 means end of file
// -2 means unused
typedef struct FAT {
    int size; // in entry count
    int* entries; // number of next block
} FAT;

typedef struct dir_entry {
    char name[30]; // file name
    int size; // file size
    int start_block; // first block of data
} Dentry;

typedef struct dir {
    Dentry entries[MAXFILES];
} Dir;

typedef struct oft_entry {
    int index_dir; // index of file in root directory
    int offset; // byte offset of file
    int mode; // mode of file access
    int free; // 0 means entry is free, 1 means used
} Oft_entry;

typedef struct OFT {
    Oft_entry entries[OFT_SIZE];
} OFT;

#endif //VSFS_UTIL_H
