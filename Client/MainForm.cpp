/****************************************************************************
 ** MainWindow class implementation. This is a main class to show, control user's data
 ** and to request database records via Server class 
 ** Author: Taras Shevchenko
 ****************************************************************************/

#include "MainForm.h"
#include <QtGui>
#include <QtNetwork>

using namespace PasswordServer;


QVector<QStringList> rowsMain;
QTableWidget* twgAccounts = NULL;

using namespace PasswordServer;

MainWindow::MainWindow()
{
    /***************************
     ** User interface settings 
     ***************************/
    setupUi(this);
    twgAccounts = twAccounts;
    setWindowTitle(QApplication::translate("MainWindow", "Client window", 0, QApplication::UnicodeUTF8));
    twgAccounts->setColumnHidden ( 0, true );
    
    /***************************
     ** Signals/slots mapping
     ***************************/
    QObject::connect(actionExit, SIGNAL(triggered()), this, SLOT(onExit()));
    QObject::connect(pbExit, SIGNAL(pressed()), this, SLOT(onExit()));
    QObject::connect(pbAdd, SIGNAL(pressed()), this, SLOT(onAdd()));
    QObject::connect(pbDelete, SIGNAL(pressed()), this, SLOT(onDelete()));
    QObject::connect(pbUpdate, SIGNAL(pressed()), this, SLOT(onUpdate()));

    /**************************************
     ** Creating and setting a socket class
     *************************************/
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readRecords()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
             this, SLOT(displayError(QAbstractSocket::SocketError)));

    /*******************************************
     ** Creating and setting a TableWidget class
     ******************************************/
    twAccounts->setRowCount(0);
    QStringList headerStrings;
    headerStrings << tr("accounts_id");
    headerStrings << tr("Name");
    headerStrings << tr("Password");

    twAccounts->setHorizontalHeaderLabels(headerStrings);
    twAccounts->setColumnWidth(1, 250);
    twAccounts->setColumnWidth(2, 250);
    
    /*******************************************
     ** Creating and setting a Dialog class
     ******************************************/
    accountDialog = new AccountWindow(reinterpret_cast<QMainWindow*>(this));
    accountDialog->setTextFields(tr("The name"), tr("...password"));
    accountDialog->setModal(true);
    
    isRequesting = false;
    requestOperation(E_RETRIEVE_ROWS, tr(""), tr(""), tr(""));
}

/***********************************************************
 ** Method to send a request to Server for data manipulating
 ***********************************************************/
void MainWindow::requestOperation( eOperations operation, QString keyID, QString userName, QString userPassword)
{
    tcpSocket->abort();
    tcpSocket->connectToHost(tr("localhost"), 65000);
    if (!tcpSocket->waitForConnected(2000))
    {
        qDebug("Error: Timeout is reached");
        tcpSocket->close();
        return;
    }
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    
    switch(operation) 
    {
    case E_RETRIEVE_ROWS:
        //Sending a retrieveRows command
        out << (quint16)101;
        //Sending an ending code
        out << (quint16)211;
    break;
    case E_UPDATE_ROW:
        //Sending an updateRow command
        out << (quint16)111;
        out << keyID;
        out << userName;
        out << userPassword;
        //Sending an ending code
        out << (quint16)211;
    break;
    case E_INSERT_ROW:
        //Sending an insertRow command
        out << (quint16)121;
        out << userName;
        out << userPassword;        
        //Sending an ending code
        out << (quint16)211;
    break;
    case E_DELETE_ROW:
        //Sending a deleteRow command
        out << (quint16)131;
        out << keyID;        
        //Sending an ending code
        out << (quint16)211;
    break;    
    }
    
    out.device()->seek(0);
    tcpSocket->write(block);          
}

MainWindow::~MainWindow()
{
	
}

void MainWindow::onExit()
{
   exit(0);
}

/**************************
 ** Method to add new record
 **************************/
void MainWindow::onAdd()
{
    mLastOperation = E_INSERT_ROW;
    accountDialog->setTextFields(tr(""), tr(""));
    accountDialog->show();
}

/******************************
 ** Method to delete new record
 ******************************/
void MainWindow::onDelete()
{
    if( ( twgAccounts->rowCount() > 0 ) and ( twAccounts->currentRow() >= 0 ) )
    {            
        requestOperation(E_DELETE_ROW, twAccounts->item(twAccounts->currentRow(), 0)->text(), tr(""), tr(""));
    }
}

