#ifndef MAINCLIENT_H
#define MAINCLIENT_H

#include <QDialog>
#include <QUrl>
#include <QTextStream>
#include <queue>
#include <dirscanner.h>
#include <QFileSystemWatcher>
#include "clientmainthread.h"
#include "senderthread.h"
#include "receiverthread.h"
#include "common.h"

namespace Ui {
class MainClient;
}

class MainClient : public QDialog
{
    Q_OBJECT

public:
    explicit MainClient(QWidget *parent = 0);
    void initialize(QString syncPath, TMY::Controller_ptr cp);
    ~MainClient();

public slots:
    void on_pushButton_clicked();
    void updateTreeModel(TMY::DirInfo dirinfo);
    void updateLog(QString str);
    void recvUpMsg(QString, float);
    void recvDownMsg(QString, float);

signals:
    void startInitSync();
    void startRecvSync();
    void startIncrSync();

private slots:
    void on_pushButton_2_clicked();

private:
    TMY::SenderThread senderThread;
    TMY::ReceiverThread receiverThread;
    TMY::Controller_ptr controller;
    Ui::MainClient *ui;
    QString syncPath;
    void addTreeItem(TMY::DirInfoEntry entry);
    void initSync();
    void recvSync();
    void pathProtection();
    void updateConfig();
};

#endif // MAINCLIENT_H
