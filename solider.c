#include "solider.h"



void activate_solider_collect(void)
{
	char *sys_info_dg = NULL;
	char *sys_info_json = NULL;

	while (true)
	{
		sys_info_json = collect_sys_info();
		sys_info_dg = mul_encap_datagram(RT_HOST, sys_info_json);
		mulcast_dg(sys_info_dg);
		free(sys_info_dg);
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


	if((conf_fd = fopen("/tmp/cMonitor/cCollection.conf.json", "r")) == NULL)
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
	int opt=1;
	setsockopt(mcast_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
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

void machine_scale_out(void)
{

	sleep(1);
	char *dg_message;
	dg_message = (char *)calloc(1024, sizeof(char));
	strcat(dg_message, "2051");
	strcat(dg_message, "|");
	char *uuid = collect_machine_uuid();
	char *machine_ip = collect_machine_ip();
	strcat(dg_message, uuid);
	strcat(dg_message, "|");
	strcat(dg_message, machine_ip);
	mulcast_scaleout_dg(dg_message);
	free(dg_message);
	dg_message = NULL;
	free(uuid);
	uuid = NULL;
	free(machine_ip);
	machine_ip = NULL;
}



void mulcast_scaleout_dg(char *data)
{
	struct sockaddr_in mcast_addr;
	int mcast_socket;

	mcast_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (mcast_socket == -1)
	{
		perror("mcast socket()");
		exit(-1);
	}
	int opt=1;
	setsockopt(mcast_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	memset(&mcast_addr, 0, sizeof(mcast_addr));
	mcast_addr.sin_family = AF_INET;
	char solider_mul_addr[20];
	char *fetch_value_buf = fetch_key_key_value_str("network", "scale_out_multicast_add");
	memset(solider_mul_addr, 0, sizeof(solider_mul_addr));
	memcpy(solider_mul_addr, fetch_value_buf, 16);
	free(fetch_value_buf);
	fetch_value_buf = NULL;

	mcast_addr.sin_addr.s_addr = inet_addr(solider_mul_addr);
	mcast_addr.sin_port = htons(SCALEOUT_MCAST_PORT);
	printf("add local host to cluster.\n");
	int status = sendto(mcast_socket, data, strlen(data), 0, (struct sockaddr*)&mcast_addr, sizeof(mcast_addr));
	if (status < 0)
	{
		perror("sendto()");
		exit(-1);
	}
	close(mcast_socket);
}


void activate_solider_merge(void)
{
	int mul_socket;
	struct sockaddr_in local_address;
	char buf[MAX_BUF_SIZE];
	dict *dg_dict = NULL;
	dict *cluster_rt_dict = NULL;

	if((mul_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		exit(0);
	}
	int opt=1;
	setsockopt(mul_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	memset(&local_address, 0, sizeof(local_address));
	local_address.sin_family = AF_INET;
	local_address.sin_addr.s_addr = htonl(INADDR_ANY);
	local_address.sin_port = htons(8192);

	if(bind(mul_socket, (struct sockaddr *)&local_address, sizeof(local_address)) < 0)
	{
		perror("bind");
		exit(0);
	}

	int loop = 1;
	if (setsockopt(mul_socket,IPPROTO_IP,IP_MULTICAST_LOOP,&loop, sizeof(loop)) < 0)
	{
		perror("IP_MULTICAST_LOOP");
		exit(0);
	}
	char solider_mul_addr[20];
	char *fetch_value_buf = fetch_key_key_value_str("network", "solider_multicast_add");
	memset(solider_mul_addr, 0, sizeof(solider_mul_addr));
	memcpy(solider_mul_addr, fetch_value_buf, 16);
	free(fetch_value_buf);
	fetch_value_buf = NULL;
	struct ip_mreq mrep;
	mrep.imr_multiaddr.s_addr = inet_addr(solider_mul_addr);
	mrep.imr_interface.s_addr = htonl(INADDR_ANY);
	//加入广播组
	if (setsockopt(mul_socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mrep, sizeof(mrep)) < 0)
	{
		perror("IP_ADD_MEMBERSHIP");
		exit(0);
	}
	cluster_rt_dict = create_dic();
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
		if (mul_parse_datagram(buf, dg_dict) == false)
		{
			printf("parse datagram error.\n");
			continue;
		}
		char *datagram= NULL;
		datagram = fetch_dictEntry(dg_dict, "datagram")->value.string_value;
		char *mmh_hash = murmurhash_str(datagram);
		if (strcmp(mmh_hash, fetch_dictEntry(dg_dict, "mmh")->value.string_value) != 0)
		{
			printf("datagram loss.\n");
			free(mmh_hash);
			mmh_hash = NULL;
			release_dict(dg_dict);
			dg_dict = NULL;
			continue;
		}
		switch (atoi(fetch_dictEntry(dg_dict, "type")->value.string_value)) {
		case RT_HOST:
		{
			merge_solider_rtdg(cluster_rt_dict, dg_dict, buf);
		}
			break;
		default:
			break;
		}
		free(mmh_hash);
		mmh_hash = NULL;
		release_dict(dg_dict);
		//sleep(fetch_key_key_value_int("collection", "sleep_time"));
	}
}

char *mul_encap_datagram(int dg_type, char *datagram)
{
	char *dg_message;
	char time_str[32];
	time_t time_now;
	memset(time_str, 0, sizeof(time_str));
	if (strlen(datagram) >= MAX_BUF_SIZE- (32*10))
	{
		printf("datagram is too long.\n");
		return NULL;
	}
	dg_message = (char *)calloc(MAX_BUF_SIZE, sizeof(char));
	switch (dg_type) {
	case RT_HOST:
	{
		char *uuid =  collect_machine_uuid();
		strcat(dg_message,uuid);
		free(uuid);
		uuid = NULL;
		strcat(dg_message, "|");
		strcat(dg_message, "0");
		strcat(dg_message, "|");
		char *mmh = murmurhash_str(datagram);
		strcat(dg_message, mmh);
		free(mmh);
		mmh = NULL;
		strcat(dg_message, "|");
		time(&time_now);
		sprintf(time_str, "%ld", (long)time_now);
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
		char *uuid =  collect_machine_uuid();
		strcat(dg_message,uuid);
		free(uuid);
		uuid = NULL;
		strcat(dg_message, "|");
		strcat(dg_message, "1");
		strcat(dg_message, "|");
		char *mmh = murmurhash_str(datagram);
		strcat(dg_message, mmh);
		free(mmh);
		mmh = NULL;
		strcat(dg_message, "|");
		time(&time_now);
		sprintf(time_str, "%ld", (long)time_now);
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
		char *uuid =  collect_machine_uuid();
		strcat(dg_message,uuid);
		free(uuid);
		uuid = NULL;
		strcat(dg_message, "|");
		strcat(dg_message, "2");
		strcat(dg_message, "|");
		char *mmh = murmurhash_str(datagram);
		strcat(dg_message, mmh);
		free(mmh);
		mmh = NULL;
		strcat(dg_message, "|");
		time(&time_now);
		sprintf(time_str, "%ld", (long)time_now);
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
		char *uuid =  collect_machine_uuid();
		strcat(dg_message,uuid);
		free(uuid);
		uuid = NULL;
		strcat(dg_message, "|");
		strcat(dg_message, "3");
		strcat(dg_message, "|");
		char *mmh = murmurhash_str(datagram);
		strcat(dg_message, mmh);
		free(mmh);
		mmh = NULL;
		strcat(dg_message, "|");
		time(&time_now);
		sprintf(time_str, "%ld", (long)time_now);
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
		char *uuid =  collect_machine_uuid();
		strcat(dg_message,uuid);
		free(uuid);
		uuid = NULL;
		strcat(dg_message, "|");
		strcat(dg_message, "4");
		strcat(dg_message, "|");
		char *mmh = murmurhash_str(datagram);
		strcat(dg_message, mmh);
		free(mmh);
		mmh = NULL;
		strcat(dg_message, "|");
		time(&time_now);
		sprintf(time_str, "%ld", (long)time_now);
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
		free(dg_message);
		break;
	}
	return NULL;

}

bool mul_parse_datagram(char *datagram, dict *dg_dict)
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
	return true;
}

bool merge_solider_rtdg(dict *cluster_rt_dict, dict *rt_dg_dict, char *buf)
{
	char *uuid = NULL;
	uuid = fetch_dictEntry(rt_dg_dict, "uuid")->value.string_value;
	if (exist_key(cluster_rt_dict, uuid) == false)
	{
		add_dict(cluster_rt_dict, uuid, STRINGTYPE, buf);
		return true;
	}
	else
	{
		cJSON *cluster_rt_root;
		char *sys_info_string = NULL;
		int key_index = 0;
		dictEntry *head = NULL;

		cluster_rt_root = cJSON_CreateObject();
		for (key_index = 0; key_index < cluster_rt_dict->hash_table[0].size; key_index++)
		{
			head = cluster_rt_dict->hash_table[0].table[key_index];
			while(head && strlen(head->key) != 0)
			{
				cJSON *rt_dg_root;
				char json_buf[MAX_BUF_SIZE];
				memset(json_buf, 0, sizeof(json_buf));
				split(json_buf, head->value.string_value, '|', 10);
				rt_dg_root = cJSON_Parse(json_buf);
				cJSON_AddItemToObject(cluster_rt_root, cJSON_GetObjectItem(rt_dg_root, "uuid")->valuestring, rt_dg_root);

				head = head->next;
			}
		}
		if(cluster_rt_dict->rehash_index != -1)
		{
			for (key_index = 0; key_index < cluster_rt_dict->hash_table[1].size; key_index++)
			{
			    head = NULL;
			    head = cluster_rt_dict->hash_table[1].table[key_index];
			    while(head && strlen(head->key) != 0 )
			    {
				    cJSON *rt_dg_root;
				    char json_buf[MAX_BUF_SIZE];
				    memset(json_buf, 0, sizeof(json_buf));
				    split(json_buf, head->value.string_value, '|', 10);
				    rt_dg_root = cJSON_Parse(json_buf);
				    cJSON_AddItemToObject(cluster_rt_root, cJSON_GetObjectItem(rt_dg_root, "uuid")->valuestring, rt_dg_root);
				    head = head->next;
			    }
			}
		}
		sys_info_string = cJSON_Print(cluster_rt_root);
		save_rr_dg(cluster_rt_root);
		free(sys_info_string);
		sys_info_string   = NULL;
		release_dict(cluster_rt_dict);
		cluster_rt_dict = NULL;
		cluster_rt_dict = create_dic();
		add_dict(cluster_rt_dict, uuid, STRINGTYPE, buf);
		return true;
	}
}

bool save_rr_dg(cJSON *cluster_rt_root)
{
	char *cluster_rr_dg = NULL;
	cJSON *cluster_rr_dg_root;
	FILE *cluster_rr_fd = NULL;
	int file_size;
	if (access("/tmp/cMonitor/rt_datagram.json", F_OK) != -1)
	{
		FILE *rt_dg_fd;
		if ((rt_dg_fd = fopen("/tmp/cMonitor/rt_datagram.json", "w")) == NULL)
		{
			perror("open rt datagram file.");
			cJSON_Delete(cluster_rt_root);
			return false;
		}
		char *rt_dg_buf = cJSON_Print(cluster_rt_root);
		fputs(rt_dg_buf, rt_dg_fd);
		fclose(rt_dg_fd);
		free(rt_dg_buf);
		save_rt_dg_to_all(cluster_rt_root);
	}
	else
	{
		FILE *rt_dg_fd;
		if ((rt_dg_fd = fopen("/tmp/cMonitor/rt_datagram.json", "a+")) == NULL)
		{
			perror("open rt datagram file.");
			cJSON_Delete(cluster_rt_root);
			return false;
		}
		char *rt_dg_buf = cJSON_Print(cluster_rt_root);
		fputs(rt_dg_buf, rt_dg_fd);
		fclose(rt_dg_fd);
		free(rt_dg_buf);
		save_rt_dg_to_all(cluster_rt_root);
	}

	if (access("/tmp/cMonitor/hour_datagram.json", F_OK) != -1)
	{
		if((cluster_rr_fd = fopen("/tmp/cMonitor/hour_datagram.json", "r")) == NULL)
		{
			perror("open rr datagram file.");
			fclose(cluster_rr_fd);
			cJSON_Delete(cluster_rt_root);
			return false;
		}
		fseek(cluster_rr_fd, 0, SEEK_END);
		file_size = ftell(cluster_rr_fd);
		fseek(cluster_rr_fd, 0, SEEK_SET);
		cluster_rr_dg = (char *)calloc(file_size + 1, sizeof(char));
		if (cluster_rr_dg == NULL)
		{
			perror("calloc memory.");
			fclose(cluster_rr_fd);
			cJSON_Delete(cluster_rt_root);
			return false;
		}
		fread(cluster_rr_dg, sizeof(char), file_size, cluster_rr_fd);
		fclose(cluster_rr_fd);

		cluster_rr_dg_root = cJSON_Parse(cluster_rr_dg);
		time_t time_now;
		time(&time_now);
		char time_str[32];
		memset(time_str, 0, 32);
		char head_time_str[32];
		memset(head_time_str, 0, 32);
		memcpy(head_time_str, cluster_rr_dg_root->child->string, strlen(cluster_rr_dg_root->child->string));
		sprintf(time_str, "%ld", (long)time_now);
		long head_time = atol(head_time_str);
		if (time_now - head_time >= 3600)
		{
			cJSON_DeleteItemFromObject(cluster_rr_dg_root, cluster_rr_dg_root->child->string);
		}
		cJSON_AddItemToObject(cluster_rr_dg_root,time_str, cluster_rt_root);
		if ((cluster_rr_fd = fopen("/tmp/cMonitor/hour_datagram.json", "w")) == NULL)
		{
			perror("open rr datagram file.");
			fclose(cluster_rr_fd);
			free(cluster_rr_dg);
			cJSON_Delete(cluster_rt_root);
			cluster_rr_dg = NULL;
			return false;
		}
		char *rt_dg_buf = cJSON_Print(cluster_rr_dg_root);
		fputs(rt_dg_buf, cluster_rr_fd);
		fclose(cluster_rr_fd);
		free(rt_dg_buf);
		free(cluster_rr_dg);
		cluster_rr_dg = NULL;
		cJSON_Delete(cluster_rr_dg_root);
		cluster_rr_dg_root = NULL;
		cluster_rt_root = NULL;
		return true;
	}
	else
	{
		cluster_rr_dg_root = cJSON_CreateObject();
		time_t time_now;
		time(&time_now);
		char time_str[32];
		memset(time_str, 0, sizeof(time_str));
		sprintf(time_str, "%ld", (long)time_now);
		cJSON_AddItemToObject(cluster_rr_dg_root, time_str, cluster_rt_root);
		if ((cluster_rr_fd = fopen("/tmp/cMonitor/hour_datagram.json", "a+")) == NULL)
		{
			perror("create rr datagram file.");
			fclose(cluster_rr_fd);
			free(cluster_rr_dg);
			cJSON_Delete(cluster_rt_root);
			cluster_rr_dg = NULL;
			return false;
		}
		char *rt_dg_buf = cJSON_Print(cluster_rr_dg_root);
		fputs(rt_dg_buf, cluster_rr_fd);
		fclose(cluster_rr_fd);
		free(rt_dg_buf);
		free(cluster_rr_dg);
		cluster_rr_dg = NULL;
		cJSON_Delete(cluster_rr_dg_root);
		cluster_rr_dg_root = NULL;
		cluster_rt_root = NULL;
		return true;
	}
}

void save_rt_dg_to_all(cJSON *rt_dg)
{
	char *cur_time;
	char *cur_uuid;
	char *cur_machine_ip;
	char *cur_dg_buf;
	char *cur_line_buf;
	cJSON *head;
	time_t time_now;
	FILE *all_dg_fd;

	if ((all_dg_fd = fopen("/tmp/cMonitor/all_datagram.data", "a+")) == NULL)
	{
		perror("create all datagram file.");
		exit(0);
	}
	time(&time_now);
	cur_time = (char *)calloc(16, sizeof(char));
	cur_uuid = (char *)calloc(16, sizeof(char));
	cur_machine_ip = (char *)calloc(16, sizeof(char));
	sprintf(cur_time, "%ld", (long)time_now);
	head = rt_dg->child;

	int rt_machine_size = 0;
	while (head)
	{
		memcpy(cur_uuid, head->string, strlen(head->string));
		memcpy(cur_machine_ip, cJSON_GetObjectItem(head, "machine_ip")->valuestring, strlen(cJSON_GetObjectItem(head, "machine_ip")->valuestring));
		cur_dg_buf = cJSON_Print(head);
		cur_line_buf = (char *)calloc(strlen(cur_dg_buf) + 48, sizeof(char));
		strcat(cur_line_buf, cur_time);
		strcat(cur_line_buf, "|");
		strcat(cur_line_buf, cur_uuid);
		strcat(cur_line_buf, "|");
		strcat(cur_line_buf, cur_machine_ip);
		strcat(cur_line_buf, "|");
		strcat(cur_line_buf, cur_dg_buf);
		if (strip(cur_line_buf) == false)
		{
			printf("strip cur line buf failed.\n");
			exit(0);
		}
		strcat(cur_line_buf, "\n");
		fputs(cur_line_buf, all_dg_fd);
		free(cur_dg_buf);
		cur_dg_buf = NULL;
		free(cur_line_buf);
		cur_line_buf = NULL;
		memset(cur_uuid, 0, strlen(cur_uuid));
		memset(cur_machine_ip, 0, strlen(cur_machine_ip));
		head = head->next;
		rt_machine_size++;
	}
	printf("%ssave real time datagram.\tmerge size:%d\n", ctime(&time_now), rt_machine_size);
	fclose(all_dg_fd);
	free(cur_time);
	cur_time = NULL;
	free(cur_uuid);
	cur_uuid = NULL;
	free(cur_machine_ip);
	cur_machine_ip = NULL;
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
	int opt=1;
	setsockopt(mul_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
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
void activate_solider_scaleout(void)
{
	int mul_socket;
	struct sockaddr_in local_address;

	if((mul_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		exit(0);
	}
	int opt=1;
	setsockopt(mul_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	memset(&local_address, 0, sizeof(local_address));
	local_address.sin_family = AF_INET;
	local_address.sin_addr.s_addr = htonl(INADDR_ANY);
	local_address.sin_port = htons(SCALEOUT_MCAST_PORT);

	if(bind(mul_socket, (struct sockaddr *)&local_address, sizeof(local_address)) < 0)
	{
		perror("bind");
		exit(0);
	}

	int loop = 1;
	if (setsockopt(mul_socket,IPPROTO_IP,IP_MULTICAST_LOOP,&loop, sizeof(loop)) < 0)
	{
		perror("IP_MULTICAST_LOOP");
		exit(0);
	}
	struct ip_mreq mrep;
	char solider_mul_addr[20];
	char *fetch_value_buf = fetch_key_key_value_str("network", "scale_out_multicast_add");
	memset(solider_mul_addr, 0, sizeof(solider_mul_addr));
	memcpy(solider_mul_addr, fetch_value_buf, 16);
	free(fetch_value_buf);
	fetch_value_buf = NULL;
	mrep.imr_multiaddr.s_addr = inet_addr(solider_mul_addr);
	mrep.imr_interface.s_addr = htonl(INADDR_ANY);
	//加入广播组
	if (setsockopt(mul_socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mrep, sizeof(mrep)) < 0)
	{
		perror("IP_ADD_MEMBERSHIP");
		exit(0);
	}

	while(1)
	{
		char buf[1024];
		memset(buf, 0, sizeof(buf));
		socklen_t address_len = sizeof(local_address);
		if(recvfrom(mul_socket, buf, 1024, 0,(struct sockaddr *)&local_address, &address_len) < 0)
		{
			perror("recvfrom");
		}
		char type[8], uuid[16], machine_ip[16];
		memset(type, 0, sizeof(type));
		memset(uuid, 0, sizeof(uuid));
		memset(machine_ip, 0, sizeof(machine_ip));
		split(type, buf , '|', 0);
		split(uuid, buf , '|', 1);
		split(machine_ip, buf , '|', 2);
		printf("add new machine: %s\n", machine_ip);
		if (atoi(type) == SCALEOUT_DG)
		{
			if (add_machine(uuid, machine_ip) == false)
			{
				printf("add a new machine: %s to monitor failed.\n", machine_ip);
				continue;
			}
		}
		if (atoi(type) == SYNC_ALIVE_MACHINE)
		{
			if (sync_alive_machines(uuid, machine_ip) == false)
			{
				printf("add a new machine: %s to monitor failed.\n", machine_ip);
				continue;
			}
		}
		if (atoi(type) == SYNC_DEAD_MACHINE)
		{
			if (sync_dead_machines(uuid, machine_ip) == false)
			{
				printf("del a machine: %s to monitor failed.\n", machine_ip);
				continue;
			}
		}

	}
	if(setsockopt(mul_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP,&mrep, sizeof(mrep)) < 0)
	{
		perror("setsockopt:IP_DROP_MEMBERSHIP");
	}
	close(mul_socket);
}

bool add_machine(char *uuid, char *machine_ip)
{
	char *dg_message;
	dg_message = (char *)calloc(64, sizeof(char));

	strcat(dg_message, "2052");
	strcat(dg_message, "|");
	strcat(dg_message, uuid);
	strcat(dg_message, "|");
	strcat(dg_message, machine_ip);
	// unix sock send
	char *recv = NULL;
	recv = send_and_recv_to_us(dg_message);
	if (atoi(recv) != SUCCESS)
	{
		printf("add machine:%s to unix sock server failed.\n", machine_ip);
		free(recv);
		recv = NULL;
		free(dg_message);
		dg_message = NULL;
		return false;
	}
	free(recv);
	recv = NULL;
	free(dg_message);
	dg_message = NULL;
	return true;
}

bool sync_alive_machines(char *uuid, char *machine_ip)
{
	char *dg_message;
	dg_message = (char *)calloc(64, sizeof(char));

	strcat(dg_message, "2055");
	strcat(dg_message, "|");
	strcat(dg_message, uuid);
	strcat(dg_message, "|");
	strcat(dg_message, machine_ip);
	// unix sock send
	char *recv = NULL;
	recv = send_and_recv_to_us(dg_message);
	if (atoi(recv) != SUCCESS)
	{
		printf("add machine:%s to unix sock server failed.\n", machine_ip);
		free(recv);
		recv = NULL;
		free(dg_message);
		dg_message = NULL;
		return false;
	}
	free(recv);
	recv = NULL;
	free(dg_message);
	dg_message = NULL;
	return true;
}

bool sync_dead_machines(char *uuid, char *machine_ip)
{
	char *dg_message;
	dg_message = (char *)calloc(64, sizeof(char));

	strcat(dg_message, "2056");
	strcat(dg_message, "|");
	strcat(dg_message, uuid);
	strcat(dg_message, "|");
	strcat(dg_message, machine_ip);
	// unix sock send
	char *recv = NULL;
	recv = send_and_recv_to_us(dg_message);
	if (atoi(recv) != SUCCESS)
	{
		printf("add machine:%s to unix sock server failed.\n", machine_ip);
		free(recv);
		recv = NULL;
		free(dg_message);
		dg_message = NULL;
		return false;
	}
	free(recv);
	recv = NULL;
	free(dg_message);
	dg_message = NULL;
	return true;
}

void activate_solider_listen(void)
{
	int listen_sock;
	struct sockaddr_in listen_addr;


	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	int opt=1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(fetch_key_key_value_int("network", "listening port"));
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listen_sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) == -1)
	{
	    perror("bind");
	    exit(0);
	}
	if (listen(listen_sock, 5) == -1)
	{
	    perror("listen");
	    exit(0);
	}
	char buf[DG_MAX_SIZE];

	while(true)
	{
	    struct sockaddr_in client_addr;
	    socklen_t len = sizeof(client_addr);

	    int client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &len);
	    if (client_sock < 0)
	    {
		    perror("accept.");
		    exit(0);
	    }
	    memset(buf, 0, sizeof(buf));
	    recv(client_sock, buf, sizeof(buf), 0);
	    char type[16];
	    memset(type, 0, sizeof(type));
	    if (split(type, buf, '|', 1) == false)
	    {
		    printf("split datagram error.'n");
		    close(client_sock);
		    continue;
	    }
	    switch (atoi(type)) {
	    case HEARTBEAT_DG:
	    {
		    respond_hb(client_sock, buf);
	    }
		    break;
	    case FETCH_RT_DG:
	    {
		    char *rt_dg_json = NULL;
		    FILE *rt_dg_fd = NULL;
		    if (access("/tmp/cMonitor/rt_datagram.json", R_OK) != 0)
		    {
			    printf("No read permission.\n");
			    exit(0);
		    }
		    if ((rt_dg_fd = fopen("/tmp/cMonitor/rt_datagram.json", "r")) == NULL)
		    {
			    perror("open alive machine file.");
			    exit(0);
		    }
		    fseek(rt_dg_fd, 0, SEEK_SET);
		    rt_dg_json = (char *)calloc(1024, sizeof(char));
		    if (rt_dg_json == NULL)
		    {
			    perror("calloc memory.");
			    fclose(rt_dg_fd);
			    exit(0);
		    }
		    while(fread(rt_dg_json, sizeof(char), 1024, rt_dg_fd))
		    {
			    printf("respond a FETCH_RT_DG request\n");
			    send(client_sock, rt_dg_json, strlen(rt_dg_json), 0);
			    memset(rt_dg_json, 0, 1024);
		    }
		    fclose(rt_dg_fd);
	    }
		    break;
	    default:
		    break;
	    }
	    close(client_sock);
	}
	close(listen_sock);
}

char *fetch_rt_dg_from_file(void)
{
	char *rt_dg_json = NULL;
	FILE *rt_dg_fd = NULL;

	if (access("/tmp/cMonitor/rt_datagram.json", R_OK) != 0)
	{
		printf("No read permission.\n");
		return false;
	}
	if ((rt_dg_fd = fopen("/tmp/cMonitor/rt_datagram.json", "r")) == NULL)
	{
		perror("open alive machine file.");
		return false;
	}
	fseek(rt_dg_fd, 0, SEEK_END);
	int alive_machine_file_size = ftell(rt_dg_fd);
	fseek(rt_dg_fd, 0, SEEK_SET);
	rt_dg_json = (char *)calloc(alive_machine_file_size + 1, sizeof(char));
	if (rt_dg_json == NULL)
	{
		perror("calloc memory.");
		fclose(rt_dg_fd);
		exit(0);
	}
	fread(rt_dg_json, sizeof(char), alive_machine_file_size, rt_dg_fd);
	fclose(rt_dg_fd);
	return rt_dg_json;
}



void respond_hb(int client_sock, char *buf)
{
	char *respond_dg = listen_encap_datagram(RESOND_HEARTBEAT);

	int ret = send(client_sock, respond_dg, strlen(respond_dg), 0);
	if (ret == -1)
	{
		printf("send error.\n");
		exit(0);
	}
	free(respond_dg);
	respond_dg = NULL;
	close(client_sock);
}

char *listen_encap_datagram(int type, ...)
{
	switch (type) {
	case RESOND_HEARTBEAT:
	{
		char *dg_message;
		dg_message = (char *)calloc(32, sizeof(char));
		char *uuid = collect_machine_uuid();
		strcat(dg_message, uuid);
		free(uuid);
		uuid = NULL;
		strcat(dg_message, "|");
		strcat(dg_message, "2049");
		return dg_message;
	}
		break;
	default:
		break;
	}
	return NULL;
}


//char *fetch_alive_machines(void)
//{
//	cJSON *machines_root;
//	machines_root = cJSON_CreateObject();
//}











