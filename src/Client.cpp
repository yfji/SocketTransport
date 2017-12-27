/*
 * Client.cpp
 *
 *      Author: yufeng
 */

#include "Client.h"
#include <mutex>
#include <memory>

#define RESIZE	0
#define FINITE_FRAMES 	1
#define COMPRESS	1
#define SIMPLE	1

vector<cv::Mat> globalFrames;
std::mutex dataMutex;
int maxQueueLen;

Client::Client(const string& ip, const int p, const int _uid, const int _action_id) {
	serverAddress=ip;
	port=p;
	uid=_uid;
	action_id=_action_id;
	server_socket.sin_family=AF_INET;
	server_socket.sin_port=htons(port);
	server_socket.sin_addr.s_addr=inet_addr(serverAddress.c_str());
	frameIndex=0;
	numEmptyFrames=0;
	bConnected=false;
}

Client::~Client() {
	if(bConnected){
		close(clientsd);
	}
}

bool Client::connectServer(){
	clientsd=socket(AF_INET, SOCK_STREAM, 0);
	if(clientsd==-1){
		cout<<"Socket error"<<endl;
		return false;
	}
	std::cout<<"Client connect request"<<std::endl;
	if(connect(clientsd, (struct sockaddr*)(&server_socket), sizeof(server_socket))<0){
		cout<<"Connect error"<<endl;
		return false;
	}
	struct timeval timeout={1,0};	//1s
	int ret=setsockopt(clientsd, SOL_SOCKET, SO_RCVTIMEO, (char*)(&timeout), sizeof(timeout));
	if(ret==-1){
		std::cout<<"Setsockopt error"<<std::endl;
		close(clientsd);
		return false;
	}
	sendMessage(handcheckMessage);
	int rn=recv(clientsd, handcheckBuff, cmd_len, MSG_WAITALL);
	cout<<"rn: "<<rn<<' '<<"errno: "<<errno<<endl;
	if(rn<=0 or errno==11){
		std::cout<<"Time out, handcheck failed"<<std::endl;
		close(clientsd);
		return false;
	}
	handcheckBuff[rn]='\0';
	cout<<handcheckBuff<<endl;
	if(strcmp(handcheckBuff, handcheckMessage)!=0){
		std::cout<<"Command error, handcheck failed"<<std::endl;
		close(clientsd);
		return false;
	}
	bConnected=true;
	cout<<"server connect successfully"<<endl;
	return true;
}

void Client::sendMessage(const char* message){
	int len=strlen(message);
	cout<<message<<endl;
	send(clientsd, message, len, 0);
}

void Client::sendSingleImage(char* image){
	FILE* fp=fopen(image, "rb");
	if(fp==NULL){
		cout<<"No such file"<<endl;
		return;
	}
	while(1){
		int rn=fread(buff, sizeof(char), MAX_LEN, fp);
		send(clientsd, buff, rn, 0);
		if(rn<=0)	break;
	}
}

void Client::sendSingleImageBuff(uchar* buffer, int size){
	int start=0, len=min(MAX_LEN,size);
	int total=0;
	while(1){
		int rn=send(clientsd, buffer+start, len, 0);
		total+=len;
		start+=len;
		if(start>=size)	break;
		if(start+len>=size)	len=size-start;
	}
}

void Client::listenAndSendFrame(){
	char message[50];
	strcpy(message, "");

	cv::Mat image;
	maxQueueLen=10;		//More than 2 times the number of GPU
	int queueFrames=0;
	globalFrames.resize(maxQueueLen);
	
	bool queryOK=spDbManager->queryImage(uid, action_id);
	
	while(1){
		memset(message, '\0', sizeof(message));
		int rn=recv(clientsd, message, sizeof(message), 0);
		message[rn]='\0';
		/*** Test ***/
		std::cout<<message<<std::endl;
		
		if(strcmp(message, "frame")==0){
			if(not queryOK){
				std::cout<<"TXT files and image files not corresponding, exit!"<<std::endl;
				sendMessage("stop");
			}
#if FINITE_FRAMES==1
			if(frameIndex==300){
				sendMessage("stop");
			}

			else{
#endif
				image=spDbManager->getImageFromDatabase();
				if(image.empty()){
					// std::cout<<"Empty image"<<std::endl;
					sendMessage("stop");
				}
				else{
#if	RESIZE==1
					cv::resize(image, image, cv::Size(image.cols/2, image.rows/2));
#endif
					image.copyTo(globalFrames[queueFrames]);
					queueFrames=(queueFrames+1)%maxQueueLen;
					char header[50];
#if COMPRESS==1
					vector<uchar> mat_data;
					cv::imencode(".jpg", image, mat_data);
					uchar* buffer=(uchar*)mat_data.data();
					int size=mat_data.size();
					sprintf(header,"sz:%d", size);
#else
					uchar* buffer=image.data;
					int size=image.rows*image.cols*image.channels();
					sprintf(header,"sz:%d,%d,%d\0",image.cols,image.rows,image.channels());
#endif
					sendMessage(header);
					while(1){
						int rn=recv(clientsd, message, sizeof(message), 0);
						message[rn]='\0';
						if(strcmp(message, "sz")==0){
							break;
						}
					}
#if SIMPLE==0
					sendMessage("image");
					while(1){
						int rn=recv(clientsd, message, sizeof(message), 0);
						message[rn]='\0';
						if(strcmp(message, "image")==0)
							break;
					}
#endif
					sendSingleImageBuff(buffer,size);
					cout<<"frame: "<<frameIndex<<endl;
					++frameIndex;
				}
#if FINITE_FRAMES==1
			}
#endif
		}
		else if(strcmp(message, "stop")==0){
			cout<<"send all frames"<<endl;
			break;
		}
		usleep(5);
	}
	bConnected=false;
	close(clientsd);
}
