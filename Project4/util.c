//
// Created by Mehmet Eren Balasar on 12.12.2023.
//
#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include "util.h"

struct timespec program_start;
int dlevel = DEEP_DEBUG;

void logg(int log_level, const char *format, ...) {
    if (log_level > dlevel){
        return;
    }
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);

    long seconds = end.tv_sec - program_start.tv_sec;
    long nanoseconds = end.tv_nsec - program_start.tv_nsec;

    if (nanoseconds < 0) {
        nanoseconds += 1e9;
        seconds -= 1;  // Adjust seconds accordingly
    }

    double elapsed = seconds * 1000.0 + nanoseconds / 1e6;

    printf("%f --- ", elapsed);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

