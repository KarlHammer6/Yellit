#ifndef WHITEBOARD_THREAD_H
#define WHITEBOARD_THREAD_H

#include "Message.h"

typedef struct {
    int idTopic;
    int id;
    char threadName[64];
    int idOwner;
    Message lastMessage;
} Thread;

#endif
