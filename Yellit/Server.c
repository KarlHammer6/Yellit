#include <stdio.h> 
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include "Whiteboard.c"

#define READ_SEM "./sem-read-key"
#define WRITE_SEM "./sem-write-key"
#define USERS_SEM "./sem-users-key"

// Buffer data structures
#define MAX_READERS 20

struct arg_struct {
	int socket_desc;
	struct sockaddr_in client_addr;
};

int read_sem, write_sem, users_sem;

void WriteInShm(int shm_id, Whiteboard* whiteboard){
    Whiteboard* str =  (Whiteboard*) shmat(shm_id, (void *) 0, 0);
    shmdt(str);
}

Whiteboard* gettShm(int shm_id){
    Whiteboard * str = (Whiteboard*) shmat(shm_id, (void *) 0, 0);
    return str;
}

int main(void){
	
	//Semaphore
	key_t s_key;
	union semun{
		int val;
		struct semid_ds *buf;
		ushort array [1];
	} sem_attr;

	//Read semaphore with starting value of MAX_READERS-1
	if ((s_key = ftok (READ_SEM, 'a')) == -1) {
		perror ("ftok"); exit (1);
	}
	if ((read_sem = semget (s_key, 1, 0660 | IPC_CREAT)) == -1) {
		perror ("semget"); exit (1);
	}
	sem_attr.val = MAX_READERS;
	if (semctl (read_sem, 0, SETVAL, sem_attr) == -1) {
		perror ("semctl cannot set value"); exit (1);
	}

	//Write semaphore, only one writer at a time can access shared memory
	if ((s_key = ftok (WRITE_SEM, 'a')) == -1) {
		perror ("ftok"); exit (1);    /* for semaphore */
	}
	if ((write_sem = semget (s_key, 1, 0660 | IPC_CREAT)) == -1) {
		perror ("semget"); exit (1);
	}
	sem_attr.val = 1;
	if (semctl (write_sem, 0, SETVAL, sem_attr) == -1) {
		perror (" semctl cannot set value"); exit (1);
	}
	
	//Users semaphore, only one writer at a time can access shared memory
	if ((s_key = ftok (USERS_SEM, 'a')) == -1) {
		perror ("ftok"); exit (1);    /* for semaphore */
	}
	if ((users_sem = semget (s_key, 1, 0660 | IPC_CREAT)) == -1) {
		perror ("semget"); exit (1);
	}
	sem_attr.val = 1;
	if (semctl (users_sem, 0, SETVAL, sem_attr) == -1) {
		perror (" semctl cannot set value"); exit (1);
	}

    struct sockaddr_in server_addr;   
    int socket_desc, client_sock,accept_fd;
    char server_message[2000], client_message[2000];
    char clientUserName[2000], clientPassword[2000];

    //Identifier for the shared memory
    int shm_id = shmget(IPC_PRIVATE,sizeof(Whiteboard)+20*sizeof(Topic) + 30*(sizeof(Thread) + sizeof(Message)*100),0666|IPC_CREAT);

	Whiteboard* whiteboard = gettShm(shm_id);
	
	WriteInShm(shm_id,whiteboard);
	
	populateTopics(whiteboard);	
	populateThreads(whiteboard);
	populateMessages(whiteboard);
	
	//Setting the Buffers to \0 
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));

    //Socket creation 
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_desc < 0){
        printf("Error in creating socket\n");
        return -1;
    }

    printf("Socket Created\n");

    //Binding IP and Port to socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(50000);	//Htons function to use proper byte order, using an ephemeral port
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        printf("Bind Failed. Error!!!!!\n");
        return -1;
    }
	printf("Bind Executed\n");

    //Listening for incoming connections
    if(listen(socket_desc, 1) < 0){
        printf("Listening Failed. Error!!!!!\n");
        return -1;
    }
	printf("Listening for Incoming Connections ...\n");

    //Accept the incoming Connections
    socklen_t addr_size = sizeof(struct sockaddr_in);
    while (1){
        if ((accept_fd = accept(socket_desc, (struct sockaddr *) &server_addr, &addr_size)) < 0) {
            printf("Accept Command Failed. Error!!!!!\n");
            return -1;
        }
        printf("Connection Accepted\n");
        //child process
        if (fork()== 0) {
   
            memset(client_message,'\0',sizeof(client_message));
            if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
                printf("LogIn userName not received. Error!!!!!\n");
                return -1;
            }
            strcpy(clientUserName,client_message);
            printf("client username : %s\n",clientUserName);
            memset(client_message,'\0',sizeof(client_message));
            if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
                printf("LogIn password not received. Error!!!!!\n");
                return -1;
            }
            strcpy(clientPassword,client_message);
            printf("client password : %s\n", client_message);

            memset(server_message, '\0', sizeof(server_message));
            whiteboard = gettShm(shm_id);

	        struct sembuf asem [1];
	     
            int currid = authenticate(whiteboard,clientUserName,client_message);
            
            if (currid < 0){
				printf("Wrong credentials! \n");
                memset(server_message, '\0', sizeof(server_message));
                strcpy(server_message, "Wrong credentials! Logging out...\n");
                if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
					printf("Error message not sent\n");
					return -1;
				}	
			}
			
            if (currid == 100){
                memset(server_message, '\0', sizeof(server_message));
                strcpy(server_message, "Welcome back administrator  ʕ•́ᴥ•̀ʔっ\n");
                
                if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
					printf("Authentication not Sent. Error!!!!\n");
					return -1;
				}
				
				int signal = 0;
				while (signal != 9){
					memset(client_message,'\0',sizeof(client_message));
					if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
						printf("Option not received. Error!!!!!\n");
						return -1;
					}
					signal = atoi(client_message);
					if (signal != 0)
						printf("Admin selected option %d\n",signal);
							
					if(signal == 1){
						//Approve and publish messages
						Whiteboard * upd_wb = gettShm(shm_id);
						char snum[16];
						int count =0;
						
						asem [0].sem_op = -MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
						
						asem [0].sem_op = -1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
								
						memset(server_message,'\0',sizeof(server_message));
						memset(client_message,'\0',sizeof(client_message));
						for (int i=0; i < upd_wb->currentMessages; i++){
							if (upd_wb->messageList[i].status == 0){
								snprintf(snum, 16, "%d", upd_wb->messageList[i].idMessage);
								strcat(server_message,snum);
								strcat(server_message," ");
								strcat(server_message, upd_wb->messageList[i].messageText);
								strcat(server_message,"\n");
								printf("%s",server_message);
								count++;
								for (int l=0; l<upd_wb->currentThreads; l++){
									if (upd_wb->threadList[l].id == upd_wb->messageList[i].idThread){
										upd_wb->threadList[l].lastMessage.idMessage = upd_wb->messageList[i].idMessage;
										upd_wb->threadList[l].lastMessage.idOwner = upd_wb->messageList[i].idOwner;
										upd_wb->threadList[l].lastMessage.idThread = upd_wb->messageList[i].idThread;
										upd_wb->threadList[l].lastMessage.status= 1;
										strcpy(upd_wb->threadList[l].lastMessage.messageText, upd_wb->messageList[i].messageText);
									}
								}
							}
						}
						
						if(count == 0){
							strcpy(server_message, "No available messages, press a key to return to the menu\n");
							if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
								printf("Authentication not Sent. Error!!!!\n");
								return -1;}
							
							asem[0].sem_op = MAX_READERS;
							if (semop (read_sem, asem, 1) == -1) {
								perror ("semop: sem_signal_malfunction"); exit (1);}
							
							asem[0].sem_op = 1;
							if (semop (write_sem, asem, 1) == -1) {
								perror ("semop: sem_signal_malfunction"); exit (1);}
								
							if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
								printf("Topic Name not received. Error!!!!!\n");
								return -1;}
							continue;
						}
						
						printf("%s\n", server_message);
						if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
							printf("Authentication not Sent. Error!!!!\n");
							return -1;}
						
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Topic Name not received. Error!!!!!\n");
							return -1;}
						
						int n = atoi(client_message);
						
						if (publishMessage(upd_wb, n) == 1)
							strcpy(server_message, "Message published with success!\n\n");
						else
							strcpy(server_message, "Couldn't publish that message, is the id correct?\n\n");
							
						asem[0].sem_op = MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						asem[0].sem_op = 1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						printf("message to send is : %s\n",server_message);
						if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
							printf("Message not Sent. Error!!!!\n");
							return -1;}

						shmdt(upd_wb);
						WriteInShm(shm_id,upd_wb);
					}
					
					else if(signal == 2){
						//delete topic
						Whiteboard * upd_wb = gettShm(shm_id);
						memset(client_message,'\0',sizeof(client_message));
						memset(server_message,'\0',sizeof(server_message));
						
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Topic Name not received. Error!!!!!\n");
							return -1;}
						
						printf("%s\n",client_message);

						asem [0].sem_op = -MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}

						asem [0].sem_op = -1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
						
						int k = deleteTopic(upd_wb, client_message,currid);
						
						asem [0].sem_op = 1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						asem [0].sem_op = MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
						
						if (k == 0){
							strcpy(server_message,"Topic Deleted\n");
							if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
								printf("Authentication not Sent. Error!!!!\n");
								return -1;}
						}
						else{
							strcpy(server_message,"Topic not found!\n");
							if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
								printf("Authentication not Sent. Error!!!!\n");
								return -1;}
						}							
						shmdt(upd_wb);
						WriteInShm(shm_id,upd_wb);
					}
					
					if(signal == 3){
						//delete a thread
						Whiteboard * upd_wb = gettShm(shm_id);
						memset(client_message,'\0',sizeof(client_message));
						memset(server_message,'\0',sizeof(server_message));
						
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Topic Name not received. Error!!!!!\n");
							return -1;}
						
						printf("%s\n",client_message);

						asem [0].sem_op = -MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}

						asem [0].sem_op = -1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);
						}
						
						if(deleteThread(upd_wb, client_message,currid) >=0){
							strcpy(server_message,"Thread Deleted\n");
						}
						else
							strcpy(server_message,"Thread not found!\n");
					
						asem [0].sem_op = 1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						asem [0].sem_op = MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
	
						if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
							printf("Authentication not Sent. Error!!!!\n");
							return -1;
						}
						shmdt(upd_wb);
						WriteInShm(shm_id,upd_wb);
					}
					
					else if(signal == 4){
						//add a user
						char username[24];
						char password[24];
						  
						memset(client_message,'\0',sizeof(client_message));
						memset(server_message,'\0',sizeof(server_message));
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Topic Name not received. Error!!!!!\n");
							return -1;}
						
						strcpy(username,client_message);
						
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Topic Name not received. Error!!!!!\n");
							return -1;}
							
						strcpy(password,client_message);
						
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Topic Name not received. Error!!!!!\n");
							return -1;}
						
						int id = atoi(client_message);

						asem [0].sem_op = -1;
						if (semop (users_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}

						if(addUser(id, username, password) < 0){
							printf("Failed to add user");
							return -1;
						}
						
						if(createRowSubs(id) < 0){
							printf("Failed to create subscriptions for user");
							return -1;
						}
						asem [0].sem_op = 1;
						if (semop (users_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						asem [0].sem_op = MAX_READERS-1;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						strcpy(server_message,"User Added\n");
						if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
							printf("Authentication not Sent. Error!!!!\n");
							return -1;
						}
					}
					
					else if(signal == 5){
						//delete user
						memset(client_message,'\0',sizeof(client_message));
						memset(server_message,'\0',sizeof(server_message));
						
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("User id not received. Error!!!!!\n");
							return -1;
						}
						int idu = atoi(client_message);

						asem [0].sem_op = -1;
						if (semop (users_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
						
						if(deleteUser(idu) == 0)
							strcpy(server_message,"User deleted\n");
						else
							strcpy(server_message,"User not found!\n");
						
						deleteRowSubs(idu);
						
						asem [0].sem_op = 1;
						if (semop (users_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
							printf("Authentication not Sent. Error!!!!\n");
							return -1;
						}
					}
					else if(signal == 6){
						//list users
						asem [0].sem_op = -1;
						if (semop (users_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
						
						memset(server_message,'\0',sizeof(server_message));

						strcat(server_message,showUsers());
						strcat(server_message,"\n");
	
						asem[0].sem_op = 1;
						if (semop (users_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						printf("list users to send: %s\n",server_message);
						if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
							printf("Message not Sent. Error!!!!\n");
							return -1;
						}
					}
				}
            }
            
			// NON ADMIN USER
            else if (currid>0){
				loadSubscriptions(currid);
                printf("User Is logged In\n");
                memset(server_message, '\0', sizeof(server_message));
                strcpy(server_message, "Logged In\n");
                strcat(server_message, "Welcome back to Yellit!\n");
                strcat(server_message, "Don't wait to see the updates on your favourite topics!\nYour favourite topics are: ");
                
                for (int k=0; k<5; k++){
					if (subs[k] != 0){
						strcat(server_message, getTopicNameById(whiteboard, subs[k]));
						strcat(server_message, "  ");	
					}
				}
				strcat(server_message, "\n");

				if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
					printf("Authentication not Sent. Error!!!!\n");
					return -1;
				}
					
				printf("Session Information Sent to client \n");
				
				int signal = 0;
				while (signal != 12){

					//Ask for signal id from user
					memset(client_message,'\0',sizeof(client_message));
					if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
						printf("Option not received. Error!!!!!\n");
						return -1;
					}
					signal = atoi(client_message);
					if (signal != 0)
						printf("client choose option no : %d\n",signal);
							
					if(signal == 1){
						//list topics
						Whiteboard * upd_wb = gettShm(shm_id);
								
						asem [0].sem_op = -1;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}

						memset(server_message,'\0',sizeof(server_message));
						int i;
						for (i = 0; i < upd_wb->currentTopics ; i++){
							printf("%s\n",upd_wb->topicList[i].topicName);
							strcat(server_message,upd_wb->topicList[i].topicName);
							strcat(server_message,"\n");
						}
					
						asem[0].sem_op = 1;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
							printf("Message not Sent. Error!!!!\n");
							return -1;
						}
	                shmdt(upd_wb);
	                WriteInShm(shm_id,upd_wb);
					}
					
					else if (signal == 2){
						//show threads in topic
						Whiteboard * upd_wb = gettShm(shm_id);

						memset(client_message,'\0',sizeof(client_message));
						memset(server_message,'\0',sizeof(server_message));
		
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Topic name not received\n");
							return -1;}
							
						asem [0].sem_op = -1;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
						
						int idt = getIdTopicByName(upd_wb,client_message);
						
						if (checkSub(idt) == 0){	
							for (int i=0; i< whiteboard->currentTopics; i++){
								if (strcmp(client_message, upd_wb->topicList[i].topicName) == 0){
									for (int j=0;  j < whiteboard->currentThreads; j++){
										if (upd_wb->topicList[i].idTopic == upd_wb->threadList[j].idTopic){
											strcat(server_message,upd_wb->threadList[j].threadName);
											strcat(server_message,"\n");	
										}
									}
								}
							}
						}
						else 
							strcat(server_message,"Make you sure you entered the right topic or that you are subscribed to it.\n");	
						
						asem[0].sem_op = 1;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						if (server_message[0]=='\0'){
							strcpy(server_message, "There is no thread available for this topic!\n\n");
							send(accept_fd, server_message, strlen(server_message), 0);
						}
						else if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
							printf("Message not Sent. Error!!!!\n");
							return -1;}
							
	                shmdt(upd_wb);
	                WriteInShm(shm_id,upd_wb);					
					}
				
					else if(signal == 3){
						//adding a topic
						Whiteboard * upd_wb = gettShm(shm_id);

						memset(client_message,'\0',sizeof(client_message));
						memset(server_message,'\0',sizeof(server_message));
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Topic Name not received. Error!!!!!\n");
							return -1;
						}
						
						asem [0].sem_op = -MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}

						asem [0].sem_op = -1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}

						if(addTopic(upd_wb,client_message,currid) != 0){
							printf("Failed to add Topic");
							return -1;
						}

						asem [0].sem_op = 1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						asem [0].sem_op = MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						strcpy(server_message,"Topic Added\n");
						if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
							printf("Authentication not Sent. Error!!!!\n");
							return -1;
						}
						shmdt(upd_wb);
						WriteInShm(shm_id,upd_wb);
					}
									
					else if(signal == 4){
						//delete topic
						Whiteboard * upd_wb = gettShm(shm_id);
						memset(client_message,'\0',sizeof(client_message));
						memset(server_message,'\0',sizeof(server_message));
						
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Topic Name not received. Error!!!!!\n");
							return -1;
						}
						
						printf("%s\n",client_message);

						asem [0].sem_op = -MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}

						asem [0].sem_op = -1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
						
						if(deleteTopic(upd_wb, client_message,currid) == 0)
							strcpy(server_message,"Topic Deleted");
					
						else
							strcpy(server_message,"Failed to delete topic! Are you the owner?");
						
						asem [0].sem_op = 1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						asem [0].sem_op = MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
	
						if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
							printf("Authentication not Sent. Error!!!!\n");
							return -1;
						}
						shmdt(upd_wb);
						WriteInShm(shm_id,upd_wb);
					}
					
					else if(signal == 5){
						//adding a thread
						Whiteboard * upd_wb = gettShm(shm_id);

						memset(client_message,'\0',sizeof(client_message));
						memset(server_message,'\0',sizeof(server_message));
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Topic Name not received. Error!!!!!\n");
							return -1;}
						
						asem [0].sem_op = -MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}

						asem [0].sem_op = -1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
						
						int idTopic = getIdTopicByName(upd_wb, client_message);
						
						memset(client_message,'\0',sizeof(client_message));
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Thread Name not received. Error!!!!!\n");
							return -1;
						}
				
						if(addThread(upd_wb,client_message,currid, idTopic) < 0){
							strcpy(server_message,"Failed to add Thread");
							return -1;}
						else 
							strcpy(server_message,"Thread Added\n");
						
						asem [0].sem_op = 1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						asem [0].sem_op = MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}

						if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
							printf("Authentication not Sent. Error!!!!\n");
							return -1;
						}
						shmdt(upd_wb);
						WriteInShm(shm_id,upd_wb);
					}
						
					else if(signal == 6){
						//delete a thread
						Whiteboard * upd_wb = gettShm(shm_id);
						memset(client_message,'\0',sizeof(client_message));
						memset(server_message,'\0',sizeof(server_message));
						
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Topic Name not received. Error!!!!!\n");
							return -1;}
						
						printf("%s\n",client_message);
						
						char ch = '\n';
						strncat(client_message, &ch, 1);
					
						asem [0].sem_op = -MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}

						asem [0].sem_op = -1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);
						}
						
						if(deleteThread(upd_wb, client_message,currid) < 0)
							strcpy(server_message,"Couldn't delete the thread. Are you the owner?\n");
						else
							strcpy(server_message,"Thread Deleted\n");					
					
						asem [0].sem_op = 1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						asem [0].sem_op = MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
	
						if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
							printf("Authentication not Sent. Error!!!!\n");
							return -1;
						}
						shmdt(upd_wb);
						WriteInShm(shm_id,upd_wb);
					}
					
					else if (signal == 7){
						//show messages in thread
						Whiteboard * upd_wb = gettShm(shm_id);

						memset(client_message,'\0',sizeof(client_message));
						memset(server_message,'\0',sizeof(server_message));	
						asem [0].sem_op = -1;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}

						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Thread name not received\n");
							return -1;}
						
						int index;
						for (int i=0; i< upd_wb->currentThreads; i++){
							if (strcmp(client_message, upd_wb->threadList[i].threadName) == 0){
								index = i;
								for (int j=0;  j < upd_wb->currentMessages; j++){
									if (upd_wb->threadList[i].id == upd_wb->messageList[j].idThread && upd_wb->messageList[j].status == 1){
										strcat(server_message,upd_wb->messageList[j].messageText);
										//strcat(server_message,"\n");
										char* name = getuserById(currid);
										strcat(server_message,"-written by ");
										strcat(server_message,name);
										strcat(server_message,"\n\n");
										free(name);
									}
								}
							}
						}
						asem[0].sem_op = 1;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						if (server_message[0]=='\0'){
							strcpy(server_message, "There is no message available for this thread!\n\n");
							send(accept_fd, server_message, strlen(server_message), 0);
						}
						else if (checkSub(upd_wb->threadList[index].idTopic) <0){
							strcpy(server_message, "Messages unavailable! You are not subscribed to this topic!\n\n");
							send(accept_fd, server_message, strlen(server_message), 0);	
						}
						else
							send(accept_fd, server_message, strlen(server_message), 0);
											
	                shmdt(upd_wb);
	                WriteInShm(shm_id,upd_wb);					
					}
					
					else if(signal == 8){
						//submit a message
						Whiteboard * upd_wb = gettShm(shm_id);
						memset(client_message,'\0',sizeof(client_message));
						memset(server_message,'\0',sizeof(server_message));
						
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Topic Name not received. Error!!!!!\n");
							return -1;}

						asem [0].sem_op = -MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}

						asem [0].sem_op = -1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
						
						int idThread = getIdThreadByName(upd_wb, client_message);
						
						memset(client_message,'\0',sizeof(client_message));
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Thread Name not received. Error!!!!!\n");
							return -1;}
				
						if(addMessage(upd_wb,client_message,currid, idThread) < 0){
							printf("Failed to add Message");
							return -1;
						}
						
						asem [0].sem_op = 1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						asem [0].sem_op = MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}

						strcpy(server_message,"Message submitted, waiting for admin approvation\n");
						if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
							printf("Authentication not Sent. Error!!!!\n");
							return -1;}
							
						// detaching shm
						shmdt(upd_wb);
						WriteInShm(shm_id,upd_wb);
					}
					else if(signal == 9){
						//subscribe/unsubscribe
						Whiteboard * upd_wb = gettShm(shm_id);

						memset(client_message,'\0',sizeof(client_message));
						memset(server_message,'\0',sizeof(server_message));
						if (recv(accept_fd, client_message, sizeof(client_message), 0) < 0) {
							printf("Topic Name not received. Error!!!!!\n");
							return -1;}
						
						asem [0].sem_op = -MAX_READERS;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}

						asem [0].sem_op = -1;
						if (semop (write_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}

						int idt = getIdTopicByName(upd_wb, client_message);
						if (idt < 0){
							asem [0].sem_op = 1;
							if (semop (write_sem, asem, 1) == -1) {
								perror ("semop: sem_signal_malfunction"); exit (1);}
							
							asem [0].sem_op = MAX_READERS;
							if (semop (read_sem, asem, 1) == -1) {
								perror ("semop: sem_signal_malfunction"); exit (1);}
							
							strcpy(server_message,"Unexistent topic\n");
							
							if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
								printf("Authentication not Sent. Error!!!!\n");
								return -1;}	
						}
						else{
							subscribeToTopic(currid, idt);
							int ret = updateSubscriptions(currid);

							asem [0].sem_op = 1;
							if (semop (write_sem, asem, 1) == -1) {
								perror ("semop: sem_signal_malfunction"); exit (1);}
								
							asem [0].sem_op = MAX_READERS;
							if (semop (read_sem, asem, 1) == -1) {
								perror ("semop: sem_signal_malfunction"); exit (1);}
							
							if (ret == 0)	
								strcpy(server_message,"Operation completed!\n");
							else
								strcpy(server_message,"Operation not permitted! Maybe you already are subscribed to 5 topics!\n");
								
							if (send(accept_fd, server_message, strlen(server_message), 0) < 0) {
								printf("Authentication not Sent. Error!!!!\n");
								return -1;}
						}
						shmdt(upd_wb);
						WriteInShm(shm_id,upd_wb);
					}
					else if (signal == 10){
						//show recent messages
						Whiteboard * upd_wb = gettShm(shm_id);

						memset(client_message,'\0',sizeof(client_message));
						memset(server_message,'\0',sizeof(server_message));	
						
						asem [0].sem_op = -1;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
					
						for (int k = 0; k<upd_wb->currentThreads; k++){
							if (checkSub(upd_wb->threadList[k].idTopic) == 0){
								strcpy(server_message, "User ");
								strcat(server_message, getuserById(upd_wb->threadList[k].lastMessage.idOwner));
								strcat(server_message, "replied: ");
								strcat(server_message, upd_wb->threadList[k].lastMessage.messageText);
								strcat(server_message, "in thread: ");
								strcat(server_message, getThreadNameById(upd_wb, upd_wb->threadList[k].lastMessage.idThread));
								strcat(server_message, ".\n\n");
							}							
						}				
						asem[0].sem_op = 1;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
						
						if (strlen(server_message)<32){
							strcpy(server_message, "There are no new messages for your topics.\n");
							send(accept_fd, server_message, strlen(server_message), 0);
						}
						else 
							send(accept_fd, server_message, strlen(server_message), 0);
					
						shmdt(upd_wb);
						WriteInShm(shm_id,upd_wb);					
					}
					else if (signal == 11){
						//show your message history
						Whiteboard * upd_wb = gettShm(shm_id);

						memset(client_message,'\0',sizeof(client_message));
						memset(server_message,'\0',sizeof(server_message));	
						
						asem [0].sem_op = -1;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}
							
						strcpy(server_message, "Your Messages:\n");
						for (int i = 0; i<upd_wb->currentMessages; i++){
							if (upd_wb->messageList[i].idOwner == currid){
								strcat(server_message, "  You wrote:  ");
								strcat(server_message, upd_wb->messageList[i].messageText);
								strcat(server_message, "  in thread:  ");
								strcat(server_message, getThreadNameById(upd_wb, upd_wb->messageList[i].idThread));
								if (upd_wb->messageList[i].status == 1)
									strcat(server_message, "\n  Message published.\n\n");
								else
									strcat(server_message, "\n  Message pending.\n\n");
							}
						}
									
						asem[0].sem_op = 1;
						if (semop (read_sem, asem, 1) == -1) {
							perror ("semop: sem_signal_malfunction"); exit (1);}						
					
						if (strcmp("Your Messages:\n",server_message) == 0){
							strcpy(server_message, "You haven't wrote any message yet.\n");
							send(accept_fd, server_message, strlen(server_message), 0);}
						
						else 
							send(accept_fd, server_message, strlen(server_message), 0);
		
						shmdt(upd_wb);
						WriteInShm(shm_id,upd_wb);					
					}
				}
            }               
        } 
        else {
        }
    }

	memset(server_message,'\0',sizeof(server_message));
	memset(client_message,'\0',sizeof(client_message));

	// remove semaphores
	if (semctl (read_sem, 0, IPC_RMID) == -1) {
		perror ("semctl IPC_RMID"); exit (1);
	}
	if (semctl (write_sem, 0, IPC_RMID) == -1) {
		perror ("semctl IPC_RMID"); exit (1);
	}
	if (semctl (users_sem, 0, IPC_RMID) == -1) {
		perror ("semctl IPC_RMID"); exit (1);
	}

	//Closing the Socket
	close(client_sock);
	close(socket_desc);
	return 0;    
}
