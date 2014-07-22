#ifndef UTIL__MONGO_H
#define UTIL__MONGO_H

#include <mongo/client/dbclient.h>
#include <mongo/bson/bson.h>

class MongoClient{
public:
	static mongo::DBClientConnection mongo_conn;
};

#endif
