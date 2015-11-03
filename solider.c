#include "solider.h"


/**
 * [激活数据采集模块]
 */
void activate_solider_collect(void)
{
	printf("Activate solider collect services.\n");
	while (true)
	{
		char *sys_info_dg = NULL;
		char *sys_info_json = NULL;
		sys_info_json = (char *)calloc(sizeof(char), MAX_SINGLE_HOST_INFO_SIZE);
		pthread_t collect_thread;
		int collect_thread_flag = -1;
		if ((collect_thread_flag = pthread_create(&collect_thread, NULL, (void *)collect_host_info, sys_info_json)) != 0)
		{
			perror("Create activate_solider_collect thread failed.");
			continue;
		}
		sleep(MAX_COLLECT_USED_TIME);
		if (collect_thread_flag == 0)
		{
			pthread_join(collect_thread, NULL);
		}
		sys_info_dg = mul_encap_datagram(RT_HOST, sys_info_json);
		printf("#######%s\n", sys_info_dg);
		if (mulcast_dg(sys_info_dg) == false)
		{
			printf("Exec activate_solider_collect/mulcast_dg  function failed.\n");
			free(sys_info_dg);
			free(sys_info_json);
			sys_info_json = NULL;
			continue;
		}
		free(sys_info_dg);
		free(sys_info_json);
		sys_info_json = NULL;
		sleep(fetch_key_key_value_int("collection", "sleep_time") - MAX_COLLECT_USED_TIME);
	}
}

/**
 * [发送数据到监控数据多播组]
 * @param 待发送的数据
 * @return [函数是否执行成功]
 */
bool mulcast_dg(char *json_data)
{
	struct sockaddr_in mcast_addr;
	int mcast_socket;
	char *conf_json = NULL;
	cJSON *conf_root = NULL;
	cJSON *conf_node = NULL;
	FILE *conf_fd = NULL;
	int conf_file_size = 0;


	if((conf_fd = fopen(CONFIG_FILE_PATH, "r")) == NULL)
	{
		perror("Exec mulcast_dg/fopen  function failed.");
		return false;
	}
	fseek(conf_fd, 0, SEEK_END);
	conf_file_size = ftell(conf_fd);
	fseek(conf_fd, 0, SEEK_SET);
	conf_json = (char *)calloc(conf_file_size + 1, sizeof(char));
	if (conf_json == NULL)
	{
		perror("Exec mulcast_dg/calloc  function failed.");
		fclose(conf_fd);
		return false;
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
		perror("Exec mulcast_dg/socket  function failed.");
		cJSON_Delete(conf_root);
		return false;
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
		perror("Exec mulcast_dg/sendto  function failed.");
		cJSON_Delete(conf_root);
		return false;
	}
	close(mcast_socket);
	cJSON_Delete(conf_root);
	return true;
}

/**
 * [封装数据报，发送节点自身信息到多播组]
 */
void machine_scale_out(void)
{
	sleep(1);
	char *dg_message = NULL;
	dg_message = (char *)calloc(1024, sizeof(char));
	if (dg_message == NULL)
	{
		perror("Exec machine_scale_out/calloc function failed\n");
		exit(0);
	}
	strcat(dg_message, "2051");
	strcat(dg_message, "|");
	char *uuid = collect_machine_uuid();
	char *machine_ip = collect_machine_ip();
	strcat(dg_message, uuid);
	strcat(dg_message, "|");
	strcat(dg_message, machine_ip);
	if (mulcast_scaleout_dg(dg_message) == false)
	{
		printf("Exec mulcast_scaleout_dg/mulcast_scaleout_dg function failed.\n");
		free(dg_message);
		dg_message = NULL;
		free(uuid);
		uuid = NULL;
		free(machine_ip);
		machine_ip = NULL;
		exit(0);
	}
	free(dg_message);
	dg_message = NULL;
	free(uuid);
	uuid = NULL;
	free(machine_ip);
	machine_ip = NULL;
}


/**
 * [发送数据到控制信息多播组]
 * @param 待发送的数据
 */
