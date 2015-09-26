#include <stdio.h>
#include <pthread.h>
#include "solider.h"
#include "unix_sock.h"


int main(void)
{
	int solider_collect_thread_flag = -1;
	pthread_t solider_collect_thread;
	int activate_us_server__flag = -1;
	pthread_t activate_us_server_thread;
	int solider_merge_thread_flag = -1;
	pthread_t solider_merge_thread;
	int solider_listening_thread_flag = -1;
	pthread_t solider_listening_thread;
	init_conf();
	init_old_cpu_info();
	if ((activate_us_server__flag = pthread_create(&activate_us_server_thread, NULL, activate_unix_sock_server, NULL)) != 0)
	{
		perror("create pthread.");
		exit(0);
	}
//	if ((solider_collect_thread_flag = pthread_create(&solider_merge_thread, NULL, unix_sock_test, NULL)) != 0)
//	{
//		perror("create pthread.");
//		exit(0);
//	}

	if (fetch_key_key_value_bool("network", "send") == true)
	{
		if ((solider_collect_thread_flag = pthread_create(&solider_collect_thread, NULL, activate_solider_collect, NULL)) != 0)
		{
			perror("create pthread.");
			exit(0);
		}
	}
	if (fetch_key_key_value_bool("network", "recv") == true)
	{
		if ((solider_merge_thread_flag = pthread_create(&solider_merge_thread, NULL, activate_solider_merge, NULL)) != 0)
		{
			perror("create pthread.");
			exit(0);
		}
	}
	if ((solider_collect_thread_flag = pthread_create(&solider_collect_thread, NULL, activate_solider_scaleout, NULL)) != 0)
	{
		perror("create pthread.");
		exit(0);
	}
	if ((solider_merge_thread_flag = pthread_create(&solider_collect_thread, NULL, machine_scale_out, NULL)) != 0)
	{
		perror("create pthread.");
		exit(0);
	}
	if ((solider_listening_thread_flag = pthread_create(&solider_listening_thread, NULL, activate_solider_listen, NULL)) != 0)
	{
		perror("create pthread.");
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
	if (solider_listening_thread_flag == 0)
	{
		pthread_join(solider_listening_thread, NULL);
	}


	return 0;
}

