/**
 * 为开发方便，故而将程序中涉及到的大部分头文件和宏定义归纳到该文件
 */
#ifndef MACRO_DEFINITION_H
#define MACRO_DEFINITION_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <string.h>
#include <ctype.h>
#include <sys/file.h>
#include <stdbool.h>
#include<sys/un.h>
#include <unistd.h>
#include <mongoc.h>
#include <pthread.h>
#include <dirent.h>

#define MAX_SINGLE_HOST_INFO_SIZE	8191
#define MAX_COLLECT_USED_TIME		2

#define RT_HOST			0
#define RT_GROUP		1
#define RT_CLUSTER		2
#define RT_HEARTBEAT		3
#define RT_OFFICER		4
#define HEARTBEAT_DG		2048
#define RESOND_HEARTBEAT	2049
#define SCALEOUT_DG		2051
#define ADD_MACHINE        	2052
#define DEL_MACHINE		2053
#define REQUEST_RT_DG		2054
#define SYNC_ALIVE_MACHINE	2055
#define SYNC_DEAD_MACHINE	2056
#define REQUEST_DATAGRAM_NAME	2057
#define REQUEST_ALIVE_MACHINES	2058

#define SUCCESS 		4096

#define MCAST_PORT		8192
#define SCALEOUT_MCAST_PORT	8193
#define MAX_BUF_SIZE		32768
#define DG_MAX_SIZE		5120

#define MERGE_HOUR_TO_DAY 	1024
#define REQUEST_ALIVE_IP 	1025

#define CONFIG_FILE_PATH 	"/tmp/cMonitor/cCollection.conf.json"
#define SHELL_FILE_DIR		"/home/cf/shell/"
//#define _FILE_PATH	""
//#define _FILE_PATH	""
//#define _FILE_PATH	""

#define MONGO_CONNECT_STR	"mongodb://10.130.152.27:27017"
#define MONGO_CMONITOR_DB	"cmonitor_db"
#define MONGO_ALIVE_HOST_COL	"alive_host_col"
#define MONGO_DEAD_HOST_COL	"dead_host_col"








#endif // MACRO_DEFINITION_H

