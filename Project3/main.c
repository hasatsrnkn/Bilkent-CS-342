#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "utils.h"
#include "file.h"
#include "algorithms.h"
#include "pagetable.h"

int initializeComponents(PhysicalMemory *physicalMemory) {
    //Initialize swap file
    create_swap_file();

    //Initialize physical memory
    physicalMemory->totalFrames = fcount;
    physicalMemory->frames = malloc(sizeof(Frame) * physicalMemory->totalFrames);
    if (physicalMemory->frames == NULL) {
        perror("Failed to allocate memory for frames");
        return 1;
    }

    // Initialize all frames to zero
    for (int i = 0; i < physicalMemory->totalFrames; i++) {
        memset(physicalMemory->frames[i].data, 0, PAGESIZE);
        physicalMemory->frames[i].containsPage = 0;
    }

    // Initialize Page Table
    initializePageTable();

    //Initialize Algorithm
    initializePageReplacementAlgorithm(physicalMemory);


    return 0;
}

unsigned int findFreeFrame(PhysicalMemory *physicalMemory) {
    for(int i = 0; i < physicalMemory->totalFrames; i++) {
        if(physicalMemory->frames[i].containsPage == 0 ) {
            return i;
        }
    }
    return -1;
}

int handlePageFault(unsigned int pageNumber, PhysicalMemory *pm){

    unsigned int frameNumber = findFreeFrame(pm);

    //No free frame
    if( frameNumber == -1 ) {

        //FIFO algorithm
        if(strcmp(algo, "FIFO") == 0) {
            unsigned int victimPage = dequeuePage();
            frameNumber = getPageFrameNumber(victimPage);

            if (getPageModified(victimPage)) {
                // Write the page back to the swap file
                write_to_swap_file(swapfile, victimPage, pm->frames[frameNumber].data);
            }

            setPageValid(victimPage,0);
            setPageReferenced(victimPage,0);
            setPageModified(victimPage,0);
            //Load pageNumber to the memory
            read_from_swap_file(swapfile, pageNumber, pm->frames[frameNumber].data);

            //Change of entries
            setPageValid(pageNumber, 1);
            setPageFrameNumber(pageNumber,frameNumber);
            setPageReferenced(pageNumber,1);
            setPageModified(pageNumber,0);

            enqueuePage(pageNumber);
        }

        //LRU algorithm
        else if(strcmp(algo,"LRU") == 0) {
            unsigned int victimPage;
            LRUFuncNode(pageNumber, &victimPage);

            if( victimPage != -1) {

                frameNumber = getPageFrameNumber(victimPage);

                if (getPageModified(victimPage)) {
                    // Write the page back to the swap file
                    write_to_swap_file(swapfile, victimPage, pm->frames[frameNumber].data);
                }
                setPageValid(victimPage,0);
                setPageReferenced(victimPage,0);
                setPageModified(victimPage,0);
                //Load pageNumber to the memory
                read_from_swap_file(swapfile, pageNumber, pm->frames[frameNumber].data);

                //Change of entries
                setPageValid(pageNumber, 1);
                setPageFrameNumber(pageNumber,frameNumber);
                setPageReferenced(pageNumber,1);
                setPageModified(pageNumber,0);


            }
        }

        //Clock Algorithm
        else if(strcmp(algo,"CLOCK") == 0) {
            unsigned int victimPage;
            addToClockBuffer(pageNumber, &victimPage);
            if( victimPage != -1) {
                frameNumber = getPageFrameNumber(victimPage);

                if (getPageModified(victimPage)) {
                    // Write the page back to the swap file
                    write_to_swap_file(swapfile, victimPage, pm->frames[frameNumber].data);
                }
                setPageValid(victimPage,0);
                setPageReferenced(victimPage,0);
                setPageModified(victimPage,0);
                //Load pageNumber to the memory
                read_from_swap_file(swapfile, pageNumber, pm->frames[frameNumber].data);

                //Change of entries
                setPageValid(pageNumber, 1);
                setPageFrameNumber(pageNumber,frameNumber);
                setPageReferenced(pageNumber,1);
                setPageModified(pageNumber,0);

            }
        }
        else if(strcmp(algo,"ECLOCK") == 0) {
            unsigned int victimPage;
            addToEnhancedClockBuffer(pageNumber, &victimPage);
            if( victimPage != -1) {
                frameNumber = getPageFrameNumber(victimPage);

                if (getPageModified(victimPage)) {
                    // Write the page back to the swap file
                    write_to_swap_file(swapfile, victimPage, pm->frames[frameNumber].data);
                }
                setPageValid(victimPage,0);
                setPageReferenced(victimPage,0);
                setPageModified(victimPage,0);
                //Load pageNumber to the memory
                read_from_swap_file(swapfile, pageNumber, pm->frames[frameNumber].data);

                //Change of entries
                setPageValid(pageNumber, 1);
                setPageFrameNumber(pageNumber,frameNumber);
                setPageReferenced(pageNumber,1);
                setPageModified(pageNumber,0);

            }

        }
    }
    else { //there is free frame
        read_from_swap_file(swapfile, pageNumber, pm->frames[frameNumber].data);
        pm->frames[frameNumber].containsPage = 1;

        setPageValid(pageNumber, 1);
        setPageFrameNumber(pageNumber,frameNumber);
        setPageReferenced(pageNumber,1);
        setPageModified(pageNumber,0);

        //FIFO algorithm
        if(strcmp(algo, "FIFO") == 0) {
            enqueuePage(pageNumber);
        }

        //LRU algorithm
        if(strcmp(algo, "LRU") == 0) {
            unsigned int victimPage;
            LRUFuncNode(pageNumber, &victimPage);
        }

        //CLOCK Algorithm
        if(strcmp(algo, "CLOCK") == 0) {
            unsigned int victimPage;
            addToClockBuffer(pageNumber, &victimPage);
        }

        //Enhanced CLOCK Algorithm
        if(strcmp(algo, "ECLOCK") == 0) {
            unsigned int victimPage;
            addToEnhancedClockBuffer(pageNumber, &victimPage);
        }

    }

    return 0;
}

