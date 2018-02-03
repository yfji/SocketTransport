#ifndef _ESTIMATOR_COSINE_H
#define _ESTIMATOR_COSINE_H

#include "estimator.h"

class EstimatorCosine : public Estimator
{
public:
	EstimatorCosine();
	virtual ~EstimatorCosine();

private:
	const double lowestScore=70;
	const double highestScore=100;
	const double cosineLowest=0.966;
	const double angleHighest=20;

	double calcScoreLimb(std::vector<int>& keypointIds, int limbIndex);

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