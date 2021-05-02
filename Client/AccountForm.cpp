/****************************************************************************
 ** AccountWindow class implementation. This is a dialog to change and
 ** insert user's name and password 
 ** Author: Taras Shevchenko
 ****************************************************************************/

#include "AccountForm.h"

using namespace PasswordServer;

AccountWindow::AccountWindow(QMainWindow* pWindow)
{
   setupUi(this);
   parentWindow = pWindow;
   QObject::connect(this, SIGNAL(NotifyCloseOperation(QString, QString, bool)), parentWindow, SLOT(onNotifyCloseOperation(QString, QString, bool)));
   QObject::connect(pbOk, SIGNAL(pressed()), this, SLOT(onOk()));
   QObject::connect(pbCancel, SIGNAL(pressed()), this, SLOT(onCancel()));
}

void AccountWindow::setTextFields(QString name, QString password)
{
   txtName->setText(name);
   txtPassword->setText(password);
}

void AccountWindow::onOk()
{
   emit NotifyCloseOperation(txtName->text(), txtPassword->text(), true);
   done(1);
}

void AccountWindow::onCancel()
{
   emit NotifyCloseOperation(txtName->text(), txtPassword->text(), false);
   done(0);
}
