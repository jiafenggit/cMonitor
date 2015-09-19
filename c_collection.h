#ifndef C_COLLECTION
#define C_COLLECTION

#include "c_str.h"
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <errno.h>
#include "cJSON.h"
#include "rehash_dict.h"

bool init_conf(void);
char *create_conf_json(void);
bool fetch_value(char *result_value, char *origin_str);
bool fetch_vaules_from_file(char *result_values,  char *file_path, short key_num, ...);
char * collect_sys_info(void);
char *convert_to_json(dict *collection_dict);
bool collect_cpu_info(cJSON *collection, dict *collection_dict);
bool collect_machine_info(cJSON *collection, dict *collection_dict);
bool collect_memory_info(cJSON *collection, dict *collection_dict);
bool collect_swap_info(cJSON *collection, dict *collection_dict);
bool collect_proc_info(cJSON *collection, dict *collection_dict);
bool collect_load_info(cJSON *collection, dict *collection_dict);
bool collect_network_info(cJSON *collection, dict *collection_dict);
bool collect_disk_info(cJSON *collection, dict *collection_dict);
char* collect_machine_ip(void);
char* collect_machine_uuid(void);


#endif // C_COLLECTION

