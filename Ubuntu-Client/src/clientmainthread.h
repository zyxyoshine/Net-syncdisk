#ifndef CLIENTMAINTHREAD_H
#define CLIENTMAINTHREAD_H

#include "common.h"
#include <QThread>

namespace TMY
{
class Client : public QThread
{
    Q_OBJECT
public:
    void run();

};
}

#endif // CLIENTMAINTHREAD_H
