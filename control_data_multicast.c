#include "control_data_multicast.h"

void activate_control_data_multicast(void)
{
	int mul_socket;
	struct sockaddr_in local_address;

	if((mul_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Exec activate_solider_scaleout/socket function failed.");
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
		perror("Exec activate_solider_scaleout/bind function failed.");
		exit(0);
	}

	int loop = 1;
	if (setsockopt(mul_socket,IPPROTO_IP,IP_MULTICAST_LOOP,&loop, sizeof(loop)) < 0)
	{
		perror("Exec activate_solider_scaleout/setsockopt IP_MULTICAST_LOOP function failed.");
		exit(0);
	}
	struct ip_mreq mrep;
	char solider_mul_addr[20];
	char *fetch_value_buf = fetch_key_key_value_str("network", "scale_out_multicast_add");
	if (fetch_value_buf == NULL)
	{
		printf("Exec activate_solider_scaleout/fetch_key_key_value_str function failed.\n");
		exit(0);
	}
	memset(solider_mul_addr, 0, sizeof(solider_mul_addr));
	memcpy(solider_mul_addr, fetch_value_buf, 16);
	free(fetch_value_buf);
	fetch_value_buf = NULL;
	mrep.imr_multiaddr.s_addr = inet_addr(solider_mul_addr);
	mrep.imr_interface.s_addr = htonl(INADDR_ANY);
	//加入广播组
	if (setsockopt(mul_socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mrep, sizeof(mrep)) < 0)
	{
		perror("Exec activate_solider_scaleout/setsockopt IP_ADD_MEMBERSHIP function failed.");
		exit(0);
	}

	while(1)
	{
		char buf[1024];
		memset(buf, 0, sizeof(buf));
		socklen_t address_len = sizeof(local_address);
		if(recvfrom(mul_socket, buf, 1024, 0,(struct sockaddr *)&local_address, &address_len) < 0)
		{
			perror("Exec activate_solider_scaleout/recvfrom function failed.");
			continue;
		}
		char type[8], uuid[16], machine_ip[16];
		memset(type, 0, sizeof(type));
		memset(uuid, 0, sizeof(uuid));
		memset(machine_ip, 0, sizeof(machine_ip));
		split(type, buf , '|', 0);
		split(uuid, buf , '|', 1);
		split(machine_ip, buf , '|', 2);
		printf("Exec activate_solider_scaleout/add_machine(%s) function succeeded.\n", machine_ip);
		switch (atoi(type)) {
		case SCALEOUT_DG:
		{
			if (add_machine(uuid, machine_ip) == false)
			{
				printf("Exec activate_solider_scaleout/add_machine(%s) function failed.\n", machine_ip);
				continue;
			}
		}
			break;
		case SYNC_ALIVE_MACHINE:
		{
			if (sync_alive_machines(uuid, machine_ip) == false)
			{
				printf("Exec activate_solider_scaleout/sync_alive_machines(%s) function failed.\n", machine_ip);
				continue;
			}
		}
			break;
		case SYNC_DEAD_MACHINE:
		{
			if (sync_dead_machines(uuid, machine_ip) == false)
			{
				printf("Exec activate_solider_scaleout/sync_dead_machines(%s) function failed.\n", machine_ip);
				continue;
			}
		}
			break;
		default:
			break;
		}

	}
	if(setsockopt(mul_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP,&mrep, sizeof(mrep)) < 0)
	{
		perror("Exec activate_solider_scaleout/setsockopt IP_DROP_MEMBERSHIP function failed.");
	}
	close(mul_socket);
}


bool add_machine(char *uuid, char *machine_ip)
{
	if (add_host_to_mongo(uuid, machine_ip) == false)
	{
		printf("Exec add_machine/add_host_to_mongo function failed.\n");
		return false;
	}
	if (add_machine_to_file(uuid, machine_ip) == false)
	{
		printf("Exec add_machine/add_machine_to_file function failed.\n");
		return false;
	}
	if (sync_alive_machines_mul() == false)
	{
		printf("Exec add_machine/sync_alive_machines_mul function failed.\n");
		return false;
	}

	return true;
}

bool sync_alive_machines(char *uuid, char *machine_ip)
{
	if (add_machine_to_file(uuid, machine_ip) == false)
	{
		printf("Exec sync_alive_machines/add_machine_to_file function failed.\n");
		return false;
	}
	return true;
}

bool sync_dead_machines(char *uuid, char *machine_ip)
{
	if (del_machine_from_file(uuid) == false)
	{
		printf("Exec respond_dg/del_machine_from_file function failed.\n");
		return false;
	}
	return true;
}

bool sync_alive_machines_mul(void)
{
	char *dg_message;
	dg_message = (char *)calloc(1024, sizeof(char));
	strcat(dg_message, "2055");
	strcat(dg_message, "|");
	char *uuid = collect_machine_uuid();
	char *machine_ip = collect_machine_ip();
	strcat(dg_message, uuid);
	strcat(dg_message, "|");
	strcat(dg_message, machine_ip);
	mulcast_sync_dg(dg_message);
	free(dg_message);
	dg_message = NULL;
	free(uuid);
	uuid = NULL;
	free(machine_ip);
	machine_ip = NULL;
	return true;
}

bool sync_dead_machines_mul(char *uuid, char *target_ip)
{
	char *dg_message;
	dg_message = (char *)calloc(1024, sizeof(char));
	strcat(dg_message, "2056");
	strcat(dg_message, "|");
	strcat(dg_message, uuid);
	strcat(dg_message, "|");
	strcat(dg_message, target_ip);
	mulcast_sync_dg(dg_message);
	free(dg_message);
	dg_message = NULL;
	return true;
}


void mulcast_sync_dg(char *data)
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
	if (fetch_value_buf == NULL)
	{
		printf("Exec fetch_key_key_value_str function failed.\n");
		return NULL;
	}
	memset(solider_mul_addr, 0, sizeof(solider_mul_addr));
	memcpy(solider_mul_addr, fetch_value_buf, 16);
	free(fetch_value_buf);
	fetch_value_buf = NULL;

	mcast_addr.sin_addr.s_addr = inet_addr(solider_mul_addr);
	mcast_addr.sin_port = htons(SCALEOUT_MCAST_PORT);
	printf("sync machines.\n");
	int status = sendto(mcast_socket, data, strlen(data), 0, (struct sockaddr*)&mcast_addr, sizeof(mcast_addr));
	if (status < 0)
	{
		perror("sendto()");
		exit(-1);
	}
	close(mcast_socket);
}


