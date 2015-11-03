#ifndef HEARTBEAT_H
#define HEARTBEAT_H


#include "macro_definition.h"
#include "configuration.h"
#include "c_collection.h"
#include "c_str.h"
#include "c_mongodb.h"
#include "control_data_multicast.h"

void activate_solider_heartbeat(void);
char *fetch_target_ip(void);
char *fetch_target_uuid(char *target_ip);
bool heartbeat_check(char *target_ip);
bool del_machine(char *uuid, char *machine_ip);


#endif // HEARTBEAT_H

