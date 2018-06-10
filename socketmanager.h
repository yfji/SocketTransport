#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H
#include "client.h"
#include "client_read_data.h"
#include <memory>
#include <vector>
#include <tuple>
#include <map>
#include <mutex>
#include <functional>
#include "config.h"

//using draw_callback = void(*)(cv::Mat&, std::vector<DataRow>&);
class SocketManager
{
public:
    SocketManager();

private:
    std::string inetAddress;
    std::string serverAddress;
    int image_port;
    int data_port;

    std::shared_ptr<Client> client_ptr;
    std::shared_ptr<ClientReadData> reader_ptr;

    std::vector<cv::Mat> globalFrames;
    std::mutex dataMutex;
    int maxQueueLen = {8};
    int sendFrameId = {0};
    int recvFrameId = {0};

    Bundle bundle;

    std::vector<cv::Scalar> pallete;
    std::map<std::string, cv::Scalar> colors;

    void randomColors();

    std::vector<int> limbSeq={
        0,1,1,2,1,8,1,5,1,11,2,3,3,4,5,6,6,7,8,9,9,10,11,12,12,13
    };

public:
    std::string filename;
    cv::VideoCapture * cap_ptr = {nullptr};

    bool connect();
    void disconnect();

    std::vector<DataRow> getPoseData(const char* pose_ptr);

    cv::Mat getImage();

    void runSendingThread(char* flag);

    void runReceivingThread(draw_callback* func);

    void drawConnections(cv::Mat& image, std::vector<DataRow>& pose_data, int np=18, std::string color="rand");
};

/****
 * 0: nose
 * 1: neck
 * 2: right shoulder
 * 3: right elbow
 * 4: right wrist
 * 5: left shoulder
 * 6: left elbow
 * 7: left wrist
 * 8: right hip
 * 9: right knee
 * 10:right ankle
 * 11:left hip
 * 12:left knee
 * 13:left ankle
 * 14:right eye
 * 15:left eye
 * 16:right ear
 * 17:left ear
 * 18:right foottop
 * 19:left foottop
 *
 *
****/
#endif // SOCKETMANAGER_H
