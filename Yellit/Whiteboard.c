#include "Whiteboard.h"
#include "User.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define BUFFER_SIZE 1000
char list[800];
int subs[] = {0, 0, 0, 0, 0};

int getLastTopicId(Whiteboard* whiteboard){	
	return whiteboard->topicList[whiteboard->currentTopics-1].idTopic;
}

int getIdTopicByName(Whiteboard* whiteboard, char *topicName){
	for (int i=0; i<whiteboard->currentTopics; i++){
		if (strcmp(whiteboard->topicList[i].topicName, topicName)==0)
			return whiteboard->topicList[i].idTopic;
	}
	return -1;
}

char* getTopicNameById(Whiteboard* whiteboard, int id){
	for (int i=0; i<whiteboard->currentTopics; i++){
		if (whiteboard->topicList[i].idTopic == id)
			return whiteboard->topicList[i].topicName;
	}
	return "";
}

int getIdThreadByName(Whiteboard* whiteboard, char* threadName){
	for (int i=0; i<whiteboard->currentThreads; i++){
		if (strcmp(whiteboard->threadList[i].threadName, threadName) ==0)
			return whiteboard->threadList[i].id;
	}
	return -1;
}

char* getThreadNameById(Whiteboard* whiteboard, int idThread){
	for (int i=0; i<whiteboard->currentThreads; i++){
		if (whiteboard->threadList[i].id == idThread)
			return whiteboard->threadList[i].threadName;
	}
	return "";
}

int getLastThreadId(Whiteboard* whiteboard){	
	return whiteboard->threadList[whiteboard->currentThreads-1].id;
}

int getLastMessageId(Whiteboard* whiteboard){
	return whiteboard->messageList[whiteboard->currentMessages-1].idMessage;
}

char* getuserById(int idUser){
	char *username = malloc (sizeof(char)*24);
	char password[24];
	int idu;
	FILE *fp = fopen("registered_users", "r");
	if (fp == NULL)
		perror("fopen");
	while (!feof(fp)) {
		fscanf(fp, "%d %s %s", &idu, username, password);
		if (idUser == idu)
			return username;
	}	
	fclose(fp);
	return "";
}

//after publishing the message it will be visible to all users
int publishMessage(Whiteboard* whiteboard, int id){
	for (int i=0; i<whiteboard->currentMessages; i++){
		if (whiteboard->messageList[i].idMessage == id)
			whiteboard->messageList[i].status = 1;
	}
	int idm;
	int change = 0;
    char delim[] = " ";
    char stringa[60];
    char tmp[60];
    FILE * fp  = fopen("message_db", "r");
    FILE * fTemp = fopen("replace.tmp", "w");   
    if (fp == NULL || fTemp == NULL)
		perror("fopen");
    while (fgets(stringa, 100, fp)) {
		strcpy(tmp, stringa);
		char *ptr = strtok(stringa, delim);
		idm = atoi(ptr);
		if (idm == id){
			tmp[2] = '1';
			change = 1;
		}
		fputs(tmp, fTemp);
	}
    fclose(fp);
    fclose(fTemp);
    remove("message_db");
	rename("replace.tmp", "message_db");
    return change;
}

//when server starts, this method will fill whiteboard with all the topics in DB
int populateTopics(Whiteboard * whiteboard){
	char nome[24];
	int idTopic;
	int idUser;
	int i = 0;
	FILE *fp = fopen("topic_db", "r");
	if (fp == NULL)
		perror("fopen");
	while (!feof(fp)) {
		fscanf(fp, "%d %s %d", &idTopic, nome, &idUser);
		whiteboard->topicList[i].idTopic = idTopic;
		strcpy(whiteboard->topicList[i].topicName, nome);
		whiteboard->topicList[i].idOwner = idUser;
		i++;
	}
	fclose(fp);
	whiteboard->currentTopics = i-1;	
	return 0;
}

//when server starts, this method will fill whiteboard with all the threads in DB
int populateThreads(Whiteboard * whiteboard){
	char stringa[60];
	char delim[] = " ";
	char delim2[] = "\n";
	int i=0;
	int n;
	FILE *fp = fopen("thread_db", "r");
	if (fp == NULL)
		perror("fopen");
	while (fgets(stringa, 100, fp)) {
		char *ptr = strtok(stringa, delim);
		n = atoi(ptr);
		whiteboard->threadList[i].id = n;
		ptr = strtok(NULL, delim);
		n = atoi(ptr);
		whiteboard->threadList[i].idTopic = n;
		ptr = strtok(NULL, delim);
		n = atoi(ptr);
		whiteboard->threadList[i].idOwner = n;
		ptr = strtok(NULL, delim2);
		strcpy(whiteboard->threadList[i].threadName, ptr);
		i++;
	}
	fclose(fp);
	whiteboard->currentThreads = i;
	return 0;	
}

