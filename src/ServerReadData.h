/*
 * ServerReadData.h
 *
 *  Created on: 2017年10月18日
 *      Author: yufeng
 */

#ifndef SRC_SERVERREADDATA_H_
#define SRC_SERVERREADDATA_H_
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <opencv2/opencv.hpp>
using namespace std;

class ServerReadData {
public:
	ServerReadData(const string& ip, const int p);
	virtual ~ServerReadData();
private:
	int listensd;
	int connState;
	string ipAddr;
	int port;
	const int queue=10;
	struct sockaddr_in server_socket;
	struct sockaddr_in client_socket;
	char buff[4096];
	bool bConnected;
	int frameIndex;
	int numEmptyData;
	cv::Mat canvas;
public:
	bool startListen();
	bool startAccept();
	inline void sendMessage(const char* message){
		send(connState, message, strlen(message), 0);
	}
	void receiveData();
	void drawKeypoints(cv::Mat& frame);
	inline void finishReceive(){}
};

#endif /* SRC_SERVERREADDATA_H_ */
