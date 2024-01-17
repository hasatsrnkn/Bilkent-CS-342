//
// Created by Mehmet Eren Balasar on 17.10.2023.
//

#ifndef SRC_PROCESS_H
#define SRC_PROCESS_H
#include <mqueue.h>

int initialize();
int process_batch(int);

void clear_message_queue(mqd_t);
int parent(mqd_t);
#endif //SRC_PROCESS_H
