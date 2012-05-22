#include <err.h>
#include <event.h>
#include <evhttp.h>

#include "llserver.h"

/* 处理模块 */
void llserver_http_handler(struct evhttp_request *req, void *arg)
{
        struct evbuffer *buf = evbuffer_new();
   	
		/* 分析URL参数 */
		char *decode_uri = strdup((char*) evhttp_request_uri(req));
		struct evkeyvalq llserver_http_query;
		evhttp_parse_query(decode_uri, &llserver_http_query);
		free(decode_uri);
		
		/* 接收GET表单参数 */
	    const char *charset = evhttp_find_header (&llserver_http_query, "charset");
		const char *opt = evhttp_find_header (&llserver_http_query, "opt"); /* 操作类别 */
		const char *key = evhttp_find_header (&llserver_http_query, "key"); /* 存储键 */
		const char *input_value = evhttp_find_header (&llserver_http_query, "value"); /* 存储数据 */
		const char *exptime = evhttp_find_header (&llserver_http_query, "exptime"); /* 过期时间 */

	    /* 判断字符集并返回给用户的Header头信息 */
		if (charset != NULL && strlen(charset) <= 40) {
			char content_type[64] = {0};
			sprintf(content_type, "text/plain; charset=%s", charset);
			evhttp_add_header(req->output_headers, "Content-Type", content_type);
		} else 
			evhttp_add_header(req->output_headers, "Content-Type", "text/plain");

		evhttp_add_header(req->output_headers, "Connection", "keep-alive");
		evhttp_add_header(req->output_headers, "Cache-Control", "no-cache");
		evhttp_add_header(req->output_headers, "Connection", "close");						
	
		/*参数是否存在判断 */
		if (opt != NULL) {
			if(exptime ==NULL) exptime = "0";

			/* 添加数据 */
			if ((strcmp(opt, "set")==0 || strcmp(opt, "add")==0 )&& key != NULL) {
			    /* 优先接收POST正文信息 */
				int buffer_data_len;
				buffer_data_len = EVBUFFER_LENGTH(req->input_buffer);
				if (buffer_data_len > 0) {
				    char *buffer_data =(char *)calloc(buffer_data_len, sizeof(char)); 
				    memcpy(buffer_data, EVBUFFER_DATA(req->input_buffer), buffer_data_len);
					iteminfo item;
					{
						item.key =(char *) key;
						item.flags = 0;
						item.exptime = strtol(exptime, NULL, 10);
						item.data = (char*)urldecode(buffer_data);
						item.data_length = strlen((char*)item.data);
					}
					put(item);
				    evhttp_add_header(req->output_headers, "Key", key);
				    evbuffer_add_printf(buf, "%s", "LLServer_SET_OK");
				    free(buffer_data);
				} 					
				/* 如果POST正文无内容，则取URL中data参数的值 */
				else if (input_value != NULL) 
				{
				    if (strlen(input_value) > 0) {		
						iteminfo item;
						{
							item.key = (char *)key;
							item.flags = 0;
							item.exptime = strtol(exptime, NULL, 10);
							item.data = (char*)input_value;
							item.data_length = strlen((char*)input_value);
						}
						put(item);	
						evhttp_add_header(req->output_headers, "Key", key);
						evbuffer_add_printf(buf, "%s", "LLServer_SET_OK");						
					 }
					 else 
						evbuffer_add_printf(buf, "%s", "LLServer_SET_END");					 
				 } 
				 else 
					evbuffer_add_printf(buf, "%s", "LLServer_SET_ERROR");				
			}
			/* 取出数据 */
			else if (strcmp(opt, "get") == 0 && key != NULL) {
				const iteminfo item = get(key);
				if(item.flags == -10 ) {
				   evbuffer_add_printf(buf, "%s", "LLServer_GETKEY_ERROR");
				}
                else{
				   evhttp_add_header(req->output_headers, "Key", key);
				   //evbuffer_add_printf(buf, "%s", item.data);
				   int i;
				   for(i = 0; i< item.data_length; i++)
				      evbuffer_add_printf(buf, "%c", item.data[i]);	
				   /*evbuffer_add_printf(buf, "\r\n%s", item.data);	
				   evbuffer_add_printf(buf, "length: %d", item.data_length);	*/
				}
			}
			/*else if (strcmp(opt, "getall") == 0) {
                evbuffer_add_printf(buf, "%s", "LLServer_GETALL");
		        leveldb::Iterator *it = lldb->NewIterator(leveldb::ReadOptions());
				
				if(!it->status().ok()){
				   evbuffer_add_printf(buf, "%s", "LLServer_GETALL_ERROR");
				}
				else{		
				    evhttp_add_header(req->output_headers, "Key", "alldata");
				    for(it->SeekToFirst();it->Valid();it->Next()) {			     	   	               
					    evbuffer_add_printf(buf, "{%s:%s}\n", 
                                               it->key().ToString().c_str(),
                                               it->value().ToString().c_str());						  
				    }
                    evbuffer_add_printf(buf, "%s", "LLServer_GETALL_OK");
				}
			}		*/	
	    	/* 删除数据 */
			else if (strcmp(opt, "delete") == 0 && key != NULL ) {
				if (!clear(key).ok()) 
					evbuffer_add_printf(buf, "%s", "LLServer_DELETE_ERROR");					
				else 
					evbuffer_add_printf(buf, "%s", "LLServer_DELETE_OK");							
            }
            /* 删除全部数据 */
			else if (strcmp(opt, "deleteall") == 0) {		                                
				if (!clearall().ok()) 
					evbuffer_add_printf(buf, "%s", "LLServer_DELETEALL_ERROR");					
				else
					evbuffer_add_printf(buf, "%s", "LLServer_DELETEALL_OK");						
            }
			else{/* 命令错误 */    
	             evbuffer_add_printf(buf, "%s", "LLServer_ERROR");
			}
	    }
		else {/* 命令错误 */
		   evbuffer_add_printf(buf, "%s", "LLServer_ERROR");				            
		}
	
		/* 输出内容给客户端 */
        evhttp_send_reply(req, HTTP_OK, "OK", buf);		
		/* 内存释放 */
		evhttp_clear_headers(&llserver_http_query);
		evbuffer_free(buf);
}



