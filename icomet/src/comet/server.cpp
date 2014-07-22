#include "../build.h"
#include <http-internal.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>
#include "server.h"
#include "subscriber.h"
#include "server_config.h"
#include "util/log.h"
#include "util/list.h"

#define USE_MEM_POOL 0
using namespace mongo;

mongo::DBClientConnection MongoClient::mongo_conn;
class HttpQuery{
private:
	struct evkeyvalq params;
public:
	HttpQuery(struct evhttp_request *req){
		evhttp_parse_query(evhttp_request_get_uri(req), &params);
	}
	int get_int(const char *name, int def){
		const char *val = evhttp_find_header(&params, name);
		return val? atoi(val) : def;
	}
	~HttpQuery(){
	 		evhttp_clear_headers(&params);
	 }
	const char* get_str(const char *name, const char *def){
		const char *val = evhttp_find_header(&params, name);
		return val? val : def;
	}
};

Server::Server(){
	this->auth = AUTH_NONE;
	subscribers = 0;
	MongoClient::mongo_conn.connect("localhost");
#if USE_MEM_POLL
	channel_slots.resize(ServerConfig::max_channels);
	for(int i=0; i<channel_slots.size(); i++){
		Channel *channel = &channel_slots[i];
		free_channels.push_back(channel);
	}
	sub_pool.pre_alloc(1024);
#else
#endif
}

Server::~Server(){
		LinkedList<Channel *>::Iterator it = used_channels.iterator();
	 	while(Channel *channel = it.next()){
	 		LinkedList<Subscriber *>::Iterator it2 = channel->subs.iterator();
	 		while(Subscriber *sub = it2.next()){
	 			delete sub;
	 		}
	 		delete channel;
	 	}
}

Channel* Server::get_channel_by_name(const std::string &cname){
	std::map<std::string, Channel *>::iterator it;
	BSONObj channel_obj =
			MongoClient::mongo_conn.findOne("icomet.group", QUERY("cname"<<cname));

	if(channel_obj.isEmpty()){
		return NULL;
	}
	//通道是否在线
	it = cname_channels.find(cname);
	if(it == cname_channels.end()){
		//上线并返回
		unsigned long msgcount = MongoClient::mongo_conn.count("icomet.message", BSON("cname"<<cname));
//		auto_ptr<DBClientCursor> list = MongoClient::mongo_conn.query("icomet.message",QUERY("cname"<<cname));
//		while (list->more()){
//			BSONObj it = list->next();
//			printf("%s:create time :%s\n",it.getStringField("cname"),it.getField("create_time").toString());
//		}
		Channel *channel=new Channel();
		channel->BSONToChannel(channel_obj,msgcount);
		used_channels.push_back(channel);
		cname_channels[channel->name] = channel;
		add_presence(PresenceOffline, channel->name);

		return channel;
	}else{
		//已在线直接返回
		return it->second;
	}


//	return it->second;
//	return channel;
}

Channel* Server::new_channel(const std::string &cname){
	if(used_channels.size >= ServerConfig::max_channels){
		return NULL;
	}
#if USE_MEM_POLL
	Channel *channel = free_channels.head;
	assert(channel->subs.size == 0);
	// first remove, then push_back, do not mistake the order
	free_channels.remove(channel);
#else
	Channel *channel = new Channel();
#endif
	used_channels.push_back(channel);

	channel->name = cname;
	cname_channels[channel->name] = channel;
	log_debug("new channel: %s", channel->name.c_str());
	
	add_presence(PresenceOnline, channel->name);
	
	if(channel->token.empty()){
		channel->create_token();
	}

	char cannel_json[200];
	sprintf(cannel_json,"{\"type\":\"sign\",\"cname\":\"%s\",\"seq\":%d,\"token\":\"%s\",\"expires\":%d,\"sub_timeout\":%d ,\"users\":[]}",
	channel->name.c_str(),
	channel->msg_seq_min(),
	channel->token.c_str(),
	ServerConfig::channel_timeout,
	ServerConfig::polling_timeout);

	MongoClient::mongo_conn.insert("icomet.group",mongo::fromjson(cannel_json));

	return channel;
}

