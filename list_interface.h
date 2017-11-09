
//generiche
list_t* list_new();


//stringhe
void list_printString(list_t*l);
void list_freeString(list_t* list);
Bool list_removeString(list_t*list,char*msg);
Bool list_checkString(list_t*list,char*msg);
void list_addString(list_t*list,char*msg);

int  Aux_list_removeS(node_t*head,char*msg);



//users
void list_printUser(list_t*l);

Bool list_removeUser(list_t*list,char*name);
user_t* list_checkUser(list_t*list,char*name);

void list_addUser(list_t*list,user_t*u);
void list_freeUser(list_t* l);
int  Aux_list_removeU(node_t*head,char*name);





