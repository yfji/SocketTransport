#include "Server.h"
#include "Client.h"
#include "ServerReadData.h"
#include "estimatorCosine.h"
#include "estimatorEuclidean.h"
#include "estimatorAngleTrunk.h"
#include <thread>
#include <chrono>

static const std::string databaseName="project";
static const std::vector<std::string> tableNames={"yg_user_images","yg_action"};
static std::shared_ptr<DatabaseManager> spDbManager;
static std::shared_ptr<Estimator> spEstimator;
static std::shared_ptr<Redis> spRedis;

void client_thread(Client* pClient){
	if(pClient->connectServer()){
		pClient->listenAndSendFrame();
	}
	// std::cout<<"send frame finished"<<std::endl;
}

void server_read_data(ServerReadData* pServer){
	if(not pServer->startListen()){
		return;
	}
	if(not pServer->startAccept()){
		return;
	}
	// cout<<"listening for pose data"<<endl;
	pServer->receiveData();
	// std::cout<<"Read data finished"<<std::endl;
}

int solve_out(int argc, char** argv){
	if(argc!=4 and argc!=3){
		// std::cout<<"you must provide at least the action_id and act_id, please check!"<<std::endl;
		return 1003;	//err code
	}
	
	const std::string inetAddress="172.17.108.58";
	int port=9010;
	int keypoint_port=9012;
	int uid, action_id, act_id;
	if(argc==3){
		uid=0;
		action_id=std::atoi(argv[1]);
		act_id=std::atoi(argv[2]);
	}
	else{
		uid=std::atoi(argv[1]);
		action_id=std::atoi(argv[2]);
		act_id = std::atoi(argv[3]);
	}
	
	Client client(inetAddress, port, uid, act_id);
	ServerReadData dataReader(inetAddress, keypoint_port, uid, action_id, act_id);
	
	client.setRedisPtr(spRedis);
	dataReader.setDatabaseManager(spDbManager);
	dataReader.setEstimator(spEstimator);
	dataReader.setRedisPtr(spRedis);
	
	std::thread t_client(&client_thread, &client);
	std::thread t_dataReader(&server_read_data, &dataReader);
	
	t_client.join();
	t_dataReader.join();
	return 200;
}

int main(int argc, char** argv){
	spDbManager=std::make_shared<DatabaseManager>();
	//spEstimator=std::make_shared<EstimatorAngleTrunk>();
	spEstimator=std::make_shared<EstimatorCosine>();
	spRedis=std::make_shared<Redis>();
	spDbManager->setDatabaseAndTables(databaseName, tableNames);
	if(not spDbManager->connectDatabase()){
		return 1003;
	}
	if(not spRedis->connect()){
		return 1003;
	}
	return solve_out(argc, argv);
}
