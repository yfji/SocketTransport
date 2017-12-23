/*
 * ClientReadData.cpp
 *
 *  Created on: 2017年10月18日
 *      Author: yufeng
 */

#include "ClientReadData.h"
#include <mutex>

#define SAVE_TXT	1

extern vector<cv::Mat> globalFrames;
extern std::mutex dataMutex;
extern int maxQueueLen;

ClientReadData::ClientReadData(const string& ip, const int p, int _uid, int _action_id) {
	// TODO Auto-generated constructor stub
	serverAddress=ip;
	port=p;
	uid=_uid;
	action_id=_action_id;
	frameIndex=0;
	bConnected=false;
	server_socket.sin_family=AF_INET;
	server_socket.sin_port=htons(port);
	server_socket.sin_addr.s_addr=inet_addr(serverAddress.c_str());
}

ClientReadData::~ClientReadData() {
	if(bConnected){
		close(clientsd);
	}
}

void ClientReadData::reset(const string& ip, const int p){
	setIpAndPort(ip, p);
	frameIndex=0;
	bConnected=false;
}		
void ClientReadData::setIpAndPort(const string& ip, const int p){
	serverAddress=ip;
	port=p;
}

bool ClientReadData::connectServer(){
	clientsd=socket(AF_INET, SOCK_STREAM, 0);
	if(clientsd==-1){
		cout<<"Socket error"<<endl;
		return false;
	}
	if(connect(clientsd, (struct sockaddr*)(&server_socket), sizeof(server_socket))<0){
		cout<<"Connect error"<<endl;
		return false;
	}
	struct timeval timeout={1,0};	//1s
	int ret=setsockopt(clientsd, SOL_SOCKET, SO_RCVTIMEO, (char*)(&timeout), sizeof(timeout));
	if(ret==-1){
		std::cout<<"Reader setsockopt error"<<std::endl;
		close(clientsd);
		return false;
	}
	sendMessage(handcheckMessage);
	int rn=recv(clientsd, handcheckBuff, cmd_len, MSG_WAITALL);
	if(rn<=0 or errno==11){
		std::cout<<"Reader time out, handcheck failed"<<std::endl;
		close(clientsd);
		return false;
	}
	handcheckBuff[rn]='\0';
	if(strcmp(handcheckBuff, handcheckMessage)!=0){
		std::cout<<"Reader command error, handcheck failed"<<std::endl;
		close(clientsd);
		return false;
	}
	bConnected=true;
	cout<<"Reader server connect successfully"<<endl;
	return true;
}


void ClientReadData::drawKeypoints(cv::Mat& frame){
	std::vector<double*> openposePeaks=spEstimator->getOpenposePeakPtr();
	// std::vector<double*> databasePeaks=spEstimator->getDatabasePeakPtr();
	const char* states=spEstimator->getActionStatePtr();
	// for(int i=0;i<10;++i){
	// 	std::cout<<(int)states[i]<<",";
	// }
	// std::cout<<std::endl;
	const int peakNum=spEstimator->getNumKeypoints();
	int x,y;
	int color=1;
	cv::Scalar colors[2]={cv::Scalar(0,0,255),cv::Scalar(0,255,0)};
	for(size_t i=0;i<peakNum-5;++i){
		cv::Point2f p1(openposePeaks[limbSeq[i][0]][0], openposePeaks[limbSeq[i][0]][1]);
		cv::Point2f p2(openposePeaks[limbSeq[i][1]][0], openposePeaks[limbSeq[i][1]][1]);
		if((p1.x>0.1 or p1.y>0.1) and (p2.x>0.1 or p2.y>0.1)){
			color=1;
			if(i==5 or i==6)
				color=(int)states[i-4];
			else if(i>=7 and i<=12)
				color=(int)states[i-3];
			if(color==0)
				drawGradientLine(frame, p1, p2, colors[color], 10);
			else{
				double angle=std::atan(1.0*(p1.y-p2.y)/(p1.x-p2.x))*180/3.1415926;
				int len=std::sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y));
				cv::ellipse(frame, cv::Point((int)(p1.x+p2.x)/2, (int)(p1.y+p2.y)/2),  cv::Size(len/2,3), angle, 0,360, colors[color],-1);
			}
		}
	}
	for(size_t i=0;i<peakNum-4;++i){
		x=(int)openposePeaks[i][0];
		y=(int)openposePeaks[i][1];
		if(x!=0 or y!=0){
			cv::circle(frame, cv::Point2f(x,y), 3, colors[0], -1);
		}
	}
}

// void ClientReadData::drawKeypoints(cv::Mat& frame){
// 	for(size_t j=0;j<strlen(buff)+1;++j){
// 		if(buff[j]==';')	buff[j]='\n';
// 		else if(buff[j]==',')	buff[j]=' ';
// 		else if(buff[j]=='\0')	{
// 			break;
// 		}
// 	}
// 	stringstream ss(buff);
// 	float x,y,prob;
// 	cv::RNG rng;
// 	while(not ss.eof()){
// 		ss>>x;
// 		ss>>y;
// 		ss>>prob;
// 		cv::Scalar s(rng.uniform(0,255),rng.uniform(0,255),rng.uniform(0,255));
// 		cv::circle(frame, cv::Point((int)x,(int)y), 5, s, -1);
// 	}
// }

