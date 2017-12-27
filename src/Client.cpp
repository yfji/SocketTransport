/*
 * Client.cpp
 *
 *      Author: yufeng
 */

#include "Client.h"

Client::Client(const string& _ip, const int _port) {
	// TODO Auto-generated constructor stub
	serverAddress=_ip;
	port=_port;
	server_socket.sin_family=AF_INET;
	server_socket.sin_port=htons(port);
	server_socket.sin_addr.s_addr=inet_addr(serverAddress.c_str());
	frameIndex=0;
	numEmptyFrames=0;
	bConnected=false;
}

Client::~Client() {
	// TODO Auto-generated destructor stub
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
	while(1){
		memset(message, '\0', sizeof(message));
		int rn=recv(clientsd, message, sizeof(message), 0);
		message[rn]='\0';
		// if(strlen(message)>0)	cout<<message<<endl;
		if(strcmp(message, "frame")==0){
#ifdef FINITE_FRAMES
			if(frameIndex==1){
				/***send only one image***/
				sendMessage("stop");
			}

			else{
#endif
				char header[50];
                                vector<uchar> mat_data;
				// image=spDbManager->getImageFromDatabase(uid, act_id);
				// std::cout<<"Reading image base64 code"<<std::endl;
				std::string tempImgBase64 = spRedis->get(spRedis->keyImage);
                                int pos = tempImgBase64.find(",");
                                string getImgBase64 = tempImgBase64.substr(pos+1);
				// std::cout<<"Base64 code length: "<<getImgBase64.length()<<std::endl;
    			        std::string deImgBase64 = base64.Decode(getImgBase64.c_str(), getImgBase64.size());
    		                vector<uchar> data(deImgBase64.begin(), deImgBase64.end());
				image = cv::imdecode(data,1);
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
				
#ifdef FINITE_FRAMES
			}
#endif
		}
		else if(strcmp(message, "stop")==0){
			// cout<<"send all frames"<<endl;
			break;
		}
		usleep(5);
	}
	bConnected=false;
	close(clientsd);
}

bool Client::queryDatabase(){
	
}
