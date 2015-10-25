#include "configuration.h"

bool init_conf(void)
{
	char *conf_json = NULL;
	FILE *conf_fd = NULL;
	if (access(CONFIG_FILE_PATH, F_OK) != -1)
	{
		if (access(CONFIG_FILE_PATH, R_OK) != 0)
		{
			perror("No read permission.\n");
			return false;
		}
		return true;
	}
	else
	{
		conf_json = create_conf_json();
		printf("conf:%s\n", conf_json);
		if ((conf_fd = fopen(CONFIG_FILE_PATH, "a+")) == NULL)
		{
			perror("open conf file.");
			return false;
		}
		fputs(conf_json, conf_fd);
		fclose(conf_fd);
		free(conf_json);
	}
	return true;

}

char *create_conf_json(void)
{
	cJSON *root, *collection, *network, *heartbeat, *cluster, *custom;
	char *conf_json = NULL;

	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "collection", collection = cJSON_CreateObject());
	cJSON_AddTrueToObject(collection, "machine_type");
	cJSON_AddTrueToObject(collection, "boot_time");
	cJSON_AddTrueToObject(collection, "os_name");
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
	cJSON_AddTrueToObject(collection, "machine_ip");
	cJSON_AddTrueToObject(collection, "proc_total");
	cJSON_AddNumberToObject(collection, "sleep_time", 10);

	cJSON_AddItemToObject(root, "network", network = cJSON_CreateObject());
	cJSON_AddTrueToObject(network, "send");
	cJSON_AddTrueToObject(network, "recv");
	cJSON_AddStringToObject(network, "solider_multicast_add", "224.0.0.19");
	cJSON_AddStringToObject(network, "scale_out_multicast_add", "224.0.0.20");
	cJSON_AddNumberToObject(network, "listening port", 10241);

	cJSON_AddItemToObject(root, "cluster", cluster = cJSON_CreateObject());
	cJSON_AddFalseToObject(cluster, "master");
	cJSON_AddNumberToObject(cluster, "backup_officer_size", 3);
	cJSON_AddFalseToObject(cluster, "save_all_data");


	cJSON_AddItemToObject(root, "heartbeat", heartbeat = cJSON_CreateObject());
	cJSON_AddNumberToObject(heartbeat, "heartbeat_port", 12048);
	cJSON_AddNumberToObject(heartbeat, "time_out", 3);
	cJSON_AddNumberToObject(heartbeat, "sleep_time", 5);

	cJSON_AddItemToObject(root, "custom", custom = cJSON_CreateObject());
	//cJSON_AddFalseToObject(custom, "enable");
	cJSON_AddTrueToObject(custom, "enable");
	cJSON_AddStringToObject(custom, "shell_interpreter", "bash");
	cJSON_AddStringToObject(custom, "shell_dir", "/home/cf/cMonitor/shell/");


	conf_json = cJSON_Print(root);
	cJSON_Delete(root);
	return conf_json;
}

char *fetch_key_key_value_str(char *first_key, char *second_key)
{
	char *conf_json = NULL;
	cJSON *conf_root = NULL;
	cJSON *conf_node = NULL;
	char *value_buf;
	FILE *conf_fd = NULL;
	int conf_file_size = 0;

	if((conf_fd = fopen(CONFIG_FILE_PATH, "r")) == NULL)
	{
		perror("open conf file.");
		return NULL;
	}
	fseek(conf_fd, 0, SEEK_END);
	conf_file_size = ftell(conf_fd);
	fseek(conf_fd, 0, SEEK_SET);
	conf_json = (char *)calloc(conf_file_size + 1, sizeof(char));
	if (conf_json == NULL)
	{
		perror("calloc memory.");
		fclose(conf_fd);
		return NULL;
	}
	fread(conf_json, sizeof(char), conf_file_size, conf_fd);
	fclose(conf_fd);

	conf_root = cJSON_Parse(conf_json);
	conf_node = cJSON_GetObjectItem(conf_root, first_key);
	free(conf_json);
	conf_json = NULL;
	value_buf = (char *)calloc(128, sizeof(char));
	memset(value_buf, '\0', 128);
	memcpy(value_buf,cJSON_GetObjectItem(conf_node, second_key)->valuestring, strlen(cJSON_GetObjectItem(conf_node, second_key)->valuestring));
	cJSON_Delete(conf_root);

	return value_buf;
}

bool fetch_key_key_value_bool(char *first_key, char *second_key)
{
	char *conf_json = NULL;
	cJSON *conf_root = NULL;
	cJSON *conf_node = NULL;
	bool value_buf;
	FILE *conf_fd = NULL;
	int conf_file_size = 0;


	if((conf_fd = fopen(CONFIG_FILE_PATH, "r")) == NULL)
	{
		perror("open conf file.");
		return NULL;
	}
	fseek(conf_fd, 0, SEEK_END);
	conf_file_size = ftell(conf_fd);
	fseek(conf_fd, 0, SEEK_SET);
	conf_json = (char *)calloc(conf_file_size + 1, sizeof(char));
	if (conf_json == NULL)
	{
		perror("calloc memory.");
		fclose(conf_fd);
		return NULL;
	}
	fread(conf_json, sizeof(char), conf_file_size, conf_fd);
	fclose(conf_fd);

	conf_root = cJSON_Parse(conf_json);
	conf_node = cJSON_GetObjectItem(conf_root, first_key);
	free(conf_json);
	conf_json = NULL;
	value_buf = cJSON_GetObjectItem(conf_node, second_key)->type;
	cJSON_Delete(conf_root);
	return value_buf;
}

int fetch_key_key_value_int(char *first_key, char *second_key)
{
	char *conf_json = NULL;
	cJSON *conf_root = NULL;
	cJSON *conf_node = NULL;
	int value_buf;
	FILE *conf_fd = NULL;
	int conf_file_size = 0;


	if((conf_fd = fopen(CONFIG_FILE_PATH, "r")) == NULL)
	{
		perror("open conf file.");
		return NULL;
	}
	fseek(conf_fd, 0, SEEK_END);
	conf_file_size = ftell(conf_fd);
	fseek(conf_fd, 0, SEEK_SET);
	conf_json = (char *)calloc(conf_file_size + 1, sizeof(char));
	if (conf_json == NULL)
	{
		perror("calloc memory.");
		return NULL;
	}
	fread(conf_json, sizeof(char), conf_file_size, conf_fd);
	fclose(conf_fd);

	conf_root = cJSON_Parse(conf_json);
	conf_node = cJSON_GetObjectItem(conf_root, first_key);
	free(conf_json);
	conf_json = NULL;
	value_buf = cJSON_GetObjectItem(conf_node, second_key)->valueint;
	cJSON_Delete(conf_root);
	return value_buf;
}

