#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include <assert.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QFont font("Microsoft YaHei", 20, 50);
    ui->txt_speed->setFont(font);
    ui->txt_round->setFont(font);
    ui->txt_calory->setFont(font);

    auto qSizeUser=ui->label_user->size();
    auto qSizeRef=ui->label_ref->size();

    user_size.width=qSizeUser.width();
    user_size.height=qSizeUser.height();
    ref_size.width=qSizeRef.width();
    ref_size.height=qSizeRef.height();

    thread_ptr.reset(new GUIThread(this, videoNames));
    thread_ptr->setSizes({user_size, ref_size});
    calc_ptr.reset(new Calculator({2,3,4,5,6,7}));

    draw_func=std::bind(&MainWindow::drawUserImage,this,std::placeholders::_1,std::placeholders::_2);
    callback func=std::bind(&MainWindow::my_update_ui,this,std::placeholders::_1,std::placeholders::_2);
    thread_ptr->setCallback(func);  //reference of func
    connectSlots();

    readStandardPoseFile();
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
    //alignStandardUser(poseData);
    //calculate(poseData);
    /****change the reference or the content of frame_user will trigger the update event!!!why?***/
    //frame_user=image;
    cv::Mat tmp;
    cap_ref.read(tmp);
#if RESIZE==1
    if(!tmp.empty()){
        cv::resize(tmp, tmp, cv::Size(), 0.5, 0.5, cv::INTER_LINEAR);
    }
#endif
#if LOOP==1
    if(tmp.empty() || new_loop){
        cap_ref.set(CV_CAP_PROP_POS_FRAMES, 0);
        cap_ref.read(tmp);
        trueFrame=0;
        curFrame=0;
        new_loop=1-new_loop;
    }
#endif
    std::vector<DataRow> row=standardPoseData[trueFrame];
    alignStandardUser(row, poseData);
    sManager.drawConnections(image, row, num_parts, "green");
    sManager.drawConnections(image, poseData, num_parts);
    if(!tmp.empty()){
        std::vector<DataRow>& data=standardPoseData[trueFrame];
        sManager.drawConnections(tmp, data, num_parts, "green");
        cv::resize(tmp, tmp, ref_size, cv::INTER_LINEAR);
        cv::cvtColor(tmp, tmp, cv::COLOR_BGR2RGB);
        frame_ref=tmp;
    }
    if(!image.empty()){
        cv::resize(image, image, user_size, cv::INTER_LINEAR);
        cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        frame_user=image;
    }
    if(curFrame==calc_interval){
        calc_ptr->setData(poseData);
        message=calc_ptr->calculate();
        curFrame=0;
    }
    ++curFrame;
    ++trueFrame;
    this->update();
}

void MainWindow::my_update_ui(cv::Mat& userImg, cv::Mat& refImg){
    frame_user=userImg;
    frame_ref=refImg;
    this->update();
}

void MainWindow::playNetworkPose(){
    if(!sManager.connect()){
        std::cout<<"No connection, exit"<<std::endl;
        sManager.disconnect();
        return;
    }
    state=CONNECTED;
    flag=1;

    cap_ref.open(videoNames[0]);
#if CAMERA==0
    cap_user.open(videoNames[1]);
#else
    cap_user.open(0);
    cap_user.set(CV_CAP_PROP_FRAME_WIDTH, frame_width);
    cap_user.set(CV_CAP_PROP_FRAME_HEIGHT, frame_height);
#endif

    //int fps_ref=cap_ref.get(CV_CAP_PROP_FPS);
    //int interval=(int)(1.0*1000/fps);

    assert(cap_ref.isOpened() && cap_user.isOpened());

    sManager.cap_ptr=&cap_user;
    sManager.loop=&new_loop;
    curFrame=0;
    trueFrame=0;
    calc_ptr->reset();

    client_thread=std::thread(&SocketManager::runSendingThread, &sManager, &flag);
    read_thread=std::thread(&SocketManager::runReceivingThread, &sManager, &draw_func);

    client_thread.detach(); //detach is very necessary!!!!!
    read_thread.detach();
}

void MainWindow::readStandardPoseFile(const int frameCount){
    std::ifstream in;
    float x,y;
    for(int i=0;i<frameCount;++i){
        std::stringstream ss;
        std::string line;
        ss<<"refPose/"<<i<<".txt";
        in.open(ss.str().c_str(), std::ios::in);
        std::vector<DataRow> frame_data;
        while(!in.eof()){
            std::getline(in, line);
            if(line.length()<=1)
                continue;
            ss.str("");
            ss<<line;
            ss>>x>>y;
            DataRow row(x,y,0.0);
            frame_data.push_back(row);
        }
        standardPoseData.push_back(frame_data);
        in.close();
    }
}

void MainWindow::alignStandardUser(std::vector<DataRow>& ref, std::vector<DataRow>& poseData){  //align your nose with the standard nose
    int anchorIndex1=8;  //left hip
    int anchorIndex2=11;    //right hip

    int anchor_x_ref=(std::get<0>(ref[anchorIndex1])+std::get<0>(ref[anchorIndex2]))/2;
    int anchor_y_ref=(std::get<1>(ref[anchorIndex1])+std::get<1>(ref[anchorIndex2]))/2;

    //int anchor_x_user=std::get<0>(poseData[anchorIndex]);
    //int anchor_y_user=std::get<1>(poseData[anchorIndex]);
    int anchor_x_user=(std::get<0>(poseData[anchorIndex1])+std::get<0>(poseData[anchorIndex2]))/2;
    int anchor_y_user=(std::get<1>(poseData[anchorIndex1])+std::get<1>(poseData[anchorIndex2]))/2;

    int diff_x=anchor_x_ref-anchor_x_user;
    int diff_y=anchor_y_ref-anchor_y_user;

    for(int i=0;i<num_parts;++i){
        int x=std::get<0>(ref[i]);
        int y=std::get<1>(ref[i]);
        float prob=std::get<2>(poseData[i]);
        if(x>0 && y>0){
            ref[i]=DataRow(x-diff_x,y-diff_y,prob);
        }
    }
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    //std::cout<<"Update"<<std::endl;
    QImage q_frame_user = QImage((const unsigned char*)(frame_user.data), frame_user.cols, frame_user.rows, frame_user.cols*frame_user.channels(), QImage::Format_RGB888);
    QImage q_frame_ref = QImage((const unsigned char*)(frame_ref.data), frame_ref.cols, frame_ref.rows, frame_ref.cols*frame_ref.channels(), QImage::Format_RGB888);

    ui->label_user->setPixmap(QPixmap::fromImage(q_frame_user));
    ui->label_ref->setPixmap(QPixmap::fromImage(q_frame_ref));
    ui->txt_speed->setText(QString::number(std::get<0>(message)));
    ui->txt_round->setText(QString::number(std::get<1>(message)));
    ui->txt_calory->setText(QString::number(std::get<2>(message)));
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
