#include "subscriber.h"
#include "channel.h"
#include "server.h"
#include "util/log.h"
#include "server_config.h"

using namespace mongo;

static std::string iframe_header = "<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'><meta http-equiv='Cache-Control' content='no-store'><meta http-equiv='Cache-Control' content='no-cache'><meta http-equiv='Pragma' content='no-cache'><meta http-equiv=' Expires' content='Thu, 1 Jan 1970 00:00:00 GMT'><script type='text/javascript'>window.onError = null;try{document.domain = window.location.hostname.split('.').slice(-2).join('.');}catch(e){};</script></head><body>";
static std::string iframe_chunk_prefix = "<script>parent.icomet_cb(";
static std::string iframe_chunk_suffix = ");</script>";

static void on_sub_disconnect(struct evhttp_connection *evcon, void *arg){
	log_debug("subscriber disconnected");
	Subscriber *sub = (Subscriber *)arg;
	Server *serv = sub->serv;
	serv->sub_end(sub);
}

void Subscriber::start(){
	evhttp_add_header(req->output_headers, "Connection", "keep-alive");
	//evhttp_add_header(req->output_headers, "Cache-Control", "no-cache");
	//evhttp_add_header(req->output_headers, "Expires", "0");
	evhttp_add_header(req->output_headers, "Content-Type", "text/html; charset=utf-8");
	
	evhttp_send_reply_start(req, HTTP_OK, "OK");
	evhttp_connection_set_closecb(req->evcon, on_sub_disconnect, this);

	if(this->type == POLL){
		//
	}else if(this->type == IFRAME){
		struct evbuffer *buf = evbuffer_new();
		evbuffer_add_printf(buf, "%s\n", iframe_header.c_str());
		evhttp_send_reply_chunk(this->req, buf);
		evbuffer_free(buf);
	}
	
	// send buffered messages
	this->send_old_msgs();

}

void Subscriber::add_listen_channel(Channel* chanl,int seq){
	chanl->user_seq_next=seq;
	this->listen_channels.push_back(chanl);
}

void Subscriber::send_old_msgs(){
	int totalcount = 0;
	bool isfull = false;
	LinkedList<Channel *>::Iterator chan_it = this->listen_channels.iterator();
	struct evbuffer *buf = evbuffer_new();
	if(this->type == POLL){
		if(!this->callback.empty()){
			evbuffer_add_printf(buf, "%s(", this->callback.c_str());
		}
		evbuffer_add_printf(buf, "[");
		std::string content ="";
		while(Channel *channel = chan_it.next()){
			int seq_next=0;
			seq_next=channel->user_seq_next;
			if(seq_next==0){
				continue;
			}
			if(channel->msg_count!=0 && channel->seq_next > seq_next){
				mongo::auto_ptr<DBClientCursor> msg_list_mongo;
					if(Channel::SEQ_LT(seq_next, channel->seq_next)){
						msg_list_mongo = MongoClient::mongo_conn.query("icomet.message",QUERY("seq" << mongo::GT << seq_next));
					}else if(Channel::SEQ_GT(seq_next, channel->seq_next)){
						printf("send_old_msgs seq error");
					}else{
						return;
					}
					log_debug("send old msg: [%d, %d]", seq_next, channel->seq_next - 1);
					//it -= (channel->seq_next - this->seq_next);




						//for(/**/; it != channel->msg_list.end(); it++, this->seq_next++){

						while(msg_list_mongo->more()){
							content.append(",");
							BSONObj obj = msg_list_mongo->next();
							char seqbuf[10];
							sprintf(seqbuf, "%d", seq_next);
							std::string seq_str = seqbuf;
							content.append("{\"type\":\"data\",\"cname\":\"").append(channel->name).append("\",\"seq\":").append(seq_str).append(",\"content\":\"").append(obj.getStringField("content")).append("\"}");


//							evbuffer_add_printf(buf,
//								"{\"type\":\"data\",\"cname\":\"%s\",\"seq\":%d,\"content\":\"%s\"}",
//								channel->name.c_str(),
//								seq_next,	//返回给客户端下标从0开始，服务端记录的是1开始
//								obj.getStringField("content"));
//							if ((seq_next < channel->seq_next - 1)  && (Channel::SEQ_LT(totalcount,ServerConfig::channel_buffer_size-1))){
//								evbuffer_add(buf, ",", 1);
//
//							}
							seq_next++;//+1之后服务端开始记录
							totalcount++;
							if(Channel::SEQ_GE(totalcount,ServerConfig::channel_buffer_size)){
								isfull=true;
								break;
							}

						}
						if(!content.empty()){
							content.erase(0,1);
						}
						evbuffer_add_printf(buf,content.c_str());
						MongoClient::mongo_conn.update("icomet.group_user_staus",QUERY("cname" << channel->name << "username" << username),BSON("$set"<<BSON("seq" << seq_next)));

						evbuffer_add_printf(buf, "]");
						if(!this->callback.empty()){
							evbuffer_add_printf(buf, ");");
						}

						if(isfull){
							break;
						}



			}
		}
	}else if(this->type == IFRAME || this->type == STREAM){
					//		for(/**/; it != channel->msg_list.end(); it++, this->seq_next++){
					//			std::string &msg = *it;
					//			this->send_chunk(this->seq_next, "data", msg.c_str());
					//		}
	}
	evbuffer_add_printf(buf, "\n");
	evhttp_send_reply_chunk(this->req, buf);
	this->close();
	evbuffer_free(buf);
//	std::vector<std::string>::iterator it = channel->msg_list.end();
//	int msg_seq_min = channel->seq_next - channel->msg_list.size();
//	//防止用户的seq小于0

//	if(Channel::SEQ_LT(ServerConfig::channel_buffer_size, span_count)){
//		seq_next = channel->seq_next-ServerConfig::channel_buffer_size;
//	}

}

