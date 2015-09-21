#ifndef UNIX_SOCK
#define UNIX_SOCK


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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/file.h>
#include <stdbool.h>
#include<sys/un.h>
#include "c_collection.h"

#define DG_MAX_SIZE 5120
#define MERGE_HOUR_TO_DAY 1024
#define REQUEST_ALIVE_IP 1025


void activate_unix_sock_server(void);
char *us_encap_datagram(int dg_type, char *datagram);
//bool parse_datagram(char *datagram, dict *dg_dict);



void unix_sock_test(void);
#endif // UNIX_SOCK