void Server::free_channel(Channel *channel){
	assert(channel->subs.size == 0);
	log_debug("free channel: %s", channel->name.c_str());
	add_presence(PresenceOffline, channel->name);

	cname_channels.erase(channel->name);
	// first remove, then push_back, do not mistake the order
	used_channels.remove(channel);
#if USE_MEM_POLL
	free_channels.push_back(channel);
	channel->reset();
#else
	LinkedList<Subscriber *>::Iterator it2 = channel->subs.iterator();
	while(Subscriber *sub = it2.next()){
		delete sub;
	}
	delete channel;
#endif
}

int Server::check_timeout(){
	//log_debug("<");
	LinkedList<Channel *>::Iterator it = used_channels.iterator();
	while(Channel *channel = it.next()){
		if(channel->subs.size == 0){
			if(--channel->idle < 0){
				this->free_channel(channel);
			}
			continue;
		}
		if(channel->idle < ServerConfig::channel_idles){
			channel->idle = ServerConfig::channel_idles;
		}

		LinkedList<Subscriber *>::Iterator it2 = channel->subs.iterator();
		while(Subscriber *sub = it2.next()){
			if(++sub->idle <= ServerConfig::polling_idles){
				continue;
			}
			sub->noop();
			sub->idle = 0;
		}
	}
	//log_debug(">");
	return 0;
}

void Server::add_presence(PresenceType type, const std::string &cname){
	if(psubs.empty()){
		return;
	}
	struct evbuffer *buf = evbuffer_new();
	evbuffer_add_printf(buf, "%d %s\n", type, cname.c_str());

	LinkedList<PresenceSubscriber *>::Iterator it = psubs.iterator();
	while(PresenceSubscriber *psub = it.next()){
		evhttp_send_reply_chunk(psub->req, buf);

		//struct evbuffer *output = bufferevent_get_output(req->evcon->bufev);
		//if(evbuffer_get_length(output) > MAX_OUTPUT_BUFFER){
		//  close_presence_subscriber();
		//}
	}
	
	evbuffer_free(buf);
}

static void on_psub_disconnect(struct evhttp_connection *evcon, void *arg){
	log_trace("presence subscriber disconnected");
	PresenceSubscriber *psub = (PresenceSubscriber *)arg;
	Server *serv = psub->serv;
	serv->psub_end(psub);
}

int Server::psub(struct evhttp_request *req){
	bufferevent_enable(req->evcon->bufev, EV_READ);

	PresenceSubscriber *psub = new PresenceSubscriber();
	psub->req = req;
	psub->serv = this;
	psubs.push_back(psub);
	log_debug("%s:%d psub, psubs: %d", req->remote_host, req->remote_port, psubs.size);

	evhttp_send_reply_start(req, HTTP_OK, "OK");
	evhttp_connection_set_closecb(req->evcon, on_psub_disconnect, psub);
	return 0;
}

int Server::psub_end(PresenceSubscriber *psub){
	struct evhttp_request *req = psub->req;
	psubs.remove(psub);
	log_debug("%s:%d psub_end, psubs: %d", req->remote_host, req->remote_port, psubs.size);
	return 0;
}

int Server::poll(struct evhttp_request *req){
	return this->sub(req, Subscriber::POLL);
}

int Server::iframe(struct evhttp_request *req){
	return this->sub(req, Subscriber::IFRAME);
}

int Server::stream(struct evhttp_request *req){
	return this->sub(req, Subscriber::STREAM);
}

