/*
 * ClientReadData.h
 *
 *  Created on: 2017年10月18日
 *      Author: yufeng
 */

#ifndef CLIENTREADDATA_H_
#define CLIENTREADDATA_H_
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
using namespace std;

class ClientReadData {
public:
	ClientReadData(const string& ip, const int p);
	virtual ~ClientReadData();
private:
	int clientsd;
	string serverAddr;
	int port;
	const int queue=10;
	struct sockaddr_in server_socket;
	char buff[4096];
	int frameIndex;
	bool bConnected;
	cv::Mat canvas;
	
	void reset(const string& ip, const int p);
	
public:
	void setIpAndPort(const string& ip, const int p);	
		
	bool connectServer();
	inline bool isConnected(){
		return bConnected;
	}
	inline void sendMessage(const char* message){
		send(clientsd, message, strlen(message), 0);
	}
	void receiveData();
	void drawKeypoints(cv::Mat& frame);
	inline void finishReceive(){}
};

#endif /* CLIENTREADDATA_H_ */
