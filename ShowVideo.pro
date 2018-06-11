#-------------------------------------------------
#
# Project created by QtCreator 2018-06-06T23:45:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ShowVideo
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += /usr/local/include

LIBS += /usr/local/lib/libopencv_core.so \
        /usr/local/lib/libopencv_highgui.so \
        /usr/local/lib/libopencv_imgproc.so \
        /usr/local/lib/libopencv_imgcodecs.so \
        /usr/local/lib/libopencv_videoio.so

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    socketmanager.cpp \
    client.cpp \
    client_read_data.cpp \
    estimator.cpp \
    guithread.cpp \
    calculator.cpp

HEADERS += \
        mainwindow.h \
    socketmanager.h \
    client_read_data.h \
    client.h \
    config.h \
    estimator.h \
    guithread.h \
    calculator.h

FORMS += \
        mainwindow.ui
