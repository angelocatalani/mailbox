#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type_interface.h"
#include "list_interface.h"
#include "utilities_interface.h"
#include "color.h"

list_t* list_new()
{
	list_t* ris=calloc(1,sizeof(list_t));
	ris->size=0;
	ris->head=NULL;
	return ris;
};

void  list_addString(list_t*list,char*msg)
{
	node_t*agg=calloc(1,sizeof(node_t));
	agg->value=calloc(strlen(msg)+1,sizeof(char));
	strcpy(agg->value,msg);
	agg->next=list->head;
	list->head=agg;
	list->size++;
	
	return;
};
void  list_addUser(list_t*list,user_t* u)
{
	node_t*agg=calloc(1,sizeof(node_t));
	agg->value=u;
	
	agg->next=list->head;
	list->head=agg;
	list->size++;
	
	return;
};
Bool list_removeString(list_t*list,char*msg)
{	
	
	if (list->size==0) {
		return 0;
	}
	//printf("head:%s msg:%s",(char*)(list->head->value),msg);fflush(0);
	if (!strcmp((char*)(list->head->value),msg)){
		
		node_t*tt=list->head;
		list->head=list->head->next;
		list->size--;
		free(tt->value);
		free(tt);
		return 1;
	}
	else if ( Aux_list_removeS(list->head,msg)){
		list->size--;
		return 1;
	}
	else return 0;
};
Bool list_checkString(list_t*list,char*msg)
{	
	
	if (list->size==0) {
		return 0;
	}
	//printf("msg:<%s> cfr:<%s>\n",msg,(char*)(list->head->value));fflush(0);
	//printf("head:%s msg:%s",(char*)(list->head->value),msg);fflush(0);
	if (!strcmp((char*)(list->head->value),msg)){
			//printf("msg:<%s> cfr:<%s>\n",msg,(char*)(list->head->value));fflush(0);
		return 1;
	}
	node_t*tmp=list->head;
	while(tmp){
		if (!strcmp((char*)(tmp->value),msg)){
			
			return 1;
		}
		tmp=tmp->next;
	}
	return 0;
};






Bool list_removeUser(list_t*list,char*name)
{
	if (list->size==0)return 0;
	if (!strcmp(  ((user_t*)(list->head->value))->name    ,name          )  ){
		node_t*tt=list->head;
		list->head=list->head->next;
		list->size--;
		user_free(tt->value);
		free(tt);
		return 1;
	}
	else if ( Aux_list_removeU(list->head,name)){
		list->size--;
		return 1;
	}
	return 0;
};


user_t* list_checkUser(list_t*list,char*name)
{
	if (list->size==0)return NULL;
	if (!strcmp(  ((user_t*)(list->head->value))->name    ,name          )  )return ((user_t*)(list->head->value));
	node_t*tmp=list->head->next;
	while(tmp){
		if (!strcmp(  ((user_t*)(tmp->value))->name    ,name          )  )return ((user_t*)(tmp->value));
		tmp=tmp->next;
	}
	return NULL;
			


}






int  Aux_list_removeS(node_t*head,char*msg)
{
	if ( head->next==NULL) return 0;
	if (strcmp(msg,((char*)head->next->value))) return Aux_list_removeS(head->next,msg);
	else {
		
		node_t*tt=head->next;
		head->next=head->next->next;
		free(tt->value);
		free(tt);
		return 1;
	}
};
int  Aux_list_removeU(node_t*head,char*name)
{
	if ( head->next==NULL) return 0;
	if (strcmp(name,((user_t*)(head->next->value))->name)) return Aux_list_removeU(head->next,name);
	else {
		node_t*tt=head->next;
		head->next=head->next->next;
		user_free(tt->value);
		free(tt);
		return 1;
	}
};


void list_printString(list_t*l)
{
	if (l==NULL){
		 printf(ANSI_COLOR_RED"THE LIST IS NOT INIT" ANSI_COLOR_RESET);
		 return;
	 }
	 if(l->size==0) {
		 printf("empty\n");
		 return ;
	 }
	printf("{");
	node_t*tmp=l->head;
	while(tmp){
		
		if (strstr((char*)tmp->value,"msglog/messageN"))printf(" %s ",((char*)tmp->value)+7);
		else printf(" %s ",((char*)tmp->value));
		//if (tmp->next!=NULL) printf(",");
		tmp=tmp->next;
	}
	printf("}\n");
	
	fflush(0);
	
}
void list_printUser(list_t*l)
{
	if (l==NULL){
		 printf(ANSI_COLOR_RED "THE LIST IS NOT INIT" ANSI_COLOR_RESET);
		 return;
	 }
	//printf("\n");
	node_t*tmp=l->head;
	while(tmp){
		//printf("\n");
		user_print(*(user_t*)tmp->value);
		//if (tmp->next!=NULL) printf(",");
		tmp=tmp->next;
		printf("\n");
	}
	
	fflush(0);
	
}

void list_freeUser(list_t* l)
{
	node_t*tmp=l->head;
	node_t*prev=tmp;
	//free(tmp->msg);
	
	while(tmp){
		user_free(tmp->value);
		//printf("%s",tmp->msg);
		tmp=tmp->next;
		free(prev);
		prev=tmp;
		
	}
	free(l);
}

void list_freeString(list_t* l)
{
	node_t*tmp=l->head;
	node_t*prev=tmp;
	//free(tmp->msg);
	
	while(tmp){
		free(tmp->value);
		//printf("%s",tmp->msg);
		tmp=tmp->next;
		free(prev);
		prev=tmp;
		
	}
	free(l);
}
