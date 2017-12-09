#include "Server.h"
#include "Client.h"
#include "ServerReadData.h"
#include "estimatorCosine.h"
#include "estimatorEuclidean.h"
#include "estimatorAngleTrunk.h"
#include <pthread.h>
#include <semaphore.h>
#include <chrono>

static const std::string databaseName="project";
static const std::vector<std::string> tableNames={"yg_user_images","yg_action"};
static std::shared_ptr<DatabaseManager> spDbManager;
static std::shared_ptr<Estimator> spEstimator;

static void* client_thread(void* args){
	Client* pClient=(Client*)args;
	pClient->queryDatabase();
	if(pClient->connectServer()){
		cout<<"listening for frame request"<<endl;
		pClient->listenAndSendFrame();
		cout<<"Send frames finished"<<endl;
	}
	else{
		return NULL;
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

int solve_out(int argc, char** argv){
	if(argc!=3){
		std::cout<<"you must provide the uid and act_id, please check!"<<std::endl;
		return -1;	//err code
	}
	int port=8010;
	int keypoint_port=8012;
	int uid=std::atoi(argv[1]);
	int act_id=std::atoi(argv[2]);
	pthread_t id_client;
	pthread_t id_server_data;
	Client client(OUTADDR, port, uid, act_id);
	ServerReadData dataReader(INETADDR, keypoint_port, uid, act_id);
	client.setDatabaseManager(spDbManager);
	dataReader.setDatabaseManager(spDbManager);
	dataReader.setEstimator(spEstimator);
	pthread_create(&id_client, NULL, client_thread, &client);
	pthread_create(&id_server_data, NULL, server_read_data, &dataReader);
	pthread_join(id_client, NULL);
	pthread_join(id_server_data, NULL);
	return 0;	//success code
}

int main(int argc, char** argv){
	spDbManager=std::make_shared<DatabaseManager>();
	//spEstimator=std::make_shared<EstimatorAngleTrunk>();
	spEstimator=std::make_shared<EstimatorCosine>();
	spDbManager->setDatabaseAndTables(databaseName, tableNames);
	if(not spDbManager->connectDatabase()){
		return 0;
	}
	return solve_out(argc, argv);
}
