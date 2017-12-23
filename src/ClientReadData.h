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

class ClientReadData {
public:
	ClientReadData(const string& ip, const int p, int uid, int action_id);
	virtual ~ClientReadData();
private:
	int clientsd;
	int port;
	const int queue=10;
	char buff[4096];
	char handcheckBuff[5];
	const int cmd_len=4;
	const char* handcheckMessage="yuge";
	int frameIndex;
	int uid;
	int action_id;
	bool bConnected;
	std::string serverAddress;
	cv::Mat canvas;
	struct sockaddr_in server_socket;
	
	std::shared_ptr<DatabaseManager> spDbManager;
	std::shared_ptr<Estimator> spEstimator;
	
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

	void drawGradientLine(cv::Mat& frame, cv::Point p1, cv::Point p2, cv::Scalar& color, int width);
	
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
	inline void setDatabaseManager(const std::shared_ptr<DatabaseManager>& sp){
		spDbManager=sp;
	}
	inline void setEstimator(const std::shared_ptr<Estimator>& sp){
		spEstimator=sp;
	}
	void receiveData();
	void drawKeypoints(cv::Mat& frame);
	void saveTxtFile(const char* fileName, const std::string& poseData);
	void readPoseData(std::string& poseData);
	inline void finishReceive(){}
};

#endif /* CLIENTREADDATA_H_ */
