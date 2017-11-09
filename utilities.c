#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h> 
#include <sys/sem.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "type_interface.h"
#include "list_interface.h"
#include "hashTable_interface.h"
#include "utilities_interface.h"
#include "common.h"
#include "color.h"
#include <sys/types.h>
#include <sys/socket.h>



char* readline(int fd,Bool*last)
{
	char ch='a';
	*last=False;
	int ret,index=0,dim=256;
	char*ris=calloc(dim,sizeof(char));

	
	while (ch!='\n'){
		ret=read(fd,&ch,1);
		if (ret==-1){
			if (errno==EINTR) continue;
			else ERROR_HELPER(ret,"17 usr_utilities");
		}
		if (ret==0){
			*last=True;
			return ris;
		}
		
		ris[index++]=ch;
		if(index>=dim) {
			ris=realloc(ris,dim*2);
			dim=dim*2;
		}
		
	}
	
	ris[index-1]='\0';
	//ris=realloc(ris,index-1);
	
	return ris;
	
}

void  user_new(user_t*r,char*name,char*password)
{
	
	
	r->name=calloc(strlen(name)+1,sizeof(char));
	r->password=calloc(strlen(password)+1,sizeof(char));
	strcpy(r->name,name);
	strcpy(r->password,password);
	r->msg_recv=list_new();
	r->msg_sent=list_new();
	r->msg_del=list_new();
	return ;
	
}


msg_box_t* msgBox_get(int users,int msg_box)
{
	
	msg_box_t*ris=calloc(1,sizeof(msg_box_t));
	ris->database=hashTable_new();
	ris->total_msg=0;
	ris->total_user=0;
	lseek(users,0,0);
	Bool flag=False;
	char*line=readline(users,&flag);
	free(line);//la prima riga è inutile ai fini del parsing
	//acquisisco utenti
	while(!flag){
		char*line=readline(users,&flag);
		if (strlen(line)<2) {
			free(line);
			break;
		}
		
		
		char*name=strtok(line," ");
		char*pass=strtok(NULL," ");
		
		user_t*r=calloc(1,sizeof(user_t));
		user_new(r,name,pass);
		hashTable_add(ris->database,r);		
		ris->total_user++;
		free(line);
	}
	flag=False;	
	//acquisisco messaggi
	line=readline(msg_box,&flag);
	free(line);
	
	while(!flag){
		char*line=readline(msg_box,&flag);
		if (strlen(line)<2){
			free(line);
			 break;
		 }
		char*sen=strtok(line," ");
		char*rec=strtok(NULL," ");
		char*mes=strtok(NULL," ");
		char*flag=strtok(NULL," ");
		if (!strcmp(flag,"0")){
			user_t*senU=list_checkUser( (ris->database)->bucket[hashCode(sen)%(ris->database->size)   ]     ,sen);
			list_addString(senU->msg_sent,mes);
			ris->total_msg++;
			user_t*recU=list_checkUser((ris->database)->bucket[hashCode(rec)%(ris->database->size)],rec);
			list_addString(recU->msg_del,mes);
		}
		else if (!strcmp(flag,"1")){
			user_t*recU=list_checkUser((ris->database)->bucket[hashCode(rec)%(ris->database->size)],rec);
			list_addString(recU->msg_recv,mes);
			ris->total_msg++;
			user_t*senU=list_checkUser( (ris->database)->bucket[hashCode(sen)%(ris->database->size)   ]     ,sen);
			list_addString(senU->msg_del,mes);
		}
		else if (!strcmp(flag,"2")){
			
			user_t*senU=list_checkUser( (ris->database)->bucket[hashCode(sen)%(ris->database->size)   ]     ,sen);
			list_addString(senU->msg_sent,mes);
			user_t*recU=list_checkUser((ris->database)->bucket[hashCode(rec)%(ris->database->size)],rec);
			list_addString(recU->msg_recv,mes);
			ris->total_msg++;
		}
		else if (!strcmp(flag,"3")){
			user_t*senU=list_checkUser( (ris->database)->bucket[hashCode(sen)%(ris->database->size)   ]     ,sen);
			list_addString(senU->msg_del,mes);
			user_t*recU=list_checkUser((ris->database)->bucket[hashCode(rec)%(ris->database->size)],rec);
			if (senU!=recU)list_addString(recU->msg_del,mes);
			

		}

		else{
			printf("ERROR");
		}
			
		ris->total_msg++;
		free(line);
	}	
	return ris;

	
}

