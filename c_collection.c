#include "c_collection.h"


bool init_conf(void)
{
	char *conf_json = NULL;
	FILE *conf_fd = NULL;
	if (access("/tmp/cCollection.conf.json", F_OK) != -1)
	{
		if (access("/tmp/cCollection.conf.json", R_OK) != 0)
		{
			printf("No read permission.\n");
			return false;
		}
		return true;
	}
	else
	{
		conf_json = create_conf_json();
		printf("conf:%s\n", conf_json);
		if ((conf_fd = fopen("/tmp/cCollection.conf.json", "a+")) == NULL)
		{
			perror("open conf file.");
			return false;
		}
		fputs(conf_json, conf_fd);
		fclose(conf_fd);
		free(conf_json);
	}
}

char *create_conf_json(void)
{
	cJSON *root, *collection, *network, *heartbeat, *solider, *officer, *cluster;
	char *conf_json = NULL;

	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "collection", collection = cJSON_CreateObject());
	cJSON_AddTrueToObject(collection, "machine_type");
	cJSON_AddTrueToObject(collection, "boot_time");
	cJSON_AddTrueToObject(collection, "os_name");
	cJSON_AddTrueToObject(collection, "host_ip");
	cJSON_AddTrueToObject(collection, "cpu_model_name");
	cJSON_AddTrueToObject(collection, "cpu_idle");
	cJSON_AddTrueToObject(collection, "cpu_nice");
	cJSON_AddTrueToObject(collection, "cpu_user");
	cJSON_AddTrueToObject(collection, "cpu_system");
	cJSON_AddTrueToObject(collection, "cpu_speed");
	cJSON_AddTrueToObject(collection, "cpu_num");
	cJSON_AddTrueToObject(collection, "cpu_utilization");
	cJSON_AddTrueToObject(collection, "load_one");
	cJSON_AddTrueToObject(collection, "load_five");
	cJSON_AddTrueToObject(collection, "load_fifteen");
	cJSON_AddTrueToObject(collection, "disk_total");
	cJSON_AddTrueToObject(collection, "disk_free");
	cJSON_AddTrueToObject(collection, "disk_utilization");
	cJSON_AddTrueToObject(collection, "mem_total");
	cJSON_AddTrueToObject(collection, "mem_free");
	cJSON_AddTrueToObject(collection, "mem_buffers");
	cJSON_AddTrueToObject(collection, "mem_cached");
	cJSON_AddTrueToObject(collection, "mem_utilization");
	cJSON_AddTrueToObject(collection, "swap_free");
	cJSON_AddTrueToObject(collection, "swap_total");
	cJSON_AddTrueToObject(collection, "swap_utilization");
	cJSON_AddTrueToObject(collection, "packages_out");
	cJSON_AddTrueToObject(collection, "packages_in");
	cJSON_AddTrueToObject(collection, "bytes_out");
	cJSON_AddTrueToObject(collection, "bytes_in");
	cJSON_AddTrueToObject(collection, "proc_total");
	cJSON_AddNumberToObject(collection, "sleep_time", 10);

	cJSON_AddItemToObject(root, "network", network = cJSON_CreateObject());
	cJSON_AddTrueToObject(network, "send");
	cJSON_AddTrueToObject(network, "recv");
	cJSON_AddStringToObject(network, "solider_multicast_add", "224.0.0.19");
	cJSON_AddStringToObject(network, "officer_multicast_add", "224.0.0.20");
	cJSON_AddStringToObject(network, "rrd_multicast_add", "224.0.0.21");
	cJSON_AddStringToObject(network, "scale_out_multicast_add", "224.0.0.22");

	cJSON_AddItemToObject(root, "solider", solider = cJSON_CreateObject());
	cJSON_AddNumberToObject(solider, "port", 10240);

	cJSON_AddItemToObject(root, "officer", officer = cJSON_CreateObject());
	cJSON_AddStringToObject(officer, "officer_ips", " ");

	cJSON_AddItemToObject(root, "cluster", cluster = cJSON_CreateObject());
	cJSON_AddNumberToObject(cluster, "group_size", 100);
	cJSON_AddNumberToObject(cluster, "backup_officer_size", 3);

	cJSON_AddItemToObject(root, "heartbeat", heartbeat = cJSON_CreateObject());
	cJSON_AddNumberToObject(heartbeat, "sleep_time", 5);

//	cJSON_AddItemToObject(root, "command", command = cJSON_CreateObject());
//	cJSON_AddStringToObject(command, "command", " ");

	conf_json = cJSON_Print(root);
	cJSON_Delete(root);
	return conf_json;
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
    }
    fclose(fstream);
    return true;
}