//群通道上线
int Server::sub(struct evhttp_request *req, Subscriber::Type sub_type){
	if(evhttp_request_get_command(req) != EVHTTP_REQ_GET){
		evhttp_send_reply(req, 405, "Method Not Allowed", NULL);
		return 0;
	}
	bufferevent_enable(req->evcon->bufev, EV_READ);

	HttpQuery query(req);
	int seq = query.get_int("seq", 0);
	int noop = query.get_int("noop", 0);
	std::string username = query.get_str("username","");
	const char *cb = query.get_str("cb", "");
	const char *token = query.get_str("token", "");
	std::string cname = query.get_str("cname", "");

	mongo::auto_ptr<DBClientCursor> gu_list_mongo;
	gu_list_mongo = MongoClient::mongo_conn.query("icomet.group_user_staus",QUERY("username"  << username));

#if USE_MEM_POLL
		Subscriber *sub = sub_pool.alloc();
	#else
		Subscriber *sub = new Subscriber();
	#endif
		//BSONObj userBSON = MongoClient::mongo_conn.findOne("icomet.group_user_staus",QUERY("cname" << cname << "username" << username));
		sub->req = req;
		sub->serv = this;
		sub->type = sub_type;
		sub->idle = 0;
		//sub->seq_next = userBSON.getIntField("seq");
		sub->seq_noop = noop;
		sub->callback = cb;
		sub->username = username;



	while(gu_list_mongo->more()){
		BSONObj obj = gu_list_mongo->next();
		Channel *channel = this->get_channel_by_name(obj.getStringField("cname"));
		if(!channel && this->auth == AUTH_NONE){
			channel = this->new_channel(cname);
			if(!channel){
				//evhttp_send_reply(req, 429, "Too many channels", NULL);
				Subscriber::send_error_reply(sub_type, req, cb, "429", "Too many channels");
				return 0;
			}
		}
		if(!channel || (this->auth == AUTH_TOKEN && channel->token != token)){
			//evhttp_send_reply(req, 401, "Token error", NULL);
			Subscriber::send_error_reply(sub_type, req, cb, "401", "Token error");
			return 0;
		}
		if(channel->subs.size >= ServerConfig::max_subscribers_per_channel){
			//evhttp_send_reply(req, 429, "Too many subscribers", NULL);
			Subscriber::send_error_reply(sub_type, req, cb, "429", "Too many subscribers");
			return 0;
		}

		if(channel->idle < ServerConfig::channel_idles){
			channel->idle = ServerConfig::channel_idles;
		}
		sub->add_listen_channel(channel,obj.getIntField("seq"));

				//channel->add_subscriber(sub);
		subscribers ++;
		log_debug("%s:%d sub %s, subs: %d, channels: %d",
			req->remote_host, req->remote_port,
			channel->name.c_str(), channel->subs.size,
			used_channels.size);
	
	}
	sub->start();

	return 0;
}

int Server::sub_end(Subscriber *sub){
	struct evhttp_request *req = sub->req;
	//Channel *channel = sub->channel;
	//channel->del_subscriber(sub);

	LinkedList<Channel *>::Iterator chan_it = sub->listen_channels.iterator();
	while(Channel *channel = chan_it.next()){
		delete channel;
	}

	subscribers --;
//	log_debug("%s:%d sub_end %s, subs: %d, channels: %d",
//		req->remote_host, req->remote_port,
//		channel->name.c_str(), channel->subs.size,
//		used_channels.size);
	log_debug("%s:%d sub_end",req->remote_host, req->remote_port);
#if USE_MEM_POLL
	sub_pool.free(sub);
#else
	delete sub;
#endif
	return 0;
}

int Server::ping(struct evhttp_request *req){
	HttpQuery query(req);
	const char *cb = query.get_str("cb", DEFAULT_JSONP_CALLBACK);

	struct evbuffer *buf = evbuffer_new();
	evbuffer_add_printf(buf,
		"%s({\"type\":\"ping\",\"sub_timeout\":%d});\n",
		cb,
		ServerConfig::polling_timeout);
	evhttp_send_reply(req, HTTP_OK, "OK", buf);
	evbuffer_free(buf);
	return 0;
}

