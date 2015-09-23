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
#define HEARTBEAT_DG	2048
#define RESOND_HEARTBEAT	2049
#define REQUEST_RT_DG	2050
#define SCALEOUT_DG		2051
#define ADD_MACHINE        2052

#define SUCCESS 4096

#define MCAST_PORT		8192
#define MAX_BUF_SIZE		32768
#define DG_MAX_SIZE		 5120

void activate_solider_collect(void);
void activate_solider_listen(void);
void activate_solider_heartbeat(void);
void respond_hb(int client_sock, char *buf);
void mulcast_dg(char *json_data);
void mulcast_solider_dg(char *data);
char *fetch_key_key_value(char *first_key, char *second_key);
char *fetch_key_key_value_str(char *first_key, char *second_key);
bool fetch_key_key_value_bool(char *first_key, char *second_key);
int fetch_key_key_value_int(char *first_key, char *second_key);
void activate_solider_merge(void);
char *mul_encap_datagram(int dg_type, char *datagram);
bool mul_parse_datagram(char *datagram, dict *dg_dict);
bool merge_solider_rtdg(dict *solider_rt_dict, dict *dg_dict, char *buf);
bool save_rr_dg(cJSON *rt_dg);
char *listen_encap_datagram(int type, ...);
char *fetch_alive_machines(void);
bool add_machine(char *uuid, char *machine_ip);
int mul_test(void);
#endif // SOLIDER_H

