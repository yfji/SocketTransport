/*
 * Client.h
 *
 *  Created on: 2017年4月12日
 *      Author: yufeng
 */

#ifndef SRC_CLIENT_H_
#define SRC_CLIENT_H_
#include <iostream>
#include <vector>
#include <fstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include "databaseManager.hpp"

#define MAX_LEN 65536

using namespace std;

class Client {
public:
	Client(const string& ip, const int p, const int uid, const int action_id);
	virtual ~Client();
private:
	int clientsd;
	int totalSize;
	int connState;
	int port;
	const int queue=10;
	struct sockaddr_in server_socket;
	struct sockaddr_in client_socket;
	std::string serverAddress;
	unsigned char buff[MAX_LEN];
	char handcheckBuff[5];
	const int cmd_len=4;
	const char* handcheckMessage="yuge";
	int frameIndex;
	int uid;
	int action_id;
	int numEmptyFrames;
	bool bConnected;
	
	std::shared_ptr<DatabaseManager> spDbManager;
public:
	inline void setDatabaseManager(const 							std::shared_ptr<DatabaseManager>& sp){
		spDbManager=sp;
	}
	bool queryDatabase();
	bool connectDatabase();
	bool connectServer();
	void sendMessage(const char* message);
	void sendSingleImage(char* image);
	void sendSingleImageBuff(uchar* buffer, int size);
	void listenAndSendFrame();
};

#endif /* SRC_CLIENT_H_ */
