#include "unix_sock.h"


void activate_unix_sock_server(void)
{
	int listen_sock;
	if ((listen_sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("unix socket");
		exit(0);
	}
	unlink("/tmp/monitor_solider.sock");
	struct sockaddr_un server_addr;
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, "/tmp/monitor_solider.sock");

	if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("unix socket");
		exit(0);
	}
	if (listen(listen_sock, 5) < 0)
	{
		perror("unix socket");
		exit(0);
	}
	while(true)
	{
		int client_sock;
		client_sock = accept(listen_sock, NULL, NULL);
		if (client_sock == -1)
		{
			perror("client socket");
			continue;
		}
		char recv_buf[1024];
		memset(recv_buf, 0, sizeof(recv_buf));
		read(client_sock, recv_buf,1024);
		respond_dg(recv_buf);

		write(client_sock, "4096", 8);
		close(client_sock);
	}
	close(listen_sock);

}


char *us_encap_datagram(int dg_type, char *datagram)
{
	char *dg_message;
	char time_str[32];
	time_t time_now;
	memset(time_str, 0, sizeof(time_str));
	dg_message = (char *)calloc(DG_MAX_SIZE, sizeof(char));
	if (strlen(datagram) >= DG_MAX_SIZE - (32*10))
	{
		printf("datagram is too long.\n");
		return NULL;
	}
	switch (dg_type) {
	case MERGE_HOUR_TO_DAY:
	{
		char *machine_uuid = collect_machine_uuid();
		strcat(dg_message, machine_uuid);
		free(machine_uuid);
		machine_uuid = NULL;
		strcat(dg_message, "|");
		strcat(dg_message, "1024");
		strcat(dg_message, "|");
		char *mmh = murmurhash_str(datagram);
		strcat(dg_message, mmh);
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
	return NULL;

}

void respond_dg(char *data_gram)
{
	char *type = str_split(data_gram, '|', 0);

	switch (atoi(type)) {
	case ADD_MACHINE:
	{
		char *uuid = str_split(data_gram, '|', 1);
		char *machine_ip = str_split(data_gram, '|', 2);
		if (add_machine_to_file(uuid, machine_ip) == false)
		{
			printf("add machine to cluster failed.\n");
			free(type);
			free(uuid);
			free(machine_ip);
			exit(0);
		}
		free(type);
		free(uuid);
		free(machine_ip);
	}
		break;
	default:
		break;
	}
}

bool add_machine_to_file(char *uuid, char *machine_ip)
{
	char *alive_machine_json = NULL;
	cJSON *root;
	FILE *alive_machine_fd = NULL;
	if (access("/home/cf/conf/alive_machine.json", F_OK) != -1)
	{
		if (access("/home/cf/conf/alive_machine.json", R_OK) != 0)
		{
			printf("No read permission.\n");
			return false;
		}
		if ((alive_machine_fd = fopen("/home/cf/conf/alive_machine.json", "r")) == NULL)
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
		if ((alive_machine_fd = fopen("/home/cf/conf/alive_machine.json", "w")) == NULL)
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
		if ((alive_machine_fd = fopen("/home/cf/conf/alive_machine.json", "a+")) == NULL)
		{
			perror("open alive machine file.");
			return false;
		}
		alive_machine_json = cJSON_Print(root);
		fputs(alive_machine_json, alive_machine_fd);
		fclose(alive_machine_fd);
		free(alive_machine_json);
		cJSON_Delete(root);
		return true;
	}
}

//bool merge_hour_to_day(void)
//{
//	char *day_dg = NULL;
//	cJSON *day_dg_root;
//	FILE *day_fd = NULL;
//	int file_size;
//	if (access("/tmp/day_datagram.json", F_OK) != -1)
//	{
//		if((day_fd = fopen("/tmp/day_datagram.json", "r")) == NULL)
//		{
//			perror("open day datagram file.");
//			exit(0);
//		}
//		fseek(day_fd, 0, SEEK_END);
//		file_size = ftell(day_fd);
//		fseek(day_fd, 0, SEEK_SET);
//		day_dg = (char *)calloc(file_size + 1, sizeof(char));
//		if (day_dg == NULL)
//		{
//			perror("calloc memory.");
//			exit(0);
//		}
//		fread(day_dg, sizeof(char), file_size, day_fd);
//		fclose(day_fd);

//		day_dg_root = cJSON_Parse(day_dg);
//		time_t time_now;
//		time(&time_now);
//		char time_str[32];
//		sprintf(time_str, "%d", time_now);
//		cJSON_AddItemToObject(day_dg_root, time_str, rt_dg);
//		if ((day_fd = fopen("/tmp/hour_datagram.json", "w")) == NULL)
//		{
//			perror("open day datagram file.");
//			fclose(day_fd);
//			free(day_dg);
//			cJSON_Delete(rt_dg);
//			day_dg = NULL;
//			return false;
//		}
//		char *rt_dg_buf = cJSON_Print(day_dg_root);
//		fputs(rt_dg_buf, day_fd);
//		fclose(day_fd);
//		free(rt_dg_buf);
//		free(day_dg);
//		day_dg = NULL;
//		cJSON_Delete(day_dg_root);
//		day_dg_root = NULL;
//		return true;
//	}
//	else
//	{
//		day_dg_root = cJSON_CreateObject();
//		time_t time_now;
//		time(&time_now);
//		char time_str[32];
//		sprintf(time_str, "%d", time_now);
//		cJSON_AddItemToObject(day_dg_root, time_str, rt_dg);
//		if ((day_fd = fopen("/tmp/hour_datagram.json", "a+")) == NULL)
//		{
//			perror("create day datagram file.");
//			fclose(day_fd);
//			free(day_dg);
//			cJSON_Delete(rt_dg);
//			day_dg = NULL;
//			return false;
//		}
//		char *rt_dg_buf = cJSON_Print(day_dg_root);
//		fputs(rt_dg_buf, day_fd);
//		fclose(day_fd);
//		free(rt_dg_buf);
//		free(day_dg);
//		day_dg = NULL;
//		cJSON_Delete(day_dg_root);
//		day_dg_root = NULL;
//		return true;
//	}
//}

void unix_sock_test(void)
{

	while(1)
	{
		int sock;
		if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
		{
			perror("socket");
			exit(0);
		}
		struct sockaddr_un serveraddr;
		memset(&serveraddr, 0, sizeof(serveraddr));
		serveraddr.sun_family = AF_UNIX;
		strcpy(serveraddr.sun_path, "/tmp/monitor_solider.sock");

		if (connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
		{
			perror("socket");
			exit(0);
		}
		char recvbuf[4096];
		char *send = us_encap_datagram(1024, " ");
		memset(recvbuf, 0, sizeof(recvbuf));
		write(sock, send, strlen(send));
		read(sock, recvbuf, sizeof(recvbuf));
		memset(send, '\0', sizeof(recvbuf));
		memset(recvbuf,' \0', sizeof(recvbuf));
		free(send);
		send = NULL;
		sleep(10);
//		close(sock);
	}
}


char *send_and_recv_to_us(char *request_dg)
{
	int us_sock;
	if ((us_sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(0);
	}
	struct sockaddr_un serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sun_family = AF_UNIX;
	strcpy(serveraddr.sun_path, "/tmp/monitor_solider.sock");

	if (connect(us_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
	{
		perror("socket");
		exit(0);
	}
	char *recvbuf = NULL;
	recvbuf = (char *)calloc(1024, sizeof(char));
	write(us_sock, request_dg, strlen(request_dg));
	read(us_sock, recvbuf, 1024);
	return recvbuf;
}
