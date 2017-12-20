#include "estimator.h"

Estimator::Estimator(){

}

Estimator::~Estimator(){

}

bool Estimator::readOpenposePeaks(const char* buff){
	char tempPoseStr[50];
	size_t pose_i=0,pose_row=0,pose_line=0;
	if(strlen(buff)<20){
		std::cout<<"No valid pose data!"<<std::endl;
		return false;
	}
	for(size_t j=0;j<strlen(buff);++j){
		if(buff[j]==';' || buff[j]=='\0'){
			tempPoseStr[pose_i]='\0';
			openposePeaks[pose_row][pose_line++]=std::atof(tempPoseStr);
			break;
		}
		else if(buff[j]==','){
			tempPoseStr[pose_i]='\0';
			openposePeaks[pose_row][pose_line++]=std::atof(tempPoseStr);
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