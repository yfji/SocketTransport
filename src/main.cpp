#include "Server.h"
#include "Client.h"
#include <pthread.h>
#include <semaphore.h>
#include <chrono>

#define DISPLAT	0

static void* server_thread(void* args){
	Server* pServer=(Server*)args;
	if(pServer->startListen()){
		if(not pServer->startAccept()){
			return NULL;
		}
	}
	else{
		return NULL;
	}
	auto start=std::chrono::high_resolution_clock::now();
	int frameCount=0;
	double fps=0;
#if DISPLAY==1
	cv::namedWindow("recv");
	cv::startWindowThread();
#endif
	for(int i=0;i<500;++i){
		cv::Mat frame=pServer->receiveFrame();
		if(frame.empty()){
			break;
		}
		++frameCount;
		auto now=std::chrono::high_resolution_clock::now();
		double duration_ns=(double)std::chrono::duration_cast<std::chrono::nanoseconds>(now-start).count();
		double seconds=duration_ns/1e9;
		fps=frameCount/seconds;
		cout<<"fps: "<<fps<<endl;
#if DISPLAY==1
		cv::imshow("recv", frame);
		cv::waitKey(1);
#endif
		//char path[50];
		//sprintf(path,"./saved_images/img_%d.jpg",i);
		//cout<<frame.rows<<","<<frame.cols<<endl;
		//cv::imwrite(string(path), frame);
	}
	cv::destroyAllWindows();
	pServer->finishReceive();
}

static void* client_thread(void* args){
	Client* pClient=(Client*)args;
	if(pClient->connectServer()){
		cout<<"listening"<<endl;
		pClient->listenAndSendFrame();
	}
}

int solve_out(){
	string inetAddr="10.106.20.8";
	string outAddr="47.93.229.233";
	int port=9010;
	pthread_t id_client;
	Client client(outAddr, port);
	pthread_create(&id_client, NULL, client_thread, &client);
	pthread_join(id_client, NULL);
	cout<<"Finished"<<endl;
	return 0;
}

int solve_inet(){
	string inetAddr="10.106.20.8";
	string outAddr="10.106.20.8";
	int port=9010;
	pthread_t id_server;
	Server server(inetAddr, port);
	
	pthread_t id_client;
	Client client(outAddr, port);
	pthread_create(&id_server, NULL, server_thread, &server);
	pthread_create(&id_client, NULL, client_thread, &client);
	pthread_join(id_server, NULL);
	usleep(100);
	pthread_join(id_client, NULL);
	cout<<"Finished"<<endl;
}

int solve_in(){
	string inetAddr="10.106.20.8";
	int port=9010;
	pthread_t id_server;
	Server server(inetAddr, port);
	pthread_create(&id_server, NULL, server_thread, &server);
	pthread_join(id_server, NULL);
	cout<<"Finished"<<endl;
	return 0;
}

int main(int argc, char** argv){
	solve_in();
	return 0;
}
