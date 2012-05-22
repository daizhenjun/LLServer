#include <iostream>
#include <pthread.h>
#include "leveldb/db.h"
#include "leveldb/comparator.h"
#include "leveldb/write_batch.h"
#include "leveldb/cache.h"
#include "llserver.h"
#include "utils.h"
#include "hstring.h"
//#include <vector>
using namespace std;

static pthread_mutex_t db_lock;

leveldb::Status opendb()
{ 
	pthread_mutex_init(&db_lock, NULL);
    /* 打开数据表 */    	
    lloptions.create_if_missing = true;
    lloptions.block_cache = leveldb::NewLRUCache(llserver_settings_cache * 1048576);//100MB cache
    return leveldb::DB::Open(lloptions, llserver_settings_dataname, &lldb);
}

leveldb::Status put(const iteminfo item)
{
	//clear(item.key);
	pthread_mutex_lock(&db_lock);
	char *info =(char *)malloc(sizeof(char)*item.data_length + MAX_BUFFER_SIZE);
	sprintf(info, "%d %d %d\r\n%s", item.flags, item.exptime, item.data_length, item.data);
#ifdef DEBUG
	warn("final result key:%s %s\n",item.key, info);
#endif
	leveldb::Status s = lldb->Put(leveldb::WriteOptions(), item.key, info);	
	free(info);
	pthread_mutex_unlock(&db_lock);
	return s;
}

const iteminfo get(const char *key)
{
	//key = urldecode(key); 
	string value;
	leveldb::Status s = lldb->Get(leveldb::ReadOptions(), key , &value);
	iteminfo item;	
	item.flags = -10;//暂为-10,如后面获取正确数据会修正
	item.key = (char *)key;
	item.errorinf = "NOT_FOUND\r\n";
	if(s.ok() && value.length()>0) {
		int pos = value.find_first_of("\r\n",0);
        const char *info = (char*)value.substr(0, pos+1).c_str();
	    int length = 0;
	    //因为有些机器上会对下面的split方法进行内存访问限制，这里将其转为char[]就没事了
	    char command[MAX_BUFFER_SIZE] = "";
	    int i;
		for(i = 0; i < strlen(info); i++)
		{
			if(i >= MAX_BUFFER_SIZE)
				break;
			command[i] = info[i];      
		}
		char **tokens = split(command, &length, ' ');    
		if(length >=3) { 
           item.flags = strtoul(tokens[0], NULL, 10);
		   item.exptime  = strtol(tokens[1], NULL, 10);
		   item.data_length = atoi(tokens[2]);		 
		   //获取data
		   hstring data = hs_init((char*)NULL);
		   hstring source = hs_init((char*)value.c_str());
		   hs_substring(&data, source, pos + 3 /*\r\n + 1起始位*/, item.data_length);
		   item.data = data.ch;
		   //item.data = (char*)value.c_str();//(char*)malloc(sizeof(char)*(item.data_length + 1));		  
		
		   //follow code can't work right, so I will remove it later
		   //std::string str = value.substr(pos + 2/*"\r\n"*/, item.data_length);
		   //std::vector<char> bytes(str.begin(), str.end());
           //bytes.push_back('\0');
		   //item.data = (char*) &bytes[0];
		   if(item.exptime>0){
			   time_t the_time;
			   (void) time(&the_time);                                 
			   if(item.exptime <= the_time) {
				  leveldb::Status s = lldb->Delete(leveldb::WriteOptions(),key);	
				  item.errorinf = "LLServer_DATA_EXPIRE_ERROR\r\n";		
				  item.flags = -10;//这里返回错误
			   }			                
		   }
	    }			
		//free(tokens);此处不必释放
	}
	return item;
}



leveldb::Status clear(const char *key) 
{
   pthread_mutex_lock(&db_lock);
   leveldb::Status s = lldb->Delete(leveldb::WriteOptions(), key);
   pthread_mutex_unlock(&db_lock);
   return s;
}


leveldb::Status clearall() 
{
	pthread_mutex_lock(&db_lock);
    leveldb::Status s = DestroyDB(llserver_settings_dataname, lloptions);
	pthread_mutex_unlock(&db_lock); 	
	return opendb();
}
