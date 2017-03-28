#include "mainclient.h"
#include "ui_mainclient.h"
#include <QAbstractItemModel>
#include "QFileDialog"
#include <QDebug>
#include <QHash>
#include <QUrl>
#include "common.h"

#define log(s) TMY::logger.log(s)

MainClient::MainClient(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainClient)
{
    
    ui->setupUi(this);
    connect(&TMY::logger, SIGNAL(newLog(QString)), this, SLOT(updateLog(QString)));
}

void MainClient::initialize(QString syncPath_, TMY::Controller_ptr cp)
{
    syncPath = syncPath_;
    controller = cp;


    if (this->syncPath != "")
    {

        senderThread.start();
        senderThread.setController(controller);
        receiverThread.start();
        receiverThread.setController(controller);

        connect(&receiverThread, SIGNAL(writeReady()), &senderThread, SLOT(setReady()));
        connect(&receiverThread, SIGNAL(passDownMsg(QString, float)), this, SLOT(recvDownMsg(QString, float)));
        connect(&senderThread, SIGNAL(passUpMsg(QString, float)), this, SLOT(recvUpMsg(QString,float)));

        ui->syncPathLabel->setText(syncPath);
        log("Detected sync path :");
        log(syncPath);

        Sleep(1);
        pathProtection();
        recvSync();
    }

}

MainClient::~MainClient()
{
    delete ui;
}

void MainClient::on_pushButton_clicked()
{

    QFileDialog fileselector;
    fileselector.setDirectory("./");
    fileselector.setFileMode(QFileDialog::FileMode::DirectoryOnly);
    fileselector.exec();

    senderThread.start();
    senderThread.setController(controller);
    receiverThread.start();
    receiverThread.setController(controller);

    connect(&receiverThread, SIGNAL(writeReady()), &senderThread, SLOT(setReady()));
    connect(&receiverThread, SIGNAL(passDownMsg(QString, float)), this, SLOT(recvDownMsg(QString, float)));
    connect(&senderThread, SIGNAL(passUpMsg(QString, float)), this, SLOT(recvUpMsg(QString,float)));

    Sleep(1);

    syncPath = fileselector.selectedFiles()[0];
    if (syncPath == "")
        return;
    log("Changing sync path to:");
    log(syncPath);
    ui->syncPathLabel->setText(syncPath);
    
    pathProtection();
    updateConfig();
    initSync();

}

void MainClient::updateTreeModel(TMY::DirInfo dirinfo)
{
    for (TMY::DirInfoEntry &entry : dirinfo)
        addTreeItem(entry);
}

void MainClient::addTreeItem(TMY::DirInfoEntry entry)
{
    TMY::PathArr path = entry.filePath.pathArr;
    for (std::string &s : path)
    {

    }

}

void MainClient::updateLog(QString str)
{
    ui->logBrowser->append(str);

}

void MainClient::initSync()
{
 //   ui->pushButton->setEnabled(false);
  //  ui->pushButton_2->setEnabled(false);

    senderThread.setMode(1, syncPath);
    receiverThread.setMode(1, syncPath);

}

void MainClient::pathProtection()
{
    QString protectionPath = syncPath;
    protectionPath += "/.protection.tmyi";

    FILE *f = fopen(protectionPath.toStdString().c_str(), "wb");

}

void MainClient::recvSync()
{
    senderThread.setMode(3, syncPath);
    receiverThread.setMode(3, syncPath);

}

void MainClient::recvDownMsg(QString msg, float f)
{
    ui->downloadInfoLabel->setText(msg);
    ui->downloadBar->setMaximum(1000);
    ui->downloadBar->setValue((int)(f * 1000));
    ui->downloadBar->update();
}

void MainClient::recvUpMsg(QString msg, float f)
{
    ui->uploadInfoLabel->setText(msg);
    ui->uploadBar->setMaximum(1000);
    ui->uploadBar->setValue((int)(f * 1000));
    ui->uploadBar->update();
}



void MainClient::on_pushButton_2_clicked()
{
    senderThread.mode = -1;
    receiverThread.mode = -1;
    syncPath = "";
    ui->syncPathLabel->setText("NULL");
    senderThread.terminate();
    receiverThread.terminate();
    updateConfig();

}

void MainClient::updateConfig()
{
    QFile file("./tmyuser.txt");
    file.open(QIODevice::ReadOnly);
    QTextStream ts(&file);
    QString ip, ports, session, sp;
    ts >> ip >> ports >> session >> sp;
    sp = syncPath;
    file.close();
    file.open(QIODevice::WriteOnly);
    ts.setDevice(&file);
    ts << ip << "\n" << ports << "\n" << session << "\n" << sp;
}
