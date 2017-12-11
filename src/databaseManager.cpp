#include "databaseManager.hpp"
#include <sstream>

DatabaseManager::DatabaseManager(const DatabaseManager& dbManager){
    isDatabaseConnected=static_cast<bool>(dbManager.isDatabaseConnected);
    isNewQuery=static_cast<bool>(dbManager.isNewQuery);
    frameIndex=dbManager.frameIndex;
    txtIndex=dbManager.txtIndex;
    recordCount=dbManager.recordCount;
    recordWriteCount=dbManager.recordWriteCount;
    serverAddress=dbManager.serverAddress;
    userName=dbManager.userName;
    password=dbManager.password;
	imageBasePath=dbManager.imageBasePath;
	txtBasePath=dbManager.txtBasePath;
}
DatabaseManager::DatabaseManager(const std::string& ip, \
                                           const std::string& user, \
                                           const std::string& passwd)
{
	isDatabaseConnected=false;
	isNewQuery=false;
	frameIndex=0;
	txtIndex=0;
	recordCount=0;
	recordWriteCount=0;
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
    txtIndex=dbManager.txtIndex;
    recordCount=dbManager.recordCount;
    recordWriteCount=dbManager.recordWriteCount;
	serverAddress=dbManager.serverAddress;
	userName=dbManager.userName;
    password=dbManager.password;
	imageBasePath=dbManager.imageBasePath;
	txtBasePath=dbManager.txtBasePath;
    return *this;
}

bool DatabaseManager::queryImage(int uid, int act_id){
	std::stringstream image_cmd;
	image_cmd<<"select id,img_url from "<<tableNames[0]<<" where uid = '"<<uid<<"' and action_id = '"<<act_id<<"'";
	int res = mysql_query(&conn, image_cmd.str().c_str());
	if(res){
		release();
		std::cout<<"Query database error!"<<std::endl;
		return false;
	}
    result = mysql_store_result(&conn);
    int rowcount = mysql_num_rows(result);
    if(rowcount==0){
        std::cout<<"No item selected, please check!"<<std::endl;
        return false;
    }
    std::cout<<rowcount<<" images selected"<<std::endl;
    if(recordCount!=0 and recordCount!=rowcount){
    	std::cout<<"Numbers of standard image and txt are not the same, please check!"<<std::endl;
        return false;
    }
    recordCount=rowcount;
    for(int i=0;i<recordCount;++i){
        MYSQL_ROW row = mysql_fetch_row(result);
        std::string name=imageBasePath+row[1];
        int imgId=std::atoi(row[0]);
        imageIds.push_back(imgId);
        imagesOneQuery.push_back(cv::imread(name));
    }
    return true;
}

bool DatabaseManager::queryTxt(int act_id){
	std::stringstream txt_cmd;
	txt_cmd<<"select id,action_txt from "<<tableNames[1]<<" where action_id = '"<<act_id<<"'";
	int res = mysql_query(&conn, txt_cmd.str().c_str());
	if(res){
		release();
		std::cout<<"Query database error!"<<std::endl;
		return false;
	}
    result = mysql_store_result(&conn);
    int rowcount = mysql_num_rows(result);
    if(rowcount==0){
        std::cout<<"No item selected, please check!"<<std::endl;
        return false;
    }
    std::cout<<rowcount<<" txts selected"<<std::endl;
    if(recordCount!=0 and recordCount!=rowcount){
    	std::cout<<"Numbers of txt and standard image are not the same, please check!"<<std::endl;
        return false;
    }
    recordCount=rowcount;
    for(int i=0;i<recordCount;++i)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        std::string txtPath=txtBasePath+row[1];
        int txtId=std::atoi(row[0]);
        txtIds.push_back(txtId);
        txtsOneQuery.push_back(txtPath);
    }
    return true;
}

bool DatabaseManager::queryBeforeGet(int uid, int act_id){
	bool ok=true;
	ok &= queryImage(uid, act_id);
	ok &= queryTxt(act_id);
	return ok;
}

