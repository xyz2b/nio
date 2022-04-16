//
// Created by xyzjiao on 4/15/22.
//

#include "../include/sighandler.h"

int sighandler::signal_sig_int() {
    if (SIG_ERR == signal(SIGINT, signal_handler)) {
        perror("signal SIGINT failed\n");
        return -1;
    } else {
        return 0;
    }
}

void sighandler::signal_handler(int sig) {
    if (SIGINT == sig) {
        printf("receive SIGINT signal\n");
        exit(-1);
    }
}