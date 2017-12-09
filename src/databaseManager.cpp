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
{
    isDatabaseConnected=false;
    isNewQuery=false;
    frameIndex=0;
    imageCount=0;
    scoreCount=0;
    txtIndex=0;
    txtCount=0;
	  imgWriteCount=0;
    scoreWriteCount=0;
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
    imgWriteCount=dbManager.imgWriteCount;
    scoreWriteCount=dbManager.scoreWriteCount;
	  serverAddress=dbManager.serverAddress;
	  userName=dbManager.userName;
    password=dbManager.password;
    imageBasePath=dbManager.imageBasePath;
	txtBasePath=dbManager.txtBasePath;
    return *this;
}

cv::Mat DatabaseManager::getImageFromDatabase(int uid, int act_id){
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
	cmd<<"select id,img_url from "<<tableNames[0]<<" where uid = '"<<uid<<"' and action_id = '"<<act_id<<"'";
	res = mysql_query(&conn, cmd.str().c_str());
	if(res){
		release();
		// std::cout<<"Query database error!"<<std::endl;
		return cv::Mat();
	}
    result = mysql_store_result(&conn);
    int rowcount = mysql_num_rows(result);
    if(rowcount==0){
        // std::cout<<"No item selected, please check!"<<std::endl;
        return cv::Mat();
    }
    // std::cout<<rowcount<<" items selected"<<std::endl;
    imageCount=rowcount;
    scoreCount=rowcount;
    for(int i=0;i<imageCount;++i){
        MYSQL_ROW row = mysql_fetch_row(result);
        std::string name=imageBasePath+row[1];
        //std::cout<<name<<std::endl;
        int imgId=std::atoi(row[0]);
        imageIds.push_back(imgId);
        imagesOneQuery.push_back(cv::imread(name));
    }
    frameIndex=1;
    return imagesOneQuery[0];
}

std::string DatabaseManager::getTxtFromDatabase(int act_id){
    //std::cout<<"get frame"<<std::endl;
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
	  cmd<<"select id,action_txt from "<<tableNames[1]<<" where action_id = '"<<act_id<<"'";
	  res = mysql_query(&conn, cmd.str().c_str());
	  if(res){
		  release();
		  // std::cout<<"Query database error!"<<std::endl;
		  return "";
	  }
    result = mysql_store_result(&conn);
    int rowcount = mysql_num_rows(result);
    if(rowcount==0){
        // std::cout<<"No item selected, please check!"<<std::endl;
        return "";
    }
    txtCount=rowcount;
    for(int i=0;i<txtCount;++i)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        std::string txtPath=txtBasePath+row[1];
        int txtId=std::atoi(row[0]);
        txtIds.push_back(txtId);
        txtsOneQuery.push_back(txtPath);
    }
    txtIndex=1;
    return txtsOneQuery[0];
}

bool DatabaseManager::writeScoreToDatabase(const double score){
    const std::lock_guard<std::mutex> lock{databaseMutex};
    std::stringstream cmd;
    cmd<<"update "<<tableNames[0]<<" set score = '"<<score<<"' where id = '"<<imageIds[scoreWriteCount]<<"'";
    res = mysql_query(&conn, cmd.str().c_str());
	  if(res){
		    release();
        // std::cout<<"Update "<<tableNames[0]<<" error!"<<std::endl;
        return false;
	  }
    // std::cout<<"Write scores successfully"<<std::endl;
    ++scoreWriteCount;
    if(scoreWriteCount==scoreCount and imgWriteCount==imageCount){
        // std::cout<<"Number of scores is larger than the number of images, please check!"<<std::endl;
        std::vector<int>().swap(imageIds);
        scoreWriteCount=0;
        imgWriteCount=0;
        // std::cout<<"One action finished, last operation is saving score"<<std::endl;
    }
    return true;
}

bool DatabaseManager::writeUrlToDatabase(const std::string& imgurl,const std::string& txtUrl){
    const std::lock_guard<std::mutex> lock{databaseMutex};
    std::stringstream cmd;
    if(txtUrl==""){
    	cmd<<"update "<<tableNames[0]<<" set img_deal = '"<<imgurl<<"' where id = '"<<imageIds[imgWriteCount]<<"'";
	}
	else{
    	cmd<<"update "<<tableNames[0]<<" set img_deal = '"<<imgurl<<"' , deal_txt = '"<<txtUrl<<"' where id = '"<<imageIds[imgWriteCount]<<"'";
	}
    res = mysql_query(&conn, cmd.str().c_str());
	  if(res){
		    release();
        // std::cout<<"Update "<<tableNames[0]<<" error!"<<std::endl;
        return false;
	  }
    // std::cout<<"Write url successfully"<<std::endl;
    ++imgWriteCount;
    if(imgWriteCount==imageCount and scoreWriteCount==scoreCount){
        // std::cout<<"Number of imgurl is larger than the number of images, please check!"<<std::endl;
        std::vector<int>().swap(imageIds);
        imgWriteCount=0;
        scoreWriteCount=0;
        // std::cout<<"One action finished, last operation is saving url"<<std::endl;
    }
    return true;
}
