/*
 * Client.cpp
 *
 *      Author: yufeng
 */

#include "Client.h"
#include <mutex>
#include <memory>

#define RESIZE	1

bool bValidDataArrived;
bool bNewDataArrived;
cv::Mat	globalFrame;
vector<cv::Mat> globalFrames;
std::mutex dataMutex;
int maxQueueLen;

Client::Client(const string& ip, const int p) {
	// TODO Auto-generated constructor stub
	clientsd=socket(AF_INET, SOCK_STREAM, 0);
	if(clientsd==-1){
		cout<<"Socket error"<<endl;
		return;
	}
	ipAddr=ip;
	port=p;
	server_socket.sin_family=AF_INET;
	server_socket.sin_port=htons(port);
	server_socket.sin_addr.s_addr=inet_addr(ipAddr.c_str());
	frameIndex=0;
	numEmptyFrames=0;
	mVideoCap.open("/home/yfji/Workspace/C++/SocketTransport_train/single_person.mp4");
	frames=mVideoCap.get(cv::CAP_PROP_FRAME_COUNT);
}

Client::~Client() {
	// TODO Auto-generated destructor stub
	if(bConnected)
		close(clientsd);
}

bool Client::connectServer(){
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
	char poseInfo[4096];
	strcpy(message, "");
	//cv::namedWindow("client");

	cv::Mat image;
	maxQueueLen=8;
	int queueFrames=0;
	globalFrames.resize(maxQueueLen);
	while(1){
		if(strcmp(message, "frame")==0){

//			if(bValidDataArrived and not bNewDataArrived and numEmptyFrames<10){
//				//cout<<"empty"<<endl;
//				sendMessage("empty");
//				++numEmptyFrames;
//			}
//			if(numEmptyFrames==5)
//				numEmptyFrames=0;
//			else{

#ifdef FINITE_FRAMES
			if(frameIndex==500){
				sendMessage("stop");
			}
			else if(frameIndex==frames){
				sendMessage("stop");
			}
			else{
#endif
				//string image_file=image_dir+image_files[frameIndex];
				//cv::Mat image=cv::imread(image_file,1);
				dataMutex.lock();
				//if(not globalFrames[queueFrames].empty())
				//	globalFrames[queueFrames].copyTo(globalFrame);
				mVideoCap>>image;
#if	RESIZE==1
				cv::resize(image, image, cv::Size(image.cols/2, image.rows/2));
#endif
				image.copyTo(globalFrames[queueFrames]);
				queueFrames=(queueFrames+1)%maxQueueLen;
				dataMutex.unlock();
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
				cout<<"frame: "<<frameIndex<<endl;
				bNewDataArrived=false;
				++frameIndex;
#ifdef FINITE_FRAMES
			}
#endif
		//	}
		}
		else if(strcmp(message, "stop")==0){
			cout<<"send all frames"<<endl;
			break;
		}

		int rn=recv(clientsd, message, sizeof(message), 0);
		message[rn]='\0';
		usleep(5);
		//if(strlen(message)>0)	cout<<message<<endl;
	}
	bConnected=false;
	close(clientsd);
}

/*
void Client::sendImages(){
	if(not bConnected){
		cerr<<"not connected"<<endl;
		return;
	}
	char* images[]={
		"1.jpg","1.jpg","1.jpg","1.jpg","1.jpg","1.jpg","1.jpg"
	};
	char message[50];//="yes";

	int cnt=0;
	const char* header="sz:1024,768,3\0";
	sendMessage(header);
	while(1){
		//strcpy(message, "");
		int rn=recv(clientsd, message, sizeof(message), 0);
		message[rn]='\0';
		if(strcmp(message, "image")==0)
			break;
	}
	while(1){
		sendMessage("image");
		while(1){
			//strcpy(message, "");
			int rn=recv(clientsd, message, sizeof(message), 0);
			message[rn]='\0';
			if(strcmp(message, "image")==0)
				break;
		}
		char dir[100]="/home/jyf/Resources/Images/";
		strcat(dir, images[cnt]);
		printf("Full name: %s\n", dir);
		cv::Mat image=cv::imread(string(dir),1);
		sendSingleImageBuff(image);
		printf("Send image finish\n");
		++cnt;
		while(1){
			//strcpy(message, "");
			int rn=recv(clientsd, message, sizeof(message), 0);
			if(message[0]=='o' && message[1]=='k'){
				message[rn]='\0';
				break;
			}
		}
		if(cnt==7)
			break;
		usleep(10);
	}
	sendMessage("stop");
	while(1){
		//strcpy(message, "");
		int rn=recv(clientsd, message, sizeof(message), 0);
		message[rn]='\0';
		if(strcmp(message, "end")==0)
			break;
	}
	printf("Send all images\n");
	close(clientsd);
}
*/
