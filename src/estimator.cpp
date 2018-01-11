#include "estimator.h"

Estimator::Estimator(){

}

Estimator::~Estimator(){

}

bool Estimator::readOpenposePeaks(const char* buff){
	char tempPoseStr[50];
	size_t pose_i=0,pose_row=0,pose_line=0;
	auto maxArea=0;
	auto leftx=10000, lefty=10000, rightx=0, righty=0;
	double tmpPeaks[numKeypoints][3];
	if(strlen(buff)<20){
		// std::cout<<"No valid pose data!"<<std::endl;
		return false;
	}
	for(size_t j=0;j<strlen(buff)+1;++j){
		if(buff[j]==';' || buff[j]=='\0'){
			tempPoseStr[pose_i]='\0';
			tmpPeaks[pose_row][pose_line++]=std::atof(tempPoseStr);
			memset(tempPoseStr, '\0', sizeof(tempPoseStr));
			for(auto i=0;i<numKeypoints;++i){
				// auto j=bboxJoints[i];
				auto x=tmpPeaks[i][0];
				auto y=tmpPeaks[i][1];
				if(x<0.01 and y<0.01)
					continue;
				if(x<leftx)
					leftx=x;
				if(y<lefty)
					lefty=y;
				if(x>rightx)
					rightx=x;
				if(y>righty)
					righty=y;
			}
			auto area=(rightx-leftx)*(righty-lefty);
			if(area>maxArea){
				for(auto r=0;r<numKeypoints;++r){
					for(auto c=0;c<3;++c)
						openposePeaks[r][c]=tmpPeaks[r][c];
				}
				maxArea=area;
			}
			pose_row=0;
		    pose_line=0;
		    pose_i=0;
			leftx=10000;
			lefty=10000;
			rightx=0;
			righty=0;	
			if(buff[j]=='\0')
				break;
		}
		else if(buff[j]==','){
			tempPoseStr[pose_i]='\0';
			tmpPeaks[pose_row][pose_line++]=std::atof(tempPoseStr);
			memset(tempPoseStr, '\0', sizeof(tempPoseStr));
			pose_i=0;
			if(pose_line==3){
				pose_row++;
				pose_line=0;
			}
		}
		else{
			tempPoseStr[pose_i++]=buff[j];
		}
	}
	return true;
}

bool Estimator::readDatabasePeaks(const char* act_path){
	std::ifstream in;
	in.open(act_path, std::ios::in);
	if(not in){
		std::cout<<"Could not open action file!"<<std::endl;
		return false;
	}
	for(size_t i=0;i<numKeypoints;++i){
		for(size_t j=0;j<4;++j){
			in>>databasePeaks[i][j];
		}
	}
	in.close();
	return true;
}
