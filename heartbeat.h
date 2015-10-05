#ifndef HEARTBEAT_H
#define HEARTBEAT_H

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
#include "macro_definition.h"
#include "configuration.h"
#include "c_collection.h"
#include "c_str.h"
#include "unix_sock.h"

void activate_solider_heartbeat(void);
char *fetch_target_ip(void);
char *fetch_target_uuid(char *target_ip);
bool heartbeat_check(char *target_ip);
bool del_machine(char *uuid, char *machine_ip);


#endif // HEARTBEAT_H

