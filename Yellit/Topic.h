#ifndef WHITEBOARD_TOPIC_H
#define WHITEBOARD_TOPIC_H

#include "Thread.h"

typedef struct {
    int idTopic;
	char topicName[64];
    int idOwner;
} Topic;

#endif
