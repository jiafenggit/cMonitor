#include "heartbeat.h"

void activate_solider_heartbeat(void)
{
	sleep(10);
	while(1)
	{
		char *target_ip = NULL;
		target_ip = fetch_target_ip();
		if (heartbeat_check(target_ip) == false)
		{
			printf("machine:%s drop.\n", target_ip);
			char *uuid = NULL;
			uuid = fetch_target_uuid(target_ip);
			if (uuid == NULL)
			{
				printf("fet target uuid error.\n");
				continue;
			}
			if (del_machine(uuid, target_ip) == false)
			{
				printf("del machine error.machine ip:%s\n", target_ip);
				continue;
			}
			printf("del machine %s.\n", target_ip);
			free(target_ip);
			target_ip = NULL;
			free(uuid);
			uuid = NULL;
			continue;
		}
		printf("target ip:%s\n", fetch_target_ip());
		sleep(fetch_key_key_value_int("heartbeat", "sleep_time"));

		free(target_ip);
		target_ip = NULL;
	}

}

char *fetch_target_ip(void)
{
	char *alive_machine_json = NULL;
	cJSON *alive_machine_root = NULL;
	cJSON *alive_machine_node = NULL;
	int alive_machine_num = 0;
	FILE *alive_machine_fd = NULL;
	int alive_machine_file_size = 0;
	char *target_ip = NULL;


	if((alive_machine_fd = fopen("/tmp/cMonitor/alive_machine.json", "r")) == NULL)
	{
		perror("open alive_machine file.");
		return NULL;
	}
	fseek(alive_machine_fd, 0, SEEK_END);
	alive_machine_file_size = ftell(alive_machine_fd);
	fseek(alive_machine_fd, 0, SEEK_SET);
	alive_machine_json = (char *)calloc(alive_machine_file_size + 1, sizeof(char));
	if (alive_machine_json == NULL)
	{
		perror("calloc memory.");
		fclose(alive_machine_fd);
		return NULL;
	}
	fread(alive_machine_json, sizeof(char), alive_machine_file_size, alive_machine_fd);
	fclose(alive_machine_fd);

	alive_machine_root = cJSON_Parse(alive_machine_json);
	alive_machine_node = alive_machine_root->child;
	while(alive_machine_node)
	{
		alive_machine_num++;
		alive_machine_node = alive_machine_node->next;
	}
	time_t time_seed;
	time(&time_seed);
	srand((unsigned)time_seed);
	int index = 0;
	alive_machine_node = alive_machine_root->child;
	for (index = 0; index < rand() % alive_machine_num; index ++)
	{
		alive_machine_node = alive_machine_node->next;
	}
	target_ip = (char *)calloc(16, sizeof(char));
	memcpy(target_ip, alive_machine_node->valuestring, strlen(alive_machine_node->valuestring));
	free(alive_machine_json);
	alive_machine_json = NULL;
	cJSON_Delete(alive_machine_root);
	return target_ip;
}

char *fetch_target_uuid(char *target_ip)
{
	char *alive_machine_json = NULL;
	cJSON *alive_machine_root = NULL;
	cJSON *alive_machine_node = NULL;
	FILE *alive_machine_fd = NULL;
	char *target_uuid = NULL;
	int alive_machine_file_size = 0;

	if((alive_machine_fd = fopen("/tmp/cMonitor/alive_machine.json", "r")) == NULL)
	{
		perror("open alive_machine file.");
		return NULL;
	}
	fseek(alive_machine_fd, 0, SEEK_END);
	alive_machine_file_size = ftell(alive_machine_fd);
	fseek(alive_machine_fd, 0, SEEK_SET);
	alive_machine_json = (char *)calloc(alive_machine_file_size + 1, sizeof(char));
	if (alive_machine_json == NULL)
	{
		perror("calloc memory.");
		fclose(alive_machine_fd);
		return NULL;
	}
	fread(alive_machine_json, sizeof(char), alive_machine_file_size, alive_machine_fd);
	fclose(alive_machine_fd);

	alive_machine_root = cJSON_Parse(alive_machine_json);
	free(alive_machine_json);
	alive_machine_json = NULL;
	alive_machine_node = alive_machine_root->child;
	while(alive_machine_node)
	{
		if (strcmp(alive_machine_node->valuestring, target_ip) == 0)
		{
			target_uuid  = (char *)calloc(16, sizeof(char));
			memcpy(target_uuid, alive_machine_node->string, strlen(alive_machine_node->string));
			cJSON_Delete(alive_machine_root);
			return target_uuid;
		}
		alive_machine_node = alive_machine_node->next;
	}
	cJSON_Delete(alive_machine_root);
	return NULL;

}


bool heartbeat_check(char *target_ip)
{
	struct sockaddr_in target_addr;
	int target_socket;

	target_socket = socket(AF_INET, SOCK_STREAM, 0);

	struct timeval time_out = {fetch_key_key_value_int("heartbeat", "time_out"), 0};
	setsockopt(target_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&time_out, sizeof(struct timeval));
	setsockopt(target_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&time_out, sizeof(struct timeval));
	int opt=1;
	setsockopt(target_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (target_socket < 0)
	{
		perror("socket.");
		return false;
	}
	memset(&target_addr, 0, sizeof(target_addr));
	target_addr.sin_family = AF_INET;
	target_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	target_addr.sin_port = htons(fetch_key_key_value_int("network", "listening port"));
	inet_pton(AF_INET, target_ip, &target_addr.sin_addr);
	if (connect(target_socket, (struct sockaddr*)&target_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("connect error");
		close(target_socket);
		return false;
	}
	char *dg_message;
	dg_message = (char *)calloc(32, sizeof(char));
	char *uuid = collect_machine_uuid();
	strcat(dg_message, uuid);
	free(uuid);
	uuid = NULL;
	strcat(dg_message, "|");
	strcat(dg_message, "2048");
	if (send(target_socket, dg_message, strlen(dg_message), 0) == -1)
	{
		printf("send time out.target ip:%s\n", target_ip);
		close(target_socket);
		return false;
	}
	free(dg_message);
	dg_message = NULL;
	char buf[32];
	if(recv(target_socket, buf, 32, 0) == -1)
	{
		printf("recv time out.target ip:%s\n", target_ip);
		close(target_socket);
		return false;
	}
	close(target_socket);
	char response[16];
	memset(response, 0, sizeof(response));
	if (split(response, buf, '|', 1) == false)
	{
		printf("split datagram error.'n");
		return false;
	}
	if (strcmp("2049", response) != 0)
	{
		printf("response str false. target ip:%s\n", target_ip);
		return false;
	}
	return true;
}

bool del_machine(char *uuid, char *machine_ip)
{
	char *dg_message;
	dg_message = (char *)calloc(64, sizeof(char));

	strcat(dg_message, "2053");
	strcat(dg_message, "|");
	strcat(dg_message, uuid);
	strcat(dg_message, "|");
	strcat(dg_message, machine_ip);
	// unix sock send
	char *recv = NULL;
	recv = send_and_recv_to_us(dg_message);
	if (atoi(recv) != SUCCESS)
	{
		printf("del machine:%s to unix sock server failed.\n", machine_ip);
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

