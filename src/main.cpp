#include "Server.h"
#include "Client.h"
#include "ClientReadData.h"
#include "ServerReadData.h"
#include "estimatorCosine.h"
#include "estimatorEuclidean.h"
#include "estimatorAngleTrunk.h"
#include <thread>
#include <chrono>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>

static const std::string databaseName="project";
static const std::vector<std::string> tableNames={"yg_user_images", "yg_action"};
static std::shared_ptr<DatabaseManager> spDbManager;
static std::shared_ptr<Estimator> spEstimator;

void client_thread(Client* pClient){
	if(pClient->connectServer()){
		//cout<<"listening for frame request"<<endl;
		pClient->listenAndSendFrame();
		//cout<<"Send frames finished"<<endl;
	}
}

void server_read_data(ServerReadData* pServer){
	if(not pServer->startListen()){
		return;
	}
	if(not pServer->startAccept()){
		return;
	}
	//cout<<"listening for pose data"<<endl;
	pServer->receiveData();
	//cout<<"Read data finished"<<endl;
}

void client_read_data(ClientReadData* pClient){
	if(not pClient->connectServer()){
		return;
	}
	pClient->receiveData();
	//cout<<"Read data finished"<<endl;
}

int solve_out(int argc, char** argv){
	if(argc!=3){
		return 1003;
	}
	int uid=std::atoi(argv[1]);
	int action_id=std::atoi(argv[2]);
#define ALI	0
#if ALI==1
	int port=8910;
	int keypoint_port=8912;
	const std::string inetAddr="10.106.20.8";
	const std::string serverAddr="47.93.229.233";
#else
	int port=9010;
	int keypoint_port=9012;
	const std::string inetAddr="192.168.3.24";
	const std::string serverAddr="192.168.3.24";
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
	int code=0;
	if(not spDbManager->connectDatabase()){
		//std::cout<<"Mysql connect error"<<std::endl;
		return 1003;
	}	
	
    char* flag;
    int shm_id;
    int sem_val;
    const char* name="USE_SOCK";
    const int memsize=512*sizeof(char);
    void* ptr;
    sem_t * sem;
    bool init=false;

    shm_id=shm_open(name, O_RDWR|O_CREAT|O_EXCL, 0777);
    if(shm_id<0){
        cout<<"Memory file already exists"<<endl;
        shm_id=shm_open(name, O_RDWR, 0777);
    }
    else{
   	   cout<<"New memory file created"<<endl;
   	   ftruncate(shm_id,memsize);
       init=true;
   }
   
   cout<<"Connect shared memory"<<endl;
   ptr=mmap(0, memsize, PROT_READ|PROT_WRITE, MAP_SHARED,shm_id,0);
   flag=(char*)ptr;
   
   sem=sem_open(name, O_CREAT,0644,0);
   if(sem==SEM_FAILED){
       cerr<<"Sem open failed"<<endl;
       return 0;
   }
   if(init){
       sem_init(sem,1,1);
       sem_wait(sem);
	   strcpy(flag,"0");
	   sem_post(sem);
   }   
   auto start=std::chrono::high_resolution_clock::now();
   double time_millisec=0;
   const double wait_time=1000; //5s
   
   cout<<"Waiting for lock..."<<endl;
   while(time_millisec<wait_time){
      auto now=std::chrono::high_resolution_clock::now();
	    time_millisec=(double)std::chrono::duration_cast<std::chrono::milliseconds>(now-start).count();
      sem_wait(sem);
      if(flag[0]=='0'){
      	break;
  	  }
  	  else
  	    sem_post(sem);
  	}
  	if(time_millisec>=wait_time){
    	cout<<"Time over, exit!"<<endl;
    	return 1003;
	}
	else{
		cout<<"Lock in hand!"<<endl;
		strcpy(flag,"1");
		sem_post(sem);
		//task
		//sleep(3);
		code=solve_out(argc, argv);
		
		sem_wait(sem);
		strcpy(flag,"0");
		//sem_getvalue(sem,&sem_val);
		//cout<<"sem value: "<<sem_val<<endl;
		sem_post(sem);
		//sem_getvalue(sem,&sem_val);
		//cout<<"sem value: "<<sem_val<<endl;
		cout<<"Release lock!"<<endl;
	}
	sem_close(sem);
    close(shm_id);
	munmap(ptr, memsize);
	return code;
}