void user_print(user_t u)
{	
	
	printf(ANSI_COLOR_CYAN "Name: %s\n" ANSI_COLOR_RESET,u.name);
	printf(ANSI_COLOR_YELLOW"MsgSent:");
	list_printString(u.msg_sent);
	printf(ANSI_COLOR_RESET "");
	printf(ANSI_COLOR_GREEN"MsgRecv:");
	list_printString(u.msg_recv);
	printf(ANSI_COLOR_RESET "");
	printf(ANSI_COLOR_RED"Trash:");
	list_printString(u.msg_del);
	printf(ANSI_COLOR_RESET "");
	
}

void user_free(user_t*user)
{
	free(user->password);
	free(user->name);
	list_freeString(user->msg_recv);
	list_freeString(user->msg_sent);
	list_freeString(user->msg_del);
	free(user);
	
}

void msgBox_free(msg_box_t* msg)
{
	hashTable_free(msg->database);
	free(msg);
}
void msgBox_print(msg_box_t msg)
{
	printf(ANSI_COLOR_BLUE "\nMSGBOX\nmsgBox totMsg: %d totUser : %d"ANSI_COLOR_RESET,msg.total_msg,msg.total_user);
	hashTable_print(*(msg.database));
	printf(ANSI_COLOR_BLUE"\nFINE\n"ANSI_COLOR_RESET );
	

}
void myWrite(int fd,char*buf,int len)
{	
	int ret=0,sent=0;
	while(len>0){
		 ret=write(fd,buf+sent,len);
		
		 if (ret==-1){
			 if(errno==EINTR) continue;
			 else ERROR_HELPER(ret,"error writing");
		 } else {
			 sent+=ret;
			 len-=sent;
			 
		 }
	}
	
	
};

Bool  msgBox_AddUser(msg_box_t*shmem,int users,char*name,char*password)
{
	user_t* u=calloc(1,sizeof(user_t));
	user_new(u,name,password);
	if (! hashTable_add(shmem->database, u) ){
		user_free(u);
		return False;
	}
	shmem->total_user++;
	lseek(users,0,2);
	
	myWrite(users,name,strlen(name));
	myWrite(users," ",strlen(" "));
	myWrite(users,password,strlen(password));
	myWrite(users,"\n",strlen("\n"));
	return True;
	
}

void msgBox_sendmsg(char*message_name,user_t* sender,user_t* receiver,int msg_box,msg_box_t*shmem)
{
	shmem->total_msg++;
	list_addString(sender->msg_sent,message_name);
	list_addString(receiver->msg_recv,message_name);

	int ret=lseek(msg_box,0,2);
	ERROR_HELPER(ret,"error seek");

	char*sen=sender->name;
	char*rec=receiver->name;
	myWrite(msg_box,sen,strlen(sen));

	myWrite(msg_box," ",strlen(" "));
	myWrite(msg_box,rec,strlen(rec));
	myWrite(msg_box," ",strlen(" "));
	myWrite(msg_box,message_name,strlen(message_name));
	myWrite(msg_box," ",strlen(" "));
	myWrite(msg_box,"2",strlen("2"));
	myWrite(msg_box,"\n",strlen("\n"));


}

char* user_createmsg(char*message_obj,char*message_text,char*mitt,char*dest,msg_box_t*shmem)
{
	user_t*s= hashTable_get(shmem->database,mitt);
	user_t*r= hashTable_get(shmem->database,dest);

	int msg_c=shmem->total_msg;
	char*msg_name=calloc(128,sizeof(char));
	strcpy(msg_name,"messageN");
	char*adds=calloc(128,sizeof(char));
	sprintf(adds,"%d",msg_c);
	strcat(msg_name,adds);
	strcpy(adds,"msglog/");
	strcat(adds,msg_name);
	strcpy(msg_name,adds);
	free(adds);

	int msg_d=open(msg_name,O_RDWR|O_CREAT|O_EXCL,0666);
	if (msg_d==-1) ERROR_HELPER(msg_d,"ERROR CREATING THE MESSAGE");
	
	myWrite(msg_d,"Sender: ",strlen("Sender: "));
	myWrite(msg_d,mitt,strlen(mitt));
	myWrite(msg_d,"\n",strlen("\n"));

	myWrite(msg_d,"Receiver: ",strlen("Receiver: "));
	myWrite(msg_d,dest,strlen(dest));
	myWrite(msg_d,"\n",strlen("\n"));
	
	myWrite(msg_d,"Object: ",strlen("Object: "));
	myWrite(msg_d,message_obj,strlen(message_obj));
	myWrite(msg_d,"\n",strlen("\n"));
	
	myWrite(msg_d,"Text: ",strlen("Text: "));
	myWrite(msg_d,message_text,strlen(message_text));
	myWrite(msg_d,"\n",strlen("\n"));
	
	
	int ret=close(msg_d);
	if (ret<0) ERROR_HELPER(msg_d,"ERROR CLOSING THE MESSAGE");
	
	return msg_name;
	
}

