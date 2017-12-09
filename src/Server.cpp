/*
 * Server.cpp
 *
 *  Created on: 2017年4月12日
 *      Author: yufeng
 */

#include "Server.h"

Server::Server(int p) {
	// TODO Auto-generated constructor stub
	listensd=socket(AF_INET, SOCK_STREAM, 0);
	if(listensd==-1){
		cout<<"Socket error"<<endl;
		return;
	}
	totalSize=-1;
	fullImageBuff=NULL;
	port=p;
	server_socket.sin_family=AF_INET;
	server_socket.sin_port=htons(port);
	server_socket.sin_addr.s_addr=inet_addr(INETADDR);
	cout<<"INADDR: "<<INADDR_ANY<<", Sin_addr: "<<htonl(INADDR_ANY)<<endl;
	hasImage=false;
	strcpy(tempFileName, "./temp.jpg");
}

Server::~Server() {
	// TODO Auto-generated destructor stub
	close(listensd);
}

bool Server::startListen(){
	//int on = 1;
	//int ret = setsockopt(listensd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
	if(bind(listensd, (struct sockaddr*)(&server_socket), sizeof(server_socket))==-1){
		cout<<"Bind socket error"<<endl;
		return false;
	}
	if(listen(listensd, queue)==-1){
		cout<<"Listen port error"<<endl;
		return false;
	}
	cout<<"bind successfully"<<endl;
	return true;
}

bool Server::startAccept(){
	socklen_t length=sizeof(struct sockaddr_in);
	cout<<"accecpting"<<endl;
	connState=accept(listensd, (struct sockaddr*)(&client_socket), &length);
	if(connState<0){
		cout<<"Accept error"<<endl;
		bConnected=false;
		return false;
	}
	cout<<"connect successfully"<<endl;
	bConnected=true;
	return true;
}

void Server::sendMessage(const char* message){
	int len=strlen(message);
	send(connState, message, len, 0);
}

cv::Mat Server::receiveFrame(){
	if(not bConnected){
		cout<<"no connection, exit"<<endl;
		return cv::Mat();
	}
	sendMessage("frame");
	char message[50];
	strcpy(message, "");
	while(1){
		int rn=recv(connState, message, sizeof(message),0);
		message[rn]='\0';
		if(message[0]=='s' and message[1]=='z'){
			char info[3][10];
			int i=3,k=0,offset=3;
			while(message[i]!='\0'){
				if(message[i]==','){
					offset=i+1;
					++i;
					++k;
					continue;
				}
				info[k][i-offset]=message[i];
				++i;
			}
			width=atoi(info[0]);
			height=atoi(info[1]);
			channels=atoi(info[2]);
			int t_size=width*height*channels;
			if(totalSize!=t_size){
				cout<<"allocate"<<endl;
				totalSize=t_size;
				if (fullImageBuff!=NULL)
					delete fullImageBuff;
				fullImageBuff=new unsigned char[totalSize];
			}
			cout<<width<<","<<height<<","<<channels<<endl;
			hasImage=true;
			sendMessage("sz");
			break;
		}
		else if(strcmp(message, "stop")==0){
			sendMessage("stop");
			bConnected=false;
			return cv::Mat();
		}
		usleep(10);
	}

	int index=0;
	bool frameReceived=false;
	cv::Mat dataFrame;
	strcpy(message, "");
	while(!frameReceived){
#ifndef SIMPLE
		if(strcmp(message,"image")==0){
			sendMessage("image");
#endif
			int cnt=0;
			int total=0;
			index=0;
			while(1){
				int rn=recv(connState, buff, sizeof(buff), 0);
				if(rn<=MAX_LEN){
					for(int k=index;k<rn+index;++k)
						fullImageBuff[k]=buff[k-index];
					index+=rn;
				}
				total+=rn;
				if(total==totalSize){
					frameReceived=true;
					break;
				}
				++cnt;
			}
			dataFrame=cv::Mat(height, width, CV_8UC3, fullImageBuff);
			//printf("receive: %d\n", total);
			//send some feedback message
#ifndef SIMPLE
		}
#endif
		/*
		else if(strcmp(message,"stop")==0){
			sendMessage("stop");
			bConnected=false;
			dataFrame=cv::Mat();
			break;
		}
		*/
		if(frameReceived){
			break;
		}
		int rn=recv(connState, message, sizeof(message), 0);
		message[rn]='\0';
		usleep(10);
	}
	return dataFrame;
}

void Server::finishReceive(){
	if(not bConnected){
		cout<<"no connection, exit"<<endl;
		return;
	}
	sendMessage("stop");
	bConnected=false;
	printf("Receive all images\n");
}

void Server::receive(){
	int cnt=0;
	printf("Accepting\n");
	socklen_t length;
	connState=accept(listensd, (struct sockaddr*)(&client_socket), &length);
	if(connState<0){
		cout<<"Accept error"<<endl;
		return;
	}
	printf("Connect succeed\n");
	while(1){
		int rn=recv(connState, buff, sizeof(buff), 0);
		buff[rn]='\0';
		if(buff[0]=='s' && buff[1]=='z'){
			char info[3][10];
			int i=3, k=0, offset=3;
			while(buff[i]!='\0'){
				if(buff[i]==','){
					offset=i+1;++i;++k;
					continue;
				}
				info[k][i-offset]=buff[i];
				++i;
			}
			width=atoi(info[0]);height=atoi(info[1]);channels=atoi(info[2]);
			printf("%d,%d,%d\n", width, height, channels);
			totalSize=width*height*channels;
			fullImageBuff=new unsigned char[totalSize];
			hasImage=true;
			sendMessage("image");
			break;
		}
	}
	char message[50];
	strcpy(message, "");
	int index=0;
	while(1){
		if(strcmp(message, "image")==0){
			sendMessage("image");
			//FILE* fp=fopen(tempFileName, "wb");
			cnt=0;
			int total=0;
			index=0;
			while(1){
				int rn=recv(connState, buff, sizeof(buff), 0);
				if(rn<=MAX_LEN){
					for(int k=index;k<rn+index;++k)
						fullImageBuff[k]=buff[k-index];
					index+=rn;
				}
				total+=rn;
				if(total==totalSize){
					break;
				}
				//strcat(fullImageBuff, buff);
				//fwrite(buff, sizeof(char), rn, fp);
				++cnt;
			}
			printf("receive: %d\n", total);
			//fclose(fp);
			usleep(10);
			//imageClass.imageProcess(tempFileName);
			//imageClass.save(tempFileName, width, height, channels, fullImageBuff);
			string feedback=string("okthis is my ar");//+imageClass.imageInfoFeedback();
			printf("Process image finish\n");
			sendMessage((char*)feedback.c_str());
		}
		else if(strcmp(message, "stop")==0){
			sendMessage("end");
			break;
		}
		strcpy(message, "");
		printf("reading...\n");
		int rn=recv(connState, message, sizeof(message), 0);
		message[rn]='\0';
		//if(rn>0 && rn<6)
		printf("read len: %d\n", rn);
		printf("message: %s\n", message);
	}
	if(hasImage)
		delete fullImageBuff;
	close(connState);
	close(listensd);
	printf("Receive all images\n");
}
