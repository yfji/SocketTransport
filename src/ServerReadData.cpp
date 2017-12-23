/*
 * ServerReadData.cpp
 *
 *  Created on: 2017年10月18日
 *      Author: yufeng
 */

#include "ServerReadData.h"
#define SAVE_TXT

extern std::vector<cv::Mat> globalFrames;
extern int maxQueueLen;

ServerReadData::ServerReadData(const string& ip, const int p, int _uid, int _act_id){
	ipAddr=ip;
	port=p;
	uid=_uid;
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

void ServerReadData::drawGradientLine(cv::Mat& frame, cv::Point p1, cv::Point p2, cv::Scalar& color, int width){
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

void ServerReadData::drawKeypoints(cv::Mat& frame){
	std::vector<double*> openposePeaks=spEstimator->getOpenposePeakPtr();
	// std::vector<double*> databasePeaks=spEstimator->getDatabasePeakPtr();
	const char* states=spEstimator->getActionStatePtr();
	// for(int i=0;i<10;++i){
	// 	std::cout<<(int)states[i]<<",";
	// }
	// std::cout<<std::endl;
	const int peakNum=spEstimator->getNumKeypoints();
	int x,y;
	int color=0;
	cv::Scalar colors[2]={cv::Scalar(0,255,0), cv::Scalar(0,0,255)};
	// colors[0]=cv::Scalar(0,255,0);
	// colors[1]=cv::Scalar(0,0,255);
	for(size_t i=0;i<peakNum-5;++i){
		cv::Point2f p1(openposePeaks[limbSeq[i][0]][0], openposePeaks[limbSeq[i][0]][1]);
		cv::Point2f p2(openposePeaks[limbSeq[i][1]][0], openposePeaks[limbSeq[i][1]][1]);
		if((p1.x>0.1 or p1.y>0.1) and (p2.x>0.1 or p2.y>0.1)){
			// if(i>6 and i<9 and states[1]==0){	//left arm
			// 	color=1;
			// }
			// else if(i>4 and i<7 and states[0]==0){	//right arm
			// 	color=1;
			// }
			// else if(i>8 and i<11 and states[3]==0){	//left leg
			// 	color=1;
			// }
			// else if(i>10 and i<13 and states[2]==0){		//right leg
			// 	color=1;
			// }
			color=0;
			if(i==5 and (int)states[1]==0)
				color=1;
			else if(i==6 and (int)states[2]==0)
				color=1;
			else if(i==7 and (int)states[4]==0)
				color=1;
			else if(i==8 and (int)states[5]==0)
				color=1;
			else if(i==9 and (int)states[6]==0)
				color=1;
			else if(i==10 and (int)states[7]==0)
				color=1;
			else if(i==11 and (int)states[8]==0)
				color=1;
			else if(i==12 and (int)states[9]==0)
				color=1;
			// double angle=std::atan(1.0*(p1.y-p2.y)/(p1.x-p2.x))*180/3.1415926;
			// int len=std::sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y));
			// cv::ellipse(frame, cv::Point((int)(p1.x+p2.x)/2, (int)(p1.y+p2.y)/2),  cv::Size(len/2,3), angle, 0,360, colors[color],-1);
			// cv::line(frame, p1, p2, colors[color], 4);
			if(color==1)
				drawGradientLine(frame, p1, p2, colors[color], 20);
			else{
				double angle=std::atan(1.0*(p1.y-p2.y)/(p1.x-p2.x))*180/3.1415926;
				int len=std::sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y));
				cv::ellipse(frame, cv::Point((int)(p1.x+p2.x)/2, (int)(p1.y+p2.y)/2),  cv::Size(len/2,10), angle, 0,360, colors[color],-1);
			}
		}
	}
	// std::cout<<"draw lines"<<std::endl;
	for(size_t i=0;i<peakNum-4;++i){
		x=(int)openposePeaks[i][0];
		y=(int)openposePeaks[i][1];
		if(x!=0 or y!=0){
			cv::circle(frame, cv::Point2f(x,y), 3, colors[0], -1);
		}
	}
	// std::cout<<"draw points"<<std::endl;
}