/******************************
 ** Method to update new record
 ******************************/
void MainWindow::onUpdate()
{
    if(twgAccounts->rowCount() > 0)
    {
        mLastOperation = E_UPDATE_ROW;
        if( ( twgAccounts->rowCount() > 0 ) and ( twAccounts->currentRow() >= 0 ) )
        {            
            accountDialog->setTextFields(twAccounts->item(twAccounts->currentRow(), 1)->text(), twAccounts->item(twAccounts->currentRow(), 2)->text());
            accountDialog->show();
        }
    }
}

/****************************************************************************************************
 ** Slot to react on closing an account dialog. If isOK is true then we can update or insert a record
 ****************************************************************************************************/
void MainWindow::onNotifyCloseOperation(QString name, QString password, bool isOK)
{
    printf("onNotifyCloseOperation. name=%s, password=%s, isOK=%d\n", name.toStdString().c_str(), password.toStdString().c_str(), isOK ? 1 : 0);
    if( isOK )
    {
        isRequesting = true;
        QString mKeyID = "";
        if( ( twgAccounts->rowCount() > 0 ) and ( twAccounts->currentRow() >= 0 ) )
        {
            mKeyID = twAccounts->item(twAccounts->currentRow(), 0)->text();
        }
        requestOperation( mLastOperation, mKeyID, name, password );
    }
}

/****************************************************************************************************
 ** Method to get and fill data have been requested before and to process errors that can occur
 ****************************************************************************************************/
void MainWindow::readRecords()
{
     //qDebug("Client got data!!!");
     QDataStream in(tcpSocket);
     in.setVersion(QDataStream::Qt_4_0);
     
     quint16 mCodeReturned = -1;
     int mRowsCount = 0;
     int mColsCount = 0;
     QString mError = "";
     if (tcpSocket->bytesAvailable() <= 0)
     {
         isRequesting = false;
         return;
     }     
     in >> mCodeReturned;
     if( ( 323 == mCodeReturned) || ( 311 != mCodeReturned ) )
     {
         if( 323 == mCodeReturned )
         {
             in >> mError;
         }
         else
         {
             mError = tr("Communication error");
         }                          
         QMessageBox::information(this, tr("Accounts Client Error"), mError);     
         isRequesting = false;                          
         return;
     }
     in >> mRowsCount;
     in >> mColsCount;
     if( ( NULL != twgAccounts) && ( mRowsCount >= 0 ) )
     {
         twgAccounts->setRowCount( mRowsCount );
         for( int rows = 0 ; rows < mRowsCount ; rows++)
         {
             for( int cols = 0 ; cols < mColsCount ; cols++)
             {        
                 QString mCell;
                 in >> mCell;         
                 QTableWidgetItem *newItem = new QTableWidgetItem(mCell);
                 newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                 twgAccounts->setItem( rows, cols, newItem );                              
             }
         }
    }
    tcpSocket->close();
    delete tcpSocket;
    tcpSocket = new QTcpSocket(this);

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readRecords()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
             this, SLOT(displayError(QAbstractSocket::SocketError)));
     
    if( isRequesting )
        requestOperation(E_RETRIEVE_ROWS, tr(""), tr(""), tr(""));
    isRequesting = false;     
}

/**********************************************
 ** Method to display an error from socket
 **********************************************/
void MainWindow::displayError(QAbstractSocket::SocketError socketError)
{
     switch (socketError) 
     {
     case QAbstractSocket::RemoteHostClosedError:
         break;
     case QAbstractSocket::HostNotFoundError:
         QMessageBox::information(this, tr("Accounts Client"),
                                  tr("The host was not found. Please check the "
                                     "host name and port settings."));
         break;
     case QAbstractSocket::ConnectionRefusedError:
         QMessageBox::information(this, tr("Accounts Client"),
                                  tr("The connection was refused by the peer. "
                                     "Make sure the Server is running, "
                                     "and check that the host name and port "
                                     "settings are correct."));
         break;
     default:
         QMessageBox::information(this, tr("Accounts Client"),
                                  tr("The following error occurred: %1.")
                                  .arg(tcpSocket->errorString()));
     }
}
