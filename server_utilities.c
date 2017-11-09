#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "type_interface.h"
#include "list_interface.h"
#include "hashTable_interface.h"
#include "utilities_interface.h"
#include "server_interface.h"
#include "common.h"
#include "color.h"
#include <dirent.h>

//global stuff
extern struct sockaddr_in* client_addr;
extern msg_box_t*shmem;
extern int u,v;
extern int listening_socket;
extern sem_t mutex_shmemOp;//mutex shmem + exit only if cs is free
extern struct timespec abs_timeout;

extern list_t*userOnline;

extern sem_t mutex_onLine;

int check_file( char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}
int check_dir( char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    
    return S_ISDIR(path_stat.st_mode);
}

Bool removeDir(char*p)
{
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir (p)) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
				if (  !strcmp(ent->d_name,".")||!strcmp(ent->d_name,"..")) continue;
				char np[256];memset(np,0,256);
				strcpy(np,p);
				strcat(np,"/");
				strcat(np,ent->d_name);
				if (check_file(np)) {
					
					int ret=unlink(np);
					
					if (ret==-1){
						 printf("error unlink :%s",np);fflush(0);
						 return False;
					 }else {
						 printf ("file deleted :<%s>\n", np);fflush(0);
					 }
				}else {
					
					int ris=removeDir(np);
					if (!ris) return False;
				}
		}
		closedir (dir);
		int ret=rmdir(p);
		if (ret==-1) {
			printf("error remove:%s",p);
			return False;
		}else{
			printf ("dir deleted :<%s>\n", p);fflush(0);
			return True;
		}
	} else {
			printf ("error accessing the direcorty");
			return False;
	}
}


void configure()
{	
	int ret=0;
	printf("Step1: Configuring users and msg_box\n");
	 ret=open("users",O_WRONLY|O_TRUNC|O_CREAT,0666);
	ERROR_HELPER(ret,"error init file");
	myWrite(ret,"NAME PASSWORD\n",strlen("NAME PASSWORD\n"));
	close(ret);
	ret=open("msg_box",O_WRONLY|O_TRUNC|O_CREAT,0666);
	ERROR_HELPER(ret,"error init file");
	myWrite(ret,"SENDER RECEIVER MESSAGE FLAG\n",strlen("SENDER RECEIVER MESSAGE FLAGn"));
	close(ret);
	printf("Step1: Done!\n");
	printf("Step2: Configuring msg_log database\n");
	ret=check_dir("msglog");
	if (ret){
		if (!removeDir("msglog") ) {
			printf("error deleting msg_log\n");
			exit(EXIT_FAILURE);
		}
		ret=mkdir("msglog",0777);
		ERROR_HELPERT(ret,"error creating dir");
	}
	else{
		ret=mkdir("msglog",0777);
		ERROR_HELPERT(ret,"error creating dir");
	}
	printf("Step2: Done!\n\n");
}





int myRecvNewThread(int fd,char*buf)
{
	char ch;
	
	int ret,index=0;
	//char*ris=calloc(dim,sizeof(char));
	
	while (1){
		
		ret=recv(fd,&ch,1,0);
		if (ret==-1){
			if (errno==EINTR) continue;
			else {
				if (errno==EAGAIN) return TIMEOUT;
				//printf("ret :%d",errno);fflush(0);
				ERROR_HELPERT(ret,"error receiving");
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

int mySendThread(int fd, char*buf, int len)
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
				 if (errno==EAGAIN) return TIMEOUT;
				 ERROR_HELPERT(ret,"error sending");
				//else ERROR_HELPERT(ret,"error sending");
			 }
		 } else {
			 sent+=ret;
			 len-=sent;
			 
		 }
		
	}
	return 1;
	
}


