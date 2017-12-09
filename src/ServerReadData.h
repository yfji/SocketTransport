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
#include <sys/stat.h> 
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include "databaseManager.hpp"
 #include "estimator.h"
using namespace std;

class ServerReadData {
public:
	ServerReadData(const string& ip, const int p, int uid, int act_id);
	virtual ~ServerReadData();
private:
	int listensd;
	int connState;
	string ipAddr;
	int port;
	const int queue=10;
	struct sockaddr_in server_socket;
	struct sockaddr_in client_socket;
	char buff[1000];
	bool bConnected;
	int frameIndex;
	int numEmptyData;
	int uid;
	int act_id;
	cv::Mat canvas;

	std::shared_ptr<DatabaseManager> spDbManager;
	std::shared_ptr<Estimator> spEstimator;
	// const char* const ptr=NULL;
	const int limbSeq[13][2] ={
		{0,1},	//0
		{1,2},	//1
		{1,8},	//2
		{1,5},	//3
		{1,11},	//4
		{2,3},	//5
		{3,4},	//6
		{5,6},	//7
		{6,7},	//8
		{8,9},	//9
		{9,10},	//10
		{11,12},	//11
		{12,13}};	//12
	bool writeScore(const double message);
	bool writeImgurl(const std::string& message);
	// bool writeDatabase(const string& message);
	void drawGradientLine(cv::Mat& frame, cv::Point p1, cv::Point p2, cv::Scalar& color, int width);

public:
	bool startListen();
	bool startAccept();
	inline void setDatabaseManager(std::shared_ptr<DatabaseManager>& sp){
		spDbManager=sp;
	}
	inline void setEstimator(std::shared_ptr<Estimator>& sp){
		spEstimator=sp;
	}
	inline void sendMessage(const char* message){
		send(connState, message, strlen(message), 0);
	}
	void receiveData();
	void drawKeypoints(cv::Mat& frame);
	void saveTxtFile(const char* fileName);
	inline void finishReceive(){}
};

#endif /* SRC_SERVERREADDATA_H_ */
