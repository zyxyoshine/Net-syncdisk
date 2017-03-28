#include "logger.h"
#include "common.h"
#include <QMutex>

TMY::Logger TMY::logger;

TMY::Logger::Logger()
{
    filename = LOGFILEHEADDER;
    filename += QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss");
    filename += ".txt";
    file.setFileName(filename);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
}

void TMY::Logger::log(const QString msg)
{
    mutex.lock();

    file.open(QIODevice::Append | QIODevice::Text);
    QTextStream ds;
    ds.setDevice(&file);

    QString fullmsg = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss : ");
    fullmsg += msg;

    qDebug() << fullmsg;
    emit newLog(fullmsg);
    fullmsg += "\n";
    ds << fullmsg;

    file.close();

    mutex.unlock();
}
