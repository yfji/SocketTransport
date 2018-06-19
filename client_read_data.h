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
#include <memory>
#include <mutex>
#include "estimator.h"

using namespace std;

using Bundle = std::tuple<std::vector<cv::Mat>*, std::mutex*, int*, int*>;
using draw_callback = std::function<void(cv::Mat&, std::vector<DataRow>&)>;

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
    char buff[8000];
    int frameIndex;
    int invalid_frame = {0};
	bool bConnected;
	cv::Mat canvas;
	
    draw_callback draw_func;
    std::shared_ptr<Estimator> estimator_ptr;
	void reset(const string& ip, const int p);
	
public:
    char roundFinish= {0};
	void setIpAndPort(const string& ip, const int p);	
		
    inline void disconnect(){
        if(bConnected){
            close(clientsd);
        }
    }
	bool connectServer();
	inline bool isConnected(){
		return bConnected;
	}
	inline void sendMessage(const char* message){
		send(clientsd, message, strlen(message), 0);
	}
    inline void setCallback(draw_callback& callback){
        draw_func=callback;
    }
    void receiveData(Bundle* bundle);
	void drawKeypoints(cv::Mat& frame);
	inline void finishReceive(){}
};

#endif /* CLIENTREADDATA_H_ */
