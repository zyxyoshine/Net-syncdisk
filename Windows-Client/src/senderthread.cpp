#include "senderthread.h"
#include "localfilemanager.h"
#include <set>
using namespace TMY;

#define log(s) logger.log(s)

void SenderThread::run()
{
    mode = 0;
    while (1)
    {
        if (mode ==1 || mode == 3)
            initSync();
    }
}

void SenderThread::setMode(int mode_, QString path)
{
    mode = mode_;
    syncPath = path;
}

void SenderThread::setController(TMY::Controller_ptr cp)
{
    controller = cp;
}

void SenderThread::initSync()
{
   // while(1);

    log("Sender: Start InitSync.");
    sync();

    log("Sender: InitSync over.");
    mode = 3;
    endSync();
}

void SenderThread::setReady()
{
    log("Sender: Received Write Ready!");
    ready = true;
}

void SenderThread::endSync()
{
    while(mode == 3)
    {
        sleep(5);
    log("Sender: Start IncrSync.");
        sync();
    log("Sender: End IncrSync.");
    }
    return;

}

void SenderThread::sync()
{
    ready = false;
    if (mode == 1)
    while(controller->makeSender(TMY::TunnelMode::INIT, sender) != 0)
        log("Sender: MakeSender Error!");
    else
    while(controller->makeSender(TMY::TunnelMode::INCR, sender) != 0)
        log("Sender: MakeSender Error!");


    while (!ready);
    log("Sender: MakeSender Success!");
    DirScanner scanner;
    DirInfo dirinfo = scanner.scan(syncPath);
    sender->sendDirInfo(dirinfo);
    log("Sender: DirInfo sent");

    std::set<QString> qsst;
    for (auto &ee : dirinfo)
    {
        qsst.insert(TMY::convert(ee.filePath));
    }

    PullReq req;

    log("Sender: Waiting Pulls");
    while (sender->waitPull(req) != 0) {
        log("Sender: WaitPull timed out");
        Sleep(1);
    }
    log("Sender: Received Pulls");

    PushReq push;
    push.clear();
    int bytes = 0;

    for (PullReqEntry &entry : req)
    {
        log("Sender: Checking pullentry");
        qsst.erase(TMY::convert(entry.filePath));
        QFileInfo finfo(syncPath + TMY::convert(entry.filePath));
        int sentBytes = finfo.size() - entry.len;
        LocalFileReader reader(syncPath);
        connect(&reader, SIGNAL(sendUpMsg(QString, float)), this, SLOT(recvUpMsg(QString, float)));
        reader.setFilePath(entry.filePath);

        PushReq singlepush;

        log("Sender: generating push");
        singlepush = reader.generatePushReqEntry(entry);
        log("Sender: generated push");

        for (PushReqEntry &pushentry : singlepush)
        {
            if (bytes + pushentry.len > 1024 * 1024)
            {
                log("Sender: Sending push, size = " + QString::number(bytes));
                sender->push(push);
                log("Sender: Sent push, size = " + QString::number(bytes));
                bytes = 0;
                push.clear();
            }
            bytes += pushentry.len;
            sentBytes += pushentry.len;
            emit passUpMsg("Uploading " + TMY::convert(entry.filePath), (float)sentBytes / finfo.size());
            push.push_back(pushentry);
            entry.offset += pushentry.len;
        }
    }
    log("Sender: Sending push, size = " + QString::number(bytes));
    if (bytes > 0)
       sender->push(push);
    log("Sender: Sent push, size = " + QString::number(bytes));
    log("Sender: All push sent");

    for (auto &qs : qsst)
    {
        log("Sender: Flash push for file : " + qs);
    }
    emit passUpMsg("NULL", 0);

}

void TMY::SenderThread::recvUpMsg(QString msg, float f)
{
    emit passUpMsg(msg, f);
}
