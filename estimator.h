#ifndef ESTIMATOR_H
#define ESTIMATOR_H
#include <tuple>
#include <vector>
#include <iostream>
#include <fstream>

using DataRow = std::tuple<int,int,float>;

class Estimator
{
public:
    Estimator();

protected:
    static const int numKeypoints = {18};

    double databasePeaks[numKeypoints][4];
    double openposePeaks[numKeypoints][3];  //the person with the largest area in the vision

    char limbCorrect[10]={1,1,1,1,1,1,1,1,1,1};

public:
    bool readOpenposePeaks(const char* buff);
    bool readDatabasePeaks(const char* act_path);

    virtual double calcScoreBody(){}
    virtual double normalizescore(double score){}

    inline const char*  getActionStatePtr(){
        return (const char*)limbCorrect;
    }

    inline std::vector<DataRow> getOpenposePeaks(){
        std::vector<DataRow> tuplePeaks(numKeypoints);
        for(int i=0;i<numKeypoints;++i)
            tuplePeaks[i]=DataRow((int)openposePeaks[i][0],(int)openposePeaks[i][1],(float)openposePeaks[i][2]);
        return tuplePeaks;
    }
    inline int getNumKeypoints() const{
        return numKeypoints;
    }
};

#endif // ESTIMATOR_H
