//
// Created by Mehmet Eren Balasar on 26.11.2023.
//

#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "globals.h"

void exit_handler(int sig){

    if (sig == 0){
        printf("Program exited successfully\n");
        exit(0);
    } else {
        printf("Program exited with signal %d\n", sig);
        exit(sig);
    }
}


int cli(int argc, char *argv[]) {
    if (argc != 15) { // Check for the exact number of required arguments
        printf("Usage: %s -p level -r addrfile -s swapfile -f fcount -a algo -t tick -o outfile\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i += 2) {
        if (argv[i][0] == '-' && i + 1 < argc) {
            switch (argv[i][1]) {
                case 'p': // Paging level
                    level = atoi(argv[i + 1]);
                    break;
                case 'r': // Address file
                    strcpy(addrfile, argv[i + 1]); // Correct
                    break;
                case 's': // Swap file
                    strcpy(swapfile, argv[i + 1]);
                    break;
                case 'f': // Frame count
                    fcount = atoi(argv[i + 1]);
                    break;
                case 'a': // Algorithm
                    strcpy(algo, argv[i + 1]);
                    break;
                case 't': // Timer tick
                    tick = atoi(argv[i + 1]);
                    break;
                case 'o': // Output file
                    strcpy(outfile, argv[i + 1]);
                    break;
                default:
                    printf("Invalid option: %s\n", argv[i]);
                    exit_handler(1);
            }
        } else {
            printf("Invalid arguments\n");
            exit_handler(1);
        }
    }
    if(level < 1 || level > 2) {
        printf("Invalid level\n");

        exit_handler(1);
    }

    if(fcount < 4 || fcount > 128) {
        printf("Invalid fcount\n");
        exit_handler(1);
    }


// validation
    printf("level is %d\n", level);
    printf("addrfile is %s\n", addrfile);
    printf("swapfile is %s\n", swapfile);
    printf("fcount is %d\n", fcount);
    printf("algo is %s\n", algo);
    printf("tick is %d\n", tick);
    printf("outfile is %s\n", outfile);


    return 0;
}

int parse_line(char *line, char* rw, unsigned int* va, unsigned int* value){
    char* token = strsep(&line, " ");
    if (token == NULL){
        return -1;
    }
    *rw = token[0];

    token = strsep(&line, " ");
    if (token == NULL){
        return -1;
    }
    *va = (unsigned int)strtol(token, NULL, 16);

    if (strcmp(rw, "r") == 0){
        //printf("RRRRR\n");
        return 0;
    }

    token = strsep(&line, " ");
    if (token == NULL){
        return -1;
    }
    *value = (unsigned int)strtol(token, NULL, 16);
    return 0;
}

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