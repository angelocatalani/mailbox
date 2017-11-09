//LIST_T
typedef struct node_s{
	void*value;
	struct node_s*next;
	
}node_t;

typedef struct list_s{
	int size;
	node_t*head;
}list_t;

//USER_T
typedef struct user_s{
	//unsigned  int ID;
	char* name;
	char* password;
	list_t*  msg_sent;
	list_t* msg_recv;
	list_t* msg_del;
}user_t;



//HASHTABLE_T
typedef struct hash_s{
	unsigned int size;
	unsigned int used;
	list_t**bucket;
	
}hashTable_t;

//MSG_BOX_T
typedef struct msg_box_s{
	int total_msg;
	int total_user;
	//int actual_size;
	//int max_size;
	//user_t**users;
	hashTable_t*database;
}msg_box_t;

//BOOLEAN
typedef enum bool_e{
	True=1,
	False=0
}Bool;
