#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>  
#include <netinet/in.h> 
#include <sys/socket.h>
#include <semaphore.h>
#include <fcntl.h>
#include "type_interface.h"
#include "utilities_interface.h"
#include "list_interface.h"
#include "hashTable_interface.h"
#include "server_interface.h"
#include "common.h"
#include "color.h"

//global stuff
struct sockaddr_in* client_addr=NULL;
msg_box_t*shmem=NULL;
int u,v;
int listening_socket;
sem_t mutex_shmemOp;//mutex shmem + exit only if cs is free
struct timespec abs_timeout;

list_t*userOnline=NULL;

sem_t mutex_onLine;

void generic_handler(int signal)
{
	
	int ret;
	printf("Server shut down... \n");
	printf("Wait for thread in critical-section...\n ");;
	
	ret=sem_timedwait(&mutex_shmemOp, &abs_timeout);
	ERROR_HELPER(ret,"error wait sem");;
	//sem_wait(&mutex_shmemOp);//continue <=> cs is free
	printf("Update file...\n ");
	fdatasync(v);;
	fdatasync(u);
	printf("close file and list socket...\n ");
	close(listening_socket);
	close(u);//close users
	close(v);//close msgbox
	printf("Free memory...\n ");
	free(client_addr);
	list_freeString(userOnline);
	msgBox_free(shmem);//free msgbox
	ret=sem_destroy(&mutex_shmemOp);
	ERROR_HELPER(ret,"error destroy sem");
	ret=sem_destroy(&mutex_onLine);
	ERROR_HELPER(ret,"error destroy sem");
	
	//printf("Complete!");
	fflush(0);
	exit(EXIT_SUCCESS);
}




