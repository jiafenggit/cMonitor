#include "solider.h"



void activate_solider_collect(void)
{
	char *sys_info_dg = NULL;
	char *sys_info_json = NULL;

	while (true)
	{
		sys_info_json = collect_sys_info();
		sys_info_dg = encap_datagram(RT_HOST, sys_info_json);
		printf("sys info dg:%s\n", sys_info_dg);
		mulcast_dg(sys_info_dg);
		free(sys_info_json);
		sys_info_json = NULL;
		sleep(fetch_key_key_value_int("collection", "sleep_time"));
	}
}

void mulcast_dg(char *json_data)
{
	struct sockaddr_in mcast_addr;
	int mcast_socket;
	char *conf_json = NULL;
	cJSON *conf_root = NULL;
	cJSON *conf_node = NULL;
	FILE *conf_fd = NULL;
	int conf_file_size = 0;


	if((conf_fd = fopen("/home/cf/conf/cCollection.conf.json", "r")) == NULL)
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
	conf_node = cJSON_GetObjectItem(conf_root, "network");
	free(conf_json);
	conf_json = NULL;

	mcast_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (mcast_socket == -1)
	{
		perror("mcast socket()");
		cJSON_Delete(conf_root);
		exit(-1);
	}
	memset(&mcast_addr, 0, sizeof(mcast_addr));
	mcast_addr.sin_family = AF_INET;
	mcast_addr.sin_addr.s_addr = inet_addr(cJSON_GetObjectItem(conf_node, "solider_multicast_add")->valuestring);
	mcast_addr.sin_port = htons(MCAST_PORT);

	int status = sendto(mcast_socket, json_data, strlen(json_data), 0, (struct sockaddr*)&mcast_addr, sizeof(mcast_addr));
	if (status < 0)
	{
		perror("sendto()");
		cJSON_Delete(conf_root);
		exit(-1);
	}
	close(mcast_socket);
	cJSON_Delete(conf_root);
}

void mulcast_solider_dg(char *data)
{
	struct sockaddr_in mcast_addr;
	int mcast_socket;

	mcast_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (mcast_socket == -1)
	{
		perror("mcast socket()");
		exit(-1);
	}
	memset(&mcast_addr, 0, sizeof(mcast_addr));
	mcast_addr.sin_family = AF_INET;
	char solider_mul_addr[20];
	memcpy(solider_mul_addr, fetch_key_key_value_str("network", "officer_multicast_add"), 16);
	mcast_addr.sin_addr.s_addr = inet_addr(solider_mul_addr);
	mcast_addr.sin_port = htons(MCAST_PORT + 12);
	printf("#########solider mulcast datagram:%s\n", data);
	int status = sendto(mcast_socket, data, strlen(data), 0, (struct sockaddr*)&mcast_addr, sizeof(mcast_addr));
	if (status < 0)
	{
		perror("sendto()");
		exit(-1);
	}
	close(mcast_socket);
}

char *fetch_key_key_value_str(char *first_key, char *second_key)
{
	char *conf_json = NULL;
	cJSON *conf_root = NULL;
	cJSON *conf_node = NULL;
	char value_buf[128];
	FILE *conf_fd = NULL;
	int conf_file_size = 0;


	if((conf_fd = fopen("/home/cf/conf/cCollection.conf.json", "r")) == NULL)
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
	conf_node = cJSON_GetObjectItem(conf_root, first_key);
	free(conf_json);
	conf_json = NULL;
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


	if((conf_fd = fopen("/home/cf/conf/cCollection.conf.json", "r")) == NULL)
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


	if((conf_fd = fopen("/home/cf/conf/cCollection.conf.json", "r")) == NULL)
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
	conf_node = cJSON_GetObjectItem(conf_root, first_key);
	free(conf_json);
	conf_json = NULL;
	value_buf = cJSON_GetObjectItem(conf_node, second_key)->valueint;
	cJSON_Delete(conf_root);
	return value_buf;
}

