#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons() and inet_addr()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>
#include "type_interface.h"
#include "client_interface.h"
#include "common.h"
#include "color.h"

int sockDesc;//Global var for event handler

void generic_handler(int signal)
{
	int ret;
	printf("Client shut down... \n");
	ret=close(sockDesc);
	ERROR_HELPER(ret,"error closign socket");
	fflush(0);
	exit(EXIT_SUCCESS);
}



int main(int argc, char* argv[]) {
    int ret;
	Bool b;
	
	//event handler
	sigset_t set;
	struct sigaction act={0};
	sigfillset(&set);
	
	act.sa_handler= generic_handler; 
	act.sa_mask =  set;
	act.sa_flags=0;
	sigaction(SIGINT,&act,NULL);

	
    // variables for handling a socket
    int socket_desc;
    struct sockaddr_in server_addr = {0}; // some fields are required to be filled with 0

    // create a socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_HELPER(socket_desc, "Could not create socket");
	sockDesc=socket_desc;
	
    // set up parameters for the connection
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT);
	
	
	
    // initiate a connection on the socket
    ret = connect(socket_desc, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
    ERROR_HELPER(ret, "Could not create connection");

   // char*com=myRecv(socket_desc);
   char com[1024];memset(com,0,1024);
   memset(com,0,1024);
   ret=myRecvNew(socket_desc,com);
   
    offlineServer(ret,socket_desc);
    char buf[1024];memset(buf,0,1024);
    printf(ANSI_COLOR_GREEN"\nServer message:%s "ANSI_COLOR_RESET,com);
   // free(com);
    int flag_log=0,flag_on=1;
    while(flag_on){
		while(1){
			printf("\nTo sign-up type: <sign>\nTo log-in type: <log>\nTo exit type <abort>\n");
			fgets(buf,1024,stdin);
			buf[strlen(buf)-1]='\0';
			
			if (!strcmp(buf,ABORT)){
				ret=mySend(socket_desc,"abort\n",strlen("abort\n"));
				brpipe_client( ret, socket_desc);
				flag_on=0;
				break;
				
			}			
			
			if (!strcmp(buf,SIGN_UP)){
				sign_up(socket_desc,buf);
				
			}
		   else  if (!strcmp(buf,LOG_IN)){
				if ( !log_in(socket_desc,buf) ) continue;
				flag_log=1;
				break;
				
			}
		}
		while (flag_log){
			printf("\nTo check online user type <online>\nTo view your mail-box type <view>\nTo read a message type <read>\nTo send a message type <send>\nTo remove a message type <remove>\nOtherwise type:<back>\n");
			fgets(buf,1024,stdin);
			buf[strlen(buf)-1]='\0';
			
			if (!strcmp(buf,BACK)){
				ret=mySend(socket_desc,"back\n",strlen("back\n"));
				brpipe_client(ret,sockDesc);
				flag_log=0;
				break;
			}
			if (!strcmp(buf,ONLINE)){
				online(socket_desc,buf);
			}
			
			if (!strcmp(buf,VIEW)){
				
				view(socket_desc,buf);
			}
			if (!strcmp(buf,READ)){
				readmess(socket_desc,buf);
				
			}		
			if (!strcmp(buf,SEND)){
				sendmess(socket_desc,buf);
			}	
			if (!strcmp(buf,REMOVE)){
				removemess(socket_desc,buf);
			}	
		
		}
	}
	
    // close the socket
    printf("\nclosing\n");
    ret = close(socket_desc);
    ERROR_HELPER(ret, "Cannot close socket");
    exit(EXIT_SUCCESS);
}
