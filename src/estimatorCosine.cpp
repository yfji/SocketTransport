#include "estimatorCosine.h"

EstimatorCosine::EstimatorCosine(){

}

EstimatorCosine::~EstimatorCosine(){
	
}
double EstimatorCosine::calcScoreLimb(std::vector<int>& keypointIds, int limbIndex){
	double stdx,stdy,refx,refy;
	double delta_stdx, delta_stdy, delta_refx, delta_refy;
	size_t size=keypointIds.size();
	char stdVisible=1;
	char refVisible=1;
	double cosineScore=0.0;
	int baseId=keypointIds[0];
	int stdVisibleIndex[2]={baseId,baseId};
	int refVisibleIndex[2]={baseId,baseId};
	
	stdx=databasePeaks[keypointIds[0]][0];
	stdy=databasePeaks[keypointIds[0]][1];
	refx=openposePeaks[keypointIds[0]][0];
	refy=openposePeaks[keypointIds[0]][1];
	stdVisible= (stdx>0.1 or stdy>0.1);
	refVisible= (refx>0.1 or refy>0.1);
	if(not stdVisible or not refVisible){
		// std::cout<<"Base keypoint must be visible!: "<<limbIndex<<std::endl;
		memset(limbCorrect, 0, sizeof(limbCorrect));
		return 0;
	}
	for(size_t i=1;i<size;++i){
		stdx=databasePeaks[keypointIds[i]][0];
		stdy=databasePeaks[keypointIds[i]][1];
		refx=openposePeaks[keypointIds[i]][0];
		refy=openposePeaks[keypointIds[i]][1];
		stdVisible= (stdx>0.1 or stdy>0.1);
		refVisible= (refx>0.1 or refy>0.1);
		
		if(stdVisible){
			stdVisibleIndex[0]=stdVisibleIndex[1];
			stdVisibleIndex[1]=keypointIds[i];
		}
		if(refVisible){
			refVisibleIndex[0]=refVisibleIndex[1];
			refVisibleIndex[1]=keypointIds[i];
		}
		if(stdVisible and refVisible){
			if(stdVisibleIndex[0]!=refVisibleIndex[0]){
				stdVisibleIndex[0]=baseId;
				refVisibleIndex[0]=baseId;
			}
			delta_stdx=databasePeaks[stdVisibleIndex[1]][0]-databasePeaks[stdVisibleIndex[0]][0];
			delta_stdy=databasePeaks[stdVisibleIndex[1]][1]-databasePeaks[stdVisibleIndex[0]][1];
			delta_refx=openposePeaks[refVisibleIndex[1]][0]-openposePeaks[refVisibleIndex[0]][0];
			delta_refy=openposePeaks[refVisibleIndex[1]][1]-openposePeaks[refVisibleIndex[0]][1];
			
			double prod=delta_stdx*delta_refx+delta_stdy*delta_refy;
			double mag=std::sqrt(delta_stdx*delta_stdx+delta_stdy*delta_stdy)*std::sqrt(delta_refx*delta_refx+delta_refy*delta_refy);
			double limbScore=prod/mag;
			double ang=std::acos(limbScore)*180/3.1415926;
			// std::cout<<keypointIds[i]<<":"<<"("<<delta_stdx<<","<<delta_stdy<<")"<<",("<<delta_refx<<","<<delta_refy<<"): "<<limbScore<<std::endl;
			if(ang>angleHighest){
				FILL_ARRAY(limbCorrect, limbIndex, i, 0)
				// if(limbIndex<2){
				// 	limbCorrect[limbIndex*3+i-1]=0;
				// }
				// else{
				// 	limbCorrect[6+(limbIndex-2)*2+i-1]=0;
				// }
			}
			cosineScore+=limbScore;		
		}
		else if(not stdVisible and not refVisible){
			cosineScore+=1;
		}
		else{
			// FILL_ARRAY(limbCorrect, limbIndex, i, 0)
			// if(limbIndex<2){
			// 	limbCorrect[limbIndex*3+i-1]=0;
			// }
			// else{
			// 	limbCorrect[6+(limbIndex-2)*2+i-1]=0;
			// }
		}
	}
	// std::cout<<std::endl;
	cosineScore=cosineScore/(1.0*(size-1));
	// cosineScore*=100;
	return cosineScore;
}

double EstimatorCosine::calcScoreBody(){
	std::vector<int> rightArm={1,2,3,4};
	std::vector<int> leftArm={1,5,6,7};
	std::vector<int> rightLeg={8,9,10};
	std::vector<int> leftLeg={11,12,13};

	memset(limbCorrect, 1, sizeof(limbCorrect));

	double rightArmScore=calcScoreLimb(rightArm, 0);
	double leftArmScore=calcScoreLimb(leftArm, 1);
	double rightLegScore=calcScoreLimb(rightLeg, 2);
	double leftLegScore=calcScoreLimb(leftLeg, 3);
	// std::cout<<std::endl;
	return (rightArmScore+leftArmScore+rightLegScore+leftLegScore)*1.0/4;
}
