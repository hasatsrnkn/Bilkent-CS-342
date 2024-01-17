//
// Created by Mehmet Eren Balasar on 26.11.2023.
//

#ifndef MEMSIM_GLOBALS_H
#define MEMSIM_GLOBALS_H

#include <time.h>

#define PAGESIZE 64
#define VIRTUAL_MEMORY_SIZE 65536 // 64 KB
#define NUM_PAGES (VIRTUAL_MEMORY_SIZE / PAGESIZE)
#define OFFSET_MASK 0x3F
#define PAGE_TABLE_SIZE_2 32
extern struct timespec program_start;

extern int dlevel;
extern int total_page_faults;
extern int current_tick;

extern int level;
extern char addrfile[64];
extern char swapfile[64];
extern int fcount;
extern char algo[64];
extern int tick;
extern char outfile[64];



typedef struct {
    char data[PAGESIZE];
    int containsPage;
} Frame;

typedef struct {
    Frame *frames;
    int totalFrames;
} PhysicalMemory;



#endif //MEMSIM_GLOBALS_H