bool add_machine_to_file(char *uuid, char *machine_ip)
{
	char *alive_machine_json = NULL;
	cJSON *root;
	FILE *alive_machine_fd = NULL;
	if (access("/tmp/cMonitor/alive_machine.json", F_OK) != -1)
	{
		if (access("/tmp/cMonitor/alive_machine.json", R_OK) != 0)
		{
			printf("No read permission.\n");
			return false;
		}
		if ((alive_machine_fd = fopen("/tmp/cMonitor/alive_machine.json", "r")) == NULL)
		{
			perror("open alive machine file.");
			return false;
		}
		fseek(alive_machine_fd, 0, SEEK_END);
		int alive_machine_file_size = ftell(alive_machine_fd);
		fseek(alive_machine_fd, 0, SEEK_SET);
		alive_machine_json = (char *)calloc(alive_machine_file_size + 1, sizeof(char));
		if (alive_machine_json == NULL)
		{
			perror("calloc memory.");
			fclose(alive_machine_fd);
			exit(0);
		}
		fread(alive_machine_json, sizeof(char), alive_machine_file_size, alive_machine_fd);
		fclose(alive_machine_fd);


		root = cJSON_Parse(alive_machine_json);
		cJSON *child = root->child;
		while(child)
		{
			if (strcmp(child->string, uuid) == 0)
			{
				free(alive_machine_json);
				alive_machine_json = NULL;
				cJSON_Delete(root);
				return true;
			}
			child = child->next;
		}
		cJSON_AddStringToObject(root,  uuid, machine_ip);
		memset(alive_machine_json, 0, alive_machine_file_size);
		free(alive_machine_json);
		alive_machine_json = NULL;
		alive_machine_json = cJSON_Print(root);
		if ((alive_machine_fd = fopen("/tmp/cMonitor/alive_machine.json", "w")) == NULL)
		{
			perror("open alive machine file.");
			free(alive_machine_json);
			alive_machine_json = NULL;
			cJSON_Delete(root);
			return false;
		}
		fwrite(alive_machine_json, sizeof(char), strlen(alive_machine_json), alive_machine_fd);
		//fputs(alive_machine_json, alive_machine_fd);
		fclose(alive_machine_fd);
		free(alive_machine_json);
		cJSON_Delete(root);
		return true;
	}
	else
	{
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root,  uuid, machine_ip);
		if ((alive_machine_fd = fopen("/tmp/cMonitor/alive_machine.json", "a+")) == NULL)
		{
			perror("open alive machine file.");
			return false;
		}
		alive_machine_json = cJSON_Print(root);
		fwrite(alive_machine_json, sizeof(char), strlen(alive_machine_json), alive_machine_fd);
		fclose(alive_machine_fd);
		free(alive_machine_json);
		cJSON_Delete(root);
		return true;
	}
}

