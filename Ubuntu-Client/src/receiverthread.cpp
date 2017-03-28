#include "receiverthread.h"
#include "localfilemanager.h"
#include <QFileInfo>
#include <QCryptographicHash>
#include <QDir>
#include <map>
#define log(s) TMY::logger.log(s)

using namespace TMY;

void ReceiverThread::run()
{
    mode = 0;
    while (1)
    {
        if (mode == 1 || mode == 3)
            initSync();
        qDebug() << mode;
    }
}

void ReceiverThread::setMode(int mode_, QString path)
{
    mode = mode_;
    syncPath = path;
}

void ReceiverThread::setController(TMY::Controller_ptr cp)
{
    controller = cp;
}

void ReceiverThread::initSync()
{
    //while(1);

    log("Receiver: Start Initial Sync");
    sync();
    log("Receiver: InitSync over!");
    mode = 3;
    endSync();
}

void ReceiverThread::endSync()
{
    while(mode == 3)
    {
        sleep(5);
        log("Receiver: Start Increment Sync");
        sync();
        log("Receiver: End Increment Sync");
    }

}


void ReceiverThread::sync()
{

    while (controller->makeReceiver(TMY::TunnelMode::INIT, receiver)!= 0)
        log("Receiver: MakeReceiver Error");
    log("Receiver: MakeReceiver Success!");

    DirInfo dirinfo;
    while (receiver->waitDirInfo(dirinfo) != 0) {
         while (controller->makeReceiver(TMY::TunnelMode::INIT, receiver)!= 0)
              log("Receiver: MakeReceiver Error");
    }
    log("Receiver: DirInfo received");

    std::map<QString, int> flen;

    for (DirInfoEntry &entry : dirinfo)
    {
        QString ffpath = syncPath;
        ffpath += TMY::convert(entry.filePath);

        QFileInfo finfo;
        finfo.setFile(ffpath);
        QFileInfo metacheck;
        metacheck.setFile(ffpath + ".tmydownload");
        log("Receiver: checking file:" + ffpath);

        flen[TMY::convert(entry.filePath)] = entry.len;

        if (metacheck.exists())
            continue;
        if ( finfo.exists() ) /* file exist */
        {
           /* full file */
                QFile file(ffpath);
                file.open(QIODevice::ReadOnly);
                QCryptographicHash hash(QCryptographicHash::Algorithm::Md5);
                hash.addData(&file);
                std::string localmd5 = QString(hash.result().toHex()).toStdString();
                log("COMPARE MD5");
                log(localmd5.c_str());
                log(entry.md5.c_str());

                if (localmd5 != entry.md5)
                {
                    if (finfo.lastModified().toTime_t() > entry.modtime)
                    {
                        if (mode == 3)
                            continue;
                        log("Receiver: change online file " + ffpath + " to .old");
                        entry.filePath.filename += ".old";
                    }
                    else
                    {
                        if (mode == 1)
                        {
                            log("Receiver: change local file " + ffpath + " to .old");
                            QString newname = ffpath;
                            newname += ".old";
                            file.rename(newname);
                        }
                    }
                }
                else
                {
                    continue;
                }
        }
            QString fpath = syncPath;
            fpath += TMY::convert(entry.filePath);
            QString mpath = fpath;
            mpath += ".tmydownload";

            QString res = syncPath + "/";
            for (std::string &str : entry.filePath.pathArr)
            {
                res += QString(str.c_str());
                res += '/';
            }
            QDir dirobj(res);
            dirobj.mkpath(res);

            QFile dfile(fpath);
            if (dfile.exists())
                dfile.remove();

            //if(!finfo.exists())
            QFile mfile(mpath);
            mfile.open(QIODevice::WriteOnly | QIODevice::Text);
            QTextStream ts(&mfile);
            ts << "[]";
            mfile.close();
            log("Receiver: online file " + ffpath + " created!");
    }

    emit writeReady();
    log("Receiver: Old File Changed");

    PullReq pulls;

    for (DirInfoEntry &entry : dirinfo)
    {
        LocalFileWriter writer(syncPath);
        writer.setFile(entry.filePath, entry.len);

        PullReqEntry pullentry;
        PullReq singlepull = writer.generatePullReqEntry();

        for (PullReqEntry &pullentry : singlepull)
            pulls.push_back(pullentry);
    }

    receiver->sendPull(pulls);
    log("Receiver: Pulls sent");

    LocalFileWriter writer(syncPath);
    connect(&writer, SIGNAL(sendDownMsg(QString, float)), this, SLOT(recvDownMsg(QString, float)));
    PushReq pushes;
    log("Receiver: waiting for pushes");
    if (!pulls.empty())
        while (receiver->waitPush(pushes) == 0)
        {
            for (PushReqEntry &entry : pushes)
            {
                writer.setFile(entry.filePath, flen[TMY::convert(entry.filePath)]);
                writer.writePush(entry);
            }
        }
    emit passDownMsg("NULL", 0);
}

void TMY::ReceiverThread::recvDownMsg(QString msg, float f)
{
    emit passDownMsg(msg, f);
}
