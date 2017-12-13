/*
 * ServerReadData.cpp
 *
 *  Created on: 2017年10月18日
 *      Author: yufeng
 */

#include "ServerReadData.h"
#include "redis.h"
#define SAVE_POSE

ServerReadData::ServerReadData(const string& ip, const int p, int _uid, int _action_id, int _act_id){
	ipAddr=ip;
	port=p;
	uid=_uid;
	action_id=_action_id;
	act_id=_act_id;
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
		// cout<<"Socket error"<<endl;
		return false;
	}
	if(bind(listensd, (struct sockaddr*)(&server_socket), sizeof(server_socket))==-1){
		// cout<<"Bind reader socket error"<<endl;
		close(listensd);
		return false;
	}
	if(listen(listensd, queue)==-1){
		// cout<<"Listen read port error"<<endl;
		close(listensd);
		return false;
	}
	// cout<<"bind reader socket successfully"<<endl;
	return true;
}

bool ServerReadData::startAccept(){
	socklen_t length=sizeof(struct sockaddr_in);
	// cout<<"accecpting"<<endl;
	connState=accept(listensd, (struct sockaddr*)(&client_socket), &length);
	if(connState<0){
		// cout<<"reader socket accept error"<<endl;
		return false;
	}
	cout<<"reader socket connect successfully"<<endl;
	bConnected=true;
	return true;
}


void ServerReadData::readPoseData(std::string& poseData){
	std::vector<double*> openposePeaks=spEstimator->getOpenposePeakPtr();
	const char* states=spEstimator->getActionStatePtr();
	int limbNum=spEstimator->getNumKeypoints()-5;
	stringstream ss_pose;
	auto size=openposePeaks.size();
	int color=1;
	
	for(auto i=0;i<limbNum;++i){
		cv::Point2f p1(openposePeaks[limbSeq[i][0]][0], openposePeaks[limbSeq[i][0]][1]);
		cv::Point2f p2(openposePeaks[limbSeq[i][1]][0], openposePeaks[limbSeq[i][1]][1]);
		if((p1.x>0.1 or p1.y>0.1) and (p2.x>0.1 or p2.y>0.1)){
			color=1;
			if(i==5 or i==6)
				color=(int)states[i-4];
			else if(i>=7 and i<=12)
				color=(int)states[i-3];
			ss_pose<<p1.x<<' '<<p1.y<<' '<<p2.x<<' '<<p2.y<<' '<<color;
		}
		else{
			ss_pose<<0<<' '<<0<<' '<<0<<' '<<0<<' '<<0;
		}
		if(i<limbNum-1)
			ss_pose<<std::endl;
	}
	poseData=ss_pose.str();
}

void ServerReadData::receiveData(){
	if(not bConnected)
		return;

	while(1)
	{
		memset(buff, '\0', sizeof(buff));
		int rn=recv(connState, buff, sizeof(buff), 0);
		buff[rn]='\0';
		if(rn==0){
			usleep(50);
			continue;
		}
		else if(strcmp(buff, "stop")==0){
			break;
		}
		
		std::string poseData="";
		if(strcmp(buff, "nop")==0){
			/***no person in image***/
			spRedis->set("result", "0");
		}
		else{
    		spEstimator->readOpenposePeaks(buff);
    		std::string actPath = spDbManager->getTxtFromDatabase(action_id, act_id); 
			std::cout<<"actPath : "<<actPath<<std::endl;
			/***obtain the standard keypoint peaks according to the act_id***/
			double poseScore=0;
			std::string txtPathDatabase="";
			if(actPath!=""){
				spEstimator->readDatabasePeaks(actPath.c_str());	
				/***calculate the score of each part***/
				poseScore =spEstimator->calcScoreBody();
				poseScore=spEstimator->normalize(poseScore);
				std::cout<<"Score: "<<poseScore<<std::endl;
			}
			if(poseScore>70){
				spRedis->set("result", "1");
			}
			else{
				spRedis->set("result", "0");
			}		
		}
		spRedis->set("pose", poseData);
		frameIndex++;
		sendMessage("data");
		usleep(5);
	}
	bConnected=false;
}
