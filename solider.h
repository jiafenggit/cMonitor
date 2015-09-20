#ifndef SOLIDER_H
#define SOLIDER_H

#include "c_collection.h"
#include <arpa/inet.h>
#include <netinet/in.h>

#define RT_HOST			0
#define RT_GROUP		1
#define RT_CLUSTER		2
#define RT_HEARTBEAT	3
#define RT_OFFICER		4
#define MCAST_PORT		8192
#define MAX_BUF_SIZE		32768

void activate_solider_collect(void);
void mulcast_dg(char *json_data);
void mulcast_solider_dg(char *data);
char *fetch_key_key_value(char *first_key, char *second_key);
char *fetch_key_key_value_str(char *first_key, char *second_key);
bool fetch_key_key_value_bool(char *first_key, char *second_key);
int fetch_key_key_value_int(char *first_key, char *second_key);
void activate_solider_merge(void);
char *encap_datagram(int dg_type, char *datagram);
bool parse_datagram(char *datagram, dict *dg_dict);
bool merge_solider_rtdg(dict *solider_rt_dict, dict *dg_dict, char *buf);
bool save_rr_dg(cJSON *rt_dg);


int mul_test(void);
#endif // SOLIDER_H

