#include "Server.h"
#include "Client.h"
#include "ClientReadData.h"
#include "ServerReadData.h"
#include "estimatorCosine.h"
#include "estimatorEuclidean.h"
#include "estimatorAngleTrunk.h"
#include <thread>
#include <chrono>

static const std::string databaseName="project";
static const std::vector<std::string> tableNames={"yg_user_images", "yg_action"};
static std::shared_ptr<DatabaseManager> spDbManager;
static std::shared_ptr<Estimator> spEstimator;

void client_thread(Client* pClient){
	if(pClient->connectServer()){
		cout<<"listening for frame request"<<endl;
		pClient->listenAndSendFrame();
		cout<<"Send frames finished"<<endl;
	}
}

void server_read_data(ServerReadData* pServer){
	if(not pServer->startListen()){
		return;
	}
	if(not pServer->startAccept()){
		return;
	}
	cout<<"listening for pose data"<<endl;
	pServer->receiveData();
	cout<<"Read data finished"<<endl;
}

void client_read_data(ClientReadData* pClient){
	if(not pClient->connectServer()){
		return;
	}
	pClient->receiveData();
	cout<<"Read data finished"<<endl;
}

int solve_out(int argc, char** argv){
	if(argc!=3){
		return 1003;
	}
	int uid=std::atoi(argv[1]);
	int action_id=std::atoi(argv[2]);
#define ALI	0
#if ALI==1
	int port=8976;
	int keypoint_port=8977;
	const std::string inetAddr="10.106.20.8";
	const std::string serverAddr="47.93.229.233";
#else
	int port=8910;
	int keypoint_port=8912;
	const std::string inetAddr="172.17.108.58";
	const std::string serverAddr="172.17.108.58";
#endif
	Client client(serverAddr, port, uid, action_id);
	ClientReadData dataReader(serverAddr, keypoint_port,uid, action_id);
	client.setDatabaseManager(spDbManager);
	dataReader.setDatabaseManager(spDbManager);
	dataReader.setEstimator(spEstimator);
	std::thread t_client(&client_thread, &client);
	std::thread t_dataReader(&client_read_data, &dataReader);
	
	t_client.join();
	t_dataReader.join();
	return 200;
}

int main(int argc, char** argv){
	spDbManager=std::make_shared<DatabaseManager>();
	spEstimator=std::make_shared<EstimatorCosine>();
	spDbManager->setDatabaseAndTables(databaseName, tableNames);
	if(not spDbManager->connectDatabase()){
		std::cout<<"Mysql connect error"<<std::endl;
		return 1003;
	}
	return solve_out(argc, argv);
}
