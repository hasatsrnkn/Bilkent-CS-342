//
// Created by Mehmet Eren Balasar on 17.10.2023.
// gcc -Wall -o pr main.c util.c process.c globals.c -lrt -lm

#include "process.h"
#include "globals.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <mqueue.h>
#include <sys/wait.h>
#include <string.h>
#include <math.h>
#include <errno.h>

// the first int is the length, the rest are the numbers, -1 length means exit
// 3, 1, 2, 3

int initialize() {
    clock_gettime(CLOCK_MONOTONIC, &program_start);

    struct mq_attr attr;
    attr.mq_maxmsg = N * 3;
    attr.mq_msgsize = sizeof(int) * (M * 2);

    mqd_t mq = mq_open(MQNAME, O_RDWR | O_CREAT, 0666, NULL, &attr);

    mq_getattr(mq, &attr);

    clear_message_queue(mq);

    if (mq < 0) {
        perror("mq_open");
        return -1;
    }

    pid_t pid;
    for(int i = 0; i < N; i++){
        pid = fork();

        if(pid < 0) {
            perror("Error forking");
            exit_handler(-1);
        }
        else if(pid == 0) {
            process_batch(i);
            exit_handler(0);
        }
        else if (pid > 0) {
        }
    }

    parent(mq);

    logger("DONE\n");

    return 0;
}


int parent(mqd_t mq) {

    struct mq_attr mq_attr;
    mq_getattr(mq, &mq_attr);

    int exit_messages = 0;
    int bufferlen = mq_attr.mq_msgsize;
    char* bufferptr = (char*) malloc(bufferlen);

    if (bufferptr == NULL) {
        perror("malloc failed");
        exit_handler(-1);
    }

    FILE* fp = fopen(outfile, "w");

    if (fp == NULL) {
        perror("Intermediate file open failed");
        exit_handler(-1);
    }

    do {
        int n = mq_receive(mq, bufferptr, bufferlen, NULL);
        if (n == -1) {
            perror("mq_receive failed\n");
            exit_handler(1);
        }


        int* msg = (int*) bufferptr;

        int length = msg[0];

        if(length == -1){
            exit_messages++;
            if (exit_messages >= N){
                break;
            }
        }

        for (int i = 1; i <= length; i++) {
            fprintf(fp, "%d\n", msg[i]);
        }

    } while(1);

    fclose(fp);
    free(bufferptr);
    mq_close(mq);

    return 1;
}




int process_batch(int batch_id) {
    mqd_t mq_child = mq_open(MQNAME, O_WRONLY, 0666, NULL);

    int exit_sent = 0;
    printf("process_batch %d\n", batch_id);

    char filename[21];
    strcpy(filename, inter_file_prefix);
    sprintf(filename + strlen(filename), "%d", batch_id);

    FILE* fp = fopen(filename, "r");

    if (fp == NULL) {
        perror("Intermediate file open failed");
        exit_handler(-1);
    }

    int num;
    int count = 0;
    int bufferlen = M * sizeof(int) + sizeof(int);
    char* bufferptr = (char*) malloc(bufferlen);

    if (bufferptr == NULL) {
        perror("malloc failed");
        exit_handler(-1);
    }

    int* msg = (int*) bufferptr;

    while (fscanf(fp, "%d", &num) == 1) {
        if (count >= M){
            msg[0] = count;

            int sent = mq_send(mq_child, bufferptr, bufferlen, 0);

            if (sent < 0) {
                if (errno == ETIMEDOUT){
                    do{
                        sent = mq_send(mq_child, bufferptr, bufferlen, 0);
                        if (sent >= 0) break;
                    } while(1);
                }
                printf("process: %d", batch_id);
                perror("mq_send failed ");
                exit_handler(-1);
            }

            count = 0;
        }

        if (isPrime(num)){
            msg[count + 1] = num;
            count++;
        }

    }

    if (count != 0){
        msg[0] = count;
        int sent = mq_send(mq_child, bufferptr, bufferlen, 0);

        if (sent < 0) {
            if (errno == ETIMEDOUT){
                do{
                    sent = mq_send(mq_child, bufferptr, bufferlen, 0);
                    if (sent >= 0) break;
                } while(1);
            }
            printf("process: %d", batch_id);
            perror("mq_send failed ");
            exit_handler(-1);
        }

        count = 0;

        // send exit message
        msg[0] = -1;
        mq_send(mq_child, bufferptr, bufferlen, 0);
        exit_sent = 1;
    }

    if (exit_sent <= 0){
        msg[0] = -1;
        int sent = mq_send(mq_child, bufferptr, bufferlen, 0);

        if (sent < 0) {
            if (errno == ETIMEDOUT){
                do{
                    sent = mq_send(mq_child, bufferptr, bufferlen, 0);
                    if (sent >= 0) break;
                } while(1);
            }
            printf("process: %d", batch_id);
            perror("mq_send failed ");
            exit_handler(-1);
        }

        exit_sent = 1;
    }
    free(bufferptr);
    fclose(fp);
    mq_close(mq_child);
    return 0;
}

void clear_message_queue(mqd_t mq) {
    struct mq_attr attr;
    mq_getattr(mq, &attr);

    char buffer[attr.mq_msgsize];

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_sec = ts.tv_sec - 100;

    while (mq_timedreceive(mq, buffer, attr.mq_msgsize, NULL, &ts) != -1) {
        printf("Discarded message\n");
    }

    if (errno != EAGAIN && errno != ENOMSG && errno != ETIMEDOUT) {
        perror("mq_receive");
    }
}