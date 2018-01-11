#ifndef _ESTIMATOR_EUCLIDEAN_H
#define _ESTIMATOR_EUCLIDEAN_H

#include "estimator.h"

class EstimatorEuclidean : public Estimator
{
public:
	EstimatorEuclidean();
	virtual ~EstimatorEuclidean();

private:
	const double armThreshold=0.6;
	const double legThreshold=0.5;
	double personArea;
	double yScale;

private:
	void calcPersonArea();
	double calcScoreLimb(std::vector<int>& keypointIds, int limbIndex);

public:
	double calcScoreBody();
	inline double normalize(double score){
		// NOT IMPLEMENTED
	}
};

#endif