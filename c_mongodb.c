#include "c_mongodb.h"

mongoc_collection_t *create_mongo_con(void)
{
	mongoc_client_t *mongo_con;

	mongo_con = mongoc_client_new (MONGO_CONNECT_STR);

	if (!mongo_con) {
		return NULL;
	}
	return mongo_con;
}

bool add_host_to_mongo(char *uuid, char *host_ip)
{
	mongoc_init ();
	mongoc_client_t *mongo_con = create_mongo_con();
	mongoc_collection_t *mongo_col;
	mongo_col = mongoc_client_get_collection(mongo_con, MONGO_CMONITOR_DB, MONGO_ALIVE_HOST_COL);
	bson_t *host_info;
	bson_t *query_str = bson_new ();
	BSON_APPEND_INT32(query_str, "uuid", atoi(uuid));
	mongoc_cursor_t *query_cursor;
	printf("bson:%s\n", bson_as_json(query_str, NULL));
	query_cursor = mongoc_collection_find (mongo_col, MONGOC_QUERY_NONE, 0, 0, 0, query_str, NULL, NULL);
	bson_t *reply;
	if (mongoc_cursor_next (query_cursor, &reply))
	{
		bson_t *activate_host = NULL;
		bson_error_t update_error;
		activate_host = BCON_NEW("$set", "{",
					 "activated", BCON_BOOL(true),
					 "}");
		if (!mongoc_collection_update(mongo_col, MONGOC_UPDATE_NONE, query_str, activate_host, NULL, &update_error))
		{
			fprintf(stderr, "Exec add_host_to_mongo/mongoc_collection_update function failed.\n%s\n", update_error.message);
			if (query_str)
				bson_destroy(query_str);
			if (activate_host)
				bson_destroy(activate_host);
			mongoc_cursor_destroy(query_cursor);
			mongoc_collection_destroy(mongo_col);
			mongoc_client_destroy(mongo_con);
			mongoc_cleanup();
			return false;
		}
		if (query_str)
			bson_destroy(query_str);
		if (activate_host)
			bson_destroy(activate_host);
		mongoc_cursor_destroy(query_cursor);
		mongoc_collection_destroy(mongo_col);
		mongoc_client_destroy(mongo_con);
		mongoc_cleanup();
		return true;
	}
	host_info = bson_new();
	bson_oid_t oid;
	bson_oid_init(&oid, NULL);
	BSON_APPEND_OID(host_info, "_id", &oid);
	BSON_APPEND_INT32(host_info, "uuid", atoi(uuid));
	BSON_APPEND_UTF8(host_info, "host_ip", host_ip);
	BSON_APPEND_BOOL(host_info, "activated", true);
	bson_error_t insert_error;
	if (!mongoc_collection_insert(mongo_col, MONGOC_INSERT_NONE, host_info, NULL, &insert_error))
	{
		fprintf(stderr, "Exec add_host_to_mongo/mongoc_collection_insert function failed.\n%s\n", insert_error.message);
		if (host_info)
			bson_destroy(host_info);
		if (query_str)
			bson_destroy(query_str);
		mongoc_cursor_destroy(query_cursor);
		mongoc_collection_destroy(mongo_col);
		mongoc_client_destroy(mongo_con);
		mongoc_cleanup();
		return false;
	}
	if (host_info)
		bson_destroy(host_info);
	if (query_str)
		bson_destroy(query_str);
	mongoc_cursor_destroy(query_cursor);
	mongoc_collection_destroy(mongo_col);
	mongoc_client_destroy(mongo_con);
	mongoc_cleanup();

	return true;
}

bool del_host_from_mongo(char *uuid)
{
	return true;
}
