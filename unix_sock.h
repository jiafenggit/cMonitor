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
#include "macro_definition.h"


void activate_unix_sock_server(void);
char *us_encap_datagram(int dg_type, char *datagram);
void respond_dg(char *data_gram);
bool merge_hour_to_day(void);
char *send_dg_to_us(char *request_dg);
bool add_machine_to_file(char *uuid, char *machine_ip);
bool sync_alive_machines_mul(void);
bool del_machine_from_file(char *uuid, char *machine_ip);
bool sync_dead_machines_mul(char *uuid, char *target_ip);
void mulcast_sync_dg(char *data);
char *send_and_recv_to_us(char *request_dg);
//bool parse_datagram(char *datagram, dict *dg_dict);



void unix_sock_test(void);
#endif // UNIX_SOCK

