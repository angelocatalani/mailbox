void user_print(user_t u);//OK
void user_free(user_t*user);//OK
void user_new(user_t*r,char*name,char*password);//OK
char* user_createmsg(char*message_obj,char*message_text,char*mitt,char*dest,msg_box_t*shmem);//OK INCR
void user_printmsg(char*message_name);//OK stampa il contenuto di message
Bool user_delmsgSen(user_t*current,char*message_name,msg_box_t*shmem,int msg_box);//OK
Bool user_delmsgRcv(user_t*current,char*message_name,msg_box_t*shmem,int msg_box);//OK


void msgBox_print(msg_box_t msg);//OK
void msgBox_free(msg_box_t* msg);//OK
Bool  msgBox_AddUser(msg_box_t*shmem,int users,char*name,char*password); //OK
msg_box_t* msgBox_get(int users,int msg_box);//OK acquisisce da file tutta la struttura dati INCR
void msgBox_sendmsg(char*message_name,user_t* sender,user_t* receiver,int msg_box,msg_box_t*shmem);//OK 

char* readline(int fd,Bool*f);//OK
void myWrite(int fd,char*buf,int len);//OK

