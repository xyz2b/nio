//
// Created by hongyujiao on 7/12/21.
//

#ifndef MARK_CLEAR_COMMON_H
#define MARK_CLEAR_COMMON_H

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <memory.h>
#include <string>
#include <csignal>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <cstring>
#include <arpa/inet.h>
#include <fcntl.h>

using namespace std;

#define MAX_EVENTS 1024
#define READ_BUFF_SIZE 1024

const int OP_ACCEPT = 1 << 4;
const int OP_READ = 1 << 0;
const int OP_WRITE = 1 << 2;

typedef void *              pvoid;
typedef unsigned char       byte;
typedef unsigned short      ushort;
typedef unsigned int        uint;
typedef unsigned long       ulong;

typedef union {
    long        l_dummy;
    double      d_dummy;
    void *      v_dummy;
} Align;

#define ALIGN_SIZE (sizeof(Align))

typedef enum {
    GC_MARK_CLEAN,
    GC_MARK_COLLECT,
    GC_MARK_COPY,
    GC_G1,
} GC_Type;

#define UseParallelGC false
#define UseG1GC false
#define UseSerialGC false
#define UseConcMarkSweepGC false
#define UseAdaptiveSizePolicy false

// gc算法
#define DEFAULT_GC_TYPE GC_MARK_COPY

/* ==============================
 * customize print
 * ============================== */
#define DEBUG           0
#define INFO            1
#define WARNING         2
#define ERROR           3

#define LOG_LEVEL         INFO

#define PRINT(level, msg, ...) do{ \
    printf("[%s] (%s:%d->%s): " msg"\n", level, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); \
}while(0)

#define DEBUG_PRINT(msg, ...) if (DEBUG >= LOG_LEVEL)  { \
    PRINT("DEBUG", msg, ##__VA_ARGS__);    \
}

#define INFO_PRINT(msg, ...) if (INFO >= LOG_LEVEL)  { \
    PRINT("INFO", msg, ##__VA_ARGS__);    \
}

#define WARNING_PRINT(msg, ...) if (WARNING >= LOG_LEVEL)  { \
    PRINT("WARNING", msg, ##__VA_ARGS__);    \
}

#define ERROR_PRINT(msg, ...) if (ERROR >= LOG_LEVEL)  { \
    PRINT("ERROR", msg, ##__VA_ARGS__);    \
}

#define NULL_POINTER(ptr) if (ptr == NULL) { \
    PRINT("ERROR", "null pointer", ##__VA_ARGS__);                                            \
}

#endif //MARK_CLEAR_COMMON_H