cv::Mat DatabaseManager::getImageFromDatabase(int uid, int act_id){
    //std::cout<<"get frame"<<std::endl;
    const std::lock_guard<std::mutex> lock{databaseMutex};
    if(frameIndex<recordCount and imagesOneQuery.size()>0){
        cv::Mat curImage=imagesOneQuery[frameIndex];
        ++frameIndex;
        return curImage;
    }
    //else if(frameIndex==imageCount && imagesOneQuery.size()>0){
    //    return cv::Mat();
    //}
	return cv::Mat();
}

std::string DatabaseManager::getTxtFromDatabase(int act_id){
    //std::cout<<"get frame"<<std::endl;
    const std::lock_guard<std::mutex> lock{databaseMutex};
    if(txtIndex<recordCount and txtsOneQuery.size()>0){
        std::string curTxt=txtsOneQuery[txtIndex];
        ++txtIndex;
        return curTxt;
    }
    //else if(txtIndex==txtCount && txtsOneQuery.size()>0){
    //    return NULL;
    //}
	return "";
}

bool DatabaseManager::writeScoreToDatabase(const double score){
    const std::lock_guard<std::mutex> lock{databaseMutex};
    std::stringstream cmd;
    cmd<<"update "<<tableNames[0]<<" set score = '"<<score<<"' where id = '"<<imageIds[recordWriteCount]<<"'";
    int res = mysql_query(&conn, cmd.str().c_str());
	if(res){
		release();
		std::cout<<"Update "<<tableNames[0]<<" for score error!"<<std::endl;
		return false;
	}
    std::cout<<"Write scores successfully"<<std::endl;
    return true;
}

bool DatabaseManager::writeImageUrlToDatabase(const std::string& imgurl){
    const std::lock_guard<std::mutex> lock{databaseMutex};
    std::stringstream cmd;
    cmd<<"update "<<tableNames[0]<<" set img_deal = '"<<imgurl<<"' where id = '"<<imageIds[recordCount]<<"'";
    int res = mysql_query(&conn, cmd.str().c_str());
	if(res){
		release();
		std::cout<<"Update "<<tableNames[0]<<" for image url error!"<<std::endl;
		return false;
	}
    std::cout<<"Write image url successfully"<<std::endl;
    return true;
}

bool DatabaseManager::writeTxtUrlToDatabase(const std::string& txtUrl){
	const std::lock_guard<std::mutex> lock{databaseMutex};
    std::stringstream cmd;
    cmd<<"update "<<tableNames[0]<<" set deal_txt = '"<<txtUrl<<"' where id = '"<<imageIds[recordCount]<<"'";
    int res = mysql_query(&conn, cmd.str().c_str());
	if(res){
		release();
		std::cout<<"Update "<<tableNames[0]<<" for txt url error!"<<std::endl;
		return false;
	}
    std::cout<<"Write txt url successfully"<<std::endl;
    return true;
}

bool DatabaseManager::writePoseDataToDatabase(const std::string& data){
	const std::lock_guard<std::mutex> lock{databaseMutex};
	std::stringstream cmd;
    cmd<<"update "<<tableNames[0]<<" set action_content = '"<<data<<"' where id = '"<<imageIds[recordCount]<<"'";
    int res = mysql_query(&conn, cmd.str().c_str());
	if(res){
		release();
		std::cout<<"Update "<<tableNames[0]<<" for pose data error!"<<std::endl;
		return false;
	}
    std::cout<<"Write pose data successfully"<<std::endl;
    return true;
}

void DatabaseManager::clear(){
	frameIndex=0;
	txtIndex=0;
    std::vector<cv::Mat>().swap(imagesOneQuery);
    std::vector<std::string>().swap(txtsOneQuery);
    std::vector<int>().swap(imageIds);
}

void DatabaseManager::writeCountPP(){
	recordCount++;
}
