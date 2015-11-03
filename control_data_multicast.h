#ifndef CONTROL_DATA_MULTICAST_H
#define CONTROL_DATA_MULTICAST_H

#include "macro_definition.h"
#include "c_str.h"
#include "c_collection.h"
#include "configuration.h"
#include "c_mongodb.h"

void activate_control_data_multicast(void);
bool add_machine(char *uuid, char *machine_ip);
bool sync_alive_machines(char *uuid, char *machine_ip);
bool sync_dead_machines(char *uuid, char *machine_ip);
bool sync_alive_machines_mul(void);
bool sync_dead_machines_mul(char *uuid, char *target_ip);
void mulcast_sync_dg(char *data);
bool add_machine_to_file(char *uuid, char *machine_ip);
bool del_machine_from_file(char *uuid);


#endif // CONTROL_DATA_MULTICAST_H

