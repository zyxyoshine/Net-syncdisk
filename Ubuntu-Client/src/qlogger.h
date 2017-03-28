
#ifndef LOGGER_H
#define LOGGER_H
#include <QThread>
#include <QFile>
#include <QTime>
#include <QTextStream>
#include <iostream>
#include <QIODevice>
#include <QMutex>
#include <QDebug>


namespace TMY
{
const QString LOGFILEHEADDER = "TMYClientLog";

class Logger : public QObject
{
    Q_OBJECT
public:
    QString filename;
    QFile file;
    Logger();
    void log(const QString);

signals:
    void newLog(QString);

private:
    QMutex mutex;
};

extern Logger logger;

}

#endif // LOGGER_H
