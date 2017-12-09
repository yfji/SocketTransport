/*
 * ServerReadData.cpp
 *
 *  Created on: 2017年10月18日
 *      Author: yufeng
 */

#include "ServerReadData.h"
#include <mutex>
#include <memory>

extern bool bValidDataArrived;
extern bool bNewDataArrived;
extern cv::Mat globalFrame;
extern vector<cv::Mat> globalFrames;
extern std::mutex dataMutex;
extern int maxQueueLen;

ServerReadData::ServerReadData(const string& ip, const int p){
	ipAddr=ip;
	port=p;
	server_socket.sin_family=AF_INET;
	server_socket.sin_port=htons(port);
	server_socket.sin_addr.s_addr=inet_addr(ipAddr.c_str());
	bConnected=false;
	frameIndex=0;
	numEmptyData=0;
}

ServerReadData:: ~ServerReadData(){
	bConnected=false;
	close(listensd);
}
bool ServerReadData::startListen(){
	listensd=socket(AF_INET, SOCK_STREAM, 0);
	if(listensd==-1){
		cout<<"Socket error"<<endl;
		return false;
	}
	if(bind(listensd, (struct sockaddr*)(&server_socket), sizeof(server_socket))==-1){
		cout<<"Bind reader socket error"<<endl;
		close(listensd);
		return false;
	}
	if(listen(listensd, queue)==-1){
		cout<<"Listen read port error"<<endl;
		close(listensd);
		return false;
	}
	cout<<"bind reader socket successfully"<<endl;
	return true;
}

bool ServerReadData::startAccept(){
	socklen_t length=sizeof(struct sockaddr_in);
	cout<<"accecpting"<<endl;
	connState=accept(listensd, (struct sockaddr*)(&client_socket), &length);
	if(connState<0){
		cout<<"reader socket accept error"<<endl;
		return false;
	}
	cout<<"reader socket connect successfully"<<endl;
	bConnected=true;
	return true;
}

void ServerReadData::drawKeypoints(cv::Mat& frame){
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

void ServerReadData::receiveData(){
	//cv::namedWindow("frame");
	//cv::startWindowThread();
	if(not bConnected)
		return;

	while(1){
		memset(buff, '\0', sizeof(buff));
		int rn=recv(connState, buff, sizeof(buff), 0);
		buff[rn]='\0';
		if(strlen(buff)==0){
			numEmptyData++;
			continue;
		}
		if(numEmptyData==20){
			cout<<"Too many empty data, exit!"<<endl;
			break;
		}
		if(strcmp(buff, "stop")==0){
			break;
		}
		else if(strcmp(buff, "nop")==0){
			bValidDataArrived=true;
			bNewDataArrived=true;
			cout<<"nop"<<endl;
			frameIndex=(frameIndex+1)%maxQueueLen;
			sendMessage("data");
			continue;
		}
		else{
			dataMutex.lock();
			bValidDataArrived=true;
			bNewDataArrived=true;

//			cout<<"person pose information:\n[";
//			for(size_t j=0;j<strlen(buff)+1;++j){
//				if(buff[j]==';')	cout<<"]\n[";
//				else if(buff[j]=='\0')	{
//					cout<<"]\n";
//					break;
//				}
//				else cout<<buff[j];
//			}
//			cout<<endl;

			if(globalFrames[frameIndex].empty()){
				sendMessage("data");
				continue;
			}
			globalFrames[frameIndex].copyTo(canvas);
			frameIndex=(frameIndex+1)%maxQueueLen;
			dataMutex.unlock();
			drawKeypoints(canvas);
			cv::imshow("frame", canvas);
			cout<<"data recved"<<endl;
			sendMessage("data");
			cv::waitKey(1);
			usleep(5);
		}
	}
	//cv::destroyAllWindows();
	bConnected=false;
}