void activate_solider_merge(void)
{
	int mul_socket;
	int count  = 0;
	struct sockaddr_in local_address;
	char buf[MAX_BUF_SIZE];
	dict *dg_dict = NULL;
	dict *solider_rt_dict = NULL;
	char type[32];

	if((mul_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		return -1;
	}
	memset(&local_address, 0, sizeof(local_address));
	local_address.sin_family = AF_INET;
	local_address.sin_addr.s_addr = htonl(INADDR_ANY);
	local_address.sin_port = htons(8192);

	if(bind(mul_socket, (struct sockaddr *)&local_address, sizeof(local_address)) < 0)
	{
		perror("bind");
		return -1;
	}

	int loop = 1;
	if (setsockopt(mul_socket,IPPROTO_IP,IP_MULTICAST_LOOP,&loop, sizeof(loop)) < 0)
	{
		perror("IP_MULTICAST_LOOP");
		return 0;
	}
	char solider_mul_addr[20];
	memcpy(solider_mul_addr, fetch_key_key_value_str("network", "solider_multicast_add"), 16);
	struct ip_mreq mrep;
	mrep.imr_multiaddr.s_addr = inet_addr(solider_mul_addr);
	mrep.imr_interface.s_addr = htonl(INADDR_ANY);
	//加入广播组
	if (setsockopt(mul_socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mrep, sizeof(mrep)) < 0)
	{
		perror("IP_ADD_MEMBERSHIP");
		return 0;
	}
	solider_rt_dict = create_dic();
	add_dict(solider_rt_dict, "merge size", INTTYPE, 1);
	while(true)
	{
		dg_dict = create_dic();
		socklen_t address_len = sizeof(local_address);
		memset(buf, 0, MAX_BUF_SIZE);
		if(recvfrom(mul_socket, buf, MAX_BUF_SIZE, 0,(struct sockaddr *)&local_address, &address_len) < 0)
		{
			perror("recvfrom");
			if(setsockopt(mul_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP,&mrep, sizeof(mrep)) < 0)
			{
				perror("setsockopt:IP_DROP_MEMBERSHIP");
			}
			close(mul_socket);
			exit(0);
		}
		printf("host msg: %s\n", buf);
		if (parse_datagram(buf, dg_dict) == false)
		{
			printf("parse datagram error.\n");
			release_dict(dg_dict);
			dg_dict = NULL;
			continue;
		}
		fetch_dict_value(dg_dict, "type", STRINGTYPE, type);
//		char datagram[MAX_BUF_SIZE];
//		fetch_dict_value(dg_dict, "datagram", STRINGTYPE, datagram);
//		char mmh[16];
//		fetch_dict_value(dg_dict, "mmh", STRINGTYPE, mmh);
//		if (strcmp(murmurhash_str(datagram), mmh) != 0)
//		{
//			printf("datagram loss.\n");
//			printf("mmh")
//			release_dict(dg_dict);
//			dg_dict = NULL;
//			continue;
//		}
		switch (atoi(type)) {
		case RT_HOST:
		{
			merge_solider_rtdg(solider_rt_dict, dg_dict, buf);
		}
			break;
		case RT_GROUP:
		{
			;
		}
			break;
		default:
			break;
		}
		release_dict(dg_dict);
		//sleep(fetch_key_key_value_int("collection", "sleep_time"));
	}
}

char *encap_datagram(int dg_type, char *datagram)
{
	char dg_message[MAX_BUF_SIZE];
	char time_str[32];
	time_t time_now;
	memset(dg_message, 0,  MAX_BUF_SIZE);
	if (strlen(datagram) >= MAX_BUF_SIZE- (32*10))
	{
		printf("datagram is too long.\n");
		return NULL;
	}
	switch (dg_type) {
	case RT_HOST:
	{
		strcat(dg_message, collect_machine_uuid());
		strcat(dg_message, "|");
		strcat(dg_message, "0");
		strcat(dg_message, "|");
		strcat(dg_message, murmurhash_str(datagram));
		strcat(dg_message, "|");
		time(&time_now);
		sprintf(time_str, "%d", time_now);
		strcat(dg_message, time_str);
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, datagram);
		return dg_message;
	}
		break;
	case RT_GROUP:
	{
		strcat(dg_message, collect_machine_uuid());
		strcat(dg_message, "|");
		strcat(dg_message, "1");
		strcat(dg_message, "|");
		strcat(dg_message, murmurhash_str(datagram));
		strcat(dg_message, "|");
		time(&time_now);
		sprintf(time_str, "%d", time_now);
		strcat(dg_message, time_str);
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, datagram);
		return dg_message;
	}
		break;
	case RT_CLUSTER:
	{
		strcat(dg_message, collect_machine_uuid());
		strcat(dg_message, "|");
		strcat(dg_message, "2");
		strcat(dg_message, "|");
		strcat(dg_message, murmurhash_str(datagram));
		strcat(dg_message, "|");
		time(&time_now);
		sprintf(time_str, "%d", time_now);
		strcat(dg_message, time_str);
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, datagram);
		return dg_message;
	}
		break;
	case RT_HEARTBEAT:
	{
		strcat(dg_message, collect_machine_uuid());
		strcat(dg_message, "|");
		strcat(dg_message, "3");
		strcat(dg_message, "|");
		strcat(dg_message, murmurhash_str(datagram));
		strcat(dg_message, "|");
		time(&time_now);
		sprintf(time_str, "%d", time_now);
		strcat(dg_message, time_str);
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, datagram);
		return dg_message;
	}
		break;
	case RT_OFFICER:
	{
		strcat(dg_message, collect_machine_uuid());
		strcat(dg_message, "|");
		strcat(dg_message, "4");
		strcat(dg_message, "|");
		strcat(dg_message, murmurhash_str(datagram));
		strcat(dg_message, "|");
		time(&time_now);
		sprintf(time_str, "%d", time_now);
		strcat(dg_message, time_str);
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, "NULL");
		strcat(dg_message, "|");
		strcat(dg_message, datagram);
		return dg_message;
	}
		break;
	default:
		break;
	}

}

