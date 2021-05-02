/****************************************************************************
 ** MainWindow class header. This is a main class to show, control user's data
 ** and to request database records via Server class 
 ** Author: Taras Shevchenko
 ****************************************************************************/

#ifndef MAINFORM_H
#define MAINFORM_H

#include <QMainWindow>
#include "ui_MainForm.h"
#include "AccountForm.h"
#include <string>
#include <QVector>
#include <QString>
#include <QDialog>
#include <QTcpSocket>

class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;

namespace PasswordServer
{
    class MainWindow : public QMainWindow, Ui_MainWindow
    {
            Q_OBJECT
    public:
        enum eOperations
        {
            E_RETRIEVE_ROWS,
            E_UPDATE_ROW,
            E_INSERT_ROW,
            E_DELETE_ROW
        };                 
        MainWindow();
        ~MainWindow(); 
        void fillList( );
        void requestOperation( eOperations operation, QString keyID, QString userName, QString userPassword);
        bool bFirstShow;
        AccountWindow* accountDialog;
        
    private:
        QTcpSocket *tcpSocket;        
        eOperations mLastOperation;
        bool isRequesting;
    private slots:
        void onExit();
        void onAdd();
        void onDelete();
        void onUpdate();
        void onNotifyCloseOperation(QString name, QString password, bool isOK);
        void readRecords();
        void displayError(QAbstractSocket::SocketError socketError);
    };
} // namespace PasswordServer

#endif

