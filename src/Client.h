/*
 * Client.h
 *
 *  Created on: 2017年4月12日
 *      Author: yufeng
 */

#ifndef SRC_CLIENT_H_
#define SRC_CLIENT_H_
#include <iostream>
#include <vector>
#include <fstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include "ZBase64.h" 
#include "Config.h"
#include "redis.h"

using namespace std;

class Client {
public:
	Client(const string& ip, const int port);
	virtual ~Client();
private:
	int clientsd;
	int totalSize;
	int connState;
	std::string serverAddress;
	int port;
	const int queue=10;
	struct sockaddr_in server_socket;
	struct sockaddr_in client_socket;
	unsigned char buff[MAX_LEN];
	char handcheckBuff[5];
	const int cmd_len=4;
	const char* handcheckMessage="yuge";
	
	int frameIndex;
	int numEmptyFrames;
	bool bConnected;
	
	ZBase64 base64;

	std::shared_ptr<Redis> spRedis;
	std::vector<cv::Mat> actionFrames;
	std::vector<std::string> frameUUID;

public:
	inline void setRedisPtr(const std::shared_ptr<Redis>& sp){
		spRedis=sp;
	}
	bool queryDatabase();
	bool connectDatabase();
	bool connectServer();
	void sendMessage(const char* message);
	void sendSingleImage(char* image);
	void sendSingleImageBuff(uchar* buffer, int size);
	void listenAndSendFrame();
	//void sendImages();
};

#endif /* SRC_CLIENT_H_ */
