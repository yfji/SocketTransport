#ifndef GUITHREAD_H
#define GUITHREAD_H
#include <QtGui>
#include <QtCore>
#include <functional>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

using callback = std::function<void(cv::Mat&, cv::Mat&)>;

class GUIThread: public QThread
{
    Q_OBJECT

public:
    explicit GUIThread(QObject *parent = 0);
    GUIThread(QObject *parent, std::vector<std::string>& files);
    ~GUIThread();

    void setSizes(const std::vector<cv::Size>& sizes){
        this->sizes=sizes;
    }
    void setCallback(callback & f){
        func=f;
    }

signals:
    void update_ui_signal(cv::Mat& userImg, cv::Mat& refImg);

private:
    std::vector<std::string> filenames;
    std::vector<cv::Size> sizes;
    cv::Mat frame_user;
    cv::Mat frame_ref;

    callback func;
protected:
    void run();
};

#endif // GUITHREAD_H
