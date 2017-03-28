#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "common.h"


extern TMY::Logger TMY::logger;
namespace Ui {
class LoginWindow;
}

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = 0);
    ~LoginWindow();

private slots:
    void on_signinButton_clicked();

    void on_accountInput_textChanged(const QString &arg1);

    void on_passwdInput_textChanged(const QString &arg1);

    void on_signupButton_clicked();

private:
    Ui::LoginWindow *ui;
    QString account, password;
    QString ip, ports, session, syncPath;
    TMY::Controller_ptr controllerPtr;

    void showMsg(QString msg);
    void reconnect();
};

#endif // LOGINDIALOG_H
