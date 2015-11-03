#ifndef MONITOR_DATA_MULTICAST_H
#define MONITOR_DATA_MULTICAST_H

#include "macro_definition.h"
#include "c_str.h"
#include "c_collection.h"
#include "configuration.h"
#include "c_mongodb.h"
#include "rehash_dict.h"
#include "cJSON.h"
#include "murmurhash.h"


void activate_monitor_data_multicast(void);
bool mul_parse_datagram(char *datagram, dict *dg_dict);
bool merge_solider_rtdg(dict *solider_rt_dict, dict *dg_dict, char *buf);
bool save_rr_dg(cJSON *rt_dg);
bool save_rt_dg_to_all(cJSON *rt_dg);

#endif // MONITOR_DATA_MULTICAST_H