char *collect_sys_info(void)
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
		printf("init conf error.\n");
		exit(0);
	}
	if((conf_fd = fopen("/tmp/cCollection.conf.json", "r")) == NULL)
	{
		perror("open conf file.");
		fclose(conf_fd);
		exit(0);
	}
	fseek(conf_fd, 0, SEEK_END);
	conf_file_size = ftell(conf_fd);
	fseek(conf_fd, 0, SEEK_SET);
	conf_json = (char *)calloc(conf_file_size + 1, sizeof(char));
	if (conf_json == NULL)
	{
		perror("calloc memory.");
		fclose(conf_fd);
		exit(0);
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

	cJSON_Delete(conf_root);
	sys_info_json = convert_to_json(collection_dict);
	//printf("json:%s\nsize:%d\n", sys_info_json, strlen(sys_info_json) *sizeof(char));
	if (release_dict(collection_dict) == false)
	{
		printf("release dict error.\n");
	}
	free(conf_json);
	return sys_info_json;
}


char* convert_to_json(dict *collection_dict)
{
	cJSON *root;
	char *sys_info_string = NULL;
	int key_index = 0;
	dictEntry *head = NULL;

	root = cJSON_CreateObject();
	for (key_index == 0; key_index < collection_dict->hash_table[0].size; key_index++)
	{
		head = collection_dict->hash_table[0].table[key_index];
		while(head && strlen(head->key) != 0)
		{
			cJSON_AddStringToObject(root, head->key, head->value.string_value);
			head = head->next;
		}
	}
	if(collection_dict->rehash_index != -1)
	{
	    head = NULL;
	    for (key_index == 0; key_index < collection_dict->hash_table[1].size; key_index++)
	    {
		    head = collection_dict->hash_table[1].table[key_index];
		    while(head && strlen(head->key) != 0)
		    {
			    cJSON_AddStringToObject(root, head->key, head->value.string_value);
			    head = head->next;
		    }
	    }
	}
	sys_info_string = cJSON_Print(root);
	cJSON_Delete(root);
	return sys_info_string;
}

