//
// Created by hasatsrnkn on 03.12.2023.
//

#ifndef MEMSIM_PAGETABLE_H
#define MEMSIM_PAGETABLE_H

#include "globals.h"

typedef struct {
    unsigned int frameNumber : 10;
    unsigned int unused      : 3;
    unsigned int valid       : 1;
    unsigned int referenced  : 1;
    unsigned int modified    : 1;
} PTE;

typedef struct {
    PTE *entries;
    int size;
} PageTable;

extern PageTable* pageTable;
extern PageTable** innerPageTables; // for 64-bit memory address lookup

int initializePageTable();
void clearRBits();
int getPageTableSize();
unsigned int getPageFrameNumber(unsigned int pageNumber);
unsigned int getPageValid(unsigned int pageNumber);
unsigned int getPageReference(unsigned int pageNumber);
unsigned int getPageModified(unsigned int pageNumber);
unsigned int getPageNumber(unsigned int frameNumber);
int setPageFrameNumber(unsigned int pageNumber, unsigned int frameNumber);
int setPageValid(unsigned int pageNumber, unsigned int valid);
int setPageReferenced(unsigned int pageNumber, unsigned int referenced);
int setPageModified(unsigned int pageNumber, unsigned int modified);

#endif //MEMSIM_PAGETABLE_H