void user_printmsg(char*message_name)
{
	int u=open(message_name,O_RDONLY);
	Bool flag=False;
	
	while(!flag){
		char*line=readline(u,&flag);
		
		printf("%s\n",line);
		free(line);
		
	}
	
}



Bool user_delmsgRcv(user_t*current,char*message_name,msg_box_t*shmem,int msg_box)
{	
	//list_printString(current->msg_recv);
	//printf("value: %s\n",(char*)(current->msg_recv->head->value));
	Bool r=list_removeString(current->msg_recv,message_name);
	if (!list_checkString(current->msg_del,message_name ))list_addString(current->msg_del,message_name);
	if (!r) return False;
	int fd=open(message_name,O_RDWR,0666);
	ERROR_HELPER(fd,"ERROR OPENING REC");
	Bool f=0;
	char *l=readline(fd,&f);
	strtok(l," ");
	char*sen=strtok(NULL," ");
	char cfr[1024];
	sprintf(cfr,"%s%s%s%s%s",sen," ",current->name," ",message_name);
	f=0;
	lseek(msg_box,0,0);
	free(readline(msg_box,&f));
	char *del=readline(msg_box,&f);
	//printf("del:<%s>",del);fflush(0);
	char tmp[1024];
	strcpy(tmp,del);
	del[strlen(del)-2]='\0';

	//printf("tmp:<%s> ",tmp);fflush(0);
	while(!f){
		if(!strcmp(del,cfr)){
			//printf("beccatooo");fflush(0);
			lseek(msg_box,-strlen("0\n"),1);
			strtok(tmp," ");
			strtok(NULL," ");
			strtok(NULL," ");
			char*fl=strtok(NULL," ");
			if (!strcmp(fl,"1")){
				
				write(msg_box,"3\n",strlen("3\n"));
				int ret=unlink(message_name);
				ERROR_HELPER(ret,"ERROR UNLINKING");
				close(fd);
				free(l);
				free(del);
				return True;
				
			}
			write(msg_box,"0\n",strlen("0\n"));
			break;
		}
		free(del);
		del=readline(msg_box,&f);
		strcpy(tmp,del);
		del[strlen(del)-2]='\0';
		
	}
	free(del);
	close(fd);
	free(l);
	return True;
	
}
Bool user_delmsgSen(user_t*current,char*message_name,msg_box_t*shmem,int msg_box)
{	
	Bool bb=list_removeString(current->msg_sent,message_name);
	if (!list_checkString(current->msg_del,message_name ))list_addString(current->msg_del,message_name);
	if (!bb) return False;
	int fd=open(message_name,O_RDWR,0666);
	ERROR_HELPER(fd,"ERROR OPENING");
	Bool f=0;
	char *l=readline(fd,&f);
	free(l);
	l=readline(fd,&f);
	strtok(l," ");
	char*rec=strtok(NULL," ");
	char cfr[1024];
	sprintf(cfr,"%s%s%s%s%s",current->name," ",rec," ",message_name);
	f=0;
	lseek(msg_box,0,0);
	free(readline(msg_box,&f));
	char *del=readline(msg_box,&f);
	char tmp[1024];
	strcpy(tmp,del);
	del[strlen(del)-2]='\0';
	while(!f){
		if(!strcmp(del,cfr)){
			lseek(msg_box,-strlen("0\n"),1);
			strtok(tmp," ");
			strtok(NULL," ");
			strtok(NULL," ");
			char*fl=strtok(NULL," ");
			if (!strcmp(fl,"0")){
				
				write(msg_box,"3\n",strlen("3\n"));
				int ret=unlink(message_name);
				ERROR_HELPER(ret,"ERROR UNLINKING");
				close(fd);
				free(del);
				free(l);
				return True;
			}
			write(msg_box,"1\n",strlen("1\n"));
			break;
		}
		free(del);
		del=readline(msg_box,&f);
		strcpy(tmp,del);
		del[strlen(del)-2]='\0';
		
		
	}
	free(del);
	close(fd);
	free(l);
	return True;
	
}