unsigned int translate_address(unsigned int virtualAddress, PhysicalMemory *pm, int* pgfault) {
    unsigned int pageNumber = virtualAddress >> 6; // Remove offset bits
    unsigned int offset = virtualAddress & OFFSET_MASK;

    if (!getPageValid(pageNumber)) {
        if (handlePageFault(pageNumber, pm) != 0) {
            fprintf(stderr, "Page fault handling failed\n");
            exit(-1);
        }
        *pgfault = 1;
    }
    else {
        *pgfault = 0;

        //LRU Algorithm
        if(strcmp(algo, "LRU") == 0) {
            unsigned int victimPage;
            LRUFuncNode(pageNumber, &victimPage);
        }

        //CLOCK Algorithm
        if(strcmp(algo, "CLOCK") == 0) {
            unsigned int victimPage;
            addToClockBuffer(pageNumber, &victimPage);
        }

        //Enhanced CLOCK Algorithm
        if(strcmp(algo, "ECLOCK") == 0) {
            unsigned int victimPage;
            addToEnhancedClockBuffer(pageNumber, &victimPage);
        }

    }

    // Assuming frameNumber is valid now
    unsigned int frameNumber = getPageFrameNumber(pageNumber);

    unsigned int physicalAddress = (frameNumber * PAGESIZE) + offset;

    return physicalAddress;
}



int main(int argc, char* argv[]) {

    cli(argc,argv);
    PhysicalMemory physicalMemory;

    if (initializeComponents(&physicalMemory) != 0) {
        fprintf(stderr, "Failed to initialize components\n");
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &program_start);

    FILE* fp = fopen(addrfile, "r");
    if (!fp) {
        perror("Error opening some.txt");
        exit(-1);
    }

    char line[32];
    // LINE BY LINE PROCESSING
    while (fgets(line, sizeof(line), fp) != NULL) {
        char rw;
        unsigned int va = 0;
        unsigned int data = 0;
        //printf("%s", line);

        if(current_tick==tick) {
            clearRBits();
        }

        parse_line(line, &rw, &va, &data);
        int pgfault;
        unsigned int physicalAddress = translate_address(va, &physicalMemory,&pgfault);

        if (pgfault) {
            total_page_faults++;
        }

        unsigned int frameIndex = physicalAddress / PAGESIZE;
        unsigned int offset = physicalAddress % PAGESIZE;

        if (rw == 'r') {
            // Read operation
           // char value = physicalMemory.frames[frameIndex].data[offset];
        } else if (rw == 'w') {
            // Write operation
            physicalMemory.frames[frameIndex].data[offset] = (char)data;
            setPageModified(va >> 6, 1); // Set modified bit for the page
        }
        //printf("rw: %c, va: %x, data: %x\n", rw, va, data);
        current_tick++;

        if( level == 1 ) {
            write_to_output_file(outfile,va,va>>6,0,offset,frameIndex,physicalAddress, pgfault);
        }
        if( level == 2 ) {
            write_to_output_file(outfile,va,va>>11,(va & 0x07c0) >> 6,offset,frameIndex,physicalAddress, pgfault);
        }
    }
    fclose(fp);

    // Write total page faults to the output file
    FILE *outFile = fopen(outfile, "a");
    if (!outFile) {
        perror("Error opening output file");
        exit(-1);
    }

    fprintf(outFile, "Total Page Faults: %d\n", total_page_faults);
    fclose(outFile);


    //Now it is time to write pm data to swapfile
    for(unsigned int i = 0; i < fcount; i++) {
        //printf("pagernumber:%d\n",getPageNumber(i));
        write_to_swap_file(swapfile, getPageNumber(i), physicalMemory.frames[i].data);
    }

    //free memory
    if(level== 2) {
        for(int i = 0; i < getPageTableSize(); i++) {
            if (pageTable->entries[i].valid == 1){
                PageTable* innerPageTable = innerPageTables[pageTable->entries[i].frameNumber];
                free(innerPageTable->entries);
                free(innerPageTable);
            }
        }
        free(innerPageTables);
    }


    free(pageTable->entries);

    free(pageTable);
    free(physicalMemory.frames);

    if (strcmp(algo, "FIFO") == 0)
        freeFIFOQueue();
    if (strcmp(algo, "LRU") == 0)
        freeLRUList();
    if (strcmp(algo, "CLOCK") == 0)
        freeClockList();
    if (strcmp(algo, "ECLOCK") == 0)
        freeEnhancedClockList();

    printf("Total Page Faults: %d\n", total_page_faults);
    printf("Program is finished successfully\n");

    return 0;
}