void offlineClient(int ret,int socket_desc,char*name)
{		
		
		if(ret==TIMEOUT||ret==0) {
			if (name!=NULL){
				int ec=sem_wait(&mutex_onLine);
				ERROR_HELPERT(ec,"error sem");
				list_removeString(userOnline,name);
				ec=sem_post(&mutex_onLine);
				ERROR_HELPERT(ec,"error sem");
			}
		if (ret==TIMEOUT){
			printf("\nYou  are offline:inactivity time-exceed\n");
			fflush(0);
		}else {
			printf("\nThe client is offline:closing\n");
			fflush(0);			
		}
		
		close(socket_desc);
		pthread_exit(NULL);
	}
}
void server_signup(int socket_desc,msg_box_t*shmem,int u)
{
	char name[1024];
	char passw[1024];
	memset(name,'\0',1024);
	memset(passw,'\0',1024);
	while(1){
		int ret;
		//char*name=myRecv(socket_desc);
		ret=myRecvNewThread(socket_desc,name);
		
		offlineClient(ret,socket_desc,NULL);
		if (!strcmp(name,"back\n")){
			return;
		}
		//char*passw=myRecv(socket_desc);
		ret=myRecvNewThread(socket_desc,passw);
		offlineClient(ret,socket_desc,NULL);
		if (!strcmp(passw,"back\n")){
			return;
		}
		
		name[strlen(name)-1]='\0';
		passw[strlen(passw)-1]='\0';
		//printf("name:<%s> password:<%s>",name,passw);fflush(0);
		ret=sem_timedwait(&mutex_shmemOp, &abs_timeout);
		if (ret==ETIMEDOUT){
			printf("deadlock detected aborting");fflush(0);
			pthread_exit(NULL);
		}
		ERROR_HELPERT(ret,"error sem");
		Bool b = msgBox_AddUser(shmem,u,name,passw);
		ret=sem_post(&mutex_shmemOp);
		ERROR_HELPERT(ret,"error sem");
		
		if(!b){
			ret=mySendThread(socket_desc,"False\n",strlen("False\n"));
			if (ret==BROKEN) {
						//free(name);
						//free(passw);
						brpipe_server(ret,socket_desc,NULL);
				}
			//free(name);
			//free(passw);
			continue;
			
			
		}else {
		ret=mySendThread(socket_desc,"True\n",strlen("True\n"));
		if (ret==BROKEN) {
				brpipe_server(ret,socket_desc,NULL);
				//free(name);
				//free(passw);
				return;
			}
		ret=mySendThread(socket_desc,"now you are signed!\n",strlen("now you are signed!\n"));
		if (ret==BROKEN) {
				//free(name);
				//free(passw);
				brpipe_server(ret,socket_desc,NULL);
				return;
			}
		//free(name);
		//free(passw);
		break;
		}
	}
}
user_t* server_login(int socket_desc,msg_box_t* shmem,int u)
{
	char name[1024];memset(name,'\0',1024);
	char passw[1024];memset(passw,'\0',1024);
	while(1){
		int ret;
		
		//char*name=myRecv(socket_desc);
		ret=myRecvNewThread(socket_desc,name);
		
		offlineClient(ret,socket_desc,NULL);
		if (!strcmp(name,"back\n")){
			return NULL;
		}
		
		ret=myRecvNewThread(socket_desc,passw);		
		//char*passw=myRecv(socket_desc);
		offlineClient(ret,socket_desc,NULL);
		if (!strcmp(passw,"back\n")){
			return NULL;
		}
		name[strlen(name)-1]='\0';
		
		
		if(list_checkString(userOnline,name)){
			//list_printString(userOnline);
			ret=mySendThread(socket_desc,"FalseL\n",strlen("FalseL\n"));
			if (ret==BROKEN) {
						brpipe_server(ret,socket_desc,NULL);
						return NULL;
				}
			continue;
		}
		
		
		
		passw[strlen(passw)-1]='\0';
		//printf("name:<%s> password:<%s>",name,passw);fflush(0);
		ret=sem_timedwait(&mutex_shmemOp,&abs_timeout);
		if (ret==ETIMEDOUT){
			printf("deadlock detected aborting");fflush(0);
			pthread_exit(NULL);
		}
		ERROR_HELPERT(ret,"error sem");
		user_t*user = hashTable_get(shmem->database,name);
		ret=sem_post(&mutex_shmemOp);
		ERROR_HELPERT(ret,"error sem");
		if(!user||strcmp(user->password,passw)){
			ret=mySendThread(socket_desc,"False\n",strlen("False\n"));
			if (ret==BROKEN) {
						brpipe_server(ret,socket_desc,NULL);
						return NULL;
				}
			continue;
		}else {
			
			ret=mySendThread(socket_desc,"True\n",strlen("True\n"));
			if (ret==BROKEN) {
				//free(name);free(passw);
				brpipe_server(ret,socket_desc,NULL);
				return NULL;
			}
			ret=mySendThread(socket_desc,"now you are logged!\n",strlen("now you are logged!\n"));
			if (ret==EPIPE) {
				//free(name);free(passw);
				return NULL;
				}
			
			//add onLine user in mutex
			ret=sem_wait(&mutex_onLine);
			ERROR_HELPERT(ret,"error sem");
			//printf(user->name);fflush(0);
			list_addString(userOnline,user->name);
			ret=sem_post(&mutex_onLine);
			ERROR_HELPERT(ret,"error sem");
			
			return user;
		}
	}
}
void server_view(int socket_desc,user_t u)
{	

	int sto=dup(1);
	dup2(socket_desc,1);
	user_print(u);fflush(stdin);

	dup2(sto,1);
	close(sto);

	int ret=mySendThread(socket_desc,"done\n",strlen("done\n"));
	brpipe_server(ret,socket_desc,u.name);


}


