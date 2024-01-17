//
// Created by Mehmet Eren Balasar on 17.10.2023.
//
#ifndef SRC_GLOBALS_H
#define SRC_GLOBALS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MQNAME "/mq"

extern int N;
extern int M;
extern char* outfile;
extern char* infile;
extern char* inter_file_prefix;

extern struct timespec program_start;

#endif