//when server starts, this method will fill whiteboard with all the messages in DB
int populateMessages(Whiteboard * whiteboard){
	char stringa[60];
	char delim[] = " ";
	char delim2[] = "\n";
	int i=0;
	int n;
	FILE *fp = fopen("message_db", "r");
	if (fp == NULL)
		perror("fopen");
	while (fgets(stringa, 100, fp)) {
		char *ptr = strtok(stringa, delim);
		n = atoi(ptr);
		whiteboard->messageList[i].idMessage = n;
		ptr = strtok(NULL, delim);
		n = atoi(ptr);
		whiteboard->messageList[i].status = n;		
		ptr = strtok(NULL, delim);
		n = atoi(ptr);
		whiteboard->messageList[i].idThread = n;
		ptr = strtok(NULL, delim);
		n = atoi(ptr);
		whiteboard->messageList[i].idOwner = n;
		ptr = strtok(NULL, delim2);
		strcpy(whiteboard->messageList[i].messageText, ptr);
		i++;
	}
	fclose(fp);
	whiteboard->currentMessages = i;
	return 0;	
}

//authenticate user and returns user id
int authenticate(Whiteboard* whiteboard, char* logUser, char* logPassword) {
	char username[24];
	char password[24];
	int idUser;
	FILE *fp = fopen("registered_users", "r");
	if (fp == NULL)
		perror("fopen");
	while (!feof(fp)) {
		fscanf(fp, "%d %s %s", &idUser, username, password);
		if (strcmp(username, logUser) == 0 && strcmp(password, logPassword) == 0) {
			if (idUser == 100){
				printf("Admin authenticated\n");
				fclose(fp);
				return 100;
			}
			else {
				printf("User authenticated\n");
				fclose(fp);
				return idUser;
			} 
		
		}
	}	
	fclose(fp);
	return -1;
}

int addTopicToDb(char* topicName, int id, int idOwner){
	FILE *fp = fopen("topic_db","a");
	if (fp == NULL)
		perror("fopen");
	fprintf(fp, "%d %s %d\n", id, topicName, idOwner);
	fclose(fp);
	return 0;
}

//create new topic in whiteboard
int addTopic(Whiteboard * whiteboard, char* topicName, int idOwner) {
	int lastId = getLastTopicId(whiteboard);
	whiteboard->topicList[whiteboard->currentTopics].idTopic = lastId+1;
	whiteboard->topicList[whiteboard->currentTopics].idOwner = idOwner;
	strcpy(whiteboard->topicList[whiteboard->currentTopics].topicName, topicName);
	addTopicToDb(topicName, lastId+1, idOwner);
	whiteboard->currentTopics++;
	return 0;
}

int addThreadToDb(char* threadName, int idTopic, int id, int idOwner){
	FILE *fp = fopen("thread_db","a");
	if (fp == NULL)
		perror("fopen");
	fprintf(fp, "%d %d %d %s\n", id, idTopic, idOwner, threadName);
	fclose(fp);
	return 0;
}

//create new thread in whiteboard
int addThread(Whiteboard * whiteboard, char* threadName, int idOwner, int idTopic) {
	int lastId = getLastThreadId(whiteboard);
	whiteboard->threadList[whiteboard->currentThreads].id = lastId+1;
	whiteboard->threadList[whiteboard->currentThreads].idTopic = idTopic;
	whiteboard->threadList[whiteboard->currentThreads].idOwner = idOwner;
	strcpy(whiteboard->threadList[whiteboard->currentThreads].threadName, threadName);
	addThreadToDb(threadName, idTopic, lastId+1, idOwner);
	whiteboard->currentThreads++;
	return 0;
}

int addMessageToDb(char* messageText, int idMessage, int idThread, int idOwner){
	FILE *fp = fopen("message_db","a");
	if (fp == NULL)
		perror("fopen");
	fprintf(fp, "%d %d %d %d %s\n", idMessage, 0, idThread, idOwner, messageText);
	fclose(fp);
	return 0;
}

//create new message in whiteboard
int addMessage(Whiteboard * whiteboard, char* messageText, int idOwner, int idThread) {
	int lastId = getLastMessageId(whiteboard);
	whiteboard->messageList[whiteboard->currentMessages].idMessage = lastId+1;
	whiteboard->messageList[whiteboard->currentMessages].idThread = idThread;
	whiteboard->messageList[whiteboard->currentMessages].idOwner = idOwner;
	strcpy(whiteboard->messageList[whiteboard->currentMessages].messageText, messageText);
	addMessageToDb(messageText, lastId+1, idThread, idOwner);
	whiteboard->currentMessages++;
	return 0;
}

