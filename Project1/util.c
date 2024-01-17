//
// Created by Mehmet Eren Balasar on 17.10.2023.
//

#include "util.h"
#include "globals.h"
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <mqueue.h>
#include <math.h>

// CLI API for the program, gets inputs from the user and sets the global variables accordingly
int cli1(int argc, char *argv[]) {
    if (argc < 2 || argc > 9) {
        printf("Usage: %s -n N -m M -i INFILE -o OUTFILE\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i += 2) {
        if (argv[i][0] == '-' && i + 1 < argc) {
            switch (argv[i][1]) {
                case 'n':
                    N = atoi(argv[i + 1]);
                    break;
                case 'm':
                    M = atoi(argv[i + 1]);
                    break;
                case 'i':
                    infile = argv[i + 1];
                    break;
                case 'o':
                    outfile = argv[i + 1];
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

    if (N < 1 || M < 1) {
        printf("N or M cannot be less than 1\n");
        exit_handler(1);
    }
    if (N > 20 || M > 20) {
        printf("N or M cannot be greater than 20\n");
        exit_handler(1);
    }
    if (infile == NULL) {
        printf("Input file must be specified\n");
        exit_handler(1);
    }

    // verification
    printf("N: %d\n", N);
    printf("M: %d\n", M);
    printf("INFILE: %s\n", infile);
    printf("OUTFILE: %s\n", outfile);


    return 0;
}

int cli2(int argc, char *argv[]) {

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            N = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-i") == 0) {
            infile = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-o") == 0) {
            outfile = argv[i + 1];
            i++;
        } else {
            fprintf(stderr, "Unrecognized option: %s\n", argv[i]);
            exit(1);
        }
    }

    if (N <= 0 || infile == NULL || outfile == NULL) {
        fprintf(stderr, "Missing or invalid arguments.\n");
        exit(1);
    }

    return 0;
}

int splitter(){
    FILE* input = fopen(infile, "r");

    // intermediate file pointers
    FILE* ifps[N];

    if (input == NULL) {
        printf("Cannot open file: %s\n", infile);
        exit_handler(1);
    }

    for(int i = 0; i < N; i++){
        char filename[21];
        strcpy(filename, inter_file_prefix);
        sprintf(filename + strlen(filename), "%d", i);

        ifps[i] = fopen(filename, "w");

        if(ifps[i] != NULL){
            printf("Created file '%s'\n", filename);
        }

    }

    int num;
    int k = 1;
    while (fscanf(input, "%d", &num) == 1) {
        fprintf(ifps[((k - 1) % N)], "%d\n", num);
        k++;
    }

    for(int i = 0; i < N; i++){
        fclose(ifps[i]);
    }

    fclose(input);
    return 0;
}

int cleanup(){
    for(int i = 0; i < N; i++){
        char filename[21];
        strcpy(filename, inter_file_prefix);
        sprintf(filename + strlen(filename), "%d", i);

        if(remove(filename) == 0){
            printf("Deleted file '%s'\n", filename);
        }
    }
    return 0;
}

int isPrime(int number) {
    if(number <= 1) {
        return 0;
    }
    double sqroot = sqrt(number);
    for(int i = 2; i<= sqroot; i++){
        if ( number % i == 0) {
            return 0;
        }
    }
    return 1;
}

void logger(const char *format, ...) {
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


void exit_handler(int sig){

    if (sig == 0){
        printf("Program exited successfully\n");
        exit(0);
    } else {
        printf("Program exited with signal %d\n", sig);
        exit(sig);
    }
}














