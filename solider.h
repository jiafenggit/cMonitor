#ifndef SOLIDER_H
#define SOLIDER_H

#include "c_collection.h"
#include "unix_sock.h"
#include "configuration.h"
#include "macro_definition.h"
#include "monitor_data_multicast.h"


void activate_solider_collect(void);
void activate_solider_listen(void);


void machine_scale_out(void);
bool respond_hb(int client_sock);
bool mulcast_dg(char *json_data);
bool mulcast_scaleout_dg(char *data);

char *mul_encap_datagram(int dg_type, char *datagram);
bool save_rr_dg(cJSON *rt_dg);
bool save_rt_dg_to_all(cJSON *rt_dg);
char *listen_encap_datagram(int type, ...);
char *fetch_alive_machines(void);

char *fetch_rt_dg_from_file(void);
int mul_test(void);
#endif // SOLIDER_H

