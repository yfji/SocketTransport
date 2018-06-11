#include "socketmanager.h"

SocketManager::SocketManager()
{
    inetAddress=std::string(INETADDR);
    serverAddress=std::string(OUTADDR);
    image_port=IMAGE_PORT;
    data_port=DATA_PORT;

    client_ptr.reset(new Client(serverAddress, image_port));
    reader_ptr.reset(new ClientReadData(serverAddress, data_port));

    globalFrames.resize(maxQueueLen);
    randomColors();

    bundle=Bundle(&globalFrames,&dataMutex,&maxQueueLen,&recvFrameId);
}

void SocketManager::randomColors(){
    cv::RNG rng;
    for(int i=0;i<30;++i){
        pallete.push_back(cv::Scalar((int)(255*(rng.uniform(0.,1.))),(int)(255*(rng.uniform(0.,1.))),(int)(255*(rng.uniform(0.,1.)))));
    }
    colors.insert(std::pair<std::string, cv::Scalar>("red", cv::Scalar(0,0,255)));
    colors.insert(std::pair<std::string, cv::Scalar>("green", cv::Scalar(0,255,0)));
    colors.insert(std::pair<std::string, cv::Scalar>("blue", cv::Scalar(255,0,0)));
    colors.insert(std::pair<std::string, cv::Scalar>("yellow", cv::Scalar(0,255,255)));
    colors.insert(std::pair<std::string, cv::Scalar>("black", cv::Scalar(0,0,0)));
    colors.insert(std::pair<std::string, cv::Scalar>("white", cv::Scalar(255,255,255)));
}

bool SocketManager::connect(){
    bool ok=(client_ptr->connectServer() && reader_ptr->connectServer());
    return ok;
}

void SocketManager::disconnect(){

}

std::vector<DataRow> SocketManager::getPoseData(const char* pose_ptr){

}

cv::Mat SocketManager::getImage(){
    assert(cap_ptr!=nullptr);
    cv::Mat frame;
    dataMutex.lock();
    cap_ptr->read(frame);
#if	RESIZE==1
    if(!frame.empty()){
        cv::resize(frame, frame, cv::Size(frame.cols/2, frame.rows/2));
    }
#endif
    frame.copyTo(globalFrames[sendFrameId]);
    sendFrameId=(sendFrameId+1)%maxQueueLen;
    dataMutex.unlock();
    return frame;
}

void SocketManager::runSendingThread(char* flag){
    sendFrameId=0;
    loader_callback loader_func=std::bind(&SocketManager::getImage,this);
    client_ptr->setCallback(loader_func);
    client_ptr->listenAndSendFrame(flag);
}

void SocketManager::runReceivingThread(draw_callback* func){
    recvFrameId=0;
    reader_ptr->setCallback(*func);
    reader_ptr->receiveData(&bundle);
}

void SocketManager::drawConnections(cv::Mat& image, std::vector<DataRow>& pose_data, int np, std::string color){
    cv::Scalar scalarA;
    cv::Scalar scalarB;
    char rand=(color=="rand"?1:0);

    assert(np<=pose_data.size());
    for(auto i=0;i<np;i+=2){
        int start=limbSeq[i];
        int end=limbSeq[i+1];
        int xA=std::get<0>(pose_data[start]);
        int yA=std::get<1>(pose_data[start]);
        int xB=std::get<0>(pose_data[end]);
        int yB=std::get<1>(pose_data[end]);

        if(xA>0 && yA>0 && xB>0 && yB>0){
            scalarA=(rand==0?colors[color]:pallete[i]);
            scalarB=(rand==0?colors[color]:pallete[i+1]);
            cv::line(image, cv::Point(xA,yA), cv::Point(xB,yB), scalarA, 4);
            cv::circle(image, cv::Point(xA,yA), 8, scalarA, -1);
            cv::circle(image, cv::Point(xB,yB), 8, scalarB, -1);
        }
    }
}

