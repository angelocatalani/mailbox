void server_signup(int socket_desc,msg_box_t*shmem,int u);
user_t* server_login(int socket_desc,msg_box_t*shmem,int u);
void server_view(int socket_desc,user_t u);
void server_createmess(int socket_desc,user_t *u,int msg_box,msg_box_t*shmem);
void server_delmess(int socket_desc,user_t *u,int msg_box,msg_box_t*shmem);
void server_removeSen(int socket_desc,user_t*current,msg_box_t*shmem,int msg_box);
void server_removeRec(int socket_desc,user_t*current,msg_box_t*shmem,int msg_box);
void brpipe_server(int ret,int socket_desc,char*name);
void offlineClient(int ret,int socket_desc,char*name);
int mySendThread(int sockfd, char*buf, int len);//flag MSG_NOSIGNAL
int myRecvNewThread(int fd,char*buf);//OK
void server_online(int socket_desc,user_t u);
void server_readmess(int socket_desc,user_t*current);
void configure();
int check_dir(char*path);
int check_file( char*path);
Bool removeDir(char*p);
typedef struct handler_args_s {
	int count;
    int socket_desc;
    struct sockaddr_in* client_addr;
} handler_args_t;