bool mulcast_scaleout_dg(char *data)
{
	struct sockaddr_in mcast_addr;
	int mcast_socket;

	mcast_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (mcast_socket == -1)
	{
		perror("Exec mulcast_scaleout_dg/mcast_socket function failed.");
		return false;
	}
	int opt=1;
	setsockopt(mcast_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	memset(&mcast_addr, 0, sizeof(mcast_addr));
	mcast_addr.sin_family = AF_INET;
	char solider_mul_addr[20];
	char *fetch_value_buf = fetch_key_key_value_str("network", "scale_out_multicast_add");
	if (fetch_value_buf == NULL)
	{
		printf("Exec mulcast_scaleout_dg/fetch_key_key_value_str function failed.\n");
		return false;
	}
	memset(solider_mul_addr, 0, sizeof(solider_mul_addr));
	memcpy(solider_mul_addr, fetch_value_buf, 16);
	free(fetch_value_buf);
	fetch_value_buf = NULL;

	mcast_addr.sin_addr.s_addr = inet_addr(solider_mul_addr);
	mcast_addr.sin_port = htons(SCALEOUT_MCAST_PORT);
	int status = sendto(mcast_socket, data, strlen(data), 0, (struct sockaddr*)&mcast_addr, sizeof(mcast_addr));
	if (status < 0)
	{
		perror("Exec mulcast_scaleout_dg/sendto function failed.");
		return false;
	}
	printf("Add local host to cluster mulcast.\n");
	close(mcast_socket);
	return true;
}

/**
 * [封装数据报]
 * @param 数据报类型
 * @param 数据报正文
 * @return [封装后的数据报]
 */
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

/**
 * [激活节点的TCP监听，该模块用户监听并响应心跳检测，实时监控数据请求，监控数据名单检查等请求]
 */
void activate_solider_listen(void)
{
	int listen_sock;
	struct sockaddr_in listen_addr;


	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Exec activate_solider_listen/socket function failed.");
		exit(0);
	}
	int opt=1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(fetch_key_key_value_int("network", "listening port"));
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listen_sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) == -1)
	{
		perror("Exec activate_solider_listen/bind function failed.");
		exit(0);
	}
	if (listen(listen_sock, 5) == -1)
	{
		perror("Exec activate_solider_listen/listen function failed.");
		exit(0);
	}
	char buf[DG_MAX_SIZE];
	printf("Activate solider listen services.\n");
	while(true)
	{
	    struct sockaddr_in client_addr;
	    socklen_t len = sizeof(client_addr);

	    int client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &len);
	    if (client_sock < 0)
	    {
		    perror("Exec activate_solider_listen/accept function failed.");
		    continue;
	    }
	    memset(buf, 0, sizeof(buf));
	    recv(client_sock, buf, sizeof(buf), 0);
	    char type[16];
	    memset(type, 0, sizeof(type));
	    if (split(type, buf, '|', 1) == false)
	    {
		    printf("Exec activate_solider_listen/split function failed.");
		    close(client_sock);
		    continue;
	    }
	    switch (atoi(type)) {
	    case HEARTBEAT_DG:
	    {
		    if (respond_hb(client_sock) == false)
		    {
			    printf("Exec activate_solider_listen/respond_hb function failed.\n");
			    break;
		    }
	    }
		    break;
	    case REQUEST_RT_DG:
	    {
		    char *rt_dg_json = NULL;
		    FILE *rt_dg_fd = NULL;
		    if (access("/tmp/cMonitor/rt_datagram.json", R_OK) != 0)
		    {
			    printf("Exec activate_solider_listen/access function failed.");
			    break;
		    }
		    if ((rt_dg_fd = fopen("/tmp/cMonitor/rt_datagram.json", "r")) == NULL)
		    {
			    perror("Exec activate_solider_listen/fopen function failed.");
			    break;
		    }
		    fseek(rt_dg_fd, 0, SEEK_SET);
		    rt_dg_json = (char *)calloc(8192, sizeof(char));
		    if (rt_dg_json == NULL)
		    {
			    perror("Exec activate_solider_listen/calloc function failed.");
			    fclose(rt_dg_fd);
			    break;
		    }
		    while(fread(rt_dg_json, sizeof(char), 8192, rt_dg_fd))
		    {
			    printf("Exec activate_solider_listen/fread function failed.\n");
			    send(client_sock, rt_dg_json, strlen(rt_dg_json), 0);
			    memset(rt_dg_json, 0, 8192);
		    }
		    free(rt_dg_json);
		    rt_dg_json = NULL;
		    fclose(rt_dg_fd);
	    }
		    break;
	    case REQUEST_DATAGRAM_NAME:
	    {
		    char *conf_json = NULL;
		    cJSON *conf_root = NULL;
		    cJSON *conf_node = NULL;
		    cJSON *dg_name_root = NULL;
		    char *dg_name_json = NULL;
		    FILE *conf_fd = NULL;
		    int conf_file_size = 0;

		    if((conf_fd = fopen(CONFIG_FILE_PATH, "r")) == NULL)
		    {
			    perror("Exec activate_solider_listen/fopen function failed.");
			    break;
		    }
		    fseek(conf_fd, 0, SEEK_END);
		    conf_file_size = ftell(conf_fd);
		    fseek(conf_fd, 0, SEEK_SET);
		    conf_json = (char *)calloc(conf_file_size + 1, sizeof(char));
		    if (conf_json == NULL)
		    {
			    perror("Exec activate_solider_listen/calloc function failed.");
			    break;
		    }
		    fread(conf_json, sizeof(char), conf_file_size, conf_fd);
		    fclose(conf_fd);

		    conf_root = cJSON_Parse(conf_json);
		    conf_node = cJSON_GetObjectItem(conf_root, "collection")->child;
		    dg_name_root = cJSON_CreateObject();
		    while(conf_node)
		    {
			    if (conf_node->type == true)
			    {
				    cJSON_AddTrueToObject(dg_name_root, conf_node->string);
			    }
			    conf_node = conf_node->next;
		    }
		    conf_node = cJSON_GetObjectItem(conf_root, "custom");
		    if (conf_node->child->type == true)
		    {
			    conf_node = conf_node->child->next;
			    while(conf_node)
			    {
				    cJSON_AddTrueToObject(dg_name_root, conf_node->string);
				    conf_node = conf_node->next;
			    }
		    }
		    dg_name_json = cJSON_Print(dg_name_root);
		    send(client_sock, dg_name_json, strlen(dg_name_json), 0);
		    free(dg_name_json);
		    dg_name_json = NULL;
		    free(conf_json);
		    conf_json = NULL;
		    conf_node = NULL;
		    cJSON_Delete(conf_root);
		    cJSON_Delete(dg_name_root);
	    }
		    break;
	    case REQUEST_ALIVE_MACHINES:
	    {
		    char *alive_machines_json = NULL;
		    FILE *alive_machines_fd = NULL;
		    if (access("/tmp/cMonitor/alive_machine.json", R_OK) != 0)
		    {
			    printf("Exec activate_solider_listen/access function failed.\n");
			    break;
		    }
		    if ((alive_machines_fd = fopen("/tmp/cMonitor/alive_machine.json", "r")) == NULL)
		    {
			    perror("Exec activate_solider_listen/fopen function failed.");
			    break;
		    }
		    fseek(alive_machines_fd, 0, SEEK_SET);
		    alive_machines_json = (char *)calloc(8192, sizeof(char));
		    if (alive_machines_json == NULL)
		    {
			    perror("Exec activate_solider_listen/calloc function failed.");
			    fclose(alive_machines_fd);
			    break;
		    }
		    while(fread(alive_machines_json, sizeof(char), 8192, alive_machines_fd))
		    {
			    printf("Exec activate_solider_listen/fread function failed.\n");
			    send(client_sock, alive_machines_json, strlen(alive_machines_json), 0);
			    memset(alive_machines_json, 0, 8192);
		    }
		    free(alive_machines_json);
		    alive_machines_json = NULL;
		    fclose(alive_machines_fd);
	    }
		    break;
	    default:
		    break;
	    }
	    close(client_sock);
	}
	close(listen_sock);
}

/**
 * [从本地文件中获取集群的实时监控数据]
 * @return [实时监控数据]
 */
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
		return NULL;
	}
	fread(rt_dg_json, sizeof(char), alive_machine_file_size, rt_dg_fd);
	fclose(rt_dg_fd);
	return rt_dg_json;
}

/**
 * [响应心跳检测，告知对方本节点运行正常]
 * @return [函数是否成功执行]
 */
bool respond_hb(int client_sock)
{
	char *respond_dg = listen_encap_datagram(RESOND_HEARTBEAT);

	int ret = send(client_sock, respond_dg, strlen(respond_dg), 0);
	if (ret == -1)
	{
		printf("send error.\n");
		return false;
	}
	free(respond_dg);
	respond_dg = NULL;
	close(client_sock);
	return true;
}

/**
 * [封装数据报]
 * @param 数据报类型
 * @param 可选参数，为丰富报文封装提供拓展性
 * @return [封装后的报文]
 */
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