int deleteMessageFromDb(int idMessage){
    int idm;
    char delim[] = " ";
    char stringa[60];
    char tmp[60];
    FILE * fp  = fopen("message_db", "r");
    FILE * fTemp = fopen("replace.tmp", "w");   
    if (fp == NULL || fTemp == NULL) 
		perror ("fopen");
    while (fgets(stringa, 100, fp)) {
		strcpy(tmp, stringa);
		char *ptr = strtok(stringa, delim);
		idm = atoi(ptr);
		if (idm != idMessage)
			fputs(tmp, fTemp);
	}
    fclose(fp);
    fclose(fTemp);
    remove("message_db");
	rename("replace.tmp", "message_db");
    return 0;
}

int deleteMessage(Whiteboard* whiteboard, int idMessage){
	for(int i=0; i<whiteboard->currentMessages; i++) {
		if(whiteboard->messageList[i].idMessage == idMessage){
			deleteMessageFromDb(idMessage);
			for (int j = i; j < whiteboard->currentMessages; j++)
				whiteboard->messageList[j] = whiteboard->messageList[j+1];
			whiteboard->currentMessages--;
			return i-1;
		}
	}
	return -1;
}

int deleteThreadFromDb(int idThread){
    int idt;
    char delim[] = " ";
    char stringa[60];
    char tmp[60];
    FILE * fp  = fopen("thread_db", "r");
    FILE * fTemp = fopen("replace.tmp", "w");   
    if (fp == NULL || fTemp == NULL) 
		perror("fopen");
    while (fgets(stringa, 100, fp)) {
		strcpy(tmp, stringa);
		char *ptr = strtok(stringa, delim);
		idt = atoi(ptr);
			
		if (idt != idThread)
			fputs(tmp, fTemp);
	}
    fclose(fp);
    fclose(fTemp);
    remove("thread_db");
	rename("replace.tmp", "thread_db");
    return 0;
}

int deleteThread(Whiteboard* whiteboard, char* threadName, int idUser){
	for(int i=0; i<whiteboard->currentThreads; i++) {
		if((strcmp(threadName, whiteboard->threadList[i].threadName) == 0) && (whiteboard->threadList[i].idOwner == idUser || idUser == 100)){
			for (int k = 0; k< whiteboard->currentMessages; k++){
				if (whiteboard->messageList[k].idThread == whiteboard->threadList[i].id)
					k =deleteMessage(whiteboard, whiteboard->messageList[k].idMessage);
			}
			deleteThreadFromDb(whiteboard->threadList[i].id);
			for (int j = i; j < whiteboard->currentThreads; j++)
				whiteboard->threadList[j] = whiteboard->threadList[j+1];
			whiteboard->currentThreads--;
			return i-1;
		}
	}
	return -1;
}

int deleteTopicFromDb(int idTopic){
    int idt,ido;
    char stringa[24];
    FILE * fp  = fopen("topic_db", "r");
    FILE * fTemp = fopen("replace.tmp", "w"); 
    
    if (fp == NULL || fTemp == NULL)
		perror("fopen");
    while(fscanf(fp, "%d %s %d", &idt, stringa, &ido) != EOF){
		if (idt != idTopic)
			fprintf(fTemp, "%d %s %d\n", idt, stringa, ido);
	}
    fclose(fp);
    fclose(fTemp);
    remove("topic_db");
	rename("replace.tmp", "topic_db");
    return 0;
}

int deleteTopic(Whiteboard* whiteboard, char* topicName, int idUser) {
	for(int i=0; i<whiteboard->currentTopics; i++) {
		if((strcmp(topicName, whiteboard->topicList[i].topicName) == 0) && (whiteboard->topicList[i].idOwner == idUser || idUser == 100)){
			for (int k = 0; k< whiteboard->currentThreads; k++){
				if (whiteboard->threadList[k].idTopic == whiteboard->topicList[i].idTopic)
					k = deleteThread(whiteboard, whiteboard->threadList[k].threadName,100);
			}
			deleteTopicFromDb(whiteboard->topicList[i].idTopic);
			for (int j = i; j < whiteboard->currentTopics; j++)
				whiteboard->topicList[j] = whiteboard->topicList[j+1];
			whiteboard->currentTopics--;
			return 0;
		}
	}
	return -1;
}

