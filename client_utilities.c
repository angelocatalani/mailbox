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
int mySend(int fd, char*buf, int len)
{
	int len2=len;
	int ret=0,sent=0;
	while(len>0){
		 ret=send(fd,buf+sent,len,MSG_NOSIGNAL);
		//if (ret==0) return 0;
		 if (ret==-1){
			 if(errno==EINTR) continue;
			 if (errno==EPIPE) {
				//printf("EPIPE RECEIVED %d\n",EPIPE);fflush(0);
				 return BROKEN;
			 }
			 else {
				 ERROR_HELPER(ret,"error sending");
				//else ERROR_HELPERT(ret,"error sending");
			 }
		 } else {
			 sent+=ret;
			 len-=sent;
			 
		 }
		
	}
	return 1;
	
}

int myRecvNew(int fd,char*buf)
{
	char ch;
	
	int ret,index=0;
	//char*ris=calloc(dim,sizeof(char));
	
	while (1){
		
		ret=recv(fd,&ch,1,0);
		if (ret==-1){
			if (errno==EINTR) continue;
			else {
				printf("ret :%d",errno);fflush(0);
				ERROR_HELPER(ret,"error receiving");
				//else ERROR_HELPERT(ret,"error receiving");
			}
		}
		if (ret==0){
			//printf("ECCOMI");fflush(0);
			return 0;
		}

		buf[index]=ch;
		index++;
		if (ch=='\n') {
			buf[index]='\0';
			//printf("ECCOMI");fflush(0);
			return 1;
		}

		
	}
		
	return 1;
	
}

void offlineServer(int c,int socket_desc)
{
	if(c==0) {
		printf("\nThe server is offline:closing");fflush(0);
		
		int ret = close(socket_desc);
		ERROR_HELPER(ret, "Cannot close socket");
		exit(EXIT_SUCCESS);
	}
}
void brpipe_client(int ret,int socket_desc)
{
	if(ret==BROKEN) {
		printf("\nThe server is offline:closing");fflush(0);
		
		ret = close(socket_desc);
		ERROR_HELPER(ret, "Cannot close socket");
		exit(EXIT_SUCCESS);
	}
}

int askName(char*name)
{
	printf("Insert Name or type <back> : ");
	//char *name=calloc(64,sizeof(char));
	
	fgets(name,64,stdin);
	if (strlen(name)<2 || strstr(name," ")  ){
		printf("\nThe name is not valid:spaces are not allowed and it must contain at least 1 char\n");
		return 0;
		
	}
	
	return 1;
}

int askPassw(char*name)
{
	printf("Insert Password or type <back>  ");
	//char *name=calloc(64,sizeof(char));
	fgets(name,64,stdin);
	if (strlen(name)<2 || strstr(name," ")  ){
		printf("\nThe password is not valid:spaces are not allowed and it must contain at least 1 char\n");
		return 0;
		
	}
	return 1;
}

void sign_up(int socket_desc,char*buf)
{
	char v1[1024],v2[1024],com[1024];
	memset(com,0,1024);
	memset(v1,0,1024);
	memset(v2,0,1024);
	buf[strlen(buf)]='\n';
	int ret=mySend(socket_desc,buf,strlen(buf));
	brpipe_client( ret, socket_desc);

	
	while(1){

		while(1){
			ret=askName(v1);
			if (!strcmp(v1,"back\n")){
				ret=mySend(socket_desc,v1,strlen(v1));
				brpipe_client( ret, socket_desc);
				//free(v1);
				return;
			}
			if (ret) break;
			//else free(v1);
		}
		while(1){
			ret=askPassw(v2);
			if (!strcmp(v2,"back\n")){
				ret=mySend(socket_desc,v2,strlen(v2));
				brpipe_client( ret, socket_desc);
				//free(v2);
				//free(v1);
				return;
			}
			if (ret) break;
			//else free(v2);
		}

		ret=mySend(socket_desc,v1,strlen(v1));
		brpipe_client( ret, socket_desc);
		ret=mySend(socket_desc,v2,strlen(v2));
		brpipe_client( ret, socket_desc);
		ret=myRecvNew(socket_desc,com);
		offlineServer(ret,socket_desc);
		com[strlen(com)-1]='\0';

		if(!strcmp(com,"False")) {
			printf(ANSI_COLOR_GREEN"\nServer message: choose a different name because it alerady exists\n\n"ANSI_COLOR_RESET);fflush(0);
			//free(v1);free(v2);free(com);
			continue;
		}
		if(!strcmp(com,"True")) {
			//free(com);
			ret=myRecvNew(socket_desc,com);
			offlineServer(ret,socket_desc);
			printf(ANSI_COLOR_GREEN"\nServer message:%s\n"ANSI_COLOR_RESET,com);fflush(0);//free(v1);free(v2);free(com);
			break;
		}
	}
}

