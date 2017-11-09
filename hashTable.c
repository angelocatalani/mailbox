#include <stdio.h>
#include <stdlib.h>
#include "type_interface.h"
#include "list_interface.h"
#include "utilities_interface.h"
#include "hashTable_interface.h"
#include <string.h>
#include "color.h"

unsigned long hashCode(unsigned char *str)
{
	unsigned long hash = 5381;
	int c;
	while (c = *str++)
		hash = ((hash << 5) + hash) + c;
	return hash;
}

hashTable_t* hashTable_new()
{
	hashTable_t* r=calloc(1,sizeof(hashTable_t));
	r->size=256;
	r->used=0;
	r->bucket=calloc(r->size,sizeof(list_t*));
	int i=0;
	for (i=0;i<r->size;i++) r->bucket[i]=list_new();
	return r;
	
}

//gestire la riallocazione:copiare tutto in un bucket grande il doppio
Bool hashTable_add(hashTable_t*table,user_t* u)
{
	int index=hashCode(u->name)%table->size;
	user_t* ris=list_checkUser(table->bucket[index],u->name);
	if (ris) return False;
	if ((double)table->used/table->size>=0.7) {
		printf(ANSI_COLOR_RED"critical memory : load factor>0.7 reallocating on load...");
		
		int s=table->size;
		table->size*=2;
		list_t**new=calloc(table->size,sizeof(list_t*));
		int i=0,j=0;
		node_t*tmp=NULL;
		for (i=0;i<table->size;i++) new[i]=list_new();
		list_t**old=table->bucket;
		for (i=0;i<s;i++){
			for(tmp=old[i]->head;tmp!=NULL;tmp=tmp->next){
				user_t*uu=tmp->value;
				user_t*u=calloc(1,sizeof(user_t));
				user_new(u,uu->name,uu->password);
				index=hashCode(u->name)%table->size;
				list_addUser(new[index],u);
			}
			
			
		}
		for (i=0;i<s;i++) list_freeUser(table->bucket[i]);
		free(table->bucket);	
		
		table->bucket=new;
		printf("DONE!\n");fflush(0);
		printf("" ANSI_COLOR_RESET);
		return hashTable_add(table,u);

	}
	list_addUser(table->bucket[index],u);
	table->used++;
	return True;
	

}
user_t* hashTable_get(hashTable_t*table,char*name)
{
	int index=hashCode(name)%table->size;
	return list_checkUser(table->bucket[index],name);
	
}

void hashTable_free(hashTable_t*table)
{
	int i=0;
	for (i=0;i<table->size;i++) list_freeUser(table->bucket[i]);
	free(table->bucket);
	free(table);
}
void hashTable_print(hashTable_t table)
{	
	int i=0;
	for (i=0;i<table.size;i++) if(table.bucket[i]->size) list_printUser(table.bucket[i]);
			
	
	
}

