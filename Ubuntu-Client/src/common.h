#ifndef COMMON_H
#define COMMON_H
#include <QString>
#include "controller.h"
#include "err.h"
#include "json.hpp"
#include "listener.h"
#include "logger.h"
#include "qlogger.h"
#include "net.h"
#include "packets.h"
#include "receiver.h"
#include "sender.h"
#include "tmy.h"
#include "tsock.h"
#include "utility.h"

namespace TMY {

    static QString convert(FilePath filepath)
    {
        QString res = "/";
        for (std::string &str : filepath.pathArr)
        {
            res += QString(str.c_str());
            res += '/';
        }
        res += QString(filepath.filename.c_str());

        return res;
    }
}

#endif // COMMON_H