Bool log_in(int socket_desc,char*buf)
{
	char v1[1024],v2[1024],com[1024];
	memset(com,0,1024);
	memset(v1,0,1024);
	memset(v2,0,1024);
	buf[strlen(buf)]='\n';
	int ret=mySend(socket_desc,buf,strlen(buf));
	brpipe_client( ret, socket_desc);
	
	while(1){
		while(1){
			ret=askName(v1);
			if (v1!=NULL&&!strcmp(v1,"back\n")){
				ret=mySend(socket_desc,v1,strlen(v1));
				brpipe_client( ret, socket_desc);
				//free(v1);
				return 0;
			}
		
			if (ret) break;
			//else free(v1);
		}
		while(1){
			ret=askPassw(v2);
			if (v2!=NULL&&!strcmp(v2,"back\n")){
				ret=mySend(socket_desc,v2,strlen(v2));
				brpipe_client( ret, socket_desc);
				//free(v1);
				//free(v2);
				return 0;
			}
			if (ret) break;
			//else free(v2);
		}

		ret=mySend(socket_desc,v1,strlen(v1));
		brpipe_client( ret, socket_desc);
		ret=mySend(socket_desc,v2,strlen(v2));
		brpipe_client( ret, socket_desc);
		ret=myRecvNew(socket_desc,com);
		offlineServer(ret, socket_desc);
		com[strlen(com)-1]='\0';
		//printf("com:<%s>",com);
		if(!strcmp(com,"FalseL")) {
			printf(ANSI_COLOR_GREEN"\nServer message:You are already logged\n\n"ANSI_COLOR_RESET);fflush(0);
			//free(v1);free(v2);free(com);
			continue;
		}
		if(!strcmp(com,"False")) {
			printf(ANSI_COLOR_GREEN"\nServer message:Name or Password are wrong\n\n"ANSI_COLOR_RESET);fflush(0);
			//free(v1);free(v2);free(com);
			continue;
		}
		if(!strcmp(com,"True")) {
			
			//free(com);
			ret=myRecvNew(socket_desc,com);
			offlineServer(ret,socket_desc);
			printf(ANSI_COLOR_GREEN"\nServer message:%s\n"ANSI_COLOR_RESET,com);fflush(0);//free(v1);free(v2);free(com);
			return 1;
		}
	}
}

void view(int socket_desc,char*buf){
	printf(ANSI_COLOR_GREEN"\nServer message:\n");fflush(0);
	buf[strlen(buf)]='\n';
	int ret=mySend(socket_desc,buf,strlen(buf));
	brpipe_client( ret, socket_desc);
	//char*com=myRecv(socket_desc);
	char com[1024];
	memset(com,0,1024);

	ret=myRecvNew(socket_desc,com);
	offlineServer(ret,socket_desc);

	printf("%s",com);
	ret=myRecvNew(socket_desc,com);
	offlineServer(ret, socket_desc);
	printf("%s",com);
	ret=myRecvNew(socket_desc,com);
	offlineServer(ret,socket_desc);
	printf("%s",com);
	ret=myRecvNew(socket_desc,com);
	offlineServer(ret,socket_desc);
	printf("%s",com);
	fflush(0);
	printf(ANSI_COLOR_RESET "");
	ret=myRecvNew(socket_desc,com);
	offlineServer(ret,socket_desc);
	com[strlen(com)-1]='\0';
	if(strcmp(com,"done")){
		 printf("error receiving ack");fflush(0);
	 }else printf("\n"ANSI_COLOR_RESET);

}

