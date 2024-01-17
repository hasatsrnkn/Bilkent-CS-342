
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "globals.h"
#include <time.h>
#include "util.h"

//
// Created by hasatsrnkn on 15.10.2023.
//
struct PrimeNumberInfo {
    int number;
    int frequency;
};

struct Node {
    struct PrimeNumberInfo data;
    struct Node* next;
};

struct LinkedList {
    struct Node* head;
    pthread_mutex_t mutex;
};

struct LinkedList* threadData;

void* processInputFile(void* arg) {
    int threadId = *((int*)arg);
    struct LinkedList* list = &threadData[threadId];

    char filename[100];  
    sprintf(filename, "inter%d", threadId);  
    FILE* inputFile = fopen(filename, "r");

    if (inputFile == NULL) {
        perror("Error opening input file");
        pthread_exit(NULL);
    }

    int number;

    while (fscanf(inputFile, "%d", &number) != EOF) {
        if (isPrime(number)) {
            // Lock the mutex before modifying the linked list.
            pthread_mutex_lock(&list->mutex);

            // Check if the prime number is already in the linked list.
            struct Node* current = list->head;
            while (current != NULL) {
                if (current->data.number == number) {
                    // Increment the frequency if the number is found again.
                    current->data.frequency++;
                    break; // Exit the loop
                }
                current = current->next;
            }

            // If the number is not found, add it to the linked list.
            if (current == NULL) {
                struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
                if (newNode == NULL) {
                    fprintf(stderr, "Memory allocation error\n");
                    exit(1);
                }
                newNode->data.number = number;
                newNode->data.frequency = 1;
                newNode->next = list->head;
                list->head = newNode;
            }

            // Unlock the mutex after modifying the linked list.
            pthread_mutex_unlock(&list->mutex);
        }
    }

    fclose(inputFile);

    // Terminate the thread.
    pthread_exit(NULL);
}

int main( int argc, char* argv[] ) {

    //Program Start
    clock_gettime(CLOCK_MONOTONIC,&program_start);

    cli2(argc,argv);

    //Creating intermediate files
    splitter();

    // Initialize global variables, including linked lists.
    threadData = malloc(N * sizeof(struct LinkedList));
    for (int i = 0; i < N; i++) {
        threadData[i].head = NULL;
        pthread_mutex_init(&threadData[i].mutex, NULL);
    }

    // Create worker threads.
    pthread_t* threads = malloc(N * sizeof(pthread_t));
    int* threadIds = malloc(N * sizeof(int));

    for (int i = 0; i < N; i++) {
        threadIds[i] = i;
        pthread_create(&threads[i], NULL, processInputFile, &threadIds[i]);
    }

    // Wait for all worker threads to finish.
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }
    FILE* output = fopen(outfile, "w");
    if (output == NULL) {
        perror("Error opening output file");
        exit(1);
    }

    //Write the data into the output file
    for (int i = 0; i < N; i++) {
        struct Node* current = threadData[i].head;
        struct Node* prev = current;
        while (current != NULL) {
            for (int j = 0; j < current->data.frequency; j++) {
                fprintf(output, "%d\n", current->data.number);
            }
            current = current->next;
            free(prev);
            prev = current;
        }
    }

    fclose(output);

    cleanup();

    // Clean up resources and exit.
    free(threads);
    free(threadIds);
    free(threadData);

    logger("Program finished\n");

    return 0;
}

