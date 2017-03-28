#ifndef SIGNUPDIALOG_H
#define SIGNUPDIALOG_H

#include <QDialog>
#include "common.h"

namespace Ui {
class SignupDialog;
}

class SignupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SignupDialog(QWidget *parent = 0);
    QString account, passwd1, passwd2;

    ~SignupDialog();

private slots:
    void on_signupButton_clicked();

    void on_accountInput_textChanged(const QString &arg1);

    void on_passwdInput_textChanged(const QString &arg1);

    void on_passwdInput_2_textChanged(const QString &arg1);

private:
    Ui::SignupDialog *ui;


    bool IsWeak(const QString pw);
    void ShowMsg(const QString msg);
};

#endif // SIGNUPDIALOG_H
