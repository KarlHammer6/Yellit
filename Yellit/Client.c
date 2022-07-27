#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(void){
	int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000], client_message[2000];
        
    //Setting the Buffers to \0 
    memset(server_message,'\0',sizeof(server_message));
    memset(client_message,'\0',sizeof(client_message));
        
    //Socket creation     
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
        
    if(socket_desc < 0){
		printf("Error in creating socket\n");
		return -1;}
        
	printf("Socket Created\n");
        
	//Specifying the IP and Port of the server to connect    
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(50000);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
	//Connect to the server   
	if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		printf("Connection Failed. Error!!!!!");
		return -1;
	}
        
	printf("Welcome to Yellit!\n");
        
	printf("Enter UserName: ");
                                        
	fgets(client_message, 24, stdin);
	if (client_message[strlen(client_message) - 1] != '\n') {
		fprintf(stderr, "Input too long, disconnecting ...\n");
		return -1;}
	client_message[strcspn(client_message, "\n")] = 0;
        
	if(send(socket_desc, client_message, strlen(client_message),0) < 0){
		printf("UserName not sent. Error!!!!\n");
		return -1;}

	memset(client_message,'\0',sizeof(client_message));
	printf("Enter Password: ");
	fgets(client_message, 24, stdin);
	if (client_message[strlen(client_message) - 1] != '\n') {
		fprintf(stderr, "Input too long, disconnecting ...\n");
		return -1;}
		
	client_message[strcspn(client_message, "\n")] = 0;
	if(send(socket_desc, client_message, strlen(client_message),0) < 0){
		printf("Password not sent. Error!!!!\n");
		return -1;}
        
	if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
		printf("Receive Failed. Error!!!!!\n");
		return -1;}
        
	printf("\nServer Message: %s\n",server_message);

	if (strcmp(server_message, "Wrong credentials! Logging out...\n") ==0)
		return -1;
	
	//ADMIN
	else if (strcmp(server_message, "Welcome back administrator  ʕ•́ᴥ•̀ʔっ\n") ==0){
		int signal = 0;
        while(signal != 10){
            printf("Enter the number of any option you want to choose \n 1-Approve messages\n 2-Delete topic\n 3-Delete thread\n 4-Add user\n 5-Delete user\n 6-Show users\n 10-Exit\n->");
            memset(client_message,'\0',sizeof(client_message));
            
            fgets(client_message, 24, stdin);
			client_message[strcspn(client_message, "\n")] = 0;
            signal = atoi(client_message);
            if (signal > 10)
				continue;
            
			if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                printf("option not sent. Error!!!!\n");
                return -1;}
            
			if (signal == 1){
                //approve messages
                memset(client_message,'\0',sizeof(client_message));
	            memset(server_message,'\0',sizeof(server_message));
	            if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
		            printf("Receive Failed. Error!!!!!\n");
		            return -1;}
		            
		        if(strcmp(server_message,"No available messages, press a key to return to the menu\n") == 0){
					printf("%s\n",server_message);
					fgets(client_message, 24, stdin);
					if(send(socket_desc, client_message, strlen(client_message),0) < 0){
						printf("topic name not sent. Error!!!!\n");
						return -1;}
                    continue;
				}
					
		            
	            printf("%s\n",server_message);
	            
	            printf("Insert the id of the message to publish, otherwise press 0\n");
	            
	            fgets(client_message, 24, stdin);
	            if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
	            client_message[strcspn(client_message, "\n")] = 0;
	            if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                    printf("topic name not sent. Error!!!!\n");
                    return -1;}
	            
	            memset(server_message,'\0',sizeof(server_message));
	            if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
		            printf("Receive Failed. Error!!!!!\n");
		            return -1;}
		        printf("%s", server_message);
            }
            
            else if(signal == 2){
				//Delete topic
                memset(client_message,'\0',sizeof(client_message));
                printf("Enter the name of topic you want to delete : ");
                fgets(client_message, 24, stdin);
                if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
					
				client_message[strcspn(client_message, "\n")] = 0;
                if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                    printf("topic name not sent. Error!!!!\n");
                    return -1;}
                    
                memset(server_message,'\0',sizeof(server_message));
                if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
                    printf("Receive Failed. Error!\n");
                    return -1;}
                    
                printf("server replied : %s\n",server_message);
            }
            else if(signal == 3){
				 //delete a thread
                memset(client_message,'\0',sizeof(client_message));
                printf("Enter the name of the thread you want to delete : ");
                fgets(client_message, 64, stdin);
                if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
				
                if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                    printf("thread name not sent. Error!!!!\n");
                    return -1;}
                    
                memset(server_message,'\0',sizeof(server_message));
                if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
                    printf("Receive Failed. Error!\n");
                    return -1;}
                    
                printf("server replied : %s\n",server_message);
            }
            
            else if (signal == 4){
				//add user
	         
                memset(client_message,'\0',sizeof(client_message));
                printf("type the new username : ");
                fgets(client_message, 24, stdin);
                if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
				client_message[strcspn(client_message, "\n")] = 0;
                if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                    printf("Username name not sent. Error!\n");
                    return -1;}
                
                printf("type the new password : ");
                fgets(client_message, 24, stdin);
                if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
				client_message[strcspn(client_message, "\n")] = 0;
                if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                    printf("Password name not sent. Error!\n");
                    return -1;}
                    
                printf("type the id for the user (be careful, id must be unique and id = 100 is reserved for administrators!!!) : ");
                fgets(client_message, 24, stdin);
                if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
				client_message[strcspn(client_message, "\n")] = 0;
                if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                    printf("Id not sent. Error!\n");
                    return -1;}
                    
                memset(server_message,'\0',sizeof(server_message));
                if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
                    printf("Receive Failed. Error!!!!!\n");
                    return -1;}
                    
                printf("server replied : %s\n",server_message);
            }
            
            else if(signal == 5){
				//delete user
                memset(client_message,'\0',sizeof(client_message));
                printf("Enter the id of the user you want to delete : ");
                fgets(client_message, 24, stdin);
                if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
				client_message[strcspn(client_message, "\n")] = 0;
                if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                    printf("user id not sent. Error!!!!\n");
                    return -1;}
                    
                memset(server_message,'\0',sizeof(server_message));
                if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
                    printf("Receive Failed. Error!!!!!\n");
                    return -1;}
                    
                printf("server replied : %s\n",server_message);
            }
            else if (signal == 6){
                //list users
	            memset(server_message,'\0',sizeof(server_message));
	            if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
		            printf("Receive Failed. Error!!!!!\n");
		            return -1;}
		            
	            printf("%s\n",server_message);
            }
                        
		}
	}

	else {		
        int signal = 0;
        while(signal != 12){
            printf("Enter the number of any option you want to choose \n 0-nothing\n 1-Topic list\n 2-Show threads in a topic\n 3-Create new topic\n 4-Delete topic (if yours!!!)\n 5-Add new thread in a topic\n 6-Delete thread\n 7-Show messages in thread\n 8-Submit message in thread\n 9-Subscribe/unsubscribe from a topic\n 10-Recent Messages from your favourite topics\n 11-Your message history\n 12-Exit\n->");
            memset(client_message,'\0',sizeof(client_message));
            fgets(client_message, 24, stdin);
            if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
			client_message[strcspn(client_message, "\n")] = 0;
            signal = atoi(client_message);

            if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                printf("option not sent. Error!!!!\n");
                return -1;}

            if (signal == 1){
                //list topics
                memset(server_message,'\0',sizeof(server_message));
	            memset(server_message,'\0',sizeof(server_message));
	            if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
		            printf("Receive Failed. Error!!!!!\n");
		            return -1;}
		            
	            printf("%s\n",server_message);
            }
            
            else if (signal == 2){
				//show threads in a topic
				memset(server_message,'\0',sizeof(server_message));
	            memset(server_message,'\0',sizeof(server_message));
	            
	            printf("Please, insert the name of the topic you want to explore\n");
	            fgets(client_message, 24, stdin);
	            if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
				client_message[strcspn(client_message, "\n")] = 0;
				if(send(socket_desc, client_message, strlen(client_message),0) < 0){
					printf("Topic not sent!\n");
					return -1;}
	
				if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
		            printf("Receive Failed. Error!!!!!\n");
		            return -1;}
	   
	            printf("%s\n",server_message);	
			}
			
			else if (signal == 3){
				//add topic
	         
                memset(client_message,'\0',sizeof(client_message));
                printf("type the name of new Topic : ");
                fgets(client_message, 24, stdin);
                if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
				client_message[strcspn(client_message, "\n")] = 0;
                if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                    printf("topic name not sent. Error!!!!\n");
                    return -1;}
                    
                memset(server_message,'\0',sizeof(server_message));
                if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
                    printf("Receive Failed. Error!!!!!\n");
                    return -1;}
                    
                printf("server replied : %s\n",server_message);
            }

            else if(signal == 4){
				//delete topic
                memset(client_message,'\0',sizeof(client_message));
                printf("Enter the name of topic you want to delete : ");
                fgets(client_message, 24, stdin);
                if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
				client_message[strcspn(client_message, "\n")] = 0;
                if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                    printf("topic name not sent. Error!!!!\n");
                    return -1;}
                    
                memset(server_message,'\0',sizeof(server_message));
                if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
                    printf("Receive Failed. Error!!!!!\n");
                    return -1;}
                    
                printf("server replied : %s\n",server_message);
            }
            
            else if (signal == 5){
				//add thread
                memset(client_message,'\0',sizeof(client_message));
                printf("type the name of the topic in which you want to create your thread : ");
                fgets(client_message, 24, stdin);
                if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
				client_message[strcspn(client_message, "\n")] = 0;
                if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                    printf("topic name not sent. Error!!!!\n");
                    return -1;}
                
                memset(client_message,'\0',sizeof(client_message));
                printf("type the name of your thread : ");
                fgets(client_message, 64, stdin);
                if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
				client_message[strcspn(client_message, "\n")] = 0;
                if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                    printf("topic name not sent. Error!!!!\n");
                    return -1;}
                    
                memset(server_message,'\0',sizeof(server_message));
                if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
                    printf("Receive Failed. Error!!!!!\n");
                    return -1;}
                    
                printf("server replied : %s\n",server_message);
            }
      
             else if(signal == 6){
				 //delete a thread
                memset(client_message,'\0',sizeof(client_message));
                printf("Enter the name of the thread you want to delete : ");
                fgets(client_message, 64, stdin);
                if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
				client_message[strcspn(client_message, "\n")] = 0;
                if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                    printf("topic name not sent. Error!!!!\n");
                    return -1;}
                    
                memset(server_message,'\0',sizeof(server_message));
                if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
                    printf("Receive Failed. Error!!!!!\n");
                    return -1;}
                    
                printf("server replied : %s\n",server_message);
            }
            
            else if (signal == 7){
				//see messages of a thread
				memset(client_message,'\0',sizeof(client_message));
	            memset(server_message,'\0',sizeof(server_message));
	            
	            printf("Please, insert the name of the thread you want to see the message of\n");
	            fgets(client_message, 64, stdin);
	            if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
				if(send(socket_desc, client_message, strlen(client_message),0) < 0){
					printf("Topic not sent!\n");
					return -1;}
	
				if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
		            printf("Receive Failed. Error!!!!!\n");
		            return -1;}
	   
	            printf("%s\n",server_message);
			}
	            
	        else if (signal == 8){
				//add a message on a thread
	         
                memset(client_message,'\0',sizeof(client_message));
                printf("type the name of the thread in which you want to submit your message : ");
                fgets(client_message, 64, stdin);
                if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
				//client_message[strcspn(client_message, "\n")] = 0;
                if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                    printf("topic name not sent. Error!!!!\n");
                    return -1;}
       
                memset(client_message,'\0',sizeof(client_message));
                printf("type your message : ");
                fgets(client_message, 128, stdin);
                if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
				client_message[strcspn(client_message, "\n")] = 0;
                if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                    printf("topic name not sent. Error!!!!\n");
                    return -1;}
                    
                memset(server_message,'\0',sizeof(server_message));
                if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
                    printf("Receive Failed. Error!!!!!\n");
                    return -1;}
                    
                printf("server replied : %s\n",server_message);
            }
            else if (signal == 9){
				//subscribe/unsubscribe
                memset(client_message,'\0',sizeof(client_message));
                printf("Type the name of the Topic you want subscribe/unsubscribe to (remember you can subscribe only to 5 topics) : ");
                fgets(client_message, 24, stdin);
                if (client_message[strlen(client_message) - 1] != '\n') {
					fprintf(stderr, "Input too long, disconnecting ...\n");
					return -1;}
				client_message[strcspn(client_message, "\n")] = 0;
                if(send(socket_desc, client_message, strlen(client_message),0) < 0){
                    printf("topic name not sent. Error!!!!\n");
                    return -1;}
                    
                memset(server_message,'\0',sizeof(server_message));
                if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
                    printf("Receive Failed. Error!!!!!\n");
                    return -1;}
                    
                printf("server replied : %s\n",server_message);
            }
            else if (signal == 10){
				//see recent messages
				memset(client_message,'\0',sizeof(client_message));
	            memset(server_message,'\0',sizeof(server_message));
	            
	            // printf("Please, insert the name of the thread you want to see the message of\n");
				if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
		            printf("Receive Failed. Error!!!!!\n");
		            return -1;}
	   
	            printf("%s\n",server_message);
			}
			else if (signal == 11){
				//show message history
				memset(client_message,'\0',sizeof(client_message));
	            memset(server_message,'\0',sizeof(server_message));
				if(recv(socket_desc, server_message, sizeof(server_message),0) < 0){
		            printf("Receive Failed. Error!!!!!\n");
		            return -1;}
	   
	            printf("%s\n",server_message);
			}
        }
	}	
	memset(server_message,'\0',sizeof(server_message));
	memset(client_message,'\0',sizeof(client_message));
		
	//Closing the Socket
	close(socket_desc);
		
	return 0;
}

