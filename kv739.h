#ifdef __cplusplus
extern "C" {
#endif

int kv739_init(char **server_name); 
int kv739_shutdown(void); 
int kv739_get(char* key, char* value); 
int kv739_put(char* key, char* value, char* old_value); 
int kv739_die(char* server_name, int clean);
int kv739_partition(char* server_name, char** reachable);

#ifdef __cplusplus
}
#endif