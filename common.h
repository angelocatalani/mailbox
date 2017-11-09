#define ERROR_HELPER(ret, message)  do {                                \
            if (ret < 0) {                                              \
                fprintf(stderr, "%s: %m\n", message);  \
               exit(EXIT_SUCCESS);                                     \
            }                                                           \
        } while (0)
      
      
#define ERROR_HELPERT(ret, message)  do {                                \
            if (ret < 0) {                                              \
                fprintf(stderr, "%s: %m\n", message);  \
               pthread_exit(NULL);                                     \
            }                                                           \
        } while (0)

/* Configuration parameters */
#define MAX_CONN_QUEUE  10   // max number of connections the server can queue
#define SERVER_ADDRESS  "127.0.0.1"
#define SERVER_PORT     8081

#define NO_SETUP "nosetup"
#define SETUP "setup"
#define BACK "back"
#define ONLINE "online"
#define SIGN_UP "sign"
#define LOG_IN "log"
#define VIEW "view"
#define READ "read"
#define SEND "send"
#define REMOVE "remove"
#define ABORT "abort"
#define BROKEN -23
#define TIMEOUT -24



