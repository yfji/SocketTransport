#include "estimatorAngleTrunk.h"

EstimatorAngleTrunk::EstimatorAngleTrunk(){

}
EstimatorAngleTrunk::~EstimatorAngleTrunk(){

}

bool EstimatorAngleTrunk::calcTrunkVector(){
	double neckx=openposePeaks[1][0], necky=openposePeaks[1][1];
	if(neckx<0.1 and necky<0.1){
		// std::cout<<"Neck not visible"<<std::endl;
		return false;
	}
	stdTrunkVector[0]=(databasePeaks[8][0]+databasePeaks[11][0])*0.5-databasePeaks[1][0];
	stdTrunkVector[1]=(databasePeaks[8][1]+databasePeaks[11][1])*0.5-databasePeaks[1][1];
	stdHoriVector[0]=1;
	stdHoriVector[1]=(-stdTrunkVector[0]/stdTrunkVector[1]);
	refTrunkVector[0]=(openposePeaks[8][0]+openposePeaks[11][0])*0.5-openposePeaks[1][0];
	refTrunkVector[1]=(openposePeaks[8][1]+openposePeaks[11][1])*0.5-openposePeaks[1][1];
	refHoriVector[0]=1;
	refHoriVector[1]=(-refHoriVector[0]/refHoriVector[1]);
}

void EstimatorAngleTrunk::calcScoreLimb(std::vector<int>& keypointIds, int limbIndex){
	double stdx,stdy,refx,refy;
	double delta_stdx, delta_stdy, delta_refx, delta_refy;
	auto size=keypointIds.size();

	char stdVisible=1;
	char refVisible=1;
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
		for(auto i=0;i<10;++i){
			stdLimbScores[i]=-10;
			refLimbScores[i]=-10;
		}
		// if(limbIndex<2){
		// 	limbCorrect[limbIndex*3+i-1]=0;
		// }
		// else{
		// 	limbCorrect[6+(limbIndex-2)*2+i-1]=0;
		// }
		return;
	}
	for(auto i=1;i<size;++i){
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
			double stdLimbScore=(delta_stdx*stdTrunkVector[0]+delta_stdy*stdTrunkVector[1])/(std::sqrt(delta_stdx*delta_stdx+delta_stdy*delta_stdy)*std::sqrt(stdTrunkVector[0]*stdTrunkVector[0]+stdTrunkVector[1]*stdTrunkVector[1]));
			double refLimbScore=(delta_refx*refTrunkVector[0]+delta_refy*refTrunkVector[1])/(std::sqrt(delta_refx*delta_refx+delta_refy*delta_refy)*std::sqrt(refTrunkVector[0]*refTrunkVector[0]+refTrunkVector[1]*refTrunkVector[1]));

			// std::cout<<keypointIds[i]<<":"<<"("<<delta_stdx<<","<<delta_stdy<<")"<<",("<<delta_refx<<","<<delta_refy<<")"<<": "<<stdLimbScore<<","<<refLimbScore<<std::endl;
			
			FILL_ARRAY(stdLimbScores, limbIndex, i, stdLimbScore)
			FILL_ARRAY(refLimbScores, limbIndex, i, refLimbScore)
			// if(limbIndex<2){
			// 	stdLimbScores[limbIndex*3+i-1]=stdLimbScore;
			// 	refLimbScores[limbIndex*3+i-1]=refLimbScore;
			// }
			// else{
			// 	stdLimbScores[6+(limbIndex-2)*2+i-1]=stdLimbScore;
			// 	refLimbScores[6+(limbIndex-2)*2+i-1]=refLimbScore;
			// }
		}
		else if(not stdVisible and not refVisible){
			FILL_ARRAY(stdLimbScores, limbIndex, i, 1)
			FILL_ARRAY(refLimbScores, limbIndex, i, 1)
		}
		else{
			FILL_ARRAY(stdLimbScores, limbIndex, i, -10);
			FILL_ARRAY(refLimbScores, limbIndex, i, -10);
		}
	}
}

double EstimatorAngleTrunk::compareVector(){
	// ignore shoulders
	double delta_angle;
	double score=0.0;
	int validLimbs=0;
	for(auto i=0;i<10;++i){
		if(stdLimbScores[i]<-1 or refLimbScores[i]<-1){
			limbCorrect[i]=0;
			continue;
		}
		delta_angle=std::abs((std::acos(stdLimbScores[i])-std::acos(refLimbScores[i]))*180/3.1415926);
		if(delta_angle>angleHighest){
			limbCorrect[i]=0;
		}
		score=score+(1-std::abs(stdLimbScores[i]-refLimbScores[i]));
		++validLimbs;
	}
	return score/validLimbs;
}

double EstimatorAngleTrunk::calcScoreBody(){
	std::vector<int> rightArm={1,2,3,4};
	std::vector<int> leftArm={1,5,6,7};
	std::vector<int> rightLeg={8,9,10};
	std::vector<int> leftLeg={11,12,13};

	memset(stdLimbScores, 0, sizeof(stdLimbScores));
	memset(refLimbScores, 0, sizeof(refLimbScores));

	if(calcTrunkVector())
		memset(limbCorrect, 1, sizeof(limbCorrect));
	else{
		memset(limbCorrect, 0, sizeof(limbCorrect));
		return 0;
	}
	calcScoreLimb(rightArm, 0);
	calcScoreLimb(leftArm, 1);
	calcScoreLimb(rightLeg, 2);
	calcScoreLimb(leftLeg, 3);

	double score=compareVector();
	// for(auto i=0;i<10;++i)
	// 	std::cout<<(int)limbCorrect[i]<<",";
	// std::cout<<std::endl;
	return score;
}