/*bufferevent 操作.并对memcached协议进行解析*/

void closeclient(ev_connect *evc)
{
    bufferevent_free(evc->buf_ev);
	close(evc->fd);
	free(evc);
}

void llserver_write_handler(struct bufferevent *bev, void *arg)
{
	/*ev_connect *evc = (ev_connect *)arg;
	char *result = "STORED\r\n";
    write(evc->fd, result, strlen(result));*/
}

void llserver_error_handler(struct bufferevent *bev, short what, void *arg)
{
	ev_connect *evc = (ev_connect *)arg;
	if (what & EVBUFFER_EOF) {
        /* 客户端断开连接，在这里移除读事件并释放客户数据结构。 */
        printf("ev_connect disconnected.\n");
		bufferevent_disable(evc->buf_ev, EV_READ);
	    struct bufferq *bufferq = TAILQ_FIRST(&evc->readq);
	    TAILQ_REMOVE(&evc->readq, bufferq, entries);	
    }
    else 
		warn("ev_connect socket error, disconnecting.\n");
    	
	closeclient(evc);	
}

void process_command(struct bufferevent *bev, ev_connect *evc, char *req, struct bufferq *bufferq, struct evbuffer *evreturn)
{
	int length = 0;
	//因为有些机器上会对下面的split方法进行内存访问限制，这里将其转为char[]就没事了
	char command[MAX_BUFFER_SIZE] = "";
	int i;
	for(i = 0; i < strlen(req); i++)
	{
		if(i >= MAX_BUFFER_SIZE)
			break;
		command[i] = req[i];      
	}	
	char **tokens = split(command, &length, ' ');    			
		
	if(strcmp(tokens[0], "set")==0 && length >=5){
		//format:set + " " + key + " " + flags + " " + exptime + " " + data length + "\r\n" + data + "\r\n"
		//response; "NOT_STORED\r\n" OR "EXISTS\r\n" 
		bufferq->item.key = (char*)calloc(strlen(tokens[1]), sizeof(char));
	    memcpy(bufferq->item.key, tokens[1], strlen(tokens[1]));
		bufferq->item.flags = strtoul(tokens[2], NULL, 10);
		bufferq->item.exptime = strtol(tokens[3], NULL, 10);
		bufferq->item.data_length = atoi(tokens[4]);
		bufferq->item.data = (char*) calloc(bufferq->item.data_length, sizeof(char));
#ifdef DEBUG
		printf("current key:%s  length: %d\n", bufferq->item.key, atoi(tokens[4]));             
#endif
		//如果保存数据大于4096，则需要修改libevent 源码包中buffer.c中EVBUFFER_MAX_READ设置
		if(bufferq->item.data_length > EVBUFFER_LENGTH(bev->input))
		{
			bufferq->readbyte = bufferevent_read(bev, bufferq->item.data, EVBUFFER_LENGTH(bev->input));
			if(bufferq->readbyte <= 0){
#ifdef DEBUG
				printf("NOT_STORED\r\n");	
#endif
				evbuffer_add_printf(evreturn,"4_NOT_STORED\r\n"); 				  
	    		bufferevent_write_buffer(bev,evreturn);	
				evbuffer_free(evreturn);
				free(req);	    
				return;
			}	
			bufferq->needbyte = bufferq->item.data_length;
			bufferq->state = MORE_DATA_EXPECTED;
			//bufferevent_enable(evc->buf_ev, EV_READ);	
			//bufferevent_setcb(evc->buf_ev, buf_read_callback, buf_write_callback, buf_error_callback, evc);	
			return;
		}		   
		else
			memcpy(bufferq->item.data, EVBUFFER_DATA(bev->input), bufferq->item.data_length);

		evbuffer_readline(bev->input);//读取最后的\r\n符
		//write data to leveldb
		if(put(bufferq->item).ok()) 
		   evbuffer_add_printf(evreturn,"STORED\r\n"); 				
		else
		   evbuffer_add_printf(evreturn,"NOT_STORED\r\n"); 				  
		
		bufferevent_write_buffer(bev,evreturn);		
	}
	else if(strcmp(tokens[0], "get")==0 && length >=2){
		//format:get <key>*\r\n
		//response: "NOT_FOUND" OR  "VALUE " + key + " " + flags + " " + data length  + "\r\n" + data + "END\r\n";
		iteminfo item = get(tokens[1]);	
		if(item.flags != -10) {   
			if(item.data_length > 0) {
		    	evbuffer_add_printf(evreturn, "VALUE %s %d %d\r\n", item.key, item.flags, item.data_length);
				int i;
				for(i = 0 ; i < item.data_length; i++)
					evbuffer_add_printf(evreturn, "%c", item.data[i]);
				evbuffer_add_printf(evreturn, "\r\nEND\r\n");
			}
			else
				evbuffer_add_printf(evreturn,"%s\r\n", item.errorinf); 		
		}else
			evbuffer_add_printf(evreturn, "%s\r\n", item.errorinf); 		

		bufferevent_write_buffer(bev,evreturn);			
	}
	else if(strcmp(tokens[0], "delete")==0 && length >=2 ){
		//format: delete <key> <time>\r\n
		//response; "DELETED\r\n"  OR "NOT_FOUND\r\n"
		const char *key = tokens[1];
		if(clear(key).ok())
		   evbuffer_add_printf(evreturn,"DELETED\r\n"); 
		else
		   evbuffer_add_printf(evreturn,"NOT_FOUND\r\n");    

		bufferevent_write_buffer(bev,evreturn);		
	}
	else if(strcmp(tokens[0], "flush_all")==0){
		//format:flush
		//删除所有数据
		if(clearall().ok())
			evbuffer_add_printf(evreturn,"OK\r\n"); 
		else
			evbuffer_add_printf(evreturn,"SERVER_ERROR\r\n");

		bufferevent_write_buffer(bev,evreturn);		
	}
	else if(strcmp(tokens[0], "version")==0){
		//format:version\r\n
  		evbuffer_add_printf(evreturn,"VERSION %s\r\nEND\r\n", PACKAGE_VERSION);  //
		bufferevent_write_buffer(bev,evreturn);		
	}	
	else if(strcmp(tokens[0], "quit")==0){//quit\r\n
		closeclient(evc);		
	}
	else {
		//删除所有数据
		evbuffer_add_printf(evreturn,"ERROR\r\n");  // - "SERVER_ERROR <error>\r\n"  "ERROR\r\n" 
		bufferevent_write_buffer(bev,evreturn);	
	}
	free(req);	  
}

