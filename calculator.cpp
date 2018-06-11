#include "calculator.h"

Calculator::Calculator(std::vector<int> jointIndex)
{
    this->jointIndex=jointIndex;
    getJointLimbMap();
}

void Calculator::getJointLimbMap(){
    for(auto i=0;i<nKeypoints;i+=2){
        int start=limbSeq[i];
        int end=limbSeq[i+1];
        for(auto j=0;j<jointIndex.size();++j){
            if(jointIndex[j]==start){
                if(!startIndex[start]){
                    jointLimb.insert(std::pair<int,std::vector<int>>(start, {end}));
                    startIndex[start]=1;
                }
                else{
                    jointLimb[start].push_back(end);
                }
                break;
            }
        }
    }
}

float Calculator::calcSpeed(float duration){
    float limbSpeed=0.0;
    int limbCnt=0;
    if(prevData.size()==0){
        return limbSpeed;
    }
    for(auto i=0;i<curData.size();++i){
        if(startIndex[i]){
            std::vector<int>& ends=jointLimb[i];
            for(auto j=0;j<ends.size();++j){
                int ccx=(std::get<0>(curData[i])+std::get<0>(curData[j]))>>1;
                int ccy=(std::get<1>(curData[i])+std::get<1>(curData[j]))>>1;
                int pcx=(std::get<0>(prevData[i])+std::get<0>(prevData[j]))>>1;
                int pcy=(std::get<1>(prevData[i])+std::get<1>(prevData[j]))>>1;
                deltax+=(ccx-pcx);
                deltay+=(ccy-pcy);
                limbSpeed+=distance(ccx,ccy,pcx,pcy);
                limbCnt+=1;
            }
        }
    }
    float dist=limbSpeed/limbCnt;
    if(dist<dist_thresh){
        moving=0;
        return 0;
    }
    moving=1;
    curSpeed=dist/duration;
    deltay/=limbCnt;
    deltax/=limbCnt;
    return curSpeed;
}

int Calculator::calcRound(){
    if(prevData.size()==0 || !moving){
        return curRound;
    }

    char dir=(deltay>0)?1:0;
    if(direction==-1){
        direction=dir;
        curRound+=1;
        return 0;
    }
    else if(direction==dir){
        if(toggle){
            curRound+=1;
            toggle=0;
        }
        return curRound;
    }
    else if(direction!=dir){
        toggle=1;
        return curRound;
    }
}

float Calculator::calcCalory(){
    curCalory+=curSpeed*calory_coeff;
    return curCalory;
}
