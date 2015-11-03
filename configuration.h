#ifndef CONFIGURATION_H
#define CONFIGURATION_H



#include "macro_definition.h"
#include "c_str.h"
#include "cJSON.h"

bool init_conf(void);
char *create_conf_json(void);
char *fetch_key_key_value_str(char *first_key, char *second_key);
bool fetch_key_key_value_bool(char *first_key, char *second_key);
int fetch_key_key_value_int(char *first_key, char *second_key);

#endif // CONFIGURATION_H

