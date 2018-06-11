#include "guithread.h"
#include <QMetaType>

GUIThread::GUIThread(QObject *parent):QThread(parent){

}
GUIThread::GUIThread(QObject *parent, std::vector<std::string>& files):QThread(parent)
{
    this->filenames=files;

    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<cv::Mat>("cv::Mat&");
}

GUIThread::~GUIThread(){

}

void GUIThread::run(){
    assert(filenames[0].length()>0 && filenames[1].length()>0);

    std::string ref_video=filenames[0];
    std::string user_video=filenames[1];

    cv::VideoCapture cap_ref;
    cap_ref.open(ref_video);
    cv::VideoCapture cap_user;
    cap_user.open(user_video);

    //cv::VideoCapture* ref_ptr=&cap_ref;
    //cv::VideoCapture* user_ptr=&cap_user;

    int fps_ref=cap_ref.get(CV_CAP_PROP_FPS);
    int fps_user=cap_user.get(CV_CAP_PROP_FPS);

    int fps=std::min(fps_ref, fps_user);

    int interval=(int)(1.0*1e6/fps);

    assert(cap_ref.isOpened() && cap_user.isOpened());

    while(1){
        cap_ref.read(frame_ref);
        cap_user.read(frame_user);
        //ref_ptr->read(frame_ref);
        //user_ptr->read(frame_user);

        if(!frame_ref.empty()){
            cv::resize(frame_ref, frame_ref, sizes[1], cv::INTER_LINEAR);
            cv::cvtColor(frame_ref, frame_ref, cv::COLOR_BGR2RGB);
        }
        if(!frame_user.empty()){
            cv::resize(frame_user, frame_user, sizes[0], cv::INTER_LINEAR);
            cv::cvtColor(frame_user, frame_user, cv::COLOR_BGR2RGB);
        }
        if(frame_ref.empty() && frame_user.empty()){
            break;
        }
        /****can either use signal/slots or callback****/
        //emit update_ui_signal(frame_user, frame_ref);
        func(frame_user, frame_ref);
        /****can not use waitkey in sub thread****/
        //cv::waitKey(interval);
        usleep(interval);
    }
}
