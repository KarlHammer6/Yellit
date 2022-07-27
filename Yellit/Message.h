#ifndef WHITEBOARD_MESSAGE_H
#define WHITEBOARD_MESSAGE_H

typedef struct{
	int idMessage;
    int status;
    int idThread;
    int idOwner;
	char messageText[1024];
} Message;

#endif
