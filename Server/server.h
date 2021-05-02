/****************************************************************************
 ** Server class header. Server part of test task. Receives requests from
 ** Client class via TCP Socket, calls Database class to process required data 
 ** Author: Taras Shevchenko
 ****************************************************************************/

#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <QStringList>
#include <QEventLoop>
#include "../Database/database.h"
#include <string>
#include <QTcpSocket>
#include <QtCore>

using namespace std;

class QTcpServer;

class Server : public QObject
{
    Q_OBJECT
public:
    Server();
    int Run();
    
private slots:
    void sendRecord();
    void displayError(QAbstractSocket::SocketError socketError);

private:
    QTcpServer *tcpServer;
    QStringList mRecords;
    Database mDatabase;                  
    bool isRunning;         
};

#endif