int addUser(int id, char* username, char* password){
	if (id == 0)
		return -1;
	FILE *fp;
	fp = fopen("registered_users","a");
	if (fp == NULL)
		perror("fopen");
	fprintf(fp, "%d %s %s\n", id, username, password);
	fclose(fp);
	return 0;
}

int deleteUser(int id){
	if (id == 0)
		return -1;
	int idu;
	int ret = -1;
    char username[24];
    char password[24];
    FILE * fp  = fopen("registered_users", "r");
    FILE * fTemp = fopen("replace.tmp", "w");   
    if (fp == NULL || fTemp == NULL)
		perror("fopen");
    while(fscanf(fp, "%d %s %s", &idu, username, password) != EOF){
		if (idu != id)
			fprintf(fTemp, "%d %s %s\n", idu, username, password);
		else
			ret = 0;
	}
    fclose(fp);
    fclose(fTemp);
    remove("registered_users");
	rename("replace.tmp", "registered_users");	
	return ret;
}

char * showUsers(){
	FILE *fp;
	char line[64];
	fp = fopen("registered_users","r");
	if (fp == NULL)
		perror("fopen");
	while (fgets(line,sizeof(line),fp) != NULL){
    strcat(list,line);
    strcat(list, "\n");
	}
	fclose(fp);
	return list;
}

//after creating a new user, initialize to 0 his subscriptions
int createRowSubs(int idUser){
	if (idUser == 100)
		return -1;
	FILE *fp;
	fp = fopen("subscriptions_db", "a");
	if (fp == NULL)
		perror("fopen");
	fprintf(fp, "%d %d %d %d %d %d\n", idUser, 0, 0, 0, 0, 0);
	fclose(fp);
	return 0;
}

//after user login, remember his subscriptions
int loadSubscriptions(int idUser){
	FILE *fp;
	fp = fopen("subscriptions_db", "r");
	if (fp == NULL)
		perror("fopen");
	int idu,s1,s2,s3,s4,s5;
	while(fscanf(fp, "%d %d %d %d %d %d", &idu, &s1, &s2, &s3, &s4, &s5) != EOF){
		if(idUser == idu){
		subs[0]=s1;
		subs[1]=s2;
		subs[2]=s3;
		subs[3]=s4;
		subs[4]=s5;
		fclose(fp);
		return 0;		
		}
	}
	fclose(fp);
	return 0;
}

//update whiteboard and db to the new subscription/unsubscription
int updateSubscriptions(int idUser){
	int idu,s1,s2,s3,s4,s5;
    FILE * fp  = fopen("subscriptions_db", "r");
    FILE * fTemp = fopen("replace.tmp", "w");   
    if (fp == NULL || fTemp == NULL)
		perror("fopen");
    while(fscanf(fp, "%d %d %d %d %d %d", &idu, &s1, &s2, &s3, &s4, &s5) != EOF){
		if (idu != idUser)
			fprintf(fTemp, "%d %d %d %d %d %d\n", idu, s1, s2, s3, s4, s5);
		else
			fprintf(fTemp, "%d %d %d %d %d %d\n", idu, subs[0], subs[1], subs[2], subs[3], subs[4]);
	}
    fclose(fp);
    fclose(fTemp);
    remove("subscriptions_db");
	rename("replace.tmp", "subscriptions_db");	
	return 0;
}
	
int subscribeToTopic(int idUser, int idTopic){
	for (int i=0; i<5; i++){
		if (subs[i] == idTopic){
			subs[i] = 0;
			return 0;
		}
	}
	for (int i=0; i<5; i++){
		if (subs[i] == 0){
			subs[i] = idTopic;
			return 0;
		}
	}
	return -1;
}

//when a user is deleted, deleted his information about subscriptions
int deleteRowSubs(int idUser){
	int idu,s1,s2,s3,s4,s5, ret;
	ret = 0;
    FILE * fp  = fopen("subscriptions_db", "r");
    FILE * fTemp = fopen("replace.tmp", "w");   
    if (fp == NULL || fTemp == NULL)
		perror("fopen");
    while(fscanf(fp, "%d %d %d %d %d %d", &idu, &s1, &s2, &s3, &s4, &s5) != EOF){
		if (idUser != idu)
			fprintf(fTemp, "%d %d %d %d %d %d\n", idu, s1, s2, s3, s4, s5);
		else
			ret=1;
	}
    fclose(fp);
    fclose(fTemp);
    remove("subscriptions_db");
	rename("replace.tmp", "subscriptions_db");	
	return ret;
}

//check if user is subscribed to given topic
int checkSub(int idTopic){
	for (int i=0; i<5; i++){
		if (subs[i]==idTopic)
		return 0;
	}
	return -1;	
}
