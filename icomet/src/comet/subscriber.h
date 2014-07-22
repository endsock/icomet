#ifndef ICOMET_SUBSCRIBER_H
#define ICOMET_SUBSCRIBER_H

#include <map>
#include <string>
#include <evhttp.h>
#include "util/list.h"

class Server;
class Channel;


class Subscriber{
public:
	enum Type{
		POLL	= 0,
		STREAM	= 1,
		IFRAME	= 2
	};
public:
	Subscriber *prev;
	Subscriber *next;
	
	Server *serv;
	//Channel *channel;

	LinkedList<Channel *> listen_channels;

	//std::map<std::string , int > listen_channels_map;
	std::string callback;
	int type;
	int idle;
	// 当类型是 data 时的 seq, 和类型是 noop 时用的不是同一套 seq.
	//int seq_next;
	// seq_noop 用于 js long-polling 判断重复连接
	int seq_noop;
	std::string username;
	struct evhttp_request *req;

	void start();
	void close();
	void send_chunk(int seq, const char *type, const char *content);
	
	void noop();
	void send_old_msgs();
	
	void add_listen_channel(Channel* chanl,int seq);

	static void send_error_reply(int sub_type, struct evhttp_request *req, const char *cb, const char *type, const char *content);
};

#endif