void server_online(int socket_desc,user_t u)
{
	int sto=dup(1);
	dup2(socket_desc,1);
	int ret=sem_wait(&mutex_onLine);
	ERROR_HELPERT(ret,"error sem");
	list_printString(userOnline);
	ret=sem_post(&mutex_onLine);
	ERROR_HELPERT(ret,"error sem");
	dup2(sto,1);
	close(sto);

	 ret=mySendThread(socket_desc,"done\n",strlen("done\n"));
	brpipe_server(ret,socket_desc,u.name);
}

void server_createmess(int socket_desc,user_t *u,int msg_box,msg_box_t*shmem)
{	
	char dest[1024];memset(dest,'\0',1024);
	
	int ret;
	//char*dest=myRecv(socket_desc);
	ret=myRecvNewThread(socket_desc,dest);
	offlineClient(ret,socket_desc,u->name);
	if (!strcmp(dest,"back\n")){
		//free(dest);
		return;
	}
	//printf("dest:<%s>",dest);
	dest[strlen(dest)-1]='\0';
	ret=sem_timedwait(&mutex_shmemOp, &abs_timeout);
	if (ret==ETIMEDOUT){
		printf("deadlock detected aborting");fflush(0);
		pthread_exit(NULL);
	}
	ERROR_HELPERT(ret,"error sem");
	user_t*r=hashTable_get(shmem->database,dest);
	ret=sem_post(&mutex_shmemOp);
	ERROR_HELPERT(ret,"error sem");
	if (r==NULL){
		ret=mySendThread(socket_desc,"False\n",strlen("False\n"));
		brpipe_server(ret,socket_desc,u->name);
		return;
	}
	ret=mySendThread(socket_desc,"done\n",strlen("done\n"));
	brpipe_server(ret,socket_desc,u->name);
	char message_obj[1024];memset(message_obj,'\0',1024);
	ret=myRecvNewThread(socket_desc,message_obj);
	//char*message_obj=myRecv(socket_desc);
	offlineClient(ret,socket_desc,u->name);
	printf("obj:<%s>",message_obj);
	if (!strcmp(message_obj,"stop\n")) {
		//free
		printf("abort\n");
		return;
	}
	char message_text[1024];memset(message_text,'\0',1024);
	char line[1024];memset(line,'\0',1024);
	int i=0;char tmp[16];
	ret=myRecvNewThread(socket_desc,tmp);
	offlineClient(ret,socket_desc,u->name);
	int num_lines=atoi(tmp);
	
	for (i=0;i<num_lines;i++){
		ret=myRecvNewThread(socket_desc,line);
		offlineClient(ret,socket_desc,u->name);
		
		if (!strcmp(message_text,"stop\n")) {
		
			printf("abort\n");
			return;
		}
		strcat(message_text,line);
		memset(line,'\0',1024);
	}
	ret=sem_timedwait(&mutex_shmemOp,&abs_timeout);
	if (ret==ETIMEDOUT){
		printf("deadlock detected aborting");fflush(0);
		pthread_exit(NULL);
	}
	ERROR_HELPERT(ret,"error sem");
	
	char*message_name= user_createmsg(message_obj,message_text,u->name,dest,shmem);
	ret=sem_post(&mutex_shmemOp);
	ERROR_HELPERT(ret,"error sem");
	
	//printf("message name:<%s>",message_name);fflush(0);
	ret=sem_timedwait(&mutex_shmemOp, &abs_timeout);
	if (ret==ETIMEDOUT){
		printf("deadlock detected aborting");fflush(0);
		pthread_exit(NULL);
	}
	ERROR_HELPERT(ret,"error sem");
	msgBox_sendmsg(message_name,u,r, msg_box,shmem);
	ret=sem_post(&mutex_shmemOp);
	ERROR_HELPERT(ret,"error sem");
	free(message_name);
	
	ret=mySendThread(socket_desc,"done\n",strlen("done\n"));
	brpipe_server(ret,socket_desc,u->name);
}

