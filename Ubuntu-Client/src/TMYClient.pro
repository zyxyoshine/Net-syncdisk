#-------------------------------------------------
#
# Project created by QtCreator 2016-12-24T23:24:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TMYClient
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        loginwindow.cpp \
    clientmainthread.cpp \
    controller.cpp \
    dirscanner.cpp \
    listener.cpp \
    localfilemanager.cpp \
    logger.cpp \
    mainclient.cpp \
    packets.cpp \
    qlogger.cpp \
    receiver.cpp \
    sender.cpp \
    signupdialog.cpp \
    treeviewmodel.cpp \
    tsock.cpp \
    utility.cpp \
    senderthread.cpp \
    receiverthread.cpp

HEADERS  += loginwindow.h \
    clientmainthread.h \
    common.h \
    controller.h \
    dirscanner.h \
    err.h \
    json.hpp \
    listener.h \
    localfilemanager.h \
    logger.h \
    mainclient.h \
    net.h \
    packets.h \
    qlogger.h \
    receiver.h \
    sender.h \
    senderthread.h \
    signupdialog.h \
    tmy.h \
    treeviewmodel.h \
    tsock.h \
    utility.h \
    receiverthread.h

FORMS    += loginwindow.ui \
    mainclient.ui \
    signupdialog.ui

DISTFILES += \
    ../../test.txt
