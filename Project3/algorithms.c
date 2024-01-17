//
// Created by hasatsrnkn on 02.12.2023.
//

#include "algorithms.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "file.h"
#include "pagetable.h"

//Fifo Components
unsigned int *fifoQueue = NULL;
unsigned int fifoFront = 0;
unsigned int fifoRear = -1;

//LRU Components
PageNode* head;
PageNode* tail;
PageNode* temp;

//Clock Components
ClockPageNode *clockHand = NULL;


void initializePageReplacementAlgorithm(PhysicalMemory *pm) {
    // FIFO
    if(strcmp(algo, "FIFO") == 0) {
        fifoQueue = (unsigned int *)malloc(sizeof(unsigned int) * pm->totalFrames);
        for (int i = 0; i < pm->totalFrames; i++) {
            fifoQueue[i] = -1;
        }
        fifoFront = 0;
        fifoRear = -1;
    }

    //LRU
    else if(strcmp(algo, "LRU") == 0) {
        for(int i = 0; i < pm->totalFrames; i++) {
            int status = addNode(-1);

            if (status < 0) {
                printf("Could not assign node\n");
            }
        }
    }

    //CLOCK
    else if((strcmp(algo, "CLOCK") == 0) || (strcmp(algo, "ECLOCK") == 0)) {
        ClockPageNode *lastNode = NULL;
        for (int i = 0; i < pm->totalFrames; i++) {
            ClockPageNode *newNode = (ClockPageNode *) malloc(sizeof(ClockPageNode));
            if (!newNode) {
                exit(EXIT_FAILURE);
            }
            newNode->pageNumber = -1; // Indicate an empty frame
            newNode->next = (i == 0) ? newNode : clockHand; // Point to the first node

            if (lastNode) {
                lastNode->next = newNode;
            }
            lastNode = newNode;

            if (i == 0) {
                clockHand = newNode; // Initialize the clock hand
            }

            if( i == pm->totalFrames - 1) {
                lastNode->next = clockHand;
            }

        }
    }

}

//FIFO
void enqueuePage(unsigned int pageNumber) {
    fifoRear = (fifoRear + 1) % fcount;
    fifoQueue[fifoRear] = pageNumber;
}

unsigned int dequeuePage() {
    unsigned int pageToReplace = fifoQueue[fifoFront];
    fifoFront = (fifoFront + 1) % fcount;
    return pageToReplace;
}

void freeFIFOQueue() {
    free(fifoQueue);
    fifoQueue = NULL;
}

//LRU
int addNode(unsigned int pageNumber) {
    if (head == NULL) {
        head = (PageNode *) malloc(sizeof(PageNode));

        if (head == NULL) {
            printf("Unable to allocate space\n");
            return -2;
        }

        head->pageNumber = pageNumber;
        tail = head;
        head->prev = NULL;
    }
    else {
        temp = tail;
        tail->next = (PageNode *) malloc(sizeof(PageNode));

        if (tail->next == NULL) {
            printf("Unable to allocate space\n");
            return -2;
        }

        tail->next->pageNumber = pageNumber;
        tail = tail->next;
        tail->prev = temp;
    }
    tail->next = NULL;
    return 0;
}

int LRUFuncNode(unsigned int pageNumber, unsigned int* victimPage) {
    if (head == NULL) {
        printf("Add a node first\n");
        return -1;
    }
    temp = head;

    // Traverse Double Linked List.
    while (temp != NULL) {
        if (temp->pageNumber == pageNumber){
            while (temp != head) {
                temp->pageNumber = temp->prev->pageNumber;
                temp = temp->prev;
            }
            head->pageNumber = pageNumber;
            return 0;
        }
        temp = temp->next;
    }

    // For new elements.
    temp = tail->prev;
    *victimPage = getLRUVictimPage();
    while (temp != NULL) {
        temp->next->pageNumber = temp->pageNumber;
        temp = temp->prev;
    }
    head->pageNumber = pageNumber;
    return 0;
}

unsigned int getLRUVictimPage() {
    if (tail == NULL) {
        printf("No pages in LRU list.\n");
        return -1; // Indicate an error or empty list
    }
    return tail->pageNumber;
}

void freeLRUList() {
    PageNode *current = head;
    while (current != NULL) {
        PageNode *next = current->next;
        free(current);
        current = next;
    }
    head = NULL;
    tail = NULL;
}

//CLOCK
void addToClockBuffer(unsigned int pageNumber, unsigned int *victimPage) {
    ClockPageNode *current = clockHand;
    do {
        if (current->pageNumber == -1 || current->pageNumber == pageNumber) {
            current->pageNumber = pageNumber;
            setPageReferenced(pageNumber, 1);
            return;
        }
        current = current->next;
    } while (current != clockHand);


    // If we reach here, all frames are full
    *victimPage = findReplacementPageClock();
    addToClockBuffer(pageNumber, victimPage); // Add the new page

}

unsigned int findReplacementPageClock() {
    while (1) {
        if (getPageReference(clockHand->pageNumber) == 0) {
            unsigned int victimPage = clockHand->pageNumber;
            clockHand->pageNumber = -1; // Reset for new page
            return victimPage;
        }
        setPageReferenced(clockHand->pageNumber, 0);
        clockHand = clockHand->next;
    }
}

void freeClockList() {
    ClockPageNode *current = clockHand;
    do {
        ClockPageNode *next = current->next;
        free(current);
        current = next;
    } while (current != clockHand);
    clockHand = NULL;
}

//ECLOCK
void addToEnhancedClockBuffer(unsigned int pageNumber, unsigned int *victimPage) {
    ClockPageNode *current = clockHand;
    do {
        if (current->pageNumber == -1 || current->pageNumber == pageNumber) {
            current->pageNumber = pageNumber;
            setPageReferenced(pageNumber, 1);
            return;
        }
        current = current->next;
    } while (current != clockHand);

    // If we reach here, all frames are full
    *victimPage = findReplacementPageEnhancedClock();

    addToEnhancedClockBuffer(pageNumber, victimPage); // Add the new page

}

unsigned int findReplacementPageEnhancedClock() {
    ClockPageNode *start = clockHand;

    // Step 1
    do {
        if (getPageReference(clockHand->pageNumber) == 0 && getPageModified(clockHand->pageNumber) == 0) {
            unsigned int victimPage = clockHand->pageNumber;
            clockHand->pageNumber = -1;
            return victimPage;
        }
        clockHand = clockHand->next;
    } while (clockHand != start);

    // Step 2
    do {
        if (getPageReference(clockHand->pageNumber)== 1) {
            setPageReferenced(clockHand->pageNumber,0);
        } else if (getPageModified(clockHand->pageNumber) == 1) {
            unsigned int victimPage = clockHand->pageNumber;
            clockHand->pageNumber = -1;
            return victimPage;
        }
        clockHand = clockHand->next;
    } while (clockHand != start);

    // Step 3 & 4 (Repeat step 1 & 2 if necessary)
    return findReplacementPageEnhancedClock();
}

void freeEnhancedClockList() {
    ClockPageNode *current = clockHand;
    do {
        ClockPageNode *next = current->next;
        free(current);
        current = next;
    } while (current != clockHand);
    clockHand = NULL;
}