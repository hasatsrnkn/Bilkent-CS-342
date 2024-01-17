//
// Created by hasatsrnkn on 03.12.2023.
//

#include "pagetable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
int counter = 0;

PageTable* pageTable = NULL;
PageTable** innerPageTables = NULL;

int initializePageTable() {
    pageTable = malloc(sizeof(PageTable));
    if (level == 1){
        pageTable->size = NUM_PAGES;
        pageTable->entries = malloc(sizeof(PTE) * pageTable->size);

        // Set all page table entries to invalid
        for (int i = 0; i < pageTable->size; i++) {
            pageTable->entries[i].frameNumber = 0;
            pageTable->entries[i].valid = 0;
            pageTable->entries[i].referenced = 0;
            pageTable->entries[i].modified = 0;
            pageTable->entries[i].unused = 0;
        }
    }

    if(level == 2) {
        pageTable->size = PAGE_TABLE_SIZE_2;
        pageTable->entries = malloc(sizeof(PTE) * pageTable->size);

        for (int i = 0; i < pageTable->size; i++) {
            pageTable->entries[i].frameNumber = 0;
            pageTable->entries[i].valid = 0;
            pageTable->entries[i].referenced = 0;
            pageTable->entries[i].modified = 0;
            pageTable->entries[i].unused = 0;
        }
        innerPageTables = malloc(sizeof(PageTable*) * pageTable->size); // for memory address lookup
    }

    return 0;
}

int initializeInnerPageTable(unsigned int pageNumber){
    unsigned int outerIndex = pageNumber >> 5;

    pageTable->entries[outerIndex].frameNumber = counter;
    pageTable->entries[outerIndex].valid = 1;
    innerPageTables[counter] = malloc(sizeof(PageTable));
    PageTable* innerPageTable = innerPageTables[counter];

    innerPageTable->size = PAGE_TABLE_SIZE_2;
    innerPageTable->entries = malloc(sizeof(PTE) * innerPageTable->size);

    for (int i = 0; i < innerPageTable->size; i++) {
        innerPageTable->entries[i].frameNumber = 0;
        innerPageTable->entries[i].valid = 0;
        innerPageTable->entries[i].referenced = 0;
        innerPageTable->entries[i].modified = 0;
        innerPageTable->entries[i].unused = 0;
    }
    counter++;
    return 0;
}

//Functions
void clearRBits() {
    if (level == 2){
        for(int i = 0; i < getPageTableSize(); i++) {
            PageTable* innerPageTable = innerPageTables[pageTable->entries[i].frameNumber];
            for(int j = 0; j < innerPageTable->size; j++) {
                innerPageTable->entries[j].referenced = 0;
            }
        }
    }
    else if (level == 1){
        for(int i = 0; i < getPageTableSize(); i++) {
            pageTable->entries[i].referenced = 0;
        }
    }
   // printf("r bits cleared\n");
    current_tick = 0;
}

//GETTERS
int getPageTableSize() {
    return pageTable->size;
}

unsigned int getPageFrameNumber(unsigned int pageNumber) {
    if (level == 2) {
        unsigned int outerIndex = pageNumber >> 5;
        unsigned int innerIndex = pageNumber & 0x1F;

        unsigned int isInitalized = pageTable->entries[outerIndex].valid;

        if (isInitalized == 0) {
            return -1;
        }

        unsigned int innerPageTableNumber = pageTable->entries[outerIndex].frameNumber;
        PageTable* innerPageTable = innerPageTables[innerPageTableNumber];

        if (getPageValid(pageNumber) == 0) {
            return -1;
        }

        return innerPageTable->entries[innerIndex].frameNumber;
    }
    else if (level == 1) {
        if (getPageValid(pageNumber) == 0) {
            return -1;
        }
        return pageTable->entries[pageNumber].frameNumber;
    }
    return -1;
}

unsigned int getPageValid(unsigned int pageNumber) {
    if (level == 2) {
        unsigned int outerIndex = pageNumber >> 5;
        unsigned int innerIndex = pageNumber & 0x1F;

        unsigned int isInitalized = pageTable->entries[outerIndex].valid;

        if (isInitalized == 0) {
            return 0;
        }

        unsigned int innerPageTableNumber = pageTable->entries[outerIndex].frameNumber;
        PageTable* innerPageTable = innerPageTables[innerPageTableNumber];

        return innerPageTable->entries[innerIndex].valid;
    }
    else if (level == 1) {
        return pageTable->entries[pageNumber].valid;
    }
    return -1;
}

unsigned int getPageReference(unsigned int pageNumber) {
    if (level == 2) {
        unsigned int outerIndex = pageNumber >> 5;
        unsigned int innerIndex = pageNumber & 0x1F;

        unsigned int isInitalized = pageTable->entries[outerIndex].valid;

        if (isInitalized == 0) {
            return 0;
        }

        unsigned int innerPageTableNumber = pageTable->entries[outerIndex].frameNumber;
        PageTable* innerPageTable = innerPageTables[innerPageTableNumber];

        if (getPageValid(pageNumber) == 0) {
            return 0;
        }

        return innerPageTable->entries[innerIndex].referenced;
    }
    else if (level == 1) {
        return pageTable->entries[pageNumber].referenced;
    }
    return -1;
}