bool parse_datagram(char *datagram, dict *dg_dict)
{
	char *value;

	value = (char *)calloc(MAX_BUF_SIZE, sizeof(char));
	memset(value, 0, MAX_BUF_SIZE);
	split(value, datagram, '|', 0);
	if (add_dict(dg_dict, "uuid", STRINGTYPE, value) == false)
	{
		printf("add dict error.\n");
		free(value);
		return false;
	}
	memset(value, 0, strlen(value));
	split(value, datagram, '|', 1);
	if (add_dict(dg_dict, "type", STRINGTYPE, value) == false)
	{
		printf("add dict error.\n");
		free(value);
		return false;
	}
	memset(value, 0, strlen(value));
	split(value, datagram, '|', 2);
	if (add_dict(dg_dict, "mmh", STRINGTYPE, value) == false)
	{
		printf("add dict error.\n");
		free(value);
		return false;
	}
	memset(value, 0, strlen(value));
	split(value, datagram, '|', 3);
	if (add_dict(dg_dict, "time", STRINGTYPE, value) == false)
	{
		printf("add dict error.\n");
		free(value);
		return false;
	}
	memset(value, 0, strlen(value));
	split(value, datagram, '|', 10);
	if (add_dict(dg_dict, "datagram", STRINGTYPE, value) == false)
	{
		printf("add dict error.\n");
		free(value);
		return false;
	}
	free(value);
}

bool merge_solider_rtdg(dict *solider_rt_dict, dict *dg_dict, char *buf)
{
	char uuid[16];
	fetch_dict_value(dg_dict, "uuid", STRINGTYPE, uuid);
	if (exist_key(solider_rt_dict, "flag") == false)
	{
		add_dict(solider_rt_dict, "flag", STRINGTYPE, uuid);
	}
	if (exist_key(solider_rt_dict, uuid) == false)
	{
		add_dict(solider_rt_dict, uuid, STRINGTYPE, buf);
		return true;
	}
	else
	{
		cJSON *group_rt_root;
		char *sys_info_string = NULL;
		int key_index = 0;
		dictEntry *head = NULL;


		group_rt_root = cJSON_CreateObject();
		for (key_index == 0; key_index < solider_rt_dict->hash_table[0].size; key_index++)
		{
			head = solider_rt_dict->hash_table[0].table[key_index];
			while(head && strlen(head->key) != 0 && strcmp(head->key, "flag") != 0 && strcmp(head->key, "merge size") != 0)
			{
				cJSON *solider_rt_root;
				char json_buf[MAX_BUF_SIZE];
				split(json_buf, head->value.string_value, '|', 10);
				printf("key:%s\n", head->key);
				printf("value:%s\n", json_buf);
				solider_rt_root = cJSON_Parse(json_buf);
				time_t time_now;
				time(&time_now);
				char time_str[32];
				sprintf(time_str, "%d", time_now);
				printf("group_rt_root:%s\n", cJSON_Print(group_rt_root));
				cJSON_AddItemToObject(group_rt_root, time_str, solider_rt_root);
				printf("group_rt_root:%s\n", cJSON_Print(group_rt_root));
				head = head->next;
			}
		}
		if(solider_rt_dict->rehash_index != -1)
		{
			for (key_index == 0; key_index < solider_rt_dict->hash_table[0].size; key_index++)
			{
			    head = NULL;
			    head = solider_rt_dict->hash_table[1].table[key_index];
			    while(head && strlen(head->key) != 0 && strcmp(head->key, "flag") != 0 && strcmp(head->key, "merge size") != 0)
			    {
				    cJSON *solider_rt_root;
				    char json_buf[MAX_BUF_SIZE];
				    split(json_buf, head->value.string_value, '|', 10);
				    solider_rt_root = cJSON_Parse(json_buf);
				    time_t time_now;
				    time(&time_now);
				    char time_str[32];
				    sprintf(time_str, "%d", time_now);
				    cJSON_AddItemToObject(group_rt_root, time_str, solider_rt_root);
				    head = head->next;
			    }
			}
		}
		sys_info_string = cJSON_Print(group_rt_root);
		save_rr_dg(group_rt_root);
		mulcast_solider_dg(sys_info_string);
		free(sys_info_string);
		release_dict(solider_rt_dict);
		solider_rt_dict = NULL;
		solider_rt_dict = create_dic();
		add_dict(solider_rt_dict, "merge size", INTTYPE, 1);
		add_dict(solider_rt_dict, uuid, STRINGTYPE, buf);
		return true;
	}
}

