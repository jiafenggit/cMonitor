#include "c_collection.h"


bool init_old_cpu_info(void)
{
	char *value = NULL;
	char *value_buf = NULL;
	FILE *fd;

	old_cpu_use = 0;
	old_cpu_total = 0;
	if ((fd = fopen("/proc/stat", "r")) == NULL)
	{
		perror("read /proc/stat.");
		return false;
	}
	value = (char *)calloc(1024, sizeof(char));
	value_buf = (char *)calloc(1024, sizeof(char));
	fgets(value_buf, 1024, fd);
	fclose(fd);
	split(value, value_buf, ' ', 1);
	old_cpu_use += atoi(value);
	memset(value, 0, strlen(value));
	split(value, value_buf, ' ', 2);
	old_cpu_use += atoi(value);
	memset(value, 0, strlen(value));
	split(value, value_buf, ' ', 3);
	old_cpu_use += atoi(value);
	memset(value, 0, strlen(value));
	split(value, value_buf, ' ', 4);
	old_cpu_total = atoi(value) + old_cpu_use;

	free(value);
	value = NULL;
	free(value_buf);
	value_buf = NULL;
	sleep(1);
	return true;
}



bool fetch_value(char *result_value, char *origin_str)
{
    if (split(result_value, origin_str, ':', 1) == false)
    {
	printf("split failed.\n");
	return false;
    }
    if(strip(result_value) == false)
    {
	printf("strip string failed.\n");
	return false;
    }
    return true;
}

bool fetch_vaules_from_file(char *result_values,  char *file_path, short key_num, ...)
{
    FILE *fstream;
    va_list arg_ptr;
    short key_index = 0;
    short read_char_num = 1024;
    char line_content[1024];
    char key_array[20][32];
    char *result_value_buf = NULL;

    if (*file_path == '\0' || strlen(file_path) == 0 || file_path == NULL)
    {
	printf("Empty file path.\n");
	return false;
    }
    memset(line_content, 0, sizeof(line_content));
    va_start(arg_ptr, key_num);
    for (; key_index < key_num;key_index++)
    {
	memcpy(key_array[key_index], va_arg(arg_ptr, char *), 32);
	if (strlen(key_array[key_index]) == 0)
	{
	    printf("Empty key.\n");
	    return false;
	}
    }
    fstream = fopen(file_path, "r");
    if (fstream == NULL)
    {
	perror("open file failed\t");
	fclose(fstream);
	return false;
    }
    while(fgets(line_content, read_char_num, fstream) != NULL)
    {
	for (key_index = 0; key_index < key_num; key_index++)
	{
	    if (strncasecmp(line_content, key_array[key_index], strlen(key_array[key_index])) == 0)
	    {
		result_value_buf = (char *)calloc(1024, sizeof(char));
		if(fetch_value(result_value_buf, line_content) == false)
		{
		    printf("fetch value failed.\n");
		    free(result_value_buf);
		    result_value_buf = NULL;
		    fclose(fstream);
		    return false;
		}
		else
		{
		    strcat(result_value_buf, ":");
		    strcat(result_values, result_value_buf);
		    free(result_value_buf);
		    result_value_buf = NULL;
		}
	    }
	}
	memset(line_content, 0, sizeof(line_content));
    }
    fclose(fstream);
    return true;
}

