//
// Created by xyzjiao on 4/15/22.
//

#ifndef NIO_SIGHANDLER_H
#define NIO_SIGHANDLER_H

#include "common.h"

class sighandler {
public:
    static int signal_sig_int();
    static void signal_handler(int sig);
};


#endif //NIO_SIGHANDLER_H
