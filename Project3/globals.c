//
// Created by Mehmet Eren Balasar on 26.11.2023.
//

#include "globals.h"
struct timespec program_start;

int dlevel = 1;
int total_page_faults = 0;
int current_tick = 0;

int level = 1;
char addrfile[64] = "some.txt";
char swapfile[64] = "swapfile.txt";
int fcount = 16;
char algo[64] = "fifo";
int tick = 10;
char outfile[64];