void server_readmess(int socket_desc,user_t*current)
{	
	int ret;
	char mess[1024];memset(mess,'\0',1024);
	ret=myRecvNewThread(socket_desc,mess);
	//char*mess=myRecv(socket_desc);
	offlineClient(ret,socket_desc,current->name);
	if (!strcmp(mess,"back\n")){
		
		//free(mess);
		return;
	}
	//printf("mess:<%s>",mess);
	mess[strlen(mess)-1]='\0';
	ret=sem_timedwait(&mutex_shmemOp, &abs_timeout);
	if (ret==ETIMEDOUT){
		printf("deadlock detected aborting");fflush(0);
		pthread_exit(NULL);
	}
	ERROR_HELPERT(ret,"error sem");
	int ffflag=list_checkString(current->msg_recv,mess)||list_checkString(current->msg_sent,mess);
	ret=sem_post(&mutex_shmemOp);
	ERROR_HELPERT(ret,"error sem");
	if(ffflag){

		user_printmsg(mess);fflush(0);
		int sto=dup(1);
		dup2(socket_desc,1);
		user_printmsg(mess);fflush(0);
		dup2(sto,1);
		close(sto);
		int ret=mySendThread(socket_desc,"done\n",strlen("done\n"));
		brpipe_server(ret,socket_desc,current->name);
	}
	else {
		ret=mySendThread(socket_desc,"False\n",strlen("False\n"));
		brpipe_server(ret,socket_desc,current->name);
	}
}
void server_removeSen(int socket_desc,user_t*current,msg_box_t*shmem,int msg_box)
{	
	int ret;
	char mess[1024];memset(mess,'\0',1024);
	ret=myRecvNewThread(socket_desc,mess);
	//char*mess=myRecv(socket_desc);
	offlineClient(ret,socket_desc,current->name);
	//printf("mess:<%s>",mess);
	mess[strlen(mess)-1]='\0';
	ret=sem_timedwait(&mutex_shmemOp, &abs_timeout);
	if (ret==ETIMEDOUT){
		printf("deadlock detected aborting");fflush(0);
		pthread_exit(NULL);
	}
	ERROR_HELPERT(ret,"error sem");
	Bool u=user_delmsgSen(current,mess,shmem,msg_box);
	ret=sem_post(&mutex_shmemOp);
	ERROR_HELPERT(ret,"error sem");
	if(u) {
		ret=mySendThread(socket_desc,"Done\n",strlen("Done\n"));
		brpipe_server(ret,socket_desc,current->name);
	}
	else {
		ret=mySendThread(socket_desc,"Failed\n",strlen("Failed\n"));
		brpipe_server(ret,socket_desc,current->name);
	}
}
void server_removeRec(int socket_desc,user_t*current,msg_box_t*shmem,int msg_box)
{
	int ret;
	char mess[1024];memset(mess,'\0',1024);
	ret=myRecvNewThread(socket_desc,mess);
	//char*mess=myRecv(socket_desc);
	offlineClient(ret,socket_desc,current->name);
	//printf("mess:<%s>",mess);
	mess[strlen(mess)-1]='\0';
	ret=sem_timedwait(&mutex_shmemOp, &abs_timeout);
	if (ret==ETIMEDOUT){
		printf("deadlock detected aborting");fflush(0);
		pthread_exit(NULL);
	}
	ERROR_HELPERT(ret,"error sem");
	Bool u=user_delmsgRcv(current,mess,shmem,msg_box);
	ret=sem_post(&mutex_shmemOp);
	ERROR_HELPERT(ret,"error sem");
	if(u) {
		ret=mySendThread(socket_desc,"Done\n",strlen("Done\n"));
		brpipe_server(ret,socket_desc,current->name);
	}
	else{
		 ret=mySendThread(socket_desc,"Failed\n",strlen("Failed\n"));
		 brpipe_server(ret,socket_desc,current->name);
	 }
}

void brpipe_server(int ret,int socket_desc,char*name)
{
	if(ret==BROKEN) {
		if (name!=NULL){
			ret=sem_wait(&mutex_onLine);
			ERROR_HELPERT(ret,"error sem");
			list_removeString(userOnline,name);
			ret=sem_post(&mutex_onLine);
			ERROR_HELPERT(ret,"error sem");
		}
		printf("\nThe client is Offline:closing");
		ret = close(socket_desc);
		ERROR_HELPERT(ret, "Cannot close socket");
		pthread_exit(NULL);
	}
	if(ret==TIMEOUT) {
		printf("\nTimeout expired:closing");
		ret = close(socket_desc);
		ERROR_HELPERT(ret, "Cannot close socket");
		pthread_exit(NULL);
	}
}
