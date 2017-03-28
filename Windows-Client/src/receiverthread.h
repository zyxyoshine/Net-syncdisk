#ifndef RECEIVERTHREAD_H
#define RECEIVERTHREAD_H

#include "common.h"
#include <QThread>

namespace TMY
{
class ReceiverThread : public QThread
{
    Q_OBJECT
public:
    void run();
    void setMode(int mode_, QString path);
    void setController(TMY::Controller_ptr);
    int mode;

signals:
    void writeReady();
    void passDownMsg(QString, float);

public slots:
    void recvDownMsg(QString, float);

private:
    Receiver_ptr receiver;
    QString syncPath;
    TMY::Controller_ptr controller;
    void initSync();
    void endSync();
    void sync();
};
}

#endif // RECEIVERTHREAD_H
