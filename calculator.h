#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <iostream>
#include <vector>
#include <tuple>
#include <map>
#include <cmath>
#include "estimator.h"
#include "config.h"

using DataCal = std::tuple<float, int, float>;

class Calculator
{
public:
    Calculator(std::vector<int> jointIndex);

private:
    std::vector<int> jointIndex;
    std::vector<DataRow> curData;
    std::vector<DataRow> prevData;

    std::map<int,std::vector<int>> jointLimb;

    static const int nKeypoints={18};
    char startIndex[nKeypoints]={0};
    const std::vector<int> limbSeq={
        0,1,1,2,2,3,3,4,1,5,5,6,6,7,1,8,8,9,9,10,1,11,11,12,12,13,0,14,14,16,0,15,15,17
    };

    const float dist_thresh = {10.0};

    float curSpeed = {0};
    float deltay= {0};
    float deltax= {0};
    int curRound = {0};
    float curCalory = {0};
    float calory_coeff={1.21};

    char direction= {-1};   //1 or 0. dy>0(up) is positive
    char moving= {0};
    char toggle ={0};

private:
    void getJointLimbMap();
    inline float distance(int x1,int y1,int x2,int y2){
        int dx=x1-x2;
        int dy=y1-y2;
        return std::sqrt(1.0*dx*dx+1.0*dy*dy);
    }

public:
    float calcSpeed(float duration=1);  //time: s
    int calcRound();
    float calcCalory();

    inline void setData(std::vector<DataRow>& poseData){
        prevData=curData;
        curData=poseData;
    }
    inline std::vector<DataRow> getData(){
        return curData;
    }

    inline DataCal calculate(float duration=1){
        float speed=calcSpeed(duration);
        int round=calcRound();
        float calory=calcCalory();
        return DataCal(speed, round, calory);
    }
    inline void reset(){
        std::vector<DataRow>().swap(curData);
        std::vector<DataRow>().swap(prevData);
        curSpeed = 0;
        deltay= 0;
        deltax= 0;
        curRound =0;
        curCalory = 0;

        direction= -1;
        moving= 0;
        toggle =0;
    }
};

#endif // CALCULATOR_H
