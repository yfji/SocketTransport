/*
 * Server.h
 *
 *  Created on: 2017年4月12日
 *      Author: yufeng
 */

#ifndef SRC_SERVER_H_
#define SRC_SERVER_H_
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
#include "Config.h"

#define IMAGE_W	1080
#define IMAGE_H	1080

using namespace std;


class Server {
public:
	Server(int p);
	virtual ~Server();

private:
	int listensd;
	int connState;
	int width;
	int height;
	int channels;
	int totalSize;
	int port;
	const int queue=10;
	struct sockaddr_in server_socket;
	struct sockaddr_in client_socket;
	unsigned char buff[MAX_LEN];
	unsigned char* fullImageBuff;
	char tempFileName[50];
	bool hasImage;
	bool bConnected;
	char stop=0;
	///MyImageClass mImageClass;
public:
	bool startListen();
	bool startAccept();
	void sendMessage(const char* message);
	void receive();
	void finishReceive();
	cv::Mat receiveFrame();
};

//static void* onImageProcess(void* args);
//static void* onInfoBackfeed(void* args);
#endif /* SRC_SERVER_H_ */