void sendmess(int socket_desc,char*buf)
{	
	buf[strlen(buf)]='\n';
	int ret=mySend(socket_desc,buf,strlen(buf));
	brpipe_client( ret, socket_desc);
	printf("Type desinatiario or <back>: ");
	char com[1024];
	char add[1024];
	memset(com,0,1024);
	memset(add,0,1024);
	
	fgets(com,1024,stdin);
	if (!strcmp(com,"back\n")){
		ret=mySend(socket_desc,com,strlen(com));
		brpipe_client( ret, socket_desc);
		return;
	}
	ret=mySend(socket_desc,com,strlen(com));
	brpipe_client( ret, socket_desc);
	//char*rrr=myRecv(socket_desc);
	char rrr[1024];
	memset(rrr,0,1024);

	ret=myRecvNew(socket_desc,rrr);
	offlineServer(ret,socket_desc);
	rrr[strlen(rrr)-1]='\0';
	if (!strcmp(rrr,"False")) {
		printf(ANSI_COLOR_GREEN"\nServer message:the user does not exist\n"ANSI_COLOR_RESET);
		return;
	}
	//free(rrr);
	printf("Type object ( max:1 line <stop> to abort )  :");
	
	memset(com,0,1024);
	fgets(com,1024,stdin);
	
	if (1024-strlen(com)<=0){
		printf("WARNING:the message is too big and it will be destroyed\n");
		ret=mySend(socket_desc,"stop\n",strlen("stop\n"));
		brpipe_client( ret, socket_desc);
	}else if (!strcmp(com,"stop\n")){
		ret=mySend(socket_desc,"stop\n",strlen("stop\n"));
		brpipe_client( ret, socket_desc);
		return;
	}else{
		ret=mySend(socket_desc,com,strlen(com));
		brpipe_client( ret, socket_desc);
	}
		
	printf("Type text ( <END> to send <stop> to abort) :");
	memset(com,'\0',1024);
	memset(add,'\0',1024);
	int num_lines=1;
	while(1){
		fgets(add,1024,stdin);
		
		if (1024-strlen(add)-strlen(com)<=0){
			printf("WARNING:the message is too big and it will be destroyed\n");
			ret=mySend(socket_desc,"stop\n",strlen("stop\n"));
			brpipe_client( ret, socket_desc);
			return;
		}
		if (!strcmp(add,"stop\n")){
			ret=mySend(socket_desc,"stop\n",strlen("stop\n"));
			brpipe_client( ret, socket_desc);
			return;
		}
		if (strstr(add,"END")!=NULL){
			
			add[strlen(add)-4]='\n';
			add[strlen(add)-3]='\0';
			
			strcat(com,add);
		    //printf("invio :<%s> righe:%d",com,num_lines);fflush(0);
			char tmp[16];memset(tmp,'\0',16);
			sprintf(tmp,"%d",num_lines);
			strcat(tmp,"\n");
			//printf("invio :<%s> ",tmp);fflush(0);
			ret=mySend(socket_desc,tmp,strlen(tmp));
			brpipe_client( ret, socket_desc);			
			ret=mySend(socket_desc,com,strlen(com));
			brpipe_client( ret, socket_desc);
			break;
		} else {
			//add[strlen(add)-1]='\0';
			strcat(com,add);
			memset(add,'\0',1024);;
			num_lines++;	
		}	;
		
	}	
	//rrr=myRecv(socket_desc);
	ret=myRecvNew(socket_desc,rrr);
	offlineServer(ret,socket_desc);
	printf(ANSI_COLOR_GREEN"\nServer message:%s\n"ANSI_COLOR_RESET,rrr);fflush(stdin);
	//free(rrr);
}

