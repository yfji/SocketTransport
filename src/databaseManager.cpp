#include "databaseManager.hpp"
#include <sstream>

DatabaseManager::DatabaseManager(const DatabaseManager& dbManager){
    isDatabaseConnected=static_cast<bool>(dbManager.isDatabaseConnected);
    isNewQuery=static_cast<bool>(dbManager.isNewQuery);
    frameIndex=dbManager.frameIndex;
    imageCount=dbManager.imageCount;
    scoreCount=dbManager.scoreCount;
    txtIndex=dbManager.txtIndex;
    txtCount=dbManager.txtCount;
    serverAddress=dbManager.serverAddress;
    userName=dbManager.userName;
    password=dbManager.password;
    imageBasePath=dbManager.imageBasePath;
	  txtBasePath=dbManager.txtBasePath;
}
DatabaseManager::DatabaseManager(const std::string& ip, \
                                           const std::string& user, \
                                           const std::string& passwd)
{http://localhost/
    isDatabaseConnected=false;
    isNewQuery=false;
    frameIndex=0;
    imageCount=0;
    scoreCount=0;
    txtIndex=0;
    txtCount=0;
	serverAddress=ip;
	userName=user;
	password=passwd;
	imageBasePath="/var/www/yuge/public";
	txtBasePath="/var/www/yuge/public/";
}

DatabaseManager::~DatabaseManager(){
	release();
}

DatabaseManager& DatabaseManager::operator=(const DatabaseManager& dbManager){
    isDatabaseConnected=static_cast<bool>(dbManager.isDatabaseConnected);
    isNewQuery=static_cast<bool>(dbManager.isNewQuery);
    frameIndex=dbManager.frameIndex;
    imageCount=dbManager.imageCount;
    scoreCount=dbManager.scoreCount;
    txtIndex=dbManager.txtIndex;
    txtCount=dbManager.txtCount;
		serverAddress=dbManager.serverAddress;
		userName=dbManager.userName;
		password=dbManager.password;
		imageBasePath=dbManager.imageBasePath;
		txtBasePath=dbManager.txtBasePath;
    return *this;
}

cv::Mat DatabaseManager::getImageFromDatabase(int uid, int action_id, int act_id){
    //std::cout<<"get frame"<<std::endl;
    const std::lock_guard<std::mutex> lock{databaseMutex};
    if(frameIndex<imageCount and imagesOneQuery.size()>0){
        cv::Mat curImage=imagesOneQuery[frameIndex];
        ++frameIndex;
        return curImage;
    }
    else if(frameIndex==imageCount && imagesOneQuery.size()>0){
        frameIndex=0;
        std::vector<cv::Mat>().swap(imagesOneQuery);
        return cv::Mat();
    }
	std::stringstream cmd;
	cmd<<"select id, img_url from "<<tableNames[0]<<" where uid = '"<<uid<<"' and action_id = '"<<action_id<<"' and act_id = '"<<act_id<<"'";
	res = mysql_query(&conn, cmd.str().c_str());
	if(res){
		release();
		return cv::Mat();
	}
    result = mysql_store_result(&conn);
    int rowcount = mysql_num_rows(result);
    if(rowcount==0){
        return cv::Mat();
    }
    imageCount=rowcount;
    scoreCount=rowcount;
    for(int i=0;i<imageCount;++i){
        MYSQL_ROW row = mysql_fetch_row(result);
        std::string name=imageBasePath+row[1];
        int imgId=std::atoi(row[0]);
        imageIds.push_back(imgId);
        imagesOneQuery.push_back(cv::imread(name));
    }
    frameIndex=1;
    return imagesOneQuery[0];
}

std::string DatabaseManager::getTxtFromDatabase(int action_id, int act_id){
    const std::lock_guard<std::mutex> lock{databaseMutex};
    if(txtIndex<txtCount and txtsOneQuery.size()>0){
        std::string curTxt=txtsOneQuery[txtIndex];
        ++txtIndex;
        return curTxt;
    }
    else if(txtIndex==txtCount && txtsOneQuery.size()>0){
        txtIndex=0;
        std::vector<std::string>().swap(txtsOneQuery);
        return "";
    }
	  std::stringstream cmd;
	  cmd<<"select id, action_txt from "<<tableNames[1]<<" where action_id = '"<<action_id<<"' and act_id = '"<<act_id<<"'";
          
	  res = mysql_query(&conn, cmd.str().c_str());
	  if(res){
		  release();
		  return "";
	  }
    result = mysql_store_result(&conn);
    int rowcount = mysql_num_rows(result);
    // std::cout<<rowcount<<std::endl;
    if(rowcount==0){
        // std::cout<<"No item selected, please check!"<<std::endl;
        return "";
    }
    txtCount=rowcount;
    for(int i=0;i<txtCount;++i)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        std::string txtPath=txtBasePath+row[1];
	// std::cout<<txtPath<<std::endl;
        int txtId=std::atoi(row[0]);
        txtIds.push_back(txtId);
        txtsOneQuery.push_back(txtPath);
    }
    txtIndex=1;
    return txtsOneQuery[0];
}
