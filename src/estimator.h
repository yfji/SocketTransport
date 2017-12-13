#ifndef _ESTIMATOR_H_
#define _ESTIMATOR_H_

#define COSINE 	0
#define EUCLID 	1
#include <iostream>
#include <vector>
#include <cmath>
#include <string.h>
#include <fstream>
#include <sstream>

#define FILL_ARRAY(array, limbInd, ind, v)	\
	if(limbInd<2)	array[limbInd*3+ind-1]=v;		\
	else	array[6+(limbInd-2)*2+ind-1]=v;

class Estimator{
public:
	Estimator();
	virtual ~Estimator();
protected:
	static const int numKeypoints=18;
	
	double databasePeaks[numKeypoints][4];
	double openposePeaks[numKeypoints][3];

	// char limbCorrect[4]={1,1,1,1};		
	// right arm, left arm, right leg, left leg

	char limbCorrect[10]={1,1,1,1,1,1,1,1,1,1};	
	// right shoulder, right upper arm, right lower arm, left shoulder, left upper arm, left lower arm, right upper leg, right lower leg, left upper leg, left lower leg

// private:

// 	double calcScoreLimb(std::vector<int>& keypointIds, int limbIndex);

// 	virtual double calcScoreLimbMethod(std::vector<int>& keypointIds, int limbIndex)=0;

public:
	bool readOpenposePeaks(const char* buff);
	bool readDatabasePeaks(const char* act_path);
	
	virtual double calcScoreBody()=0;
	virtual double normalize(double score)=0;

	inline const char*  getActionStatePtr(){
		return (const char*)limbCorrect;
	}
	inline std::vector<double*> getOpenposePeakPtr(){
		std::vector<double*> vPeaks(numKeypoints);
		for(int i=0;i<numKeypoints;++i)
			vPeaks[i]=(double*)openposePeaks[i];
		return vPeaks;
	}
	inline std::vector<double*> getDatabasePeakPtr(){
		std::vector<double*> vPeaks(numKeypoints);
		for(int i=0;i<numKeypoints;++i)
			vPeaks[i]=(double*)databasePeaks[i];
		return vPeaks;
	}
	inline int getNumKeypoints() const{
		return numKeypoints;
	}
};

#endif