void readmess(int socket_desc,char*buf)
{
	buf[strlen(buf)]='\n';
	int ret=mySend(socket_desc,buf,strlen(buf));
	brpipe_client( ret, socket_desc);
	printf("Type <message_name> or <back>:");
	char com[1024];
	char add[1024];
	memset(com,0,1024);
	memset(add,0,1024);
	
	fgets(com,1024,stdin);
	strcpy(add,"msglog/");
	strcat(add,com);
	if (!strcmp(com,"back\n")){
		ret=mySend(socket_desc,com,strlen(com));
		brpipe_client( ret, socket_desc);
		return;
	}
	ret=mySend(socket_desc,add,strlen(add));
	brpipe_client( ret, socket_desc);
	
	//char*rrr=myRecv(socket_desc);
	char rrr[1024];
	memset(rrr,0,1024);

	ret=myRecvNew(socket_desc,rrr);
	offlineServer(ret,socket_desc);
	rrr[strlen(rrr)-1]='\0';
	if (!strcmp(rrr,"False")) {
		printf(ANSI_COLOR_GREEN"\nServer message:the message does not exist aborting\n"ANSI_COLOR_RESET);
		//free(rrr);
		return;
	}
	else {
		
		printf(ANSI_COLOR_GREEN"\nServer message:\n%s\n",rrr);fflush(0);
		while(strcmp(rrr,"done\n")){
			//free(rrr);
			//rrr=myRecv(socket_desc);
			ret=myRecvNew(socket_desc,rrr);
			offlineServer(ret,socket_desc);
			if (!strcmp(rrr,"done\n")) {
				printf(ANSI_COLOR_RESET"");
				fflush(0);
				//free(rrr);
				return;
			}
			if (strcmp(rrr,"\n")>0)printf("\n%s",rrr);
			fflush(0);
			memset(rrr,0,1024);
			
		}

	}

}

void online(int socket_desc,char*buf)
{
		buf[strlen(buf)]='\n';
		int ret=mySend(socket_desc,buf,strlen(buf));
		brpipe_client( ret, socket_desc);
		char rrr[1024];
		memset(rrr,0,1024);
		printf(ANSI_COLOR_GREEN"Server message:\n");
		while(strcmp(rrr,"done\n")){
			ret=myRecvNew(socket_desc,rrr);
			offlineServer(ret,socket_desc);
			if (!strcmp(rrr,"done\n")) {
				printf(ANSI_COLOR_RESET"");
				fflush(0);
				//free(rrr);
				return;
			}
			if (strcmp(rrr,"\n")>0)printf("\n%s",rrr);
			fflush(0);
			memset(rrr,0,1024);
			
		}
}

void removemess(int socket_desc,char*buf)
{	
	buf[strlen(buf)]='\n';
	int ret=mySend(socket_desc,buf,strlen(buf));
	brpipe_client( ret, socket_desc);
	printf("Is it a sent or received message ?<S/R> or <back>:");
	char com[1024];
	char add[1024];
	memset(com,0,1024);
	memset(add,0,1024);
	fgets(com,1024,stdin);
	if(!strcmp(com,"back\n")){
		ret=mySend(socket_desc,com,strlen(com));
		brpipe_client( ret, socket_desc);
		return;
	}
	if(strcmp(com,"S\n") && strcmp(com,"R\n")) {
		ret=mySend(socket_desc,com,strlen(com));
		brpipe_client( ret, socket_desc);
		//char*rr=myRecv(socket_desc);
		char rr[1024];
		memset(rr,0,1024);
	
		ret=myRecvNew(socket_desc,rr);
		offlineServer(ret, socket_desc);
		printf(ANSI_COLOR_GREEN"\nServer message:%s\n"ANSI_COLOR_RESET,rr);
		//free(rr);
		return;
	}
	ret=mySend(socket_desc,com,strlen(com));
	brpipe_client( ret, socket_desc);
	printf("Type <message_name>:");
	fgets(com,1024,stdin);
	strcpy(add,"msglog/");
	strcat(add,com);
	ret=mySend(socket_desc,add,strlen(add));
	brpipe_client( ret, socket_desc);
	char r[1024];
	memset(r,0,1024);

	ret=myRecvNew(socket_desc,r);
	offlineServer(ret,socket_desc);
	printf(ANSI_COLOR_GREEN"\nServer message:%s\n"ANSI_COLOR_RESET,r);


}

