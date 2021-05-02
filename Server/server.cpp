/********************************************************************************
 ** Server class implementation. Server part of test task. Receives requests from
 ** Client class via TCP Socket, calls Database class to process required data 
 ** Author: Taras Shevchenko
 ********************************************************************************/

#include <QtNetwork>
#include <stdlib.h>
#include <stdio.h>
#include "server.h"
  
Server::Server()
{
     tcpServer = new QTcpServer(this);
     if (!tcpServer->listen(QHostAddress::Any, 65000)) 
     {
        qDebug("Error: Unable to start the server: %s\n", tcpServer->errorString().toStdString().c_str());                        
         return;
     }
     qDebug("The server is running on port : %d...\n", tcpServer->serverPort());                             
     connect(tcpServer, SIGNAL(newConnection()), this, SLOT(sendRecord()));
}

void Server::displayError(QAbstractSocket::SocketError socketError)
{
     switch (socketError) 
     {
     case QAbstractSocket::RemoteHostClosedError:
         break;
     case QAbstractSocket::HostNotFoundError:
         qDebug(tr("The host was not found. Please check the host name and port settings.").toStdString().c_str());
         break;
     case QAbstractSocket::ConnectionRefusedError:
         qDebug(tr("The connection was refused by the peer. Make sure the Server is running,and check that the host name and port settings are correct.").toStdString().c_str());
         break;
     }

}

/********************************************************************************
 ** Method to send an information from database that has been requested by Client.
 ********************************************************************************/
void Server::sendRecord()
{
     QTcpSocket *mSocket = tcpServer->nextPendingConnection();
     connect(mSocket, SIGNAL(disconnected()), mSocket, SLOT(deleteLater()));
     connect(mSocket, SIGNAL(error(QAbstractSocket::SocketError)),
             this, SLOT(displayError(QAbstractSocket::SocketError)));
     
    if (!mSocket->waitForReadyRead(2000))
    {
        qDebug("Error: Timeout is reached");
        mSocket->close();
        return;
    }
    quint16 mCommand = -1;
    quint16 mEndingCode = -1;
    QString mKeyID, mUserName, mPassword, mErrorMessage;
    bool mIsError = false;
    QByteArray block;
    QDataStream mInputStream(mSocket);
    mInputStream.setVersion(QDataStream::Qt_4_0);               
    mInputStream >> mCommand;
    switch(mCommand)
    {
    case 101:
        //RetrieveRows
        mInputStream >> mEndingCode;
        if( 211 == mEndingCode )
        {
            mIsError = mDatabase.retrieveRows( );     
        }         
        break;
    case 111:
        //UpdateRow
        mInputStream >> mKeyID;
        mInputStream >> mUserName;
        mInputStream >> mPassword;                  
        mInputStream >> mEndingCode;
        if(mUserName.length() == 0)
        {
            mIsError = true;
            mDatabase.setLastErrorMessage( tr("User name cannot be empty").toStdString() );
            break;
        }
        if( 211 == mEndingCode )
        {
            mIsError = mDatabase.updateRow( mKeyID.toStdString(), mUserName.toStdString(), mPassword.toStdString() );          
            qDebug("UpdateRow called");
        } 
        break;
    case 121:
        //InsertRow
        mInputStream >> mUserName;
        mInputStream >> mPassword;         
        mInputStream >> mEndingCode;
        if(mUserName.length() == 0)
        {
            mIsError = true;
            mDatabase.setLastErrorMessage( tr("User name cannot be empty").toStdString() );
            break;
        }
        if( 211 == mEndingCode )
        {
            mIsError = mDatabase.insertRow( mUserName.toStdString(), mPassword.toStdString() );
        } 
        break;
     case 131:
        //DeleteRow
        mInputStream >> mKeyID;
        mInputStream >> mEndingCode;
        if( 211 == mEndingCode )
        {
            mIsError = mDatabase.deleteRow( mKeyID.toStdString() );
        } 
        break;                  
     }

     QDataStream out(&block, QIODevice::ReadWrite);
     out.setVersion(QDataStream::Qt_4_0);     
     
     if( 211 != mEndingCode ) 
     {
         mErrorMessage = tr("Incorrect ending code has been gotten from a client.");
         mIsError = true;
     }
     else
     {      
         if( mIsError ) mErrorMessage = QString::fromStdString( mDatabase.getLastErrorMessage() );                           
     }
     if(mIsError)
     {
         //An error occurs
         out << (quint16)323;
         out << mErrorMessage;
     }     
     else
     {
         //Its OK
         out << (quint16)311;     
     }
     
    out << mDatabase.getRowsCount( );
    out << mDatabase.getColsCount( );
    //printf("RowsCount=%d\n", mDatabase.getRowsCount() ); 
    //printf("ColsCount=%d\n", mDatabase.getColsCount() );
    
    for(int rows = 0 ; rows < mDatabase.getRowsCount( ) ; rows++)
    {
       for(int cols = 0 ; cols < mDatabase.getColsCount( ) ; cols++)    
       {
           out << QString::fromStdString( mDatabase.getValue( rows, cols ) );
           //printf("Data[row=%d][col=%d]=%s\n", rows, cols, mDatabase.getValue( rows, cols ).c_str());
       }
    }         
    out.device()->seek(0);
    mSocket->write(block);
    mSocket->disconnectFromHost();
}

/***************************************
 ** Making a loop to stay in processing.
 ***************************************/
int Server::Run()
{
    isRunning = true;     
    QEventLoop mEventLoop;                  
    while(isRunning)mEventLoop.exec();
    return 0;
}

