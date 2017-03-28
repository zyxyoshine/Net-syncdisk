#include "loginwindow.h"
#include "signupdialog.h"
#include "common.h"
#include "mainclient.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    initsock();

    LoginWindow w;
   // MainClient w;
    w.show();

    return a.exec();
}
