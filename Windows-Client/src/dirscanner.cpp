#include "dirscanner.h"
#define TMYIGNORE "./tmyignore.txt"
#include <QFile>

using namespace TMY;

#define log(s) TMY::logger.log(s)

DirInfo TMY::DirScanner::scan(QString rootPath)
{
    QFile ignfile(TMYIGNORE);
    ignfile.open(QIODevice::ReadOnly | QIODevice::Text);
    if (ignfile.error())
        log("Scanner: error loading ignore file");
    else
    {
        log("Scanner: loading ignore file");
        regs.clear();
        while (!ignfile.atEnd())
        {
            QString s = ignfile.readLine();
            s.remove(s.length() - 1, 1);
            regs.push_back(QRegExp(s));
            log("Scanner: ignore " + s);
        }
    }

    result.clear();

    FilePath path;
    path.filename = "";
    path.pathArr.clear();

    rscan(rootPath, path);

    return result;
}

void TMY::DirScanner::rscan(QString rootPath, FilePath filePath)
{
    QString path = rootPath;
    path += TMY::convert(filePath);

    QFileInfo metacheck(path + ".tmydownload");
    if (metacheck.exists())
        return;
    QFileInfo finfo(path);

    QDir dirobj(path);
    QStringList sublist = dirobj.entryList();

    if (finfo.isDir())
    {
        log("Scanner: scaning dir: ");
        log(rootPath);
        for (int i = 0; i < sublist.size(); ++i)
        {
            if (sublist[i] == ".")
                continue;
            if (sublist[i] == "..")
                continue;

            FilePath next(filePath);
            next.pathArr.push_back(filePath.filename);
            next.filename = sublist[i].toStdString();
            rscan(rootPath, next);
        }
        log("Scanner: scaning end ");
    }
    else
    {
        for (QRegExp &reg : regs)
        {
            if (reg.indexIn(QString(filePath.filename.c_str())) != -1)
                return;
        }
        if (finfo.size() == 0)
            return;

        DirInfoEntry entry;
        entry.modtime = finfo.lastModified().toTime_t();
        if (filePath.pathArr[0] == "")
            filePath.pathArr.erase(filePath.pathArr.begin());
        entry.filePath = filePath;

        QFile file(path);
        file.open(QIODevice::ReadOnly);
        QCryptographicHash hash(QCryptographicHash::Algorithm::Md5);
        hash.addData(&file);
        entry.md5 = QString(hash.result().toHex()).toStdString();
        entry.len = finfo.size();

        log("Scanner: File Found: " + TMY::convert(filePath));

        result.push_back(entry);
    }

}
