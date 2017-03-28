#pragma once

#include "common.h"
#include <fstream>
#include <QObject>

namespace TMY
{

    class LocalFileWriter : public QObject
    {
        Q_OBJECT
    public:
        LocalFileWriter(QString syncPath_);
        void setFile(const FilePath, const int);
        TMY::PullReq generatePullReqEntry();
        void writePush(PushReqEntry &pushReq);
        bool finished;
    signals:
        void sendDownMsg(QString, float);

    private:
        int fulllen;
        QFile file;
        FilePath filePath;
        QString path, metaPath;
        QString syncPath;
        Chunks chunks;
        int chunksWriteCnt;
        QString METATAIL = ".tmydownload";
        void updateChunks();

    };

    class LocalFileReader : public QObject
    {
        Q_OBJECT
    public:
        LocalFileReader(QString syncPath_);
        void setFilePath(const FilePath);
        PushReq generatePushReqEntry(PullReqEntry);
    signals:
        void sendUpMsg(QString, float);
    private:
        QFile file;
        FilePath filePath;
        QString path;
        QString syncPath;
    };

}
