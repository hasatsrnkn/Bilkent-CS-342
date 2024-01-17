//
// Created by Mehmet Eren Balasar on 17.10.2023.
//

#ifndef SRC_ENTRYPOINT_H
#define SRC_ENTRYPOINT_H

int cli1(int, char*[]);
int cli2(int, char*[]);
int splitter();
int cleanup();
int isPrime(int);
void logger(const char* format, ...);
void exit_handler(int);

#endif //SRC_ENTRYPOINT_H
