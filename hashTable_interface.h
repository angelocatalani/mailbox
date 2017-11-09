hashTable_t* hashTable_new();
Bool hashTable_add(hashTable_t*table,user_t* u);//loadfactor e realloc in caso + ritona 0 se l utente gia è presente
void hashTable_remove(hashTable_t*table,user_t* u);
user_t* hashTable_get(hashTable_t*table,char*name);
void hashTable_free(hashTable_t*table);
void hashTable_print(hashTable_t table);
unsigned long hashCode(unsigned char *str);