void llserver_read_handler(struct bufferevent *bev, void *arg)
{	
    ev_connect *evc = (ev_connect *)arg;
	/*将第一个元素移出写队列.我们可能不用再探测写队列是否是空,但是应该确认返回值不是NULL*/
	struct bufferq *bufferq = TAILQ_FIRST(&evc->readq);	
   
    if (bufferq == NULL)
        return;

#ifdef DEBUG
	printf("read bufferrq, state : %d, readbyte: %d\n", bufferq->state, bufferq->readbyte);	
#endif

    struct evbuffer *evreturn = evbuffer_new(); 
	if(bufferq->state == READ_LINE)
	{
			
			/*过滤掉BUFFER中的空行信息*/		
			char *req;
			int times;
			for(times = 0 ; times < 3; times++) {
				req = evbuffer_readline(bev->input);
				if (req != NULL && strlen(req)>0)
					 break;
			}		
#ifdef DEBUG
			printf("buffer: %s\n, length:%d\n", req, EVBUFFER_LENGTH(bev->input));
#endif
			//如获取三次后仍未读出有效信息(为空)则直接返回
			if (req == NULL || strlen(req)==0)
		 		return;				
			process_command(bev, evc, req, bufferq, evreturn);
	}
	else if(bufferq->state == MORE_DATA_EXPECTED)
	{	
			int leave = bufferq->needbyte - bufferq->readbyte;
			if(leave > EVBUFFER_LENGTH(bev->input))
			{
				char* data = (char*)calloc(EVBUFFER_LENGTH(bev->input), sizeof(char));
#ifdef DEBUG
				printf("MORE_DATA_EXPECTED calloc call\n");
#endif
				int readbyte = bufferevent_read(bev, data, EVBUFFER_LENGTH(bev->input));		
				if(readbyte <= 0){
#ifdef DEBUG
					printf("NOT_STORED\r\n");	
#endif
					evbuffer_add_printf(evreturn,"3_NOT_STORED\r\n"); 				  
	        		bufferevent_write_buffer(bev,evreturn);	
					evbuffer_free(evreturn);
					return;
				}	
				memcpy(bufferq->item.data + bufferq->readbyte, data, readbyte);		
				bufferq->readbyte += readbyte;	 
				bufferq->state = MORE_DATA_EXPECTED;
				//bufferevent_enable(evc->buf_ev, EV_READ);		
#ifdef DEBUG	
				printf("readbyte : %d  sum: %d   leave: %d\n", bufferq->readbyte, bufferq->needbyte, leave);		
#endif
				free(data);
	        }
			else
			{
				char* data = (char*)calloc(leave, sizeof(char));
#ifdef DEBUG
				printf("MORE_DATA_EXPECTED leave calloc call \ncurrent key:%s  length: %d\n", bufferq->item.key, bufferq->item.data_length);	
#endif
				int readbyte = bufferevent_read(bev, data, leave);
				if(readbyte <= 0){
#ifdef DEBUG
					printf("NOT_STORED\r\n");	
#endif
					evbuffer_add_printf(evreturn,"2_NOT_STORED\r\n"); 				  
	        		bufferevent_write_buffer(bev,evreturn);	
					evbuffer_free(evreturn);
					return;
				}	
				memcpy(bufferq->item.data + bufferq->readbyte, data, readbyte);		
				bufferq->readbyte += readbyte;	 			
				//bufferevent_enable(evc->buf_ev, EV_READ);		
				//bufferevent_setcb(evc->buf_ev, buf_read_callback, buf_write_callback, buf_error_callback, evc);
				free(data);
#ifdef DEBUG
				printf("leveldb read : %d  sum: %d\n", bufferq->readbyte, bufferq->needbyte);		
				printf("current key:%s  length: %d\n", bufferq->item.key, bufferq->item.data_length);
#endif
				//write data to leveldb
				if(put(bufferq->item).ok()) {
				   evbuffer_add_printf(evreturn,"STORED\r\n"); 				
#ifdef DEBUG
				   printf("STORED\r\n");	
#endif
				}
				else{
				   evbuffer_add_printf(evreturn,"1_NOT_STORED\r\n"); 	
#ifdef DEBUG
				   printf("NOT_STORED\r\n");	
#endif
				}
				bufferevent_write_buffer(bev,evreturn);	    	
				//重置信息
				bufferq->needbyte = bufferq->readbyte = 0;
				bufferq->state = READ_LINE;
    		}
		}
		else if(bufferq->state == ALL_DATA_READ)//如已读完数据
		{
#ifdef DEBUG
			printf("leveldb read : %d  sum: %d\n", bufferq->readbyte, bufferq->needbyte);		
			printf("current key:%s  length: %d\n", bufferq->item.key, bufferq->item.data_length);
#endif			
			bufferq->needbyte = 0;
			bufferq->readbyte = 0;
			bufferq->state = READ_LINE;
			/*TAILQ_REMOVE(&evc->readq, bufferq, entries);
			TAILQ_INSERT_TAIL(&evc->readq, bufferq, entries);*/
			//bufferevent_disable(evc->buf_ev, EV_READ);		
		} 
	
	evbuffer_free(evreturn);
}