void ServerReadData::saveTxtFile(const char* fileName){
/*
	std::vector<double*> openposePeaks=spEstimator->getOpenposePeakPtr();
	std::ofstream out;
	out.open(fileName, std::ios::out);
	size_t size=openposePeaks.size();
	for(size_t i=0;i<size;++i){
		out<<openposePeaks[i][0]<<' '<<openposePeaks[i][1]<<' '<<openposePeaks[i][2]<<' '<<i;
		if(i<size-1)
			out<<std::endl;
	}
	out.close();
	*/
	std::vector<double*> openposePeaks=spEstimator->getOpenposePeakPtr();
	const char* states=spEstimator->getActionStatePtr();
	std::ofstream out;
	out.open(fileName, std::ios::out);
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
			out<<p1.x<<' '<<p1.y<<' '<<p2.x<<' '<<p2.y<<' '<<color;
		}
		else{
			out<<0<<' '<<0<<' '<<0<<' '<<0<<' '<<0;
		}
		if(i<limbNum-1)
			out<<std::endl;
	}
	out.close();
}

void ServerReadData::receiveData(){
	//cv::namedWindow("frame");
	//cv::startWindowThread();
	if(not bConnected)
		return;

	// create the path of the dealt image according to the uid and act_id
	char uidStr[5],act_idStr[5], frameStr[20], txtStr[20];
	sprintf(uidStr,"%d",uid);
	sprintf(act_idStr,"%d",act_id);
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
	// std::cout<<"ImgUrl: "<<imgDealPath<<std::endl;
	// std::cout<<"txtUrl: "<<txtDealPath<<std::endl;

	while(1)
	{
		memset(buff, '\0', sizeof(buff));
		int rn=recv(connState, buff, sizeof(buff), 0);
		buff[rn]='\0';
		// std::cout<<"Unblocked"<<std::endl<<std::endl;
		if(rn==0){
			// cout<<"No valid data: "<<rn<<endl;
			usleep(50);
			continue;
		}
		else if(strcmp(buff, "stop")==0){
			break;
		}
		else if(strcmp(buff, "nop")==0){
			// cout<<"nop"<<endl;
			cv::Mat deal_img = globalFrames[frameIndex].clone();
			sprintf(frameStr, "%d.jpg", (frameIndex+1));
			std::string imgPathDatabase=imgDealPath+std::string(frameStr);
			std::string imgPath=imgBasePath+imgPathDatabase;
			cv::imwrite(imgPath, deal_img);
			spDbManager->writeScoreToDatabase(0);
			spDbManager->writeUrlToDatabase(imgPathDatabase, "");
			// continue;
		}
		else{
			// cout<<"person pose information:\n";
			// print
			// cout<<"person pose information:\n[";
			// for(size_t j=0;j<strlen(buff)+1;++j)
			// {
			// 	if(buff[j]==';')	cout<<"]\n[";
			// 		else if(buff[j]=='\0')	{
			// 			cout<<"]\n";
			// 		break;
			// 	}
			//    	else cout<<buff[j];
			// }
			// cout<<endl;
			// obtain the keypoint of the first person from the keypoint string
    		spEstimator->readOpenposePeaks(buff);
    		std::string actPath = spDbManager->getTxtFromDatabase(act_id); 
			std::cout<<"actPath : "<<actPath<<std::endl;
			// obtain the standard keypoint peaks according to the act_id
			double poseScore=0;
			std::string txtPathDatabase="";
			if(actPath!=""){
				spEstimator->readDatabasePeaks(actPath.c_str());	
				/***calculate the score of each part***/
				poseScore =spEstimator->calcScoreBody();
				poseScore=spEstimator->normalize(poseScore);
				std::cout<<"Score: "<<poseScore<<std::endl;
				// writeScore(poseScore);

				/**************** draw and save the dealt image ****************/
				
#ifdef SAVE_TXT
				sprintf(txtStr, "%d.txt", (frameIndex+1));
				txtPathDatabase=txtDealPath+std::string(txtStr);
				std::string txtPath=txtBasePath+txtPathDatabase;
				saveTxtFile(txtPath.c_str());
#endif
				// std::cout<<"Write records successfully"<<std::endl;
			}
			cv::Mat deal_img = globalFrames[frameIndex].clone();
			sprintf(frameStr, "%d.jpg", (frameIndex+1));
			std::string imgPathDatabase=imgDealPath+std::string(frameStr);
			std::string imgPath =imgBasePath+imgPathDatabase;
			drawKeypoints(deal_img);
			cv::imwrite(imgPath, deal_img);
			/****update image url, txt url and pose data****/
			spDbManager->writeScoreToDatabase(poseScore);
			spDbManager->writeUrlToDatabase(imgPathDatabase, txtPathDatabase);
		}
		frameIndex=(frameIndex+1)%maxQueueLen;
		sendMessage("data");
		usleep(5);
	}
	bConnected=false;
}

bool ServerReadData::writeScore(const double message){
	return spDbManager->writeScoreToDatabase(message);
}
bool ServerReadData::writeImgurl(const std::string& message){
	return false;
}