void collect_sys_info(char *sys_info_json_arg)
{
	char *conf_json = NULL;
	FILE *conf_fd = NULL;
	int conf_file_size = 0;
	cJSON *conf_root = NULL;
	cJSON *collection = NULL;
	dict *collection_dict = NULL;
	char *sys_info_json = NULL;


	if (init_conf() == false)
	{
		printf("Exec collect_sys_info function failed.\n");
		return;
	}
	if((conf_fd = fopen(CONFIG_FILE_PATH, "r")) == NULL)
	{
		perror("Exec collect_sys_info/fopen function failed.");
		return;
	}
	fseek(conf_fd, 0, SEEK_END);
	conf_file_size = ftell(conf_fd);
	fseek(conf_fd, 0, SEEK_SET);
	conf_json = (char *)calloc(conf_file_size + 1, sizeof(char));
	if (conf_json == NULL)
	{
		perror("Exec collect_sys_info/calloc function failed.");
		free(conf_json);
		fclose(conf_fd);
		return;
	}
	fread(conf_json, sizeof(char), conf_file_size, conf_fd);
	fclose(conf_fd);

	conf_root = cJSON_Parse(conf_json);
	collection = cJSON_GetObjectItem(conf_root, "collection");
	collection_dict = create_dic();
	collect_cpu_info(collection, collection_dict);
	collect_machine_info(collection, collection_dict);
	collect_memory_info(collection, collection_dict);
	collect_swap_info(collection, collection_dict);
	collect_load_info(collection, collection_dict);
	collect_proc_info(collection, collection_dict);
	collect_network_info(collection, collection_dict);
	collect_disk_info(collection, collection_dict);

	collect_custom_data(conf_root, collection_dict);

	cJSON_Delete(conf_root);
	sys_info_json = convert_to_json(collection_dict);
	if (release_dict(collection_dict) == false)
	{
		printf("Exec collect_sys_info/release_dict function failed.\n");
		free(conf_json);
		return;
	}
	memcpy(sys_info_json_arg, sys_info_json, strlen(sys_info_json));
	printf("collect_sys_info succeeded.size:%ld\n", strlen(sys_info_json) *sizeof(char));
	free(conf_json);
	free(sys_info_json);
}


char* convert_to_json(dict *collection_dict)
{
	cJSON *root;
	char *sys_info_string = NULL;
	int key_index = 0;
	dictEntry *head = NULL;

	root = cJSON_CreateObject();
	for (key_index = 0; key_index < collection_dict->hash_table[0].size; key_index++)
	{
		head = collection_dict->hash_table[0].table[key_index];
		while(head && strlen(head->key) != 0)
		{
			switch (head->value_type) {
			case BOOL_FALSE:
			{
				cJSON_AddFalseToObject(root, head->key);
			}
				break;
			case BOOL_TRUE:
			{
				cJSON_AddTrueToObject(root, head->key);
			}
				break;
			case INTTYPE:
			{
				cJSON_AddNumberToObject(root, head->key, head->value.num_value);
			}
				break;
			case DECIMALTYPE:
			{
				cJSON_AddNumberToObject(root, head->key, head->value.decimal_value);
			}
				break;
			case STRINGTYPE:
			{
				cJSON_AddStringToObject(root, head->key, head->value.string_value);
			}
				break;
			default:
				break;
			}
			head = head->next;
		}
	}
//	if(collection_dict->rehash_index != -1)
//	{
//	    head = NULL;
//	    for (key_index = 0; key_index < collection_dict->hash_table[1].size; key_index++)
//	    {
//		    head = collection_dict->hash_table[1].table[key_index];
//		    while(head && strlen(head->key) != 0)
//		    {
//			    cJSON_AddStringToObject(root, head->key, head->value.string_value);
//			    head = head->next;
//		    }
//	    }
//	}
	sys_info_string = cJSON_Print(root);
	cJSON_Delete(root);
	return sys_info_string;
}

