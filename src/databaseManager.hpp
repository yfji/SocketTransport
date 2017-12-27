#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <iostream>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <mysql/mysql.h>
#include <fstream>
#include <atomic>
#include <mutex>

class DatabaseManager
{
public:
    DatabaseManager(const DatabaseManager& dbManager);
    explicit DatabaseManager(const std::string& ip="localhost", \
                             const std::string& user="root", \
                             const std::string& password="yg123456yg");
  	virtual ~DatabaseManager();
public:
	inline bool isOpened() const{
		return (isDatabaseConnected==true);
	}

  	inline void release(){
		if(isDatabaseConnected){
			mysql_close(&conn);
			isDatabaseConnected=false;
		}
	}

	inline void setDatabaseAndTables(const std::string& database, \
                                     const std::vector<std::string>& tables){
		databaseName=database;
		tableNames=tables;
		// std::cout<<"databaseName:"<<std::endl;
		// std::cout<<'\t'<<databaseName<<std::endl;
		// std::cout<<"table names:"<<std::endl;
		// for(size_t i=0;i<tableNames.size();++i){
		// 	std::cout<<'\t'<<tableNames[i]<<std::endl;
		// }
	}
	inline bool connectDatabase(){
		mysql_init(&conn);
		isDatabaseConnected=mysql_real_connect(&conn,serverAddress.c_str(),userName.c_str(),password.c_str(),databaseName.c_str(),0,NULL,CLIENT_FOUND_ROWS);
		if(not isDatabaseConnected){
			// std::cout<<"Connect database error!"<<std::endl;
			;
		}
		else{
			// std::cout<<"connect database successfully"<<std::endl<<std::endl;
			;
		}
		return isDatabaseConnected;
	}
    
	DatabaseManager& operator=(const DatabaseManager& dbManager);

private:
	std::string serverAddress;
	std::string userName;
	std::string password;
	std::string  databaseName;
	std::string imageBasePath;
	std::string txtBasePath;
	std::vector<std::string> tableNames;		//imagetable, feedbacktable
	std::mutex databaseMutex;

	std::atomic<bool> isDatabaseConnected;
	std::atomic<bool> isNewQuery;
	int frameIndex;
	int imageCount;
	int scoreCount;
	int txtIndex;
	int txtCount;
	int res;
	std::vector<cv::Mat> imagesOneQuery;
	std::vector<std::string> txtsOneQuery;
	std::vector<int> imageIds;
	std::vector<int> txtIds;

	MYSQL conn;
	MYSQL_RES *result = NULL;
	MYSQL_FIELD *field = NULL;

public:
	cv::Mat getImageFromDatabase(int uid, int action_id, int act_id);
	std::string getTxtFromDatabase(int action_id, int act_id);
};

#endif
