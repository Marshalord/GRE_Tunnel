#ifndef __SOCKETCONTROLLER_H_
#define __SOCKETCONTROLLER_H_
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
typedef struct socketBufferHeader{
    unsigned short Length;
    unsigned short Command;
    unsigned int Serial;
} SOCKETBUFFERHEADER;
typedef struct socketBufferUpload{
    unsigned short Length;
    unsigned short Command;
    unsigned int Serial;
    unsigned int IP;
} SOCKETBUFFERUPLOAD;
typedef struct socketBufferResult{
    unsigned short Length;
    unsigned short Command;
    unsigned int Serial;
    unsigned char Result;
} SOCKETBUFFERRESULT;
int listenBySocket();
#endif