bool collect_cpu_info(cJSON *collection, dict *collection_dict)
{
	char *value = NULL;
	char *value_buf = NULL;
	FILE *fd;
	int denominator = 0;
	int numerator = 0;

	value = (char *)calloc(1024, sizeof(char));
	value_buf = (char *)calloc(1024, sizeof(char));
	if (cJSON_GetObjectItem(collection, "cpu_model_name")->type == true)
	{
		fetch_vaules_from_file(value_buf, "/proc/cpuinfo", 1, "model name");
		split(value, value_buf, ':', 1);
		if(add_dict(collection_dict, "cpu_model_name", STRINGTYPE, value) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "cpu_num")->type == true)
	{
		fetch_vaules_from_file(value_buf, "/proc/cpuinfo", 1, "cpu cores");
		split(value, value_buf, ':', 0);
		if(add_dict(collection_dict, "cpu_num", INTTYPE, atoi(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "cpu_speed")->type == true)
	{
		fetch_vaules_from_file(value_buf, "/proc/cpuinfo", 1, "cpu MHz");
		split(value, value_buf, ':', 0);
		if(add_dict(collection_dict, "cpu_speed", DECIMALTYPE, atof(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "cpu_user")->type == true)
	{
		if ((fd = fopen("/proc/stat", "r")) == NULL)
		{
			perror("read /proc/stat.");
			return false;
		}
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 1);
		if(add_dict(collection_dict, "cpu_user", INTTYPE, atoi(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "cpu_nice")->type == true)
	{
		if ((fd = fopen("/proc/stat", "r")) == NULL)
		{
			perror("read /proc/stat.");
			return false;
		}
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 2);
		if(add_dict(collection_dict, "cpu_nice", INTTYPE, atoi(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "cpu_system")->type == true)
	{
		if ((fd = fopen("/proc/stat", "r")) == NULL)
		{
			perror("read /proc/stat.");
			return false;
		}
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 3);
		if(add_dict(collection_dict, "cpu_system", INTTYPE, atoi(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "cpu_idle")->type == true)
	{
		if ((fd = fopen("/proc/stat", "r")) == NULL)
		{
			perror("read /proc/stat.");
			return false;
		}
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 4);
		if(add_dict(collection_dict, "cpu_idle", INTTYPE, atoi(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "cpu_utilization")->type == true)
	{
		if ((fd = fopen("/proc/stat", "r")) == NULL)
		{
			perror("read /proc/stat.");
			return false;
		}
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 1);
		numerator += atoi(value);
		memset(value, 0, strlen(value));
		split(value, value_buf, ' ', 2);
		numerator += atoi(value);
		memset(value, 0, strlen(value));
		split(value, value_buf, ' ', 3);
		numerator += atoi(value);
		memset(value, 0, strlen(value));
		split(value, value_buf, ' ', 4);
		denominator = atoi(value) + numerator;
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
		sprintf(value_buf, "%f", ((float)numerator - old_cpu_use)/ ((float)denominator - old_cpu_total));
		old_cpu_use = numerator;
		old_cpu_total = denominator;
		memcpy(value, value_buf, strlen(value_buf));

		if(add_dict(collection_dict, "cpu_utilization", DECIMALTYPE, atof(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	free(value);
	value = NULL;
	free(value_buf);
	value_buf = NULL;
	return true;
}

bool collect_machine_info(cJSON *collection, dict *collection_dict){
	char *value = NULL;
	char *value_buf = NULL;
	char *tmp = NULL;
	FILE *fd;

	value = (char *)calloc(1024, sizeof(char));
	value_buf = (char *)calloc(1024, sizeof(char));
	char *machine_ip = collect_machine_ip();
	memcpy(value,  machine_ip, strlen(machine_ip));
	free(machine_ip);
	machine_ip = NULL;
	if(add_dict(collection_dict, "machine_ip", STRINGTYPE, value) == false)
	{
		printf("dict add error.\n");
		return false;
	}
	memset(value, 0, strlen(value));
	tmp = collect_machine_uuid();
	if(add_dict(collection_dict, "uuid", INTTYPE, atoi(tmp)) == false)
	{
		printf("dict add error.\n");
		return false;
	}
	free(tmp);
//	if (cJSON_GetObjectItem(collection, "machine_type")->type == true)
//	{
//		fetch_vaules_from_file(value_buf, "/proc/cpuinfo", 1, "flag");
//		if(strstr(value_buf, "lm") == NULL)
//		{
//			memcpy(value, "32 bit", 6);
//		}
//		else
//		{
//			memcpy(value, "64 bit", 6);
//		}
//		printf("\n\nfetch:%s\n", value);
//		if(add_dict(collection_dict, "machine_type", 2, value) == false)
//		{
//			printf("dict add error.\n");
//			return false;
//		}
//		memset(value, 0, strlen(value));
//		memset(value_buf, 0, strlen(value_buf));
//	}

	if (cJSON_GetObjectItem(collection, "boot_time")->type == true)
	{
		if ((fd = fopen("/proc/uptime", "r")) == NULL)
		{
			perror("read /proc/uptime.");
			return false;
		}
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 0);
		if(add_dict(collection_dict, "boot_time", DECIMALTYPE, atof(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "os_name")->type == true)
	{
		if ((fd = fopen("/etc/issue", "r")) == NULL)
		{
			perror("read /etc/issue.");
			return false;
		}
		fgets(value, 1024, fd);
		fclose(fd);
		r_strip(value, " \n \l \\ n \n");
		if(add_dict(collection_dict, "os_name", STRINGTYPE, value) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}

	free(value);
	value = NULL;
	free(value_buf);
	value_buf = NULL;
	return true;
}

char* collect_machine_ip(void)
{
    int sock_get_ip;
    char *ipaddr;

    struct   sockaddr_in *sin;
    struct   ifreq ifr_ip;

    ipaddr = (char *)calloc(32, sizeof(char));
    if ((sock_get_ip=socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
	perror("socket:");
	return NULL;
    }

    memset(&ifr_ip, 0, sizeof(ifr_ip));
    strncpy(ifr_ip.ifr_name, "eth0", sizeof(ifr_ip.ifr_name) - 1);

    if( ioctl( sock_get_ip, SIOCGIFADDR, &ifr_ip) < 0 )
    {
	    perror("ioctl");
	    return NULL;
    }
    sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;
    strcpy(ipaddr,inet_ntoa(sin->sin_addr));
    close(sock_get_ip);
    return ipaddr;
}

char* collect_machine_uuid(void)
{
	uint32_t uuid = 0;
	char uuid_str[1024];
	char *value_buf = NULL;
	char *value = NULL;
	char    mac[18];
	memset(mac, 0, sizeof(mac));
	int        nRtn = collect_mac_addr(mac, sizeof(mac));
	if(nRtn < 0)
	{
		printf("collect mac error.\n");
		return NULL;

	}
	memset(uuid_str, 0, sizeof(uuid));
	value = (char *)calloc(1024, sizeof(char));
	value_buf = (char *)calloc(1024, sizeof(char));
	strcat(uuid_str, mac);
	char *machine_ip =  collect_machine_ip();
	strcat(uuid_str, machine_ip);
	free(machine_ip);
	machine_ip = NULL;
	uuid = murmurhash(uuid_str, (uint32_t )strlen(uuid_str), MMHASH_SEED);
	memset(uuid_str, 0, sizeof(uuid_str));
	sprintf(uuid_str, "%ld", (long)uuid);
	free(value);
	value = NULL;
	free(value_buf);
	value_buf = NULL;
	return murmurhash_str(uuid_str);
}

int collect_mac_addr(char * mac, int len_limit)
{
    struct ifreq ifreq;
    int sock;

    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
	perror ("socket");
	return -1;
    }
    strcpy (ifreq.ifr_name, "eth0");    //only get eth0

    if (ioctl (sock, SIOCGIFHWADDR, &ifreq) < 0)
    {
	perror ("ioctl");
	return -1;
    }
    close(sock);
    return snprintf (mac, len_limit, "%X:%X:%X:%X:%X:%X", (unsigned char) ifreq.ifr_hwaddr.sa_data[0], (unsigned char) ifreq.ifr_hwaddr.sa_data[1], (unsigned char) ifreq.ifr_hwaddr.sa_data[2], (unsigned char) ifreq.ifr_hwaddr.sa_data[3], (unsigned char) ifreq.ifr_hwaddr.sa_data[4], (unsigned char) ifreq.ifr_hwaddr.sa_data[5]);
}

bool collect_memory_info(cJSON *collection, dict *collection_dict)
{
	char *value = NULL;
	char *value_buf = NULL;
	int denominator = 0;
	int numerator = 0;
	int buffer_cache = 0;

	value = (char *)calloc(1024, sizeof(char));
	value_buf = (char *)calloc(1024, sizeof(char));
	if (cJSON_GetObjectItem(collection, "mem_total")->type == true)
	{
		fetch_vaules_from_file(value_buf, "/proc/meminfo", 1, "MemTotal");
		split(value, value_buf, ':', 0);
		r_strip(value, "kB");
		if(add_dict(collection_dict, "mem_total", INTTYPE, atoi(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "mem_free")->type == true)
	{
		fetch_vaules_from_file(value_buf, "/proc/meminfo", 1, "MemFree");
		split(value, value_buf, ':', 0);
		r_strip(value, "kB");
		if(add_dict(collection_dict, "mem_free", INTTYPE, atoi(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "mem_buffers")->type == true)
	{
		fetch_vaules_from_file(value_buf, "/proc/meminfo", 1, "Buffers");
		split(value, value_buf, ':', 0);
		buffer_cache += atoi(value);
		r_strip(value, "kB");
		if(add_dict(collection_dict, "mem_buffers", INTTYPE, atoi(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "mem_cached")->type == true)
	{
		fetch_vaules_from_file(value_buf, "/proc/meminfo", 1, "Cached");
		split(value, value_buf, ':', 0);
		buffer_cache += atoi(value);
		r_strip(value, "kB");
		if(add_dict(collection_dict, "mem_cached", INTTYPE, atoi(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "mem_utilization")->type == true)
	{
		fetch_vaules_from_file(value_buf, "/proc/meminfo", 1, "MemTotal");
		split(value, value_buf, ':', 0);
		memset(value_buf, 0, strlen(value_buf));
		memcpy(value_buf, value, strlen(value) - 2);
		denominator = atoi(value_buf);
		memset(value_buf, 0, strlen(value_buf));
		fetch_vaules_from_file(value_buf, "/proc/meminfo", 1, "MemFree");
		split(value, value_buf, ':', 0);
		memset(value_buf, 0, strlen(value_buf));
		memcpy(value_buf, value, strlen(value) - 2);
		numerator = atoi(value_buf);
		memset(value, 0, strlen(value));
		sprintf(value, "%f", 1.0 - ((float)numerator + buffer_cache )/ (float)denominator);
		if(add_dict(collection_dict, "mem_utilization", DECIMALTYPE, atof(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}

	free(value);
	value = NULL;
	free(value_buf);
	value_buf = NULL;
	return true;
}

bool collect_swap_info(cJSON *collection, dict *collection_dict)
{
	char *value = NULL;
	char *value_buf = NULL;
	int denominator = 0;
	int numerator = 0;

	value = (char *)calloc(1024, sizeof(char));
	value_buf = (char *)calloc(1024, sizeof(char));
	if (cJSON_GetObjectItem(collection, "swap_total")->type == true)
	{
		fetch_vaules_from_file(value_buf, "/proc/meminfo", 1, "SwapTotal");
		split(value, value_buf, ':', 0);
		r_strip(value, "kB");
		if(add_dict(collection_dict, "swap_total", INTTYPE, atoi(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "swap_free")->type == true)
	{
		fetch_vaules_from_file(value_buf, "/proc/meminfo", 1, "SwapFree");
		split(value, value_buf, ':', 0);
		r_strip(value, "kB");
		if(add_dict(collection_dict, "swap_free", INTTYPE, atoi(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "swap_utilization")->type == true)
	{
		fetch_vaules_from_file(value_buf, "/proc/meminfo", 1, "SwapTotal");
		split(value, value_buf, ':', 0);
		memset(value_buf, 0, strlen(value_buf));
		memcpy(value_buf, value, strlen(value) - 2);
		denominator = atoi(value_buf);
		memset(value_buf, 0, strlen(value_buf));
		fetch_vaules_from_file(value_buf, "/proc/meminfo", 1, "SwapFree");
		split(value, value_buf, ':', 0);
		memset(value_buf, 0, strlen(value_buf));
		memcpy(value_buf, value, strlen(value) - 2);
		numerator = atoi(value_buf);
		memset(value, 0, strlen(value));
		sprintf(value, "%f", 1 - (float)numerator / (float)denominator);
		if(add_dict(collection_dict, "swap_utilization", DECIMALTYPE, atof(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}

	free(value);
	value = NULL;
	free(value_buf);
	value_buf = NULL;
	return true;
}

bool collect_load_info(cJSON *collection, dict *collection_dict)
{
	char *value = NULL;
	char *value_buf = NULL;
	FILE *fd;
	value = (char *)calloc(1024, sizeof(char));
	value_buf = (char *)calloc(1024, sizeof(char));
	if (cJSON_GetObjectItem(collection, "load_one")->type == true)
	{
		if ((fd = fopen("/proc/loadavg", "r")) == NULL)
		{
			perror("read /proc/loadavg.");
			return false;
		}
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 0);
		if(add_dict(collection_dict, "load_one", DECIMALTYPE, atof(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "load_five")->type == true)
	{
		if ((fd = fopen("/proc/loadavg", "r")) == NULL)
		{
			perror("read /proc/loadavg.");
			return false;
		}
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 1);
		if(add_dict(collection_dict, "load_five", DECIMALTYPE, atof(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "load_fifteen")->type == true)
	{
		if ((fd = fopen("/proc/loadavg", "r")) == NULL)
		{
			perror("read /proc/loadavg.");
			return false;
		}
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 2);
		if(add_dict(collection_dict, "load_fifteen", DECIMALTYPE, atof(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	free(value);
	value = NULL;
	free(value_buf);
	value_buf = NULL;
	return true;
}

bool collect_proc_info(cJSON *collection, dict *collection_dict)
{
	char *value = NULL;
	char *value_buf = NULL;
	FILE *fd;
	value = (char *)calloc(1024, sizeof(char));
	value_buf = (char *)calloc(1024, sizeof(char));
	if (cJSON_GetObjectItem(collection, "proc_total")->type == true)
	{
		if ((fd = fopen("/proc/loadavg", "r")) == NULL)
		{
			perror("read /proc/loadavg.");
			return false;
		}
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 3);
		memset(value_buf, 0, strlen(value_buf));
		memcpy(value_buf, value, strlen(value));
		memset(value, 0, strlen(value));
		split(value, value_buf, '/', 1);
		if(add_dict(collection_dict, "proc_total", INTTYPE, atoi(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	free(value);
	value = NULL;
	free(value_buf);
	value_buf = NULL;
	return true;
}

bool collect_network_info(cJSON *collection, dict *collection_dict)
{
	char *value = NULL;
	char *value_buf = NULL;
	FILE *fd;


	value = (char *)calloc(1024, sizeof(char));
	value_buf = (char *)calloc(1024, sizeof(char));
	if (cJSON_GetObjectItem(collection, "bytes_in")->type == true)
	{
		if ((fd = fopen("/proc/net/dev", "r")) == NULL)
		{
			perror("read /proc/net/dev.");
			free(value);
			value = NULL;
			free(value_buf);
			value_buf = NULL;
			return false;
		}
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 1);
		if(add_dict(collection_dict, "bytes_in", INTTYPE, atoi(value)) == false)
		{
			free(value);
			value = NULL;
			free(value_buf);
			value_buf = NULL;
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "packages_in")->type == true)
	{
		if ((fd = fopen("/proc/net/dev", "r")) == NULL)
		{
			perror("read /proc/net/dev.");
			free(value);
			value = NULL;
			free(value_buf);
			value_buf = NULL;
			return false;
		}
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 2);
		if(add_dict(collection_dict, "packages_in", INTTYPE, atoi(value)) == false)
		{
			free(value);
			value = NULL;
			free(value_buf);
			value_buf = NULL;
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "bytes_out")->type == true)
	{
		if ((fd = fopen("/proc/net/dev", "r")) == NULL)
		{
			perror("read /proc/net/dev.");
			free(value);
			value = NULL;
			free(value_buf);
			value_buf = NULL;
			return false;
		}
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 9);
		if(add_dict(collection_dict, "bytes_out", INTTYPE, atoi(value)) == false)
		{
			free(value);
			value = NULL;
			free(value_buf);
			value_buf = NULL;
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "packages_out")->type == true)
	{
		if ((fd = fopen("/proc/net/dev", "r")) == NULL)
		{
			perror("read /proc/net/dev.");
			free(value);
			value = NULL;
			free(value_buf);
			value_buf = NULL;
			return false;
		}
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 10);
		if(add_dict(collection_dict, "packages_out", INTTYPE, atoi(value)) == false)
		{
			free(value);
			value = NULL;
			free(value_buf);
			value_buf = NULL;
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	free(value);
	value = NULL;
	free(value_buf);
	value_buf = NULL;
	return true;
}

bool collect_disk_info(cJSON *collection, dict *collection_dict)
{
	char *value = NULL;
	char *value_buf = NULL;
	char *command_buf = NULL;
	char file_name[20];
	time_t current_time;
	char *current_time_str = NULL;
	FILE *fd;
	value = (char *)calloc(1024, sizeof(char));
	value_buf = (char *)calloc(1024, sizeof(char));
	current_time_str = (char *)calloc(1024, sizeof(char));
	command_buf = (char *)calloc(1024, sizeof(char));
	memset(file_name, 0, sizeof(file_name));
	memcpy(command_buf, "df -l >> ", 10);
	time(&current_time);
	sprintf(current_time_str, "%ld", (long)current_time);
	sprintf(file_name, "%ld", (long)murmurhash(current_time_str, (uint32_t)strlen(current_time_str), MMHASH_SEED));
	strcat(command_buf, file_name);
	system(command_buf);
	free(current_time_str);
	current_time_str = NULL;
	free(command_buf);
	command_buf = NULL;
	if (cJSON_GetObjectItem(collection, "disk_total")->type == true)
	{
		if ((fd = fopen(file_name, "r")) == NULL)
		{
			perror("read file.");
			return false;
		}
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 1);
		if(add_dict(collection_dict, "disk_total", INTTYPE, atoi(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "disk_free")->type == true)
	{
		if ((fd = fopen(file_name, "r")) == NULL)
		{
			perror("read file.");
			return false;
		}
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 3);
		if(add_dict(collection_dict, "disk_free", INTTYPE, atoi(value)) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (cJSON_GetObjectItem(collection, "disk_utilization")->type == true)
	{
		if ((fd = fopen(file_name, "r")) == NULL)
		{
			perror("read file.");
			return false;
		}
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 4);
		r_strip(value, "%");
		if(add_dict(collection_dict, "disk_utilization", DECIMALTYPE, atof(value) / 100) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (remove(file_name) != 0)
	{
		perror("remove file.");
		return false;
	}
	free(value);
	value = NULL;
	free(value_buf);
	value_buf = NULL;
	return true;
}


bool collect_custom_data(cJSON *conf_root, dict *collection_dict)
{
	cJSON *custom = NULL;
	custom = cJSON_GetObjectItem(conf_root, "custom");
	if (cJSON_GetObjectItem(custom, "enable")->type == false)
	{
		return true;
	}
	cJSON *custom_node = NULL;
	custom_node = custom->child->next;
	DIR *shell_dir;
	struct dirent *dir_element;
	struct stat element_stat;

	if (!(shell_dir = opendir(custom_node->next->valuestring)))
	{
		perror("Exec collect_custom_data/opendir function error.");
		return false;
	}
	while ((dir_element = readdir(shell_dir)) != NULL)
	{
		memset(&element_stat, 0, sizeof(element_stat));
		if (strncmp(dir_element->d_name, ".", 1) == 0)
			continue;
		char *dir_element_path = NULL;
		dir_element_path = (char *)calloc(strlen(custom_node->next->valuestring) + strlen(dir_element->d_name) + 1, sizeof(char));
		strcat(dir_element_path, custom_node->next->valuestring);
		strcat(dir_element_path, dir_element->d_name);
		if (stat(dir_element_path, &element_stat) != 0)
		{
			perror("Exec collect_custom_data/stat function error.");
			return false;
		}
		if (S_ISDIR(element_stat.st_mode))
		{
			free(dir_element_path);
			dir_element_path = NULL;
			continue;
		}
		else
		{
			char *exec_reply = NULL;
			exec_reply = exec_shell_scripte(custom_node->valuestring, dir_element_path);
			char *type = NULL;
			char *key = NULL;
			type = str_split(exec_reply, '|', 1);
			key = str_split(exec_reply, '|', 0);
			switch (atoi(type)) {
			case BOOL_FALSE:
			{
				if(add_dict(collection_dict, key, BOOL_FALSE) == false)
				{
					printf("dict add error.\n");
					return false;
				}
			}
				break;
			case BOOL_TRUE:
			{
				if(add_dict(collection_dict, key, BOOL_TRUE) == false)
				{
					printf("dict add error.\n");
					return false;
				}
			}
				break;
			case INTTYPE:
			{
				char *value = NULL;
				value = str_split(exec_reply, '|', 2);
				if(add_dict(collection_dict, key, INTTYPE, atoi(value)) == false)
				{
					printf("dict add error.\n");
					free(value);
					value = NULL;
					return false;
				}
				free(value);
				value = NULL;
			}
				break;
			case DECIMALTYPE:
			{
				char *value = NULL;
				value = str_split(exec_reply, '|', 2);
				if(add_dict(collection_dict, key, DECIMALTYPE, atof(value)) == false)
				{
					printf("dict add error.\n");
					free(value);
					value = NULL;
					return false;
				}
				free(value);
				value = NULL;
			}
				break;
			case STRINGTYPE:
			{
				char *value = NULL;
				value = str_split(exec_reply, '|', 2);
				if(add_dict(collection_dict, key, STRINGTYPE, value) == false)
				{
					printf("dict add error.\n");
					free(value);
					value = NULL;
					return false;
				}
				free(value);
				value = NULL;
			}
				break;
			default:
				break;
			}
			free(type);
			type = NULL;
			free(key);
			key = NULL;
			free(dir_element_path);
			dir_element_path = NULL;
			free(exec_reply);
			exec_reply = NULL;
		}
	}
	closedir(shell_dir);
//	while(custom_node)
//	{
//		FILE *fd;
//		char *key = custom_node->string;
//		char *value = custom_node->valuestring;
//		char *command_str = NULL;
//		char *command_value = NULL;
//		command_str = (char *)calloc(strlen(value) + 16, sizeof(char));
//		strcat(command_str, value);
//		strcat(command_str, " >> command.data ");
//		system(command_str);
//		if ((fd = fopen("command.data", "r")) == NULL)
//		{
//			perror("read file.");
//			return false;
//		}
//		command_value = (char *) calloc(64, sizeof(char));
//		fgets(command_value, 64, fd);
//		fclose(fd);
//		strip(command_value);
//		if(add_dict(collection_dict, key, STRINGTYPE, command_value) == false)
//		{
//			printf("dict add error.\n");
//			return false;
//		}
//		if (remove("command.data") != 0)
//		{
//			perror("remove file.");
//			return false;
//		}
//		free(command_str);
//		command_str = NULL;
//		free(command_value);
//		command_value = NULL;
//		custom_node = custom_node->next;
//	}
	return true;
}

char *exec_shell_scripte(char *shell_interpreter, char *script_path)
{
	FILE *shell_reply_stream;
	char *exec_reply = NULL;
	char *shell_code = NULL;
	shell_code = (char *)calloc(strlen(script_path) + 10, sizeof(char));
	strcat(shell_code, shell_interpreter);
	strcat(shell_code, " ");
	strcat(shell_code, script_path);
	shell_reply_stream = popen(shell_code, "r");
	fseek(shell_reply_stream, 0, SEEK_END);
	int reply_size = ftell(shell_reply_stream);
	fseek(shell_reply_stream, 0, SEEK_SET);
	exec_reply = (char *)calloc(reply_size + 1, sizeof(char));
	fread(exec_reply, sizeof(char), reply_size, shell_reply_stream);
	fclose(shell_reply_stream);
	free(shell_code);
	shell_code = NULL;
	r_strip(exec_reply, "\n");
	return exec_reply;
}
