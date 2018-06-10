#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include "socketmanager.h"
#include "guithread.h"

using namespace cv;
using namespace std;

enum conn_state{
    CONNECTED,
    CONNECTING,
    DISCONNECTED
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void actionOpen_triggered();

    void btn_stop_clicked();

    void btn_play_clicked();

    void my_update_ui(cv::Mat& userImg, cv::Mat& refImg);

private:
    Ui::MainWindow *ui;

private:
    std::string dlg_titles[2]={"select reference video", "select user video"};
    char ref_selected={0};
    char user_selected={0};

    const int num_parts={14};   // no nose

    std::vector<std::string> videoNames={"ref_video.mp4","test_video.mp4"};

    cv::VideoCapture cap_user;
    cv::VideoCapture cap_ref;

    cv::Mat frame_user;
    cv::Mat frame_ref;
    cv::Size user_size;
    cv::Size ref_size;

    char buff[4096];
    std::vector<DataRow> standardPoseData;

    conn_state state;
    char flag = {1};

    SocketManager sManager;

    std::shared_ptr<GUIThread> thread_ptr;

private:
    virtual void paintEvent(QPaintEvent *e);
    void connectSlots();

    void playVideo();

    void playNetworkPose();

    void readStandardPoseFile();
    void alignStandardUser(std::vector<DataRow>& poseData);

    void connectServer();

    //callback function
    void drawUserImage(cv::Mat& image, std::vector<DataRow>& poseData);

};

#endif // MAINWINDOW_H
