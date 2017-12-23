#ifndef _ESTIMATOR_ANGLE_TRUNK_H_
#define _ESTIMATOR_ANGLE_TRUNK_H_

#include "estimator.h"	

class EstimatorAngleTrunk: public Estimator
{
public:
	EstimatorAngleTrunk();
	virtual ~EstimatorAngleTrunk();

private:
	const double lowestScore=70;
	const double highestScore=100;
	double stdLimbScores[10];
	double refLimbScores[10];
	double stdTrunkVector[2];
	double refTrunkVector[2];
	double stdHoriVector[2];
	double refHoriVector[2];

	const double angleHighest=20;

	bool calcTrunkVector();
	void calcScoreLimb(std::vector<int>& keypointIds, int limbIndex);
	double compareVector();
public:
	double calcScoreBody();
	inline double normalize(double score){
		double low=0.9;
		if(score<low)
			return lowestScore;
		return (score-low)/(1-low)*(highestScore-lowestScore)+lowestScore;
	}
};

#endif