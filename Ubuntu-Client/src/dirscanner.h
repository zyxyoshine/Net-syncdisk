#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegExp>
#include <QDir>
#include <iostream>
#include "common.h"
#include <QFile>
#include <QCryptographicHash>

#ifndef DIRSCANNER_H
#define DIRSCANNER_H

namespace TMY
{

enum FileFindResult {EXIST, NOTEXSIT, NEWER, OLDER};

class DirScanner : public QObject
{
    Q_OBJECT
public:
    DirInfo scan(QString rootPath);
private:
    std::vector<QRegExp> regs;
    DirInfo result;
    void rscan(QString rootPath, FilePath filePath);
    FileFindResult find(QString rootPath, DirInfoEntry entry);
};
}

#endif // DIRSCANNER_H
