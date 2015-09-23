#include <stdio.h>
#include <pthread.h>
#include "solider.h"
#include "unix_sock.h"



int main(void)
{
	int solider_collect_thread_flag = -1;
	pthread_t solider_collect_thread;
	int solider_merge_thread_flag = -1;
	pthread_t solider_merge_thread;
	init_conf();
	printf("%s\n", collect_machine_uuid());

//	if ((solider_collect_thread_flag = pthread_create(&solider_collect_thread, NULL, activate_unix_sock_server, NULL)) != 0)
//	{
//		perror("create pthread.");
//		exit(0);
//	}
//	if ((solider_collect_thread_flag = pthread_create(&solider_merge_thread, NULL, unix_sock_test, NULL)) != 0)
//	{
//		perror("create pthread.");
//		exit(0);
//	}

//	if (fetch_key_key_value_bool("network", "send") == true)
//	{
//		if ((solider_collect_thread_flag = pthread_create(&solider_collect_thread, NULL, activate_solider_collect, NULL)) != 0)
//		{
//			perror("create pthread.");
//			exit(0);
//		}
//	}
//	if (fetch_key_key_value_bool("network", "recv") == true)
//	{
//		if ((solider_merge_thread_flag = pthread_create(&solider_merge_thread, NULL, activate_solider_merge, NULL)) != 0)
//		{
//			perror("create pthread.");
//			exit(0);
//		}
//	}
	if ((solider_collect_thread_flag = pthread_create(&solider_collect_thread, NULL, activate_solider_listen, NULL)) != 0)
	{
		perror("create pthread.");
		exit(0);
	}


	if (solider_collect_thread_flag == 0)
	{
		pthread_join(solider_collect_thread, NULL);
	}
	if (solider_merge_thread_flag == 0)
	{
		pthread_join(solider_merge_thread, NULL);
	}


	return 0;
}

