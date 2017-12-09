/*
 * Client.cpp
 *
 *  Created on: 2017年4月12日
 *      Author: yufeng
 */

#include "Client.h"

Client::Client(string ip, int p) {
	// TODO Auto-generated constructor stub
	clientsd=socket(AF_INET, SOCK_STREAM, 0);
	if(clientsd==-1){
		cout<<"Socket error"<<endl;
		exit(0);
	}
	port=p;
	serverAddr=ip;
	frameIndex=0;
	server_socket.sin_family=AF_INET;
	server_socket.sin_port=htons(port);
	server_socket.sin_addr.s_addr=inet_addr(serverAddr.c_str());
	mVideoCap.open("/home/yfji/Workspace/C++/SocketTransport/single_person.mp4");
}

Client::~Client() {
	// TODO Auto-generated destructor stub
	if(bConnected){
		close(clientsd);
	}
}

bool Client::connectServer(){
	if(connect(clientsd, (struct sockaddr*)(&server_socket), sizeof(server_socket))<0){
		printf("Connect error\n");
		bConnected=false;
		return false;
	}
	bConnected=true;
	cout<<"Server connect successfully"<<endl;
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
	int start=0, len=min(MAX_LEN, size);
	int total=0;
	while(1){
		//for(int i=0;i<len;++i)
		//	buff[i]=buffer[i+start];
		int rn=send(clientsd, buffer+start, len, 0);
		total+=len;
		//std::cout<<total<<std::endl;
		start+=len;
		if(start>=size)	break;
		if(start+len>=size)	len=size-start;
	}
}

void Client::listenAndSendFrame(){
	char message[50];
	strcpy(message, "");
	
	cv::Mat frame;
	while(1){
		if(strcmp(message, "frame")==0){
			if(frameIndex==1000){
				sendMessage("stop");
			}
			else{
				mVideoCap>>frame;
				char header[50];
				vector<uchar> mat_data;
				cv::imencode(".jpg",frame,mat_data);
				uchar* buffer=(uchar*)mat_data.data();
				int size=mat_data.size();
				sprintf(header,"sz:%d",size);
				sendMessage(header);
				while(1){
					int rn=recv(clientsd, message, sizeof(message), 0);
					message[rn]='\0';
					if(strcmp(message, "sz")==0)
						break;
				}
				sendSingleImageBuff(buffer, size);
				//cout<<"frame: "<<frameIndex<<endl;
				++frameIndex;
			}
		}
		else if(strcmp(message, "stop")==0){
			cout<<"send all frames"<<endl;
			break;
		}
		usleep(5);
		int rn=recv(clientsd, message, sizeof(message), 0);
		//std::cout<<"Unblocked"<<std::endl;
		message[rn]='\0';
	}
	close(clientsd);
}

void Client::sendImages(){
/*
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
*/
}
