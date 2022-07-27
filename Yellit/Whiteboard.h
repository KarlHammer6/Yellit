#ifndef WHITEBOARD_H
#define WHITEBOARD_H

#include "Topic.h"

typedef struct {
	Topic topicList[10];
    Thread threadList[20];
    Message messageList[30];
    int currentTopics;
    int currentThreads;
    int currentMessages;
} Whiteboard;

#endif
