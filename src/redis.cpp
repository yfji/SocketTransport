#include "redis.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <assert.h>

void Redis::loadPortsFromFile(const char* cfgFile){
	std::ifstream in;
	in.open(cfgFile, std::ios::in);
	if(not in){
		std::cerr<<"File not exists"<<std::endl;
		return;
	}
	int lineIndex=0;
	while(not in.eof()){
		std::string line;
		std::getline(in,line);
		std::stringstream to;
		to<<"ports_"<<lineIndex;
		set(to.str(), line);
		to.str("");
		to<<"avail_"<<lineIndex;
		set(to.str(),"1");
		lineIndex++;
	}
	maxPorts=lineIndex;
	in.close();
}

bool Redis::readAvailablePorts(std::vector<int>& ports){
	try{
		int index=0;
		for(auto i=0;i<maxPorts;++i){
			std::stringstream ss;
			ss<<"avail_"<<index;
			int available=std::atoi(get(ss.str()).c_str());
			if(available){
				ss.str("");
				ss<<"ports_"<<index;
				std::stringstream pss(get(ss.str()));
				pss>>ports[0]>>ports[1];
				ss.str("");
				ss<<"avail_"<<index;
				set(ss.str(),"0");
				curIndex=index;
				return true;
			}
			++index;
		}
	}
	catch(std::exception& e){
		std::cerr<<"Key not exists"<<std::endl;
    		unLock();
		return false;
	}
	return false;
}

void Redis::releasePorts(){
	if(curIndex==-1)
		return;
	std::stringstream ss;
	ss<<"avail_"<<curIndex;
	set(ss.str(),"1");
}
