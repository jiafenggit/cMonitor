#include <stdio.h>
#include <pthread.h>
#include "solider.h"
#include "unix_sock.h"
#include "heartbeat.h"
#include "configuration.h"
#include "c_mongodb.h"


int main(void)
{
	add_host_to_mongo("12345678", "10.0.0.56");
	add_host_to_mongo("12345675", "10.0.0.57");
	add_host_to_mongo("12345678", "10.0.0.58");
	add_host_to_mongo("12345678", "10.0.0.58");

	system("rm -rf /tmp/cMonitor");
	system("mkdir /tmp/cMonitor ");
	int solider_collect_thread_flag = -1;
	pthread_t solider_collect_thread;
	int activate_us_server__flag = -1;
	pthread_t activate_us_server_thread;
	int solider_merge_thread_flag = -1;
	pthread_t solider_merge_thread;
	int solider_scaleout_thread_flag = -1;
	pthread_t solider_scaleout_thread;
	int machine_scale_out_thread_flag = -1;
	pthread_t machine_scale_out_thread;
	int solider_listening_thread_flag = -1;
	pthread_t solider_listening_thread;
	int heartbeat_thread_flag = -1;
	pthread_t heartbeat_thread;

	if (init_conf() == false)
	{
		printf("Exec init_conf function failed.\n");
		exit(0);
	}
	if (init_old_cpu_info() == false)
	{
		printf("Exec init_old_cpu_info function failed.\n");
		exit(0);
	}
	if ((activate_us_server__flag = pthread_create(&activate_us_server_thread, NULL, (void *)activate_unix_sock_server, NULL)) != 0)
	{
		perror("Create activate_us_server_thread thread failed.");
		exit(0);
	}
	if (fetch_key_key_value_bool("network", "send") == true)
	{
		if ((solider_collect_thread_flag = pthread_create(&solider_collect_thread, NULL, (void *)activate_solider_collect, NULL)) != 0)
		{
			perror("Create activate_solider_collect thread failed.");
			exit(0);
		}
	}
	else if (fetch_key_key_value_bool("network", "send") == NULL)
	{
		printf("Exec fetch_key_key_value_bool function failed.\n");
		exit(0);
	}
	if (fetch_key_key_value_bool("network", "recv") == true)
	{
		if ((solider_merge_thread_flag = pthread_create(&solider_merge_thread, NULL, (void *)activate_monitor_data_multicast, NULL)) != 0)
		{
			perror("Create activate_solider_merge thread failed.");
			exit(0);
		}
	}
	else if (fetch_key_key_value_bool("network", "recv") == NULL)
	{
		printf("Exec fetch_key_key_value_bool function failed.\n");
		exit(0);
	}
	if ((solider_scaleout_thread_flag = pthread_create(&solider_scaleout_thread, NULL, (void *)activate_control_data_multicast, NULL)) != 0)
	{
		perror("Create activate_solider_scaleout thread failed.");
		exit(0);
	}
	if ((machine_scale_out_thread_flag = pthread_create(&machine_scale_out_thread, NULL, (void *)machine_scale_out, NULL)) != 0)
	{
		perror("Create machine_scale_out thread failed.");
		exit(0);
	}
	if ((solider_listening_thread_flag = pthread_create(&solider_listening_thread, NULL, (void *)activate_solider_listen, NULL)) != 0)
	{
		perror("Create activate_solider_listen thread failed.");
		exit(0);
	}
	if ((heartbeat_thread_flag = pthread_create(&heartbeat_thread, NULL, (void *)activate_solider_heartbeat, NULL)) != 0)
	{
		perror("Create activate_solider_heartbeat thread failed.");
		exit(0);
	}



	if (activate_us_server__flag == 0)
	{
		pthread_join(activate_us_server_thread, NULL);
	}
	if (solider_collect_thread_flag == 0)
	{
		pthread_join(solider_collect_thread, NULL);
	}
	if (solider_merge_thread_flag == 0)
	{
		pthread_join(solider_merge_thread, NULL);
	}
	if (solider_scaleout_thread_flag == 0)
	{
		pthread_join(solider_scaleout_thread, NULL);
	}
	if (machine_scale_out_thread_flag == 0)
	{
		pthread_join(machine_scale_out_thread, NULL);
	}
	if (solider_listening_thread_flag == 0)
	{
		pthread_join(solider_listening_thread, NULL);
	}
	if (heartbeat_thread_flag == 0)
	{
		pthread_join(heartbeat_thread, NULL);
	}


	return 0;
}

