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
#include "Config.h"

using namespace std;

class Client {
public:
	Client(const string& ip, const int p);
	virtual ~Client();
private:
	int clientsd;
	int totalSize;
	int connState;
	string ipAddr;
	int port;
	const int queue=10;
	struct sockaddr_in server_socket;
	struct sockaddr_in client_socket;
	unsigned char buff[MAX_LEN];
	char fileName[50];
	string image_dir;
	vector<string> image_files;
	cv::VideoCapture mVideoCap;
	int frameIndex;
	int numEmptyFrames;
	bool bConnected;
	int frames;
public:
	bool connectServer();
	void sendMessage(const char* message);
	void sendSingleImage(char* image);
	void sendSingleImageBuff(uchar* buffer, int size);
	void listenAndSendFrame();
	//void sendImages();
};

#endif /* SRC_CLIENT_H_ */
