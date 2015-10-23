#ifndef C_MONGODB_H
#define C_MONGODB_H

#include "macro_definition.h"


mongoc_collection_t *create_mongo_con(void);
bool add_host_to_mongo(char *uuid, char *host_ip);
bool del_host_from_mongo(char *uuid);

#endif // C_MONGODB_H

