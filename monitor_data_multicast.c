#include "monitor_data_multicast.h"



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
				printf("%s\n", json_buf);
				char uuid[32];
				memset(uuid, 0, sizeof(uuid));
				sprintf(uuid, "%d", cJSON_GetObjectItem(rt_dg_root, "uuid")->valueint);
				cJSON_AddItemToObject(cluster_rt_root, uuid, rt_dg_root);

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
				    cJSON_AddItemToObject(cluster_rt_root, cJSON_GetObjectItem(rt_dg_root, "uuid")->valueint, rt_dg_root);
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




















