#include "Server.h"
#include "Client.h"
#include "ClientReadData.h"
#include "ServerReadData.h"
#include <pthread.h>
#include <semaphore.h>
#include <chrono>

static void* server_thread(void* args){
	Server* pServer=(Server*)args;
	if(not pServer->startAccept()){
		return NULL;
	}
	auto start=std::chrono::high_resolution_clock::now();
	int frameCount=0;
	double fps=0;
	for(int i=0;i<5000;++i){
		cv::Mat frame=pServer->receiveFrame();
		if(frame.empty()){
			break;
		}
		++frameCount;
		auto now=std::chrono::high_resolution_clock::now();
		double duration_ns=(double)std::chrono::duration_cast<std::chrono::nanoseconds>\
				(now-start).count();
		double seconds=duration_ns/1e9;
		fps=frameCount/seconds;
		cout<<"fps: "<<fps<<endl;
		char path[50];
		sprintf(path,"./saved_images/img_%d.jpg",i);
		cv::imwrite(string(path), frame);
	}
	pServer->finishReceive();
}

static void* client_thread(void* args){
	Client* pClient=(Client*)args;
	if(pClient->connectServer()){
		cout<<"listening for frame request"<<endl;
		pClient->listenAndSendFrame();
		cout<<"Send frames finished"<<endl;
	}
}

static void* server_read_data(void* args){
	ServerReadData* pServer=(ServerReadData*)args;
	if(not pServer->startListen()){
		return NULL;
	}
	if(not pServer->startAccept()){
		return NULL;
	}
	cout<<"listening for pose data"<<endl;
	pServer->receiveData();
	cout<<"Read data finished"<<endl;
}

static void* client_read_data(void* args){
	ClientReadData* pClient=(ClientReadData*)args;
	if(not pClient->connectServer()){
		return NULL;
	}
	pClient->receiveData();
	cout<<"Read data finished"<<endl;
}

int solve_out(int argc, char** argv){
#define ALI	1
#if ALI==1
	int port=8910;
	int keypoint_port=8912;
	string inetAddr="10.106.20.8";
	string serverAddr="47.93.229.233";
#else
	int port=8010;
	int keypoint_port=8012;
	string inetAddr="10.106.20.8";
	string serverAddr="10.106.20.8";
#endif
	pthread_t id_client;
	pthread_t id_client_data;
	Client client(serverAddr, port);
	ClientReadData clientReadData(serverAddr, keypoint_port);
	pthread_create(&id_client, NULL, client_thread, &client);
	pthread_create(&id_client_data, NULL, client_read_data, &clientReadData);
	pthread_join(id_client, NULL);
	pthread_join(id_client_data, NULL);
	return 0;
}

int solve_in(int argc, char** argv){
	int port=8010;
	pthread_t id_server;
	pthread_t id_client;
	Server server(port);
	Client client(INETADDR, port);

	if(!server.startListen()){
		cout<<"Error, exit"<<endl;
		return 0;
	}

	pthread_create(&id_server, NULL, server_thread, &server);
	pthread_create(&id_client, NULL, client_thread, &client);
	pthread_join(id_server, NULL);
	usleep(100);
	pthread_join(id_client, NULL);
	cout<<"Finished"<<endl;
	return 0;
}

int main(int argc, char** argv){
#ifdef IN
	return solve_in(argc, argv);
#else
	return solve_out(argc, argv);
#endif
}
