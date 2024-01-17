//
// Created by Mehmet Eren Balasar on 29.11.2023.
//

#include "file.h"
#include "globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int create_swap_file() {
    FILE *file;
    char buffer[PAGESIZE];

    file = fopen(swapfile, "r");
    if (file) {
        fclose(file);
        file = fopen(swapfile, "r+b");
    } else {
        file = fopen(swapfile, "w+b");
        if (!file) {
            perror("Error creating swap file");
            return 1;
        }

        memset(buffer, 0, PAGESIZE);

        for (int i = 0; i < NUM_PAGES; i++) {
            if (fwrite(buffer, sizeof(char), PAGESIZE, file) != PAGESIZE) {
                perror("Error writing to swap file");
                fclose(file);
                return 1;
            }
        }

        rewind(file);
    }
    fclose(file);

    return 0;
}


void read_from_swap_file(const char *file_name, int page_number, char *page_buffer) {
    FILE *file = fopen(file_name, "rb");
    if (!file) {
        perror("Error opening swap file");
        exit(EXIT_FAILURE);
    }

    long offset = (long)page_number * PAGESIZE;
    fseek(file, offset, SEEK_SET);

    size_t bytesRead = fread(page_buffer, 1, PAGESIZE, file);
    if (bytesRead != PAGESIZE) {
        perror("Error reading from swap file");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

void write_to_swap_file(const char *file_name, int page_number, char *page_buffer) {
    if (page_number < 0 || page_number >= NUM_PAGES) {
        return;
    }
    FILE *file = fopen(file_name, "rb+");
    if (!file) {
        perror("Error opening swap file");
        exit(EXIT_FAILURE);
    }

    long offset = (long)page_number * PAGESIZE;
    fseek(file, offset, SEEK_SET);

    size_t bytesWritten = fwrite(page_buffer, 1, PAGESIZE, file);
    if (bytesWritten != PAGESIZE) {
        perror("Error writing to swap file");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

void write_to_output_file(const char* file_name, unsigned int va, unsigned int pte1,
                          unsigned int pte2, unsigned int offset, unsigned int pfn, unsigned int pa, int pgfault) {
    FILE *file = fopen(file_name, "a"); // Append mode
    if (!file) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    if (pgfault) {
        fprintf(file, "0x%04X 0x%X 0x%X 0x%X 0x%X 0x%04X pgfault\n", va, pte1, pte2, offset, pfn, pa);
    } else {
        fprintf(file, "0x%04X 0x%X 0x%X 0x%X 0x%X 0x%04X\n", va, pte1, pte2, offset, pfn, pa);
    }

    fclose(file);
}
