#include "loginwindow.h"
#include "common.h"
#include "ui_loginwindow.h"
#include "signupdialog.h"
#include "mainclient.h"
#include <QMessageBox>
#include <QTextStream>
#include <QFile>

#define log(s) TMY::logger.log(s)

LoginWindow::LoginWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    reconnect();
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::on_signinButton_clicked()
{
    TMY::LoginReq req;
    req.password = password.toStdString();
    req.username = account.toStdString();
    req.session = session.toStdString();

    log("LoginWindow: Logining in as " + account);

    TMY::LoginRes res;
    res.code = 0;
    controllerPtr->login(req, res);
    if (res.code == 0)
    {
        QFile file("./tmyuser.txt");
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream ts(&file);
        session = QString(res.session.c_str());
        ts << ip << "\n" << ports << "\n" << session << "\n" << syncPath;
        file.close();

        MainClient client;
        client.initialize(syncPath, controllerPtr);
        client.exec();
        exit(0);
    }
    else
    {
        log("LoginWindowLogin in failed, msg: " + QString(res.message.c_str()));
        showMsg(QString(res.message.c_str()));
        reconnect();
    }
}

void LoginWindow::reconnect()
{

    QFile file("./tmyuser.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        showMsg("No Config File");
        exit(0);
    }

    QTextStream ds(&file);

    ip = ds.readLine();
    ports = ds.readLine();
    session = ds.readLine();
    syncPath = ds.readLine();
    short port = ports.toInt();
    //ds >> ip >> port;

    file.close();

    SA4 sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = inet_addr(ip.toStdString().c_str());

    QString msg = "LoginWindow: connecting server ... ";
    msg += ip;
    msg += ' ';
    msg += QString::number(port);
    log(msg);

    int res;
    while(res = TMY::Controller::connect((const SA *)&sockaddr, controllerPtr) != 0)
    {
        showMsg(QString("Cannot connect to server"));
        log("LoginWindow: connection failed, retry.  error code: " + QString::number(res));
    }

    log("LoginWindow: connected to server");

}

void LoginWindow::on_accountInput_textChanged(const QString &arg1)
{
    account = arg1;
}

void LoginWindow::on_passwdInput_textChanged(const QString &arg1)
{

    password = arg1;
}

void LoginWindow::on_signupButton_clicked()
{
    SignupDialog sgdlg;
    sgdlg.exec();
    TMY::SignupReq req;
    req.username = sgdlg.account.toStdString();
    req.password = sgdlg.passwd1.toStdString();
    TMY::SignupRes res;
    controllerPtr->signup(req, res);
    showMsg(QString(res.message.c_str()));
    reconnect();
}

void LoginWindow::showMsg(QString msg)
{
    QMessageBox msgbox;
    msgbox.setText(msg);
    msgbox.exec();
}

