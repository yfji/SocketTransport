/*
 * Client.cpp
 *
 *      Author: yufeng
 */

#include "Client.h"

cv::Mat	globalFrame;
vector<cv::Mat> globalFrames;
int maxQueueLen;

Client::Client(const string& _ip, const int _port, const int _uid, const int _act_id) {
	// TODO Auto-generated constructor stub
	clientsd=socket(AF_INET, SOCK_STREAM, 0);
	if(clientsd==-1){
		// cout<<"Socket error"<<endl;
		exit(0);
	}
	uid=_uid;
	act_id=_act_id;
	ipAddr=_ip;
	port=_port;
	server_socket.sin_family=AF_INET;
	server_socket.sin_port=htons(port);
	server_socket.sin_addr.s_addr=inet_addr(ipAddr.c_str());
	frameIndex=0;
	maxQueueLen=8;
	// numEmptyFrames=0;
}

Client::~Client() {
	// TODO Auto-generated destructor stub
	if(bConnected)
		close(clientsd);
}

bool Client::connectServer(){
	if(connect(clientsd, (struct sockaddr*)(&server_socket), sizeof(server_socket))<0){
		// printf("Connect error\n");
		bConnected=false;
		return false;
	}
	bConnected=true;
	// cout<<"server connect successfully"<<endl;
	return true;
}

void Client::sendMessage(const char* message){
	int len=strlen(message);
	send(clientsd, message, len, 0);
}

void Client::sendSingleImage(char* image){
	FILE* fp=fopen(image, "rb");
	if(fp==NULL){
		// cout<<"No such file"<<endl;
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
		//for(int i=0;i<len;++i)
		//	buff[i]=buffer[i+start];
		int rn=send(clientsd, buffer+start, len, 0);
		//cout<<rn<<endl;
		total+=len;
		start+=len;
		if(start>=size)	break;
		if(start+len>=size)	len=size-start;
	}
}

void Client::listenAndSendFrame(){
	char message[50];
	strcpy(message, "");
	//cv::namedWindow("client");

	cv::Mat image;
	// const int numGpu=7;
	maxQueueLen=10;
	int queueFrames=0;
	// int gpuFrames=0;
	globalFrames.resize(maxQueueLen);
	while(1){
		if(strcmp(message, "frame")==0){
#ifdef FINITE_FRAMES
			if(frameIndex==500){
				sendMessage("stop");
			}

			else{
#endif
				image=spDbManager->getImageFromDatabase(uid, act_id);
				if(image.empty()){
					sendMessage("stop");
				}
				else{
					image.copyTo(globalFrames[queueFrames]);
					queueFrames=(queueFrames+1)%maxQueueLen;
					char header[50];
					vector<uchar> mat_data;
					cv::imencode(".jpg", image, mat_data);
					uchar* buffer=(uchar*)mat_data.data();
					int size=mat_data.size();
					sprintf(header,"sz:%d", size);
					sendMessage(header);
					while(1){
						int rn=recv(clientsd, message, sizeof(message), 0);
						message[rn]='\0';
						if(strcmp(message, "sz")==0){
							break;
						}
					}
					sendSingleImageBuff(buffer,size);
					// cout<<"frame: "<<frameIndex<<endl;
					++frameIndex;
				}
#ifdef FINITE_FRAMES
			}
#endif
		}
		else if(strcmp(message, "stop")==0){
			// cout<<"send all frames"<<endl;
			break;
		}
		usleep(5);
		int rn=recv(clientsd, message, sizeof(message), 0);
		message[rn]='\0';
		// if(strlen(message)>0)	cout<<message<<endl;
	}
	bConnected=false;
	close(clientsd);
}

bool Client::queryDatabase(){
	
}
