#include "monitor_data_multicast.h"


/**
 * [激活监控数据多播组，该模块用于向共享同一个多播组的集群分发监控数据]
 */
void activate_monitor_data_multicast(void)
{
	int mul_socket;
	struct sockaddr_in local_address;
	char buf[MAX_BUF_SIZE];
	dict *dg_dict = NULL;
	dict *cluster_rt_dict = NULL;

	if((mul_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Exec activate_solider_merge/socket function failed");
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
		perror("Exec activate_solider_merge/bind function failed");
		exit(0);
	}

	int loop = 1;
	if (setsockopt(mul_socket,IPPROTO_IP,IP_MULTICAST_LOOP,&loop, sizeof(loop)) < 0)
	{
		perror("Exec activate_solider_merge/setsockopt IP_MULTICAST_LOOP function failed");
		exit(0);
	}
	char solider_mul_addr[20];
	char *fetch_value_buf = fetch_key_key_value_str("network", "solider_multicast_add");
	if (fetch_value_buf == NULL)
	{
		printf("Exec activate_solider_merge/fetch_key_key_value_str function failed.\n");
		exit(0);
	}
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
		perror("Exec activate_solider_merge/setsockopt IP_ADD_MEMBERSHIP function failed");
		exit(0);
	}
	cluster_rt_dict = create_dic();
	printf("Activate solider merge services.\n");
	while(true)
	{
		dg_dict = create_dic();
		socklen_t address_len = sizeof(local_address);
		memset(buf, 0, MAX_BUF_SIZE);
		if(recvfrom(mul_socket, buf, MAX_BUF_SIZE, 0,(struct sockaddr *)&local_address, &address_len) < 0)
		{
			perror("Exec activate_solider_merge/recvfrom function failed");
			if(setsockopt(mul_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP,&mrep, sizeof(mrep)) < 0)
			{
				perror("Exec activate_solider_merge/setsockopt IP_DROP_MEMBERSHIP function failed");
			}
			close(mul_socket);
			continue;
		}
		if (mul_parse_datagram(buf, dg_dict) == false)
		{
			printf("Exec activate_solider_merge/mul_parse_datagram function failed.\n");
			continue;
		}
		char *datagram= NULL;
		datagram = fetch_dictEntry(dg_dict, "datagram")->value.string_value;
		char *mmh_hash = murmurhash_str(datagram);
		printf("%s\t %s\n",fetch_dictEntry(dg_dict, "mmh")->value.string_value , mmh_hash);
		if (strcmp(mmh_hash, fetch_dictEntry(dg_dict, "mmh")->value.string_value) != 0)
		{
			printf("Exec activate_solider_merge/fetch_dictEntry function failed.\n");
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
		printf("Recv data from mulcat & Merge data to File.\n");
		//sleep(fetch_key_key_value_int("collection", "sleep_time"));
	}
}

/**
 * [解析数据报，并将获取到的信息添加到dict]
 * @param 待解析的数据报
 * @param 存放解析信息的dict
 * @return [函数是否成功执行]
 */
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

/**
 * [若数据采集满，则合并数据到本地文件；反之，将数据添加到dict中]
 * @param 保存当前采集周期集群监控数据的dict
 * @param 保存接受到的数据报文中相关信息的dict
 * @param 接受到的数据报文
 * @return [函数是否成功执行]
 */
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
				printf("%s\n", json_buf);
				char uuid[32];
				memset(uuid, 0, sizeof(uuid));
				sprintf(uuid, "%d", cJSON_GetObjectItem(rt_dg_root, "uuid")->valueint);
				cJSON_AddItemToObject(cluster_rt_root, uuid, rt_dg_root);

				head = head->next;
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

/**
 * [将dict中保存的监控数据转换为JSON保存到本地文件中]
 * @param 保存当前采集周期集群监控数据的dict
 * @return [函数是否成功执行]
 */
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
		if (fetch_key_key_value_bool("cluster", "save_all_data") == true)
		{
			if (save_rt_dg_to_all(cluster_rt_root) == false)
			{
				printf("Exec /  function failed.\n");
				return false;
			}
		}

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
		if (fetch_key_key_value_bool("cluster", "save_all_data") == true)
		{
			if (save_rt_dg_to_all(cluster_rt_root) == false)
			{
				printf("Exec /  function failed.\n");
				return false;
			}
		}
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

/**
 * [当需要保存所有时间的数据，将dict中保存的当前监控数据转换为JSON合并到本地文件中]
 * @param 保存当前采集周期集群监控数据的dict
 * @return [函数是否成功执行]
 */
bool save_rt_dg_to_all(cJSON *rt_dg)
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
		return false;
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
			return false;
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
	return true;
}


