//群通道上线的，开始发送消息
int Server::pub(struct evhttp_request *req){
	if(evhttp_request_get_command(req) != EVHTTP_REQ_GET){
		evhttp_send_reply(req, 405, "Invalid Method", NULL);
		return 0;
	}
	
	HttpQuery query(req);
	const char *cb = query.get_str("cb", NULL);
	std::string cname = query.get_str("cname", "");
	const char *content = query.get_str("content", "");
	std::string username = query.get_str("username", "");
	
	Channel *channel = NULL;
	channel = this->get_channel_by_name(cname);
	if(!channel || channel->idle == -1){
		channel = this->new_channel(cname);
		if(!channel){
			evhttp_send_reply(req, 429, "Too Many Channels", NULL);
			return 0;
		}
		int expires = ServerConfig::channel_timeout;
		log_debug("auto sign channel on pub, cname:%s, t:%s, expires:%d",
			cname.c_str(), channel->token.c_str(), expires);
		channel->idle = expires/CHANNEL_CHECK_INTERVAL;
		/*
		struct evbuffer *buf = evbuffer_new();
		log_trace("cname[%s] not connected, not pub content: %s", cname.c_str(), content);
		evbuffer_add_printf(buf, "cname[%s] not connected\n", cname.c_str());
		evhttp_send_reply(req, 404, "Not Found", buf);
		evbuffer_free(buf);
		return 0;
		*/
	}
	log_debug("channel: %s, subs: %d, pub content: %s", channel->name.c_str(), channel->subs.size, content);
		
	// response to publisher
	evhttp_add_header(req->output_headers, "Content-Type", "text/javascript; charset=utf-8");
	struct evbuffer *buf = evbuffer_new();
	if(cb){
		evbuffer_add_printf(buf, "%s(", cb);
	}
	evbuffer_add_printf(buf, "{\"type\":\"ok\"}");
	if(cb){
		evbuffer_add(buf, ");\n", 3);
	}else{
		evbuffer_add(buf, "\n", 1);
	}
	evhttp_send_reply(req, 200, "OK", buf);
	evbuffer_free(buf);

	// push to subscribers
	if(channel->idle < ServerConfig::channel_idles){
	 	channel->idle = ServerConfig::channel_idles;
	}
	channel->send("data", content,username);
	MongoClient::mongo_conn.update("icomet.group",QUERY("cname" << cname),BSON("$set"<<BSON("seq" << channel->seq_next)));
	return 0;
}

//如果群不存在则创建，并且将群上线，将用户与群在mongo中绑定
int Server::sign(struct evhttp_request *req){
	HttpQuery query(req);
	int expires = query.get_int("expires", -1);
	const char *cb = query.get_str("cb", NULL);
	std::string cname = query.get_str("cname", "");
	std::string username = query.get_str("username", "");
	std::string password = query.get_str("password", "");

	if(expires <= 0){
		expires = ServerConfig::channel_timeout;
	}
	

	//
	Channel *channel = this->get_channel_by_name(cname);
	if(!channel){
		channel = this->new_channel(cname);
	}	
	if(!channel){
		evhttp_send_reply(req, 429, "Too Many Channels", NULL);
		return 0;
	}

	if(channel->idle == -1){
		log_debug("%s:%d sign cname:%s, t:%s, expires:%d",
			req->remote_host, req->remote_port,
			cname.c_str(), channel->token.c_str(), expires);
	}else{
		log_debug("%s:%d re-sign cname:%s, t:%s, expires:%d",
			req->remote_host, req->remote_port,
			cname.c_str(), channel->token.c_str(), expires);
	}
	channel->idle = expires/CHANNEL_CHECK_INTERVAL;

	//add user

	if (MongoClient::mongo_conn.findOne("icomet.group_user_staus",QUERY("cname" << cname << "username" << username)).isEmpty()){
		BSONObj obj = BSON("cname" << cname << "username" << username << "seq" << 0 << "noop" << 0);
		MongoClient::mongo_conn.insert("icomet.group_user_staus",obj);
	}

	if (MongoClient::mongo_conn.findOne("icomet.user",QUERY("username" << username)).isEmpty()){
		MongoClient::mongo_conn.insert("icomet.user",BSON("username" << username << "password" << password));
	}

	evhttp_add_header(req->output_headers, "Content-Type", "text/html; charset=utf-8");
	struct evbuffer *buf = evbuffer_new();
	if(cb){
		evbuffer_add_printf(buf, "%s(", cb);
	}

	char cannel_json[200];
	sprintf(cannel_json,"{\"type\":\"sign\",\"cname\":\"%s\",\"seq\":%d,\"token\":\"%s\",\"expires\":%d,\"sub_timeout\":%d}",
		channel->name.c_str(),
		channel->msg_seq_min(),
		channel->token.c_str(),
		expires,
		ServerConfig::polling_timeout);

	evbuffer_add_printf(buf,cannel_json);
	if(cb){
		evbuffer_add(buf, ");\n", 3);
	}else{
		evbuffer_add(buf, "\n", 1);
	}
	evhttp_send_reply(req, 200, "OK", buf);
	evbuffer_free(buf);

	return 0;
}