bool save_rr_dg(cJSON *rt_dg)
{
	char *rr_dg = NULL;
	cJSON *rr_dg_root;
	FILE *rr_fd = NULL;
	char *file_name[16];
	int file_size;
	if (access("rr_datagram.json", F_OK) != -1)
	{
		if((rr_fd = fopen("rr_datagram.json", "r")) == NULL)
		{
			perror("open rr datagram file.");
			fclose(rr_fd);
			cJSON_Delete(rt_dg);
			exit(0);
		}
		fseek(rr_fd, 0, SEEK_END);
		file_size = ftell(rr_fd);
		fseek(rr_fd, 0, SEEK_SET);
		rr_dg = (char *)calloc(file_size + 1, sizeof(char));
		if (rr_dg == NULL)
		{
			perror("calloc memory.");
			fclose(rr_fd);
			cJSON_Delete(rt_dg);
			exit(0);
		}
		fread(rr_dg, sizeof(char), file_size, rr_fd);
		fclose(rr_fd);

		rr_dg_root = cJSON_Parse(rr_dg);
		time_t time_now;
		time(&time_now);
		char time_str[32];
		sprintf(time_str, "%d", time_now);
		cJSON_AddItemToObject(rr_dg_root,time_str, rt_dg);
		if ((rr_fd = fopen("rr_datagram.json", "w")) == NULL)
		{
			perror("open rr datagram file.");
			fclose(rr_fd);
			free(rr_dg);
			cJSON_Delete(rt_dg);
			rr_dg = NULL;
			return false;
		}
		char *rt_dg_buf = cJSON_Print(rr_dg_root);
		fputs(rt_dg_buf, rr_fd);
		fclose(rr_fd);
		free(rt_dg_buf);
		free(rr_dg);
		rr_dg = NULL;
		cJSON_Delete(rr_dg_root);
		rr_dg_root = NULL;
		return true;
	}
	else
	{
		rr_dg_root = cJSON_CreateObject();
		time_t time_now;
		time(&time_now);
		char time_str[32];
		sprintf(time_str, "%d", time_now);
		cJSON_AddItemToObject(rr_dg_root, time_str, rt_dg);
		if ((rr_fd = fopen("rr_datagram.json", "a+")) == NULL)
		{
			perror("create rr datagram file.");
			fclose(rr_fd);
			free(rr_dg);
			cJSON_Delete(rt_dg);
			rr_dg = NULL;
			return false;
		}
		char *rt_dg_buf = cJSON_Print(rr_dg_root);
		fputs(rt_dg_buf, rr_fd);
		fclose(rr_fd);
		free(rt_dg_buf);
		free(rr_dg);
		rr_dg = NULL;
		cJSON_Delete(rr_dg_root);
		rr_dg_root = NULL;
		return true;
	}
}


int mul_test(void)
{
	int mul_socket;
	int count  = 0;
	struct sockaddr_in local_address;

    if((mul_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
	perror("socket");
	return -1;
    }
    memset(&local_address, 0, sizeof(local_address));
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(8192);

    if(bind(mul_socket, (struct sockaddr *)&local_address, sizeof(local_address)) < 0)
    {
	perror("bind");
	return -1;
    }

    int loop = 1;
    if (setsockopt(mul_socket,IPPROTO_IP,IP_MULTICAST_LOOP,&loop, sizeof(loop)) < 0)
    {
	perror("IP_MULTICAST_LOOP");
	return 0;
    }

    struct ip_mreq mrep;
    mrep.imr_multiaddr.s_addr = inet_addr("224.0.0.19");
    mrep.imr_interface.s_addr = htonl(INADDR_ANY);
    //加入广播组
    if (setsockopt(mul_socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mrep, sizeof(mrep)) < 0)
    {
	perror("IP_ADD_MEMBERSHIP");
	return 0;
    }

    while(1)
    {
	char buf[4096];
	memset(buf, 0, sizeof(buf));
	socklen_t address_len = sizeof(local_address);
	if(recvfrom(mul_socket, buf, 4096, 0,(struct sockaddr *)&local_address, &address_len) < 0)
	{
	    perror("recvfrom");
	}
	printf("msg from server: %s\n", buf);
	printf("count:%d\n", count++);
	sleep(1);
    }
	if(setsockopt(mul_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP,&mrep, sizeof(mrep)) < 0)
    {
	perror("setsockopt:IP_DROP_MEMBERSHIP");
    }
    close(mul_socket);
    return 0;
}


















