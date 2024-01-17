#include "util.h"
#include "process.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>

int main(int argc, char *argv[]) {

        cli1(argc, argv);
        splitter();
        initialize();

        mqd_t mq = mq_open(MQNAME, O_RDWR, 0666, NULL);
        if(mq >= 0){
            mq_close(mq);
            mq_unlink(MQNAME);
        }
        cleanup();

    return 0;
}
