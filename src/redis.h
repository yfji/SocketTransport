#ifndef _REDIS_H_
#define _REDIS_H_

#include <iostream>
#include <string.h>
#include <string>
#include <stdio.h>
#include <mutex>
#include <map>
#include <hiredis/hiredis.h>

class Redis
{
public:
    Redis(){}
    ~Redis()
    {
        this->_connect = NULL;
        this->_reply = NULL;                
    }
	const std::string keyImage="ImgData";
	const std::string keyResult="Result";
	const std::string keyPose="PoseData";
	const std::string keyPortValue="port_value";
	const std::string keyPortUse="port_use";

    bool connect()
    {
        this->_connect = redisConnect(inetAddress.c_str(), defaultPort);
        if(this->_connect != NULL && this->_connect->err)
        {
            // printf("redis connect error: %s\n", this->_connect->errstr);
            return 0;
        }
		// printf("redis connect successfully\n");
        return 1;
    }

    bool connect(std::string host, int port)
    {
        this->_connect = redisConnect(host.c_str(), port);
        if(this->_connect != NULL && this->_connect->err)
        {
            // printf("redis connect error: %s\n", this->_connect->errstr);
            return 0;
        }
		// printf("redis connect successfully\n");
        return 1;
    }

    std::string get(std::string key)
    {
    	mMutex.lock();
        this->_reply = (redisReply*)redisCommand(this->_connect, "GET %s", key.c_str());
        std::string str = this->_reply->str;
        freeReplyObject(this->_reply);
        mMutex.unlock();
        return str;
    }

    void set(std::string key, std::string value)
    {
    	mMutex.lock();
        redisCommand(this->_connect, "SET %s %s", key.c_str(), value.c_str());
        mMutex.unlock();
    }
	
	void setPortPairState(int state){
	}
	std::pair<int,int> getAvailablePortPair(){
		i
		
private:

    redisContext* _connect;
    redisReply* _reply;
    
    std::mutex mMutex;
    const std::string inetAddress="localhost";
    const int defaultPort= 6379;
    int numPortPair;

};

#endif  //_REDIS_H_