void* connection_handler(void* arg) 
{	

    handler_args_t* args = (handler_args_t*)arg;
    int socket_desc = args->socket_desc;
    struct sockaddr_in* client_addr = args->client_addr;
    int ret;
    char out[1024];
   // char*in;
	char in[1024];memset(in,'\0',1024);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    uint16_t client_port = ntohs(client_addr->sin_port); 
	printf("Connection established!\n");fflush(0);
    user_t*current=NULL;
    sprintf(out, "Hi! I'm a Mail server. You are %s talking on port %hu.\n", client_ip, client_port);
   
	
    ret=mySendThread(args->socket_desc,out,strlen(out));
	brpipe_server(ret,socket_desc,NULL);
	while(1){
		printf("Waiting for command...\n");fflush(0);
		
		//in=myRecv(args->socket_desc);
		
		ret=myRecvNewThread(socket_desc,in);
		if (current!=NULL)offlineClient(ret,socket_desc,current->name);
		else offlineClient(ret,socket_desc,NULL);
		printf("Received:<%s>\n",in);fflush(0);
		if (strlen(in)==0||!strcmp(in,"abort\n")){
			printf("abort");fflush(0);

			break;
			
			
		}
		in[strlen(in)-1]='\0';
		if (!strcmp(in,BACK)){
			printf("removing user...");
				int ec=sem_wait(&mutex_onLine);
				ERROR_HELPERT(ec,"error sem");
				list_removeString(userOnline,current->name);
				ec=sem_post(&mutex_onLine);
				ERROR_HELPERT(ec,"error sem");
			printf("done!\n");
			
		}		

		
		if (!strcmp(in,SIGN_UP)){
			//free(in);
			server_signup(socket_desc,shmem,u);
		}
		else if (!strcmp(in,LOG_IN)){
			//free(in);
			current=server_login(socket_desc,shmem,u);
		}
		if (!strcmp(in,ONLINE)){
			server_online(socket_desc,*current);
		}
		else if (!strcmp(in,VIEW)){
			//free(in);
			server_view(socket_desc,*current);
		}
		else if (!strcmp(in,SEND)){
			//free(in);
			//void server_createmess(int socket_desc,user_t *u,int msg_box,msg_box_t*shmem)
			server_createmess(socket_desc,current,v,shmem);
		}
		else if (!strcmp(in,READ)){
			//free(in);
			server_readmess(socket_desc,current);
		}
		else if (!strcmp(in,REMOVE)){
			//free(in);
			char rin[1024];memset(rin,'\0',1024);
			ret=myRecvNewThread(socket_desc,rin);
			//char*rin=myRecv(socket_desc);
			offlineClient(ret,socket_desc,current->name);
			
			if(!strcmp(rin,"back\n")){
				printf("abort");
			}
			
			if(strcmp(rin,"S\n") && strcmp(rin,"R\n")) {
				
				ret=mySendThread(socket_desc,"Failure\n",strlen("Failure\n"));
				brpipe_server(ret,socket_desc,current->name);
			}
		
	
			else if (!strcmp(rin,"S\n")){
				//void server_removeRec(int socket_desc,user_t*current,msg_box_t*shmem,int msg_box)
				server_removeSen(socket_desc,current,shmem,v);
			}
			else if (!strcmp(rin,"R\n")){
				server_removeRec(socket_desc,current,shmem,v);
			}
			
		}
		//else// free(in);
	}
	
    ret = close(socket_desc);
    ERROR_HELPERT(ret, "Cannot close socket for incoming connection");
	
	if (current!=NULL){
		sem_wait(&mutex_onLine);
		list_removeString(userOnline,current->name);
		sem_post(&mutex_onLine);
	
	}
    
    free(args->client_addr); 
    free(args);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
	//removeDir("msglog");return 0;
	if (argc==2){
		if (!strcmp(argv[1],SETUP)){
			printf("Auto-configure\n\n");
			configure();
		}else if (!strcmp(argv[1],NO_SETUP)){
			printf("The server will use the last configuration \n\n");
			
		}else {
			printf("\nCommand not recognized\nUsage: ./server (setup|nosetup)\nabort\n");return 0;
		}
	}else {
		printf("\nUsage: ./server (setup|nosetup)\nabort\n");return 0;
	}
	
	
	
	//event handler
	sigset_t set;
	struct sigaction act={0};
	sigfillset(&set);
	
	act.sa_handler= generic_handler; 
	act.sa_mask =  set;
	act.sa_flags=0;
	sigaction(SIGINT,&act,NULL);
	
	
	
	int ret;

    //connection setup
    
	int socket_desc, client_desc;
    struct sockaddr_in server_addr = {0};
    int sockaddr_len = sizeof(struct sockaddr_in); 
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    ERROR_HELPER(socket_desc, "Could not create socket");
    
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT); 
    ret = bind(socket_desc, (struct sockaddr*) &server_addr, sockaddr_len);

    ERROR_HELPER(ret, "Cannot bind address to socket");
    ret = listen(socket_desc, MAX_CONN_QUEUE);
    ERROR_HELPER(ret, "Cannot listen on socket");
    
    listening_socket=socket_desc;
	//memory+sem seutp
	printf("\nDoing the household chores...\n\n");
	 u=open("users",O_RDWR,0666);
	ERROR_HELPER(u,"Error opening");
	 v=open("msg_box",O_RDWR,0666);
	ERROR_HELPER(v,"Error opening");
	
	shmem=msgBox_get(u,v);
	printf("Message box created!\n");
	ret=sem_init(&mutex_shmemOp,0, 1);
	ERROR_HELPER(ret,"error opening sem");
	ret=sem_init(&mutex_onLine,0, 1);
	ERROR_HELPER(ret,"error opening sem");
	printf("Semaphores init!\n");
	abs_timeout.tv_sec=100;
    abs_timeout.tv_nsec=0;
	printf("Deadlock-timeout set to 100sec \n");
	printf("stand-by-timeout set to 1000sec \n");
	userOnline=list_new();
	printf("onLine list init!\n");
	printf("Done!\n");
	fflush(0);
   
    //sock option
    int reuseaddr_opt = 1;
    ret = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));
    ERROR_HELPER(ret, "Cannot set SO_REUSEADDR option");
	struct timeval timeout;
	timeout.tv_sec = 1000;
	timeout.tv_usec = 0;
	
	
	


     client_addr = calloc(1, sizeof(struct sockaddr_in));
	//struct sockaddr_in* client_addr = calloc(1, sizeof(struct sockaddr_in));
    
    
    while (1) {
        client_desc = accept(socket_desc, (struct sockaddr*) client_addr, (socklen_t*) &sockaddr_len);
        
        if (client_desc == -1 && errno == EINTR) continue; // check for interruption by signals
       
        if (client_desc==-1){
			close(u);//close users
			close(v);//close msgbox
			printf("Free memory...\n ");
			free(client_addr);
			list_freeString(userOnline);
			msgBox_free(shmem);//free msgbox
			ret=sem_destroy(&mutex_shmemOp);
			ERROR_HELPERT(ret,"error destroy sem");
			ret=sem_destroy(&mutex_onLine);
			ERROR_HELPERT(ret,"error destroy sem");
			ERROR_HELPER(client_desc, "Cannot open socket for incoming connection");
		}
        setsockopt(client_desc, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        pthread_t thread;
        handler_args_t* thread_args = malloc(sizeof(handler_args_t));
       
        thread_args->socket_desc = client_desc;
        thread_args->client_addr = client_addr;

		
        if (pthread_create(&thread, NULL, connection_handler, (void*)thread_args) != 0) {
            fprintf(stderr, "Can't create a new thread, error %d\n", errno);
            exit(EXIT_FAILURE);
        }
        pthread_detach(thread);
        client_addr = calloc(1, sizeof(struct sockaddr_in));
        //bzero(client_addr,sizeof(struct sockaddr_in));
        
    }

  
}