bool del_machine_from_file(char *uuid)
{
	char *alive_machine_json = NULL;
	cJSON *root;
	FILE *alive_machine_fd = NULL;
	if (access("/tmp/cMonitor/alive_machine.json", F_OK) != -1)
	{
		if (access("/tmp/cMonitor/alive_machine.json", R_OK) != 0)
		{
			printf("No read permission.\n");
			return false;
		}
		if ((alive_machine_fd = fopen("/tmp/cMonitor/alive_machine.json", "r")) == NULL)
		{
			perror("open alive machine file.");
			return false;
		}
		fseek(alive_machine_fd, 0, SEEK_END);
		int alive_machine_file_size = ftell(alive_machine_fd);
		fseek(alive_machine_fd, 0, SEEK_SET);
		alive_machine_json = (char *)calloc(alive_machine_file_size + 1, sizeof(char));
		if (alive_machine_json == NULL)
		{
			perror("calloc memory.");
			fclose(alive_machine_fd);
			exit(0);
		}
		fread(alive_machine_json, sizeof(char), alive_machine_file_size, alive_machine_fd);
		fclose(alive_machine_fd);


		root = cJSON_Parse(alive_machine_json);
		cJSON *child = root->child;
		while(child)
		{
			if (strcmp(child->string, uuid) == 0)
			{
				if (child->next == NULL)
				{
					if (child->prev != NULL)
					{
						child->prev->next = NULL;
					}
					else
					{
						root->child = root->child->next;
					}
				}
				else
				{
					if (child->prev != NULL)
					{
						child->prev->next = child->next;
					}
					else
					{
						root->child = root->child->next;
					}
					child->next->prev = child->prev;
				}
				child->next = NULL;
				child->prev = NULL;
				cJSON_Delete(child);
				free(alive_machine_json);
				alive_machine_json = NULL;
				alive_machine_json = cJSON_Print(root);
				if ((alive_machine_fd = fopen("/tmp/cMonitor/alive_machine.json", "w")) == NULL)
				{
					perror("open alive machine file.");
					free(alive_machine_json);
					alive_machine_json = NULL;
					cJSON_Delete(root);
					return false;
				}
				fwrite(alive_machine_json, sizeof(char), strlen(alive_machine_json), alive_machine_fd);
				fclose(alive_machine_fd);
				free(alive_machine_json);
				cJSON_Delete(root);
				return true;
			}
			child = child->next;
		}
		free(alive_machine_json);
		alive_machine_json = NULL;
		cJSON_Delete(root);
		return true;
	}
	else
	{
		return false;
	}


}

