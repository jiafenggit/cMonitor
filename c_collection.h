#ifndef C_COLLECTION
#define C_COLLECTION

#include "c_str.h"
#include "cJSON.h"
#include "rehash_dict.h"
#include "configuration.h"
#include "macro_definition.h"

int old_cpu_use, old_cpu_total;
bool init_old_cpu_info(void);
bool fetch_value(char *result_value, char *origin_str);
bool fetch_vaules_from_file(char *result_values,  char *file_path, short key_num, ...);
void collect_host_info(char *sys_info_json_str);
char *convert_to_json(dict *collection_dict);
char* collect_machine_ip(void);
char* collect_machine_uuid(void);
int collect_mac_addr(char * mac, int len_limit);
bool collect_cpu_info(cJSON *collection, dict *collection_dict);
bool collect_machine_info(cJSON *collection, dict *collection_dict);
bool collect_memory_info(cJSON *collection, dict *collection_dict);
bool collect_swap_info(cJSON *collection, dict *collection_dict);
bool collect_proc_info(cJSON *collection, dict *collection_dict);
bool collect_load_info(cJSON *collection, dict *collection_dict);
bool collect_network_info(cJSON *collection, dict *collection_dict);
bool collect_disk_info(cJSON *collection, dict *collection_dict);
bool collect_custom_data(cJSON *conf_root, dict *collection_dict);
char *exec_shell_scripte(char *shell_interpreter, char *script_path);



#endif // C_COLLECTION