unsigned int getPageModified(unsigned int pageNumber) {
    if (level == 2) {
        unsigned int outerIndex = pageNumber >> 5;
        unsigned int innerIndex = pageNumber & 0x1F;

        unsigned int isInitalized = pageTable->entries[outerIndex].valid;

        if (isInitalized == 0) {
            return 0;
        }

        unsigned int innerPageTableNumber = pageTable->entries[outerIndex].frameNumber;
        PageTable* innerPageTable = innerPageTables[innerPageTableNumber];

        if (getPageValid(pageNumber) == 0) {
            return 0;
        }

        return innerPageTable->entries[innerIndex].modified;
    }
    else if (level == 1) {
        return pageTable->entries[pageNumber].modified;
    }
    return -1;
}

unsigned int getPageNumber(unsigned int frameNumber) {
    if (level == 2){
        for(int i = 0; i < getPageTableSize(); i++) {
            unsigned int isOuterEntryValid = pageTable->entries[i].valid;
            if (isOuterEntryValid == 0) {
                continue;
            }
            PageTable* innerPageTable = innerPageTables[pageTable->entries[i].frameNumber];
            for(int j = 0; j < innerPageTable->size; j++) {
                if (innerPageTable->entries[j].frameNumber == frameNumber && innerPageTable->entries[j].valid) {
                    return (i << 5) | j;
                }
            }
        }
        return -1;
    }
    else if (level == 1){
        for(int i = 0; i < getPageTableSize(); i++) {
            if (pageTable->entries[i].frameNumber == frameNumber && getPageValid(i)) {
                return i;
            }
        }
        return -1;
    }
    return -1;
}

//SETTERS
int setPageFrameNumber(unsigned int pageNumber, unsigned int frameNumber) {
    if (level == 2){
        unsigned int outerIndex = pageNumber >> 5;
        unsigned int innerIndex = pageNumber & 0x1F;

        unsigned int isInitalized = pageTable->entries[outerIndex].valid;

        if (isInitalized == 0) {
            initializeInnerPageTable(pageNumber);
        }

        unsigned int innerPageTableNumber = pageTable->entries[outerIndex].frameNumber;
        PageTable* innerPageTable = innerPageTables[innerPageTableNumber];

        innerPageTable->entries[innerIndex].frameNumber = frameNumber;
        return 0;
    }
    else if (level == 1){
        pageTable->entries[pageNumber].frameNumber = frameNumber;
        return 0;
    }
    return -1;
}

int setPageValid(unsigned int pageNumber, unsigned int valid) {
    if (level == 2){
        unsigned int outerIndex = pageNumber >> 5;
        unsigned int innerIndex = pageNumber & 0x1F;

        unsigned int isInitalized = pageTable->entries[outerIndex].valid;

        if (isInitalized == 0) {
            initializeInnerPageTable(pageNumber);
        }

        unsigned int innerPageTableNumber = pageTable->entries[outerIndex].frameNumber;
        PageTable* innerPageTable = innerPageTables[innerPageTableNumber];

        innerPageTable->entries[innerIndex].valid = valid;
        return 0;
    }
    else if (level == 1){
        pageTable->entries[pageNumber].valid = valid;
        return 0;
    }
    return -1;
}

int setPageReferenced(unsigned int pageNumber, unsigned int referenced) {
    if (level == 2){
        unsigned int outerIndex = pageNumber >> 5;
        unsigned int innerIndex = pageNumber & 0x1F;

        unsigned int isInitalized = pageTable->entries[outerIndex].valid;

        if (isInitalized == 0) {
            initializeInnerPageTable(pageNumber);
        }

        unsigned int innerPageTableNumber = pageTable->entries[outerIndex].frameNumber;
        PageTable* innerPageTable = innerPageTables[innerPageTableNumber];

        innerPageTable->entries[innerIndex].referenced = referenced;
        return 0;
    }
    else if (level == 1){
        pageTable->entries[pageNumber].referenced = referenced;
        return 0;
    }
    return -1;
}

int setPageModified(unsigned int pageNumber, unsigned int modified) {
    if (level == 2){
        unsigned int outerIndex = pageNumber >> 5;
        unsigned int innerIndex = pageNumber & 0x1F;

        unsigned int isInitalized = pageTable->entries[outerIndex].valid;

        if (isInitalized == 0) {
            initializeInnerPageTable(pageNumber);
        }

        unsigned int innerPageTableNumber = pageTable->entries[outerIndex].frameNumber;
        PageTable* innerPageTable = innerPageTables[innerPageTableNumber];

        innerPageTable->entries[innerIndex].modified = modified;
        return 0;
    }
    else if (level == 1){
        pageTable->entries[pageNumber].modified = modified;
        return 0;
    }
    return -1;
}