int Server::close(struct evhttp_request *req){
	HttpQuery query(req);
	std::string cname = query.get_str("cname", "");

	Channel *channel = this->get_channel_by_name(cname);
	if(!channel){
		log_warn("channel %s not found", cname.c_str());
		struct evbuffer *buf = evbuffer_new();
		evbuffer_add_printf(buf, "channel[%s] not connected\n", cname.c_str());
		evhttp_send_reply(req, 404, "Not Found", buf);
		evbuffer_free(buf);
		return 0;
	}
	log_debug("close channel: %s, subs: %d", cname.c_str(), channel->subs.size);
		
	// response to publisher
	evhttp_add_header(req->output_headers, "Content-Type", "text/html; charset=utf-8");
	struct evbuffer *buf = evbuffer_new();
	evbuffer_add_printf(buf, "ok %d\n", channel->seq_next);
	evhttp_send_reply(req, 200, "OK", buf);
	evbuffer_free(buf);

	// push to subscribers
	if(channel->idle != -1){
		channel->send("close", "","");
		this->free_channel(channel);
	}

	return 0;
}

int Server::clear(struct evhttp_request *req){
	HttpQuery query(req);
	std::string cname = query.get_str("cname", "");

	Channel *channel = this->get_channel_by_name(cname);
	if(!channel){
		log_warn("channel %s not found", cname.c_str());
		struct evbuffer *buf = evbuffer_new();
		evbuffer_add_printf(buf, "channel[%s] not connected\n", cname.c_str());
		evhttp_send_reply(req, 404, "Not Found", buf);
		evbuffer_free(buf);
		return 0;
	}
	log_debug("clear channel: %s, subs: %d", cname.c_str(), channel->subs.size);
		
	// response to publisher
	evhttp_add_header(req->output_headers, "Content-Type", "text/html; charset=utf-8");
	struct evbuffer *buf = evbuffer_new();
	evbuffer_add_printf(buf, "ok %d\n", channel->seq_next);
	evhttp_send_reply(req, 200, "OK", buf);
	evbuffer_free(buf);

	channel->clear();

	return 0;
}

int Server::info(struct evhttp_request *req){
	HttpQuery query(req);
	std::string cname = query.get_str("cname", "");

	evhttp_add_header(req->output_headers, "Content-Type", "text/html; charset=utf-8");
	struct evbuffer *buf = evbuffer_new();
	if(!cname.empty()){
		Channel *channel = this->get_channel_by_name(cname);
		// TODO: if(!channel) 404
		int onlines = channel? channel->subs.size : 0;
		evbuffer_add_printf(buf,
			"{cname: \"%s\", subscribers: %d}\n",
			cname.c_str(),
			onlines);
	}else{
		evbuffer_add_printf(buf,
			"{channels: %d, subscribers: %d}\n",
			used_channels.size,
			subscribers);
	}
	evhttp_send_reply(req, 200, "OK", buf);
	evbuffer_free(buf);

	return 0;
}

int Server::check(struct evhttp_request *req){
	HttpQuery query(req);
	std::string cname = query.get_str("cname", "");

	evhttp_add_header(req->output_headers, "Content-Type", "text/html; charset=utf-8");
	struct evbuffer *buf = evbuffer_new();
	Channel *channel = this->get_channel_by_name(cname);
	if(channel && channel->idle != -1){
		evbuffer_add_printf(buf, "{\"%s\": 1}\n", cname.c_str());
	}else{
		evbuffer_add_printf(buf, "{}\n");
	}
	evhttp_send_reply(req, 200, "OK", buf);
	evbuffer_free(buf);

	return 0;
}
