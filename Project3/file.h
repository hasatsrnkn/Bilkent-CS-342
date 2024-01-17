//
// Created by Mehmet Eren Balasar on 29.11.2023.
//

#ifndef MEMSIM_FILE_H
#define MEMSIM_FILE_H
#include <stdio.h>

void read_from_swap_file(const char* file_name, int page_number, char* page_buffer);
void write_to_swap_file(const char* file_name, int page_number, char* page_buffer);
int create_swap_file();
void write_to_output_file(const char* file_name, unsigned int va, unsigned int pte1, unsigned int pte2, unsigned int offset, unsigned int pfn, unsigned int pa, int pgfault);
#endif //MEMSIM_FILE_H
