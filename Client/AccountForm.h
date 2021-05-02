/****************************************************************************
 ** AccountWindow class header. This is a dialog to change and
 ** insert user's name and password 
 ** Author: Taras Shevchenko
 ****************************************************************************/

#ifndef ACCOUNTFORM_H
#define ACCOUNTFORM_H

#include "ui_AccountForm.h"
#include <QtGui/QMainWindow>
#include <string>
#include <QVector>
#include <QString>

using namespace std;

namespace PasswordServer
{
    class AccountWindow : public QDialog, Ui_AccountWindow
    {
        Q_OBJECT
    public:
        AccountWindow(QMainWindow* pWindow);
        void setTextFields(QString name, QString password);

    private:
        QMainWindow* parentWindow;

    private slots:
        void onOk();
        void onCancel();

    signals:
        void NotifyCloseOperation(QString name, QString password, bool isOK);
    };
} // namespace PasswordServer

#endif
