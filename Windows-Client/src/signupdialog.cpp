#include "signupdialog.h"
#include "ui_signupdialog.h"
#include "QMessageBox"

SignupDialog::SignupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignupDialog)
{
    ui->setupUi(this);
}

SignupDialog::~SignupDialog()
{
    delete ui;
}

void SignupDialog::ShowMsg(const QString msg)
{
    QMessageBox msgbox;
    msgbox.setText(msg);
    msgbox.exec();
}

bool SignupDialog::IsWeak(const QString pw)
{
    if (pw.size() < 8)
        return true;

    int str = 0;
    for (int i = 0; i < pw.size(); ++i)
        if ((pw[i] >= 'a') && (pw[i] <= 'z'))
        {
            ++str;
            break;
        }
    for (int i = 0; i < pw.size(); ++i)
        if ((pw[i] >= '0') && (pw[i] <= '9'))
        {
            ++str;
            break;
        }
    for (int i = 0; i < pw.size(); ++i)
        if ((pw[i] >= 'A') && (pw[i] <= 'Z'))
        {
            ++str;
            break;
        }
    if (str < 3)
        return true;
    return false;
}

void SignupDialog::on_signupButton_clicked()
{
    if (passwd1 != passwd2)
    {
        ShowMsg("Password Confirm Error");
        return;
    }
    if (IsWeak(passwd1))
    {
        ShowMsg("Password Too Weak!");
        return;
    }
    this->accept();
}

void SignupDialog::on_accountInput_textChanged(const QString &arg1)
{
    account = arg1;
}

void SignupDialog::on_passwdInput_textChanged(const QString &arg1)
{
    passwd1 = arg1;
}

void SignupDialog::on_passwdInput_2_textChanged(const QString &arg1)
{
    passwd2 = arg1;
}
