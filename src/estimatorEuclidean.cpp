#include "estimatorEuclidean.h"

EstimatorEuclidean::EstimatorEuclidean(){

}

EstimatorEuclidean::~EstimatorEuclidean(){

}
void  EstimatorEuclidean::calcPersonArea(){
	double limb_y_ratio;
	int area;
	if ((openposePeaks[1][0] or openposePeaks[1][1]) and (openposePeaks[10][0] or openposePeaks[10][1]) and (openposePeaks[13][0] or openposePeaks[13][1])){ 
		if((databasePeaks[10][1] + databasePeaks[13][1])/2 - databasePeaks[1][1] != 0)
			limb_y_ratio = ((openposePeaks[10][1] + openposePeaks[13][1])/2 - openposePeaks[1][1])/((databasePeaks[10][1] + databasePeaks[13][1])/2 - databasePeaks[1][1]);
	 	double left = 0,right = 0, top = 0, bottom = 0;
	 	for(size_t j=0; j < numKeypoints; j++){
			if(openposePeaks[j][0] < left)
	  			left = openposePeaks[j][0];		
	  		if(openposePeaks[j][0] > right)
				right = openposePeaks[j][0];    
			if(openposePeaks[j][1] < bottom)
				bottom = openposePeaks[j][1];
			if(openposePeaks[j][1] > top)
				top = openposePeaks[j][1]; 
	        }
	        area = (right - left)*(top - bottom);
	}
	else
		area=0;
	personArea=area;
	yScale=limb_y_ratio;
}

double EstimatorEuclidean::calcScoreLimb(std::vector<int>& keypointIds, int limbIndex){
	double stdx,stdy,refx,refy;
	double deltax, deltay;
	size_t size=keypointIds.size();

	double peak_temp[2];
	double euclideanScore=0;
	int baseId=keypointIds[0];
	char stdVisible=1;
	char refVisible=1;
	for(size_t i=0;i<size;++i){
		stdx=databasePeaks[keypointIds[i]][0];
		stdy=databasePeaks[keypointIds[i]][1];
		refx=openposePeaks[keypointIds[i]][0];
		refy=openposePeaks[keypointIds[i]][1];
		stdVisible= (stdx>0.1 or stdy>0.1);
		refVisible= (refx>0.1 or refy>0.1);
		if(i==0 and (not stdVisible or not refVisible)){
			// std::cout<<"Base keypoint must be visible!"<<std::endl;
			memset(limbCorrect, 0, sizeof(limbCorrect));
			return 0;
		}
		if(stdVisible and refVisible){
			peak_temp[0]=openposePeaks[baseId][0]+(databasePeaks[keypointIds[i]][0]-databasePeaks[baseId][0])*yScale;
			peak_temp[1]=openposePeaks[baseId][1]+(databasePeaks[keypointIds[i]][1]-databasePeaks[baseId][1])*yScale;
			deltax=openposePeaks[keypointIds[i]][0]-peak_temp[0];
			deltay=openposePeaks[keypointIds[i]][1]-peak_temp[1];
			double limbScore=std::sqrt(deltax*deltax+deltay*deltay)*1000/(1.0*personArea);
			if(limbScore>armThreshold or limbScore>legThreshold){
				FILL_ARRAY(limbCorrect, limbIndex, i, 0)
			}
			euclideanScore+=limbScore;
		}
		else if(not(stdVisible & refVisible)){
			FILL_ARRAY(limbCorrect, limbIndex, i, 0)
		}
	}
	return euclideanScore;
}

double EstimatorEuclidean::calcScoreBody(){
	std::vector<int> rightArm={1,2,3,4};
	std::vector<int> leftArm={1,5,6,7};
	std::vector<int> rightLeg={8,9,10};
	std::vector<int> leftLeg={11,12,13};

	memset(limbCorrect, 1, sizeof(limbCorrect));

	calcPersonArea();
	double rightArmScore=calcScoreLimb(rightArm, 0);
	double leftArmScore=calcScoreLimb(leftArm, 1);
	double rightLegScore=calcScoreLimb(rightLeg, 2);
	double leftLegScore=calcScoreLimb(leftLeg, 3);
	return (rightArmScore+leftArmScore+rightLegScore+leftLegScore);
}
