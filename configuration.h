#ifndef CONFIGURATION_H
#define CONFIGURATION_H


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

#include "c_str.h"
#include "cJSON.h"

bool init_conf(void);
char *create_conf_json(void);
char *fetch_key_key_value_str(char *first_key, char *second_key);
bool fetch_key_key_value_bool(char *first_key, char *second_key);
int fetch_key_key_value_int(char *first_key, char *second_key);

#endif // CONFIGURATION_H

