//
// Created by Mehmet Eren Balasar on 26.11.2023.
//

#ifndef MEMSIM_UTILS_H
#define MEMSIM_UTILS_H

#include <time.h>
#include "globals.h"
#include <stdarg.h>
#include <stdio.h>

extern struct timespec program_start;

void logg(int log_level, const char *format, ...);
int parse_line(char *line, char* rw, unsigned int* va, unsigned int* value);
void exit_handler(int sig);
int cli(int argc, char *argv[]);

#endif //MEMSIM_UTILS_H
