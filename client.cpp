/*
 * Client.cpp
 *
 *      Author: yufeng
 */

#include "client.h"
#include <mutex>
#include <memory>

cv::Mat	globalFrame;
vector<cv::Mat> globalFrames;
std::mutex dataMutex;
int maxQueueLen;

Client::Client(const string& ip, const int p) {
	// TODO Auto-generated constructor stub
	ipAddr=ip;
	port=p;
	server_socket.sin_family=AF_INET;
	server_socket.sin_port=htons(port);
	server_socket.sin_addr.s_addr=inet_addr(ipAddr.c_str());
	numEmptyFrames=0;
}

Client::~Client() {
	// TODO Auto-generated destructor stub
	if(bConnected)
		close(clientsd);
}

bool Client::connectServer(){
    clientsd=socket(AF_INET, SOCK_STREAM, 0);
    if(clientsd==-1){
        cout<<"Socket error"<<endl;
    }
	std::cout<<"connect"<<std::endl;
	if(connect(clientsd, (struct sockaddr*)(&server_socket), sizeof(server_socket))<0){
		printf("Connect error\n");
		bConnected=false;
		return false;
	}
	bConnected=true;
	cout<<"server connect successfully"<<endl;
	return true;
}

void Client::sendMessage(const char* message){
	int len=strlen(message);
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
		//cout<<rn<<endl;
		total+=len;
		start+=len;
		if(start>=size)	break;
		if(start+len>=size)	len=size-start;
	}
}

void Client::listenAndSendFrame(char* flag){
	char message[50];
	strcpy(message, "");
    char* localFlag=flag;

	cv::Mat image;
    /*
     * move to SocketManager
	maxQueueLen=8;
	int queueFrames=0;
	globalFrames.resize(maxQueueLen);
    */
	while(1){
		if(strcmp(message, "frame")==0){
            if(*localFlag==0){
                sendMessage("stop");
            }
            else{
                //dataMutex.lock();
                image=loader_func();
                if(image.empty()){
                    sendMessage("stop");
                }
                else{
                /*
                 * move to SocketManager
#if	RESIZE==1
				cv::resize(image, image, cv::Size(image.cols/2, image.rows/2));
#endif
				image.copyTo(globalFrames[queueFrames]);
				queueFrames=(queueFrames+1)%maxQueueLen;
				dataMutex.unlock();
                */

                    char header[50];
#ifdef COMPRESS
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
#ifndef SIMPLE
                    sendMessage("image");
                    while(1){
                        int rn=recv(clientsd, message, sizeof(message), 0);
                        message[rn]='\0';
                        if(strcmp(message, "image")==0)
                            break;
                    }
#endif
                    sendSingleImageBuff(buffer,size);
                    //cout<<"frame: "<<frameIndex<<endl;
                    //++frameIndex;
                }
			}
		}
		else if(strcmp(message, "stop")==0){
			cout<<"send all frames"<<endl;
			break;
		}

		int rn=recv(clientsd, message, sizeof(message), 0);
		message[rn]='\0';
		usleep(5);
	}
	bConnected=false;
	close(clientsd);
}
