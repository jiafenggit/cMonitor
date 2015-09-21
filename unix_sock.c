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
		char recv_buf[DG_MAX_SIZE];
		read(client_sock, recv_buf, sizeof(recv_buf));
		printf("recv:%s\n", recv_buf);
		respond_dg(recv_buf);

		write(client_sock, "success", 7);
		close(client_sock);
	}
	close(listen_sock);

}


char *us_encap_datagram(int dg_type, char *datagram)
{
	char *dg_message;
	char time_str[32];
	time_t time_now;
	dg_message = (char *)calloc(DG_MAX_SIZE, sizeof(char));
	if (strlen(datagram) >= DG_MAX_SIZE - (32*10))
	{
		printf("datagram is too long.\n");
		return NULL;
	}
	switch (dg_type) {
	case MERGE_HOUR_TO_DAY:
	{
		printf("%s\n", collect_machine_uuid());
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
	char *type = str_split(data_gram, '|', 1);

	switch (atoi(type)) {
	case MERGE_HOUR_TO_DAY:
	{
		if (merge_hour_to_day() == false)
		{
			printf("merge hour to day error.\n");
		}
	}
		break;
	default:
		break;
	}
}

bool merge_hour_to_day(void)
{
	char *day_dg = NULL;
	cJSON *day_dg_root;
	FILE *day_fd = NULL;
	int file_size;
	if (access("/tmp/day_datagram.json", F_OK) != -1)
	{
		if((day_fd = fopen("/tmp/day_datagram.json", "r")) == NULL)
		{
			perror("open day datagram file.");
			cJSON_Delete(rt_dg);
			exit(0);
		}
		fseek(day_fd, 0, SEEK_END);
		file_size = ftell(day_fd);
		fseek(day_fd, 0, SEEK_SET);
		day_dg = (char *)calloc(file_size + 1, sizeof(char));
		if (day_dg == NULL)
		{
			perror("calloc memory.");
			cJSON_Delete(rt_dg);
			exit(0);
		}
		fread(day_dg, sizeof(char), file_size, day_fd);
		fclose(day_fd);

		day_dg_root = cJSON_Parse(day_dg);
		time_t time_now;
		time(&time_now);
		char time_str[32];
		sprintf(time_str, "%d", time_now);
		cJSON_AddItemToObject(day_dg_root,time_str, rt_dg);
		if ((day_fd = fopen("/tmp/hour_datagram.json", "w")) == NULL)
		{
			perror("open day datagram file.");
			fclose(day_fd);
			free(day_dg);
			cJSON_Delete(rt_dg);
			day_dg = NULL;
			return false;
		}
		char *rt_dg_buf = cJSON_Print(day_dg_root);
		fputs(rt_dg_buf, day_fd);
		fclose(day_fd);
		free(rt_dg_buf);
		free(day_dg);
		day_dg = NULL;
		cJSON_Delete(day_dg_root);
		day_dg_root = NULL;
		return true;
	}
	else
	{
		day_dg_root = cJSON_CreateObject();
		time_t time_now;
		time(&time_now);
		char time_str[32];
		sprintf(time_str, "%d", time_now);
		cJSON_AddItemToObject(day_dg_root, time_str, rt_dg);
		if ((day_fd = fopen("/tmp/hour_datagram.json", "a+")) == NULL)
		{
			perror("create day datagram file.");
			fclose(day_fd);
			free(day_dg);
			cJSON_Delete(rt_dg);
			day_dg = NULL;
			return false;
		}
		char *rt_dg_buf = cJSON_Print(day_dg_root);
		fputs(rt_dg_buf, day_fd);
		fclose(day_fd);
		free(rt_dg_buf);
		free(day_dg);
		day_dg = NULL;
		cJSON_Delete(day_dg_root);
		day_dg_root = NULL;
		return true;
	}
}

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
		write(sock, send, strlen(send));
		read(sock, recvbuf, sizeof(recvbuf));
		printf("recv:%s\n", recvbuf);
		memset(send, '\0', sizeof(recvbuf));
		memset(recvbuf,' \0', sizeof(recvbuf));
		free(send);
		send = NULL;
		sleep(10);
//		close(sock);
	}
}
