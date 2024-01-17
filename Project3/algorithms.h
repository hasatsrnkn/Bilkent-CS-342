//
// Created by hasatsrnkn on 02.12.2023.
//

#ifndef MEMSIM_ALGORITHMS_H
#define MEMSIM_ALGORITHMS_H
#include "globals.h"
#include "pagetable.h"
extern unsigned int *fifoQueue;
extern unsigned int fifoFront;
extern unsigned int fifoRear;
void initializePageReplacementAlgorithm(PhysicalMemory *pm);
unsigned int dequeuePage();
void enqueuePage(unsigned int pageNumber);
void freeFIFOQueue();

// LRU Declarations
typedef struct PageNode {
    unsigned int pageNumber;
    struct PageNode *prev;
    struct PageNode *next;
} PageNode;
int addNode(unsigned int pageNumber);
int LRUFuncNode(unsigned int pageNumber, unsigned int* victimPage);
unsigned int getLRUVictimPage();
void freeLRUList();

//Clock Declarations
typedef struct ClockPageNode {
    unsigned int pageNumber;
    struct ClockPageNode *next;
} ClockPageNode;
void addToClockBuffer(unsigned int pageNumber, unsigned int *victimPage);
unsigned int findReplacementPageClock();
void freeClockList();

void addToEnhancedClockBuffer(unsigned int pageNumber, unsigned int *victimPage);
unsigned int findReplacementPageEnhancedClock();
void freeEnhancedClockList();
#endif //MEMSIM_ALGORITHMS_H
