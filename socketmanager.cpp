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

cv::Mat SocketManager::getImage(){
    assert(cap_ptr!=nullptr);
    cv::Mat frame;
    cap_ptr->read(frame);
#if LOOP==1
    if(frame.empty()){
        std::cout<<"Black image"<<std::endl;
        reader_ptr->roundFinish=1;  //prepared for black images
        return cv::Mat::zeros(480, 640, CV_8UC3);
    }
#endif
#if	RESIZE==1
    if(!frame.empty()){
        cv::resize(frame, frame, cv::Size(), 0.5,0.5, cv::INTER_LINEAR);
    }
#endif
/*
#if CAMERA==0
    if(frame.empty()){
        cap_ptr->set(CV_CAP_PROP_POS_FRAMES, 0);
        cap_ptr->read(frame);
    }
#endif
#if LOOP==1
    if(frame.empty() || *loop){
        dataMutex.lock();
        std::vector<cv::Mat>().swap(globalFrames);
        globalFrames.resize(maxQueueLen);
        dataMutex.unlock();
        sendFrameId=0;
        recvFrameId=0;
        *loop=1-*loop;
    }
#endif
*/
    dataMutex.lock();
    frame.copyTo(globalFrames[sendFrameId]);
    sendFrameId=(sendFrameId+1)%maxQueueLen;
    dataMutex.unlock();
    return frame;
}

void SocketManager::resetNewLoop(){
    dataMutex.lock();
    reader_ptr->roundFinish=0;  //new loop, do not drawUserImage until valid image comes
    std::vector<cv::Mat>().swap(globalFrames);
    globalFrames.clear();   //clear is very necessary
    globalFrames.resize(maxQueueLen);
    sendFrameId=0;
    recvFrameId=0;
    //reader_ptr->roundFinish=0;
    dataMutex.unlock();
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

void SocketManager::drawConnections(cv::Mat& image, std::vector<DataRow>& pose_data, int num_limb, std::string color){
    cv::Scalar scalarA;
    cv::Scalar scalarB;
    char rand=(color=="rand"?1:0);

    assert(num_limb<=pose_data.size());
    for(auto i=0;i<num_limb;++i){
        int ind=2*i;
        int start=limbSeq[ind];
        int end=limbSeq[ind+1];
        int xA=std::get<0>(pose_data[start]);
        int yA=std::get<1>(pose_data[start]);
        int xB=std::get<0>(pose_data[end]);
        int yB=std::get<1>(pose_data[end]);

        if(xA>0 && yA>0 && xB>0 && yB>0){
            scalarA=(rand==0?colors[color]:pallete[ind]);
            scalarB=(rand==0?colors[color]:pallete[ind+1]);
            cv::line(image, cv::Point(xA,yA), cv::Point(xB,yB), scalarA, 2);
            cv::circle(image, cv::Point(xA,yA), 5, scalarA, -1);
            cv::circle(image, cv::Point(xB,yB), 5, scalarB, -1);
        }
    }
}

