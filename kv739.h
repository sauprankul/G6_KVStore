#ifdef __cplusplus
extern "C" {
#endif

int kv739_init(char *server_name); 
int kv739_shutdown(void); 
int kv739_get(char* key, char* value); 
int kv739_put(char* key, char* value, char* old_value); 

#ifdef __cplusplus
}
#endif