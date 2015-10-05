#ifndef SOLIDER_H
#define SOLIDER_H

#include "c_collection.h"
#include "unix_sock.h"
#include "configuration.h"
#include "macro_definition.h"
#include <arpa/inet.h>
#include <netinet/in.h>


void activate_solider_collect(void);
void activate_solider_listen(void);

void activate_solider_scaleout(void);
void machine_scale_out(void);
void respond_hb(int client_sock, char *buf);
void mulcast_dg(char *json_data);
void mulcast_scaleout_dg(char *data);

void activate_solider_merge(void);
char *mul_encap_datagram(int dg_type, char *datagram);
bool mul_parse_datagram(char *datagram, dict *dg_dict);
bool merge_solider_rtdg(dict *solider_rt_dict, dict *dg_dict, char *buf);
bool save_rr_dg(cJSON *rt_dg);
void save_rt_dg_to_all(cJSON *rt_dg);
char *listen_encap_datagram(int type, ...);
char *fetch_alive_machines(void);
bool add_machine(char *uuid, char *machine_ip);
bool sync_alive_machines(char *uuid, char *machine_ip);
bool sync_dead_machines(char *uuid, char *machine_ip);
char *fetch_rt_dg_from_file(void);
int mul_test(void);
#endif // SOLIDER_H