bool collect_cpu_info(cJSON *collection, dict *collection_dict)
{
	char *value = NULL;
	char *value_buf = NULL;
	FILE *fd;
	int count = 0;
	int denominator = 0;
	int numerator = 0;

	value = (char *)calloc(1024, sizeof(char));
	value_buf = (char *)calloc(1024, sizeof(char));
	if (cJSON_GetObjectItem(collection, "cpu_model_name")->type == true)
	{
		fetch_vaules_from_file(value_buf, "/proc/cpuinfo", 1, "model name");
		split(value, value_buf, ':', 1);
		if(add_dict(collection_dict, "cpu_model_name", 2, value) == false)
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
		if(add_dict(collection_dict, "cpu_num", 2, value) == false)
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
		if(add_dict(collection_dict, "cpu_speed", 2, value) == false)
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
		if(add_dict(collection_dict, "cpu_user", 2, value) == false)
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
		if(add_dict(collection_dict, "cpu_nice", 2, value) == false)
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
		if(add_dict(collection_dict, "cpu_system", 2, value) == false)
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
		if(add_dict(collection_dict, "cpu_idle", 2, value) == false)
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
		sprintf(value_buf, "%f", (float)numerator / (float)denominator);
		memcpy(value, value_buf, strlen(value_buf));

		if(add_dict(collection_dict, "cpu_utilization", 2, value) == false)
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
	memcpy(value,  machine_ip, 1024);
	machine_ip = NULL;
	if(add_dict(collection_dict, "machine_ip", 2, value) == false)
	{
		printf("dict add error.\n");
		return false;
	}
	memset(value, 0, strlen(value));
	tmp = collect_machine_uuid();
	if(add_dict(collection_dict, "uuid", 2, tmp) == false)
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
		if(add_dict(collection_dict, "boot_time", 2, value) == false)
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
		if(add_dict(collection_dict, "os_name", 2, value) == false)
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
	char *uuid_str = NULL;
	char *value_buf = NULL;
	char *value = NULL;
	char    mac[18];
	int        nRtn = collect_mac_addr(mac, sizeof(mac));
	if(nRtn < 0)
	{
		printf("collect mac error.\n");
		return NULL;

	}
	fprintf(stderr, "MAC ADDR: %s\n", mac);
	uuid_str = (char *)calloc(1024, sizeof(char));
	value = (char *)calloc(1024, sizeof(char));
	value_buf = (char *)calloc(1024, sizeof(char));
	strcat(uuid_str, mac);
	char *machine_ip =  collect_machine_ip();
	strcat(uuid_str, machine_ip);
	free(machine_ip);
	machine_ip = NULL;
	uuid = murmurhash(uuid_str, (uint32_t )strlen(uuid_str), MMHASH_SEED);
	memset(uuid_str, 0, sizeof(uuid_str));
	sprintf(uuid_str, "%ld", uuid);
	free(value);
	value = NULL;
	free(value_buf);
	value_buf = NULL;
	printf("uuid:%s\n", uuid_str);
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
	FILE *fd;
	int count = 0;
	int denominator = 0;
	int numerator = 0;

	value = (char *)calloc(1024, sizeof(char));
	value_buf = (char *)calloc(1024, sizeof(char));
	if (cJSON_GetObjectItem(collection, "mem_total")->type == true)
	{
		fetch_vaules_from_file(value_buf, "/proc/meminfo", 1, "MemTotal");
		split(value, value_buf, ':', 0);
		if(add_dict(collection_dict, "mem_total", 2, value) == false)
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
		if(add_dict(collection_dict, "mem_free", 2, value) == false)
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
		if(add_dict(collection_dict, "mem_buffers", 2, value) == false)
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
		if(add_dict(collection_dict, "mem_cached", 2, value) == false)
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
		sprintf(value, "%f", (float)numerator / (float)denominator);
		if(add_dict(collection_dict, "mem_utilization", 2, value) == false)
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
	FILE *fd;
	int count = 0;
	int denominator = 0;
	int numerator = 0;

	value = (char *)calloc(1024, sizeof(char));
	value_buf = (char *)calloc(1024, sizeof(char));
	if (cJSON_GetObjectItem(collection, "swap_total")->type == true)
	{
		fetch_vaules_from_file(value_buf, "/proc/meminfo", 1, "SwapTotal");
		split(value, value_buf, ':', 0);
		if(add_dict(collection_dict, "swap_total", 2, value) == false)
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
		if(add_dict(collection_dict, "swap_free", 2, value) == false)
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
		sprintf(value, "%f", (float)numerator / (float)denominator);
		if(add_dict(collection_dict, "swap_utilization", 2, value) == false)
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
		if(add_dict(collection_dict, "load_one", 2, value) == false)
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
		if(add_dict(collection_dict, "load_five", 2, value) == false)
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
		if(add_dict(collection_dict, "load_fifteen", 2, value) == false)
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
		if(add_dict(collection_dict, "proc_total", 2, value) == false)
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
	int count = 0;
	int denominator = 0;
	int numerator = 0;

	value = (char *)calloc(1024, sizeof(char));
	value_buf = (char *)calloc(1024, sizeof(char));
	if (cJSON_GetObjectItem(collection, "bytes_in")->type == true)
	{
		if ((fd = fopen("/proc/net/dev", "r")) == NULL)
		{
			perror("read /proc/net/dev.");
			return false;
		}
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 1);
		if(add_dict(collection_dict, "bytes_in", 2, value) == false)
		{
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
			return false;
		}
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 2);
		if(add_dict(collection_dict, "packages_in", 2, value) == false)
		{
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
			return false;
		}
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 9);
		if(add_dict(collection_dict, "bytes_out", 2, value) == false)
		{
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
			return false;
		}
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		memset(value_buf, 0, strlen(value_buf));
		fgets(value_buf, 1024, fd);
		fclose(fd);
		split(value, value_buf, ' ', 10);
		if(add_dict(collection_dict, "packages_out", 2, value) == false)
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
	memcpy(command_buf, "df -l >> ", 10);
	time(&current_time);
	sprintf(current_time_str, "%d", current_time);
	sprintf(file_name, "%ld", murmurhash(current_time_str, (uint32_t)strlen(current_time_str), MMHASH_SEED));
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
		if(add_dict(collection_dict, "disk_total", 2, value) == false)
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
		if(add_dict(collection_dict, "disk_free", 2, value) == false)
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
		if(add_dict(collection_dict, "disk_utilization", 2, value) == false)
		{
			printf("dict add error.\n");
			return false;
		}
		memset(value, 0, strlen(value));
		memset(value_buf, 0, strlen(value_buf));
	}
	if (remove(file_name) != 0)
	{
		perror('remove file.');
		exit(0);
	}
	free(value);
	value = NULL;
	free(value_buf);
	value_buf = NULL;
	return true;
}