void Subscriber::close(){
//	MongoClient::mongo_conn.update("icomet.group_user_staus",QUERY("cname" << this->channel->name << "username" << this->username ),BSON("$set"<<BSON("seq" << this->seq_next)),true,true);
	evhttp_send_reply_end(this->req);
	evhttp_connection_set_closecb(this->req->evcon, NULL, NULL);
	this->serv->sub_end(this);
}

void Subscriber::noop(){
	this->send_chunk(this->seq_noop, "noop", "");
}

void Subscriber::send_chunk(int seq, const char *type, const char *content){
	struct evbuffer *buf = evbuffer_new();
	
	if(this->type == POLL){
		if(!this->callback.empty()){
			evbuffer_add_printf(buf, "%s(", this->callback.c_str());
		}
	}else if(this->type == IFRAME){
		evbuffer_add_printf(buf, "%s", iframe_chunk_prefix.c_str());
	}
	
	evbuffer_add_printf(buf,
		//"{\"type\":\"%s\",\"cname\":\"%s\",\"seq\":%d,\"content\":\"%s\"}",
			"{\"type\":\"%s\",\"seq\":%d,\"content\":\"%s\"}",
		type,
		//this->channel->name.c_str(),
		seq,
		content);

	if(this->type == POLL){
		if(!this->callback.empty()){
			evbuffer_add_printf(buf, ");");
		}
	}else if(this->type == IFRAME){
		evbuffer_add_printf(buf, "%s", iframe_chunk_suffix.c_str());
	}

	evbuffer_add_printf(buf, "\n");
	evhttp_send_reply_chunk(this->req, buf);
	evbuffer_free(buf);

	this->idle = 0;
	if(this->type == POLL){
		this->close();
	}
}

void Subscriber::send_error_reply(int sub_type, struct evhttp_request *req, const char *cb, const char *type, const char *content){
	struct evbuffer *buf = evbuffer_new();
	
	if(sub_type == POLL){
		evbuffer_add_printf(buf, "%s(", cb);
	}else if(sub_type == IFRAME){
		evbuffer_add_printf(buf, "%s", iframe_chunk_prefix.c_str());
	}
	
	evbuffer_add_printf(buf,
		"{\"type\":\"%s\",\"cname\":\"%s\",\"seq\":%d,\"content\":\"%s\"}",
		type,
		"",
		0,
		content);

	if(sub_type == POLL){
		evbuffer_add_printf(buf, ");");
	}else if(sub_type == IFRAME){
		evbuffer_add_printf(buf, "%s", iframe_chunk_suffix.c_str());
	}

	evbuffer_add_printf(buf, "\n");
	evhttp_send_reply(req, HTTP_OK, "OK", buf);
	evbuffer_free(buf);
}

