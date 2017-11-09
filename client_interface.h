int askName(char*name);
int askPassw(char*name);
void sign_up(int socket_desc,char*buf);
void view(int socket_desc,char*buf);
void sendmess(int socket_desc,char*buf);
Bool log_in(int socket_desc,char*buf);
void brpipe_client(int ret,int socket_desc);
void offlineServer(int c,int socket_desc);
void online(int socket_desc,char*buf);
int myRecvNew(int fd,char*buf);//OK
int mySend(int sockfd, char*buf, int len);//flag MSG_NOSIGNAL
void readmess(int socket_desc,char*buf);
void removemess(int socket_desc,char*buf);