void ClientReadData::drawGradientLine(cv::Mat& frame, cv::Point p1, cv::Point p2, cv::Scalar& color, int width){
	int b=color.val[0];
	int g=color.val[1];
	int r=color.val[2];
	int c=std::max(std::max(b,g),r);
	int index;
	if(b==c)	index=0;
	else if(g==c)	index=1;
	else	index=2;
	int minWidth=2;
	int lineNum=width-minWidth+1;
	int minColor=20;
	int scale=(c-minColor)/lineNum;
	
	int wScale=(width-minWidth)/lineNum;
	for(int i=0;i<lineNum;++i){
		cv::line(frame, p1, p2, cv::Scalar(index==0?minColor:c-minColor, index==1?minColor:c-minColor, index==2?minColor:c-minColor), width-i);
		minColor+=scale;
	}
}

void ClientReadData::readPoseData(std::string& poseData){
	std::vector<double*> openposePeaks=spEstimator->getOpenposePeakPtr();
	const char* states=spEstimator->getActionStatePtr();
	stringstream ss_pose;
	size_t size=openposePeaks.size();
	auto limbNum=spEstimator->getNumKeypoints()-5;
	int color=1;
	for(size_t i=0;i<limbNum;++i){
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

void ClientReadData::saveTxtFile(const char* fileName, const std::string& poseData){
	std::ofstream out;
	out.open(fileName, std::ios::out);
	out<<poseData;
	out.close();
}

void ClientReadData::receiveData(){
	if(not bConnected){
		return;
	}
	char uidStr[5],act_idStr[5], frameStr[20], txtStr[20];
	sprintf(uidStr,"%d",uid);
	sprintf(act_idStr,"%d",action_id);
	std::string imgBasePath = "/var/www/yuge/public";
	std::string txtBasePath="/var/www/yuge/public";
	std::string imgDealPath=std::string("/test/deal_img/")+std::string(uidStr);
	std::string txtDealPath=std::string("/test/deal_txt/")+std::string(uidStr);
	
	imgDealPath += "_";
	txtDealPath += "_";
	imgDealPath += act_idStr;
	txtDealPath += act_idStr;
	mkdir((imgBasePath+imgDealPath).c_str(),S_IRWXU);
	mkdir((txtBasePath+txtDealPath).c_str(),S_IRWXU);
	imgDealPath += "/";
	txtDealPath += "/";
	std::cout<<"ImgUrl: "<<imgDealPath<<std::endl;
	std::cout<<"txtUrl: "<<txtDealPath<<std::endl;
	
	bool hasPerson=true;
	bool queryOK=spDbManager->queryTxt(action_id);
	
	if(not queryOK){
		std::cout<<"TXT files and image files not corresponding, exit!"<<std::endl;
		return;
	}
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
		double poseScore=0;
		std::string imgPathDatabase="";
		std::string txtPathDatabase="";
		std::string poseData="";
		hasPerson=true;
		/***no person in image***/
		if(strcmp(buff, "nop")==0){
			std::cout<<"nop"<<std::endl;
			hasPerson=false;
			// DO SOMETHING
		}
		else{
			spEstimator->readOpenposePeaks(buff);
    		
    		readPoseData(poseData);
    		// std::cout<<poseData<<std::endl;
    		std::string actPath = spDbManager->getTxtFromDatabase(); 
    		
    		/***obtain the standard keypoint peaks according to the act_id***/
			if(actPath!=""){
				std::cout<<"actPath : "<<actPath<<std::endl;
				spEstimator->readDatabasePeaks(actPath.c_str());	
				/***calculate the score of each part***/
				poseScore =spEstimator->calcScoreBody();
				poseScore=spEstimator->normalize(poseScore);
				std::cout<<"Score: "<<poseScore<<std::endl;
#if SAVE_TXT==1
				sprintf(txtStr, "%d.txt", (frameIndex+1));
				txtPathDatabase=txtDealPath+std::string(txtStr);
				std::string txtPath=txtBasePath+txtPathDatabase;
				saveTxtFile(txtPath.c_str(), poseData);
#endif
			}
		}
		/****update scores****/
		spDbManager->writeScoreToDatabase(poseScore);
		
		/**************** draw and save the dealt image ****************/
		cv::Mat deal_img = globalFrames[frameIndex].clone();
		sprintf(frameStr, "%d.jpg", (frameIndex+1));
		imgPathDatabase=imgDealPath+std::string(frameStr);
		std::string imgPath =imgBasePath+imgPathDatabase;
		
		if(hasPerson)
			drawKeypoints(deal_img);
		cv::imwrite(imgPath, deal_img);
		
		/****update image url, txt url and pose data****/
		spDbManager->writeImageUrlToDatabase(imgPathDatabase);
		// spDbManager->writeTxtUrlToDatabase(txtPathDatabase);
		// spDbManager->writePoseDataToDatabase(poseData);
		
		std::cout<<"Write records successfully"<<std::endl;
		
		spDbManager->writeCountPP();
		
		frameIndex=(frameIndex+1)%maxQueueLen;
		sendMessage("data");
		usleep(5);
	}
	spDbManager->clear();
	bConnected=false;
	close(clientsd);
}

