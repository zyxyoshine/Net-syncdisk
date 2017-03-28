#ifndef SENDERTHREAD_H
#define SENDERTHREAD_H

#include "common.h"
#include <QThread>
#include <QFileSystemWatcher>
#include "dirscanner.h"

namespace TMY
{
class SenderThread : public QThread
{
    Q_OBJECT
public:
    DirScanner scanner;
    void run();
    void setMode(int mode_, QString path);  // 1 for init , 2 for recv , 3 for done and incr
    void setController(TMY::Controller_ptr);
    int mode;

    TMY::Sender_ptr sender;

public slots:
    void setReady();
    void recvUpMsg(QString, float);

signals:
    void passUpMsg(QString, float);

private:
    bool ready;
    QFileSystemWatcher watcher;
    QString syncPath;
    TMY::Controller_ptr controller;
    void initSync();
    void endSync();
    void sync();
};
}

#endif // SENDERTHREAD_H
