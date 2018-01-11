/*
 * ClientReadData.cpp
 *
 *  Created on: 2017年10月18日
 *      Author: yufeng
 */

#include "ClientReadData.h"
#include <chrono>
#include <mutex>

extern vector<cv::Mat> globalFrames;
extern std::mutex dataMutex;
extern int maxQueueLen;

ClientReadData::ClientReadData(const string& ip, const int p) {
	// TODO Auto-generated constructor stub
	reset(ip, p);
	server_socket.sin_family=AF_INET;
	server_socket.sin_port=htons(port);
	server_socket.sin_addr.s_addr=inet_addr(serverAddr.c_str());
}

ClientReadData::~ClientReadData() {
	if(bConnected)
		close(clientsd);
}

void ClientReadData::reset(const string& ip, const int p){
	setIpAndPort(ip, p);
	frameIndex=0;
	bConnected=false;
}		
void ClientReadData::setIpAndPort(const string& ip, const int p){
	serverAddr=ip;
	port=p;
}

bool ClientReadData::connectServer(){
	clientsd=socket(AF_INET, SOCK_STREAM, 0);
	if(clientsd==-1){
		cout<<"Socket error"<<endl;
		return false;
	}
	if(connect(clientsd, (struct sockaddr*)(&server_socket), sizeof(server_socket))<0){
		printf("Reader server connect error\n");
		close(clientsd);
		return false;
	}
	bConnected=true;
	cout<<"server connect successfully"<<endl;
	return true;
}

void ClientReadData::drawKeypoints(cv::Mat& frame){
	for(size_t j=0;j<strlen(buff)+1;++j){
		if(buff[j]==';')	buff[j]='\n';
		else if(buff[j]==',')	buff[j]=' ';
		else if(buff[j]=='\0')	{
			break;
		}
	}
	stringstream ss(buff);
	float x,y,prob;
	cv::RNG rng;
	while(not ss.eof()){
		ss>>x;
		ss>>y;
		ss>>prob;
		cv::Scalar s(rng.uniform(0,255),rng.uniform(0,255),rng.uniform(0,255));
		cv::circle(frame, cv::Point((int)x,(int)y), 5, s, -1);
	}
}

void ClientReadData::receiveData(){
	auto start=std::chrono::high_resolution_clock::now();
	int frameCount=0;
	double fps=0;
	while(1){
		memset(buff, '\0', sizeof(buff));
		int rn=recv(clientsd, buff, sizeof(buff), 0);
		buff[rn]='\0';
		if(rn==0){
			std::cout<<"No valid data"<<std::endl;
			usleep(50);
			continue;
		}
		if(strcmp(buff, "stop")==0){
			break;
		}
		else if(strcmp(buff, "nop")==0){
			std::cout<<"nop"<<std::endl;
			// DO SOMETHING
		}
		else{
			if(globalFrames[frameIndex].empty()){
				sendMessage("data");
				continue;
			}
			dataMutex.lock();
			globalFrames[frameIndex].copyTo(canvas);
			dataMutex.unlock();
			drawKeypoints(canvas);
			// cv::imshow("frame", canvas); cv::waitKey(1);
			auto now=std::chrono::high_resolution_clock::now();
			double duration_ns=(double)std::chrono::duration_cast<std::chrono::nanoseconds>\
				(now-start).count();
			double seconds=duration_ns/1e9;
			fps=(++frameCount)/seconds;
			cout<<"fps: "<<fps<<endl;
			// cout<<"data recved"<<endl;
		}
		frameIndex=(frameIndex+1)%maxQueueLen;
		sendMessage("data");
		usleep(5);
	}
	bConnected=false;
	close(clientsd);
}

