/****************************************************************************
 ** Database class header. Managing connections with database (in our case 
 ** it is SQLite), making requests and receiving responses. 
 ** Interacts with Server class which uses it.
 ** Author: Taras Shevchenko
 ****************************************************************************/

#ifndef DATABASE_H
#define DATABASE_H

#define NUMBER_OF_SHOTS 5
#define DELAY_TIMER 200
#define DELAY_PAUSE 50000

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "sqlite3.h"
#include <string>
#include <vector>

using namespace std;
typedef vector<string> StringArray;

class Database
{
public:
    Database( );
    virtual ~Database( );
    bool retrieveRows( );
    bool updateRow( string keyID, string userName, string userPassword );
    bool insertRow( string userName, string userPassword );
    bool deleteRow( string keyID );
    
    string getLastErrorMessage();                                
    void setLastErrorMessage( string errorMessage);
    int getRowsCount( );
    int getColsCount( );
    string getValue( int rowNum, int colNum );    
    
    static bool mGetDataFailed;
    static int mDataCount;
    static int mTimerCounter;    
    static bool mbIsGettingCount;
    static vector<StringArray>* mDataArray;
private:  
    bool execSQL( string sqlText, string tableName, bool isRetrieve = true, bool toOpen = true, bool toClose = true );
    bool processError( int resultDB );
    string correctStringForSql( string inputString );
    bool isRunning;         
    sqlite3 *ppDb;
    bool bFirstShow;    
    string mLastError;
};

#endif
