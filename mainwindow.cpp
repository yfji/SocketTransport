#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include <assert.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    auto qSizeUser=ui->label_user->size();
    auto qSizeRef=ui->label_ref->size();

    user_size.width=qSizeUser.width();
    user_size.height=qSizeUser.height();
    ref_size.width=qSizeRef.width();
    ref_size.height=qSizeRef.height();

    thread_ptr.reset(new GUIThread(this, videoNames));
    thread_ptr->setSizes({user_size, ref_size});

    callback func=std::bind(&MainWindow::my_update_ui,this,std::placeholders::_1,std::placeholders::_2);
    thread_ptr->setCallback(func);  //reference of func
    connectSlots();

    draw_func=std::bind(&MainWindow::drawUserImage,this,std::placeholders::_1,std::placeholders::_2);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectSlots(){
    connect(ui->actionOpen, SIGNAL(triggered()),this,SLOT(actionOpen_triggered()));
    connect(ui->btn_play, SIGNAL(clicked()), this, SLOT(btn_play_clicked()),Qt::UniqueConnection);
    connect(ui->btn_stop, SIGNAL(clicked()), this, SLOT(btn_stop_clicked()),Qt::UniqueConnection);
    connect(thread_ptr.get(), SIGNAL(update_ui_signal(cv::Mat&, cv::Mat&)), this, SLOT(my_update_ui(cv::Mat&, cv::Mat&)),Qt::UniqueConnection);
}

void MainWindow::actionOpen_triggered()
{
    QFileDialog *fileDialog = new QFileDialog(this);
    std::string title="";
    if(!ref_selected){
        title=dlg_titles[0];
        ref_selected=1;
    }
    else if(!user_selected){
        title=dlg_titles[1];
        user_selected=1;
    }
    else{
        title=dlg_titles[0];
    }

    //定义文件对话框标题
    fileDialog->setWindowTitle(tr(title.c_str()));
    //设置默认文件路径
    fileDialog->setDirectory(".");
    //设置文件过滤器
    fileDialog->setNameFilter(tr("Videos(*.avi *.mp4 *.flv)"));
    //设置可以选择多个文件,默认为只能选择一个文件QFileDialog::ExistingFiles
    //fileDialog->setFileMode(QFileDialog::ExistingFiles);
    //设置视图模式
    fileDialog->setViewMode(QFileDialog::Detail);
    //打印所有选择的文件的路径
    QStringList fileNames;
    if(fileDialog->exec())
    {
        fileNames = fileDialog->selectedFiles();
    }
    assert(fileNames.size()==1);

    if(ref_selected)
        videoNames[0]=std::string(fileNames.at(0).toLatin1().data());
    else if(user_selected)
        videoNames[1]=std::string(fileNames.at(1).toLatin1().data());
    else
        videoNames[0]=std::string(fileNames.at(0).toLatin1().data());
    //for(auto tmp:fileNames)
    //        qDebug()<<tmp<<endl;
}

void MainWindow::playVideo(){
    thread_ptr->start();
}

void MainWindow::drawUserImage(cv::Mat& image, std::vector<DataRow>& poseData){
    //do if standard pose provided
    readStandardPoseFile();
    //alignStandardUser(poseData);
    sManager.drawConnections(image, poseData, num_parts);
    frame_user=image;
    cap_ref.read(frame_ref);

    if(!frame_ref.empty()){
        sManager.drawConnections(frame_ref, poseData, num_parts, "green");
        cv::resize(frame_ref, frame_ref, ref_size, cv::INTER_LINEAR);
        cv::cvtColor(frame_ref, frame_ref, cv::COLOR_BGR2RGB);
    }
    if(!frame_user.empty()){
        cv::resize(frame_user, frame_user, user_size, cv::INTER_LINEAR);
        cv::cvtColor(frame_user, frame_user, cv::COLOR_BGR2RGB);
    }
    this->update();
}

void MainWindow::my_update_ui(cv::Mat& userImg, cv::Mat& refImg){
    frame_user=userImg;
    frame_ref=refImg;
    this->update();
}

void MainWindow::playNetworkPose(){
    if(!sManager.connect()){
        //qDebug()<<"No connection, exit";
        ui->btn_play->setEnabled(true);
        return;
    }
    state=CONNECTED;

    cap_ref.open(videoNames[0]);
    cap_user.open(videoNames[1]);

    //int fps_ref=cap_ref.get(CV_CAP_PROP_FPS);
    //int interval=(int)(1.0*1000/fps);

    assert(cap_ref.isOpened() && cap_user.isOpened());

    sManager.cap_ptr=&cap_user;

    client_thread=std::thread(&SocketManager::runSendingThread, &sManager, &flag);
    read_thread=std::thread(&SocketManager::runReceivingThread, &sManager, &draw_func);
}

void MainWindow::readStandardPoseFile(){
    //stardPoseData=...
}

void MainWindow::alignStandardUser(std::vector<DataRow>& poseData){  //align your nose with the standard nose
    int anchorIndex=0;  //nose
    DataRow& nose_info=standardPoseData[anchorIndex];
    int anchor_x_ref=std::get<0>(nose_info);
    int anchor_y_ref=std::get<1>(nose_info);

    int anchor_x_user=std::get<0>(poseData[anchorIndex]);
    int anchor_y_user=std::get<1>(poseData[anchorIndex]);

    int diff_x=anchor_x_user-anchor_x_ref;
    int diff_y=anchor_y_user-anchor_y_ref;

    for(int i=0;i<num_parts;++i){
        int x=std::get<0>(poseData[i]);
        int y=std::get<0>(poseData[i]);
        float prob=std::get<2>(poseData[i]);
        poseData[i]=DataRow(x-diff_x,y-diff_y,prob);
    }
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    QImage q_frame_user = QImage((const unsigned char*)(frame_user.data), frame_user.cols, frame_user.rows, frame_user.cols*frame_user.channels(), QImage::Format_RGB888);
    QImage q_frame_ref = QImage((const unsigned char*)(frame_ref.data), frame_ref.cols, frame_ref.rows, frame_ref.cols*frame_ref.channels(), QImage::Format_RGB888);

    ui->label_user->setPixmap(QPixmap::fromImage(q_frame_user));
    ui->label_ref->setPixmap(QPixmap::fromImage(q_frame_ref));
    ui->label_user->show();
    ui->label_ref->show();
}

void MainWindow::btn_stop_clicked()
{
    state=DISCONNECTED;
    flag=0;
}

void MainWindow::btn_play_clicked()
{
    std::cout<<"Play video"<<std::endl;
    //playVideo();
    playNetworkPose();
    //cannot update UI in the sub thread
}
