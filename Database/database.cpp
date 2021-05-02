/****************************************************************************
 ** Database class implementation. Managing connections with database  
 ** (in our caseit is SQLite), making requests and receiving responses. 
 ** Interacts with Server class which uses it.
 ** Autor: Taras Shevchenko
 ****************************************************************************/

#include "database.h"

/**********************************
 ** Static variables initialization
 **********************************/
bool Database::mGetDataFailed = true;
bool Database::mbIsGettingCount = false;
int Database::mDataCount = 0;
int Database::mTimerCounter = 1;
vector<StringArray>* Database::mDataArray = new vector<StringArray>;

/********************************************************
 ** Callback function declaration to get data from SQLite
 ********************************************************/
static int callbackGetData(void *NotUsed, int argc, char **argv, char **azColName);

/**************************************************
 ** Delay function and functions to work with timer
 **************************************************/
int delay(int i);
int SetTicker(int n_msecs);
void CounterChange(int signum);

int delay(int i)
{
    struct timeval timeout;
    if (i>0)
    {
        timeout.tv_usec = i % (unsigned long) 100000;
	timeout.tv_sec = i / (unsigned long) 100000;
	select(0, NULL, NULL, NULL, &timeout);
    }
    return (i>0 ? i : 0);
}

void CounterChange(int signum)
{    
    if(--Database::mTimerCounter > 0)
    {
        printf("wait : %d\n", Database::mTimerCounter);
	if(SetTicker(DELAY_TIMER)==-1)
	{	    
            printf("Abnormal Database termination\n");	
	    exit(1);	
	}    
    }
    else
    {
        printf("Alarm interval has been expired. Exiting from waiting procedures\n");
        Database::mbIsGettingCount = false;
        Database::mDataCount = 0;
    }
}

int SetTicker(int n_msecs)
{
    struct itimerval new_timeset;
    long n_sec,n_usecs;
    n_sec = n_msecs / 1000;
    n_usecs = (n_msecs % 1000) * 1000L;
    new_timeset.it_interval.tv_sec = n_sec;
    new_timeset.it_value.tv_sec = n_sec;
    new_timeset.it_value.tv_usec = n_usecs;
    return setitimer(ITIMER_REAL, &new_timeset, NULL);
}

/********************************************
 ** Callback function to get data from SQLite
 *******************************************/
static int callbackGetData(void *NotUsed, int argc, char **argv, char **azColName)
{
    int i;
    string mColumn;
    string mDataElement;
  
    if( ( Database::mbIsGettingCount ) && ( argc >= 0 ))
    {
        /******************************************************************************
         ** Obtaining a record count of table to have bounds while asynchronous reading
         *****************************************************************************/
        Database::mDataCount = argv[0] ? atoi(argv[0]) : 0;
        Database::mbIsGettingCount = false;
        return 0;
    }
    
    if(Database::mDataCount <= 0) return 0;
    StringArray mDataRow;
    for(i=0; i<argc; i++)
    {
      mDataElement = argv[i] ? argv[i] : "NULL";
      mDataRow.push_back(mDataElement);
    }
    Database::mDataArray->push_back(mDataRow);
    Database::mDataCount--;
    return 0;
}

Database::Database()
: ppDb( NULL )
{
}

Database::~Database()
{
    if( ppDb ) sqlite3_close( ppDb );
}

/***************************************************************
 ** Method to send a request to database and waiting for results
 **************************************************************/
bool Database::execSQL( string sqlText, string tableName, bool isRetrieve, bool toOpen, bool toClose )
{
    char *zErrMsg  = 0;
    char *filename = "../Database/accounts.db";
    int mResultDB  = 0;
    
    if( toOpen )
    {
        mResultDB = sqlite3_open( filename, &ppDb );
        if( processError( mResultDB ) ) return true;
    }
    /********************************
     ** Amount of alarm timer's ticks
     ********************************/
    mTimerCounter = 10;
    /***********************************************
     ** Flag of getting a count of rows from a table
     ***********************************************/
    mbIsGettingCount = true;    
    /**************************************
     ** Initialization of number of records
     **************************************/
    mDataCount = 0;    
    /**************************
     ** Enabling an alarm timer
     **************************/
    signal(SIGALRM, CounterChange);
    
    if( isRetrieve )
    {
        /***************************************************************************************
         ** Filling a string for getting a count of records from the table and executing a query
         ***************************************************************************************/
        string mSqlTextGetCount = string("select count(*) from ") + tableName;
        mResultDB = sqlite3_exec(ppDb, mSqlTextGetCount.c_str() , callbackGetData, 0, &zErrMsg);    
        if( processError( mResultDB ) ) return true;    
        while( mbIsGettingCount ) delay(0);
    }
    /*****************************
     ** Preparing to get real data
     *****************************/
    mDataArray->clear();

    /********************
     ** Getting real data
     ********************/
    mResultDB = sqlite3_exec(ppDb, sqlText.c_str() , callbackGetData, 0, &zErrMsg);
    if( processError( mResultDB ) ) return true;
    if( isRetrieve )
    {    
        while(mDataCount > 0)delay(0);
    }
    if( toClose )
    {
       sqlite3_close(ppDb);
       ppDb = NULL;
    }
    return false;
}

/********************************
 ** Error discovering and saving
 *******************************/
bool Database::processError( int resultDB )
{
    mLastError = "";
    if( resultDB )
    {
        mLastError = sqlite3_errmsg(ppDb);
        printf("Database error: %s\n", sqlite3_errmsg(ppDb));
	sqlite3_close(ppDb);
	ppDb = NULL;
	return true;
    }
    return false;
}

/***********************************************************
 ** Method to prepare and send data for updating in database
 **********************************************************/
bool Database::updateRow( string keyID, string userName, string userPassword )
{
    string mSql = string("update accounts_list set user_name=");
    mSql += correctStringForSql(userName);
    mSql += string(", user_password=");
    mSql += correctStringForSql(userPassword);
    mSql += string(" where accounts_list_id=");
    mSql += keyID;
    bool mResult = execSQL( mSql, "accounts_list", false );
    if( !mResult )
    {
        mResult = retrieveRows( );
    }
    return mResult;
}

/***********************************************************
 ** Method to prepare and send data for inserting in database
 **********************************************************/
bool Database::insertRow( string userName, string userPassword )
{
    string mSql = string("insert into accounts_list(user_name, user_password) values(");
    mSql += correctStringForSql(userName);
    mSql += string(",");
    mSql += correctStringForSql(userPassword);
    mSql += string(")");
    bool mResult = execSQL( mSql, "accounts_list", false );
    if( !mResult )
    {
        mResult = retrieveRows( );
    }
    return mResult;
}

/***********************************************************
 ** Method to prepare and send data for deleting in database
 **********************************************************/
bool Database::deleteRow( string keyID )
{
    bool mResult = execSQL( string("delete from accounts_list where accounts_list_id=") + keyID, "accounts_list", false );
    if( !mResult )
    {
        mResult = retrieveRows( );
    }
    return mResult;
}

/************************************
 ** Method to get all data from table
 ************************************/
bool Database::retrieveRows( )
{
    return execSQL( "select * from accounts_list", "accounts_list" );
}

/************************************
 ** Getting a message with last error 
 ************************************/
string Database::getLastErrorMessage()
{
    return mLastError;
}

/************************************
 ** Setting a message with last error 
 ************************************/                
void Database::setLastErrorMessage( string errorMessage)
{
    mLastError = errorMessage;
}
            
/****************************************
 ** Getting a rows number from data array 
 ***************************************/                
int Database::getRowsCount( )
{
    return mDataArray->size();
}

/*******************************************
 ** Getting a columns number from data array 
 ******************************************/                
int Database::getColsCount( )
{
    if( mDataArray->size() > 0 )
    {
        return (*mDataArray)[0].size();
    }    
    return 0;
}

/****************************************************
 ** Getting a value by row and column from data array 
 ***************************************************/                
string Database::getValue( int rowNum, int colNum )
{
    if( ( rowNum >= 0 ) && ( colNum >= 0) && ( rowNum < getRowsCount( ) ) && ( colNum < getColsCount( ) ) )
    {
        return (*mDataArray)[rowNum][colNum];
    }
    return string("");
}

/****************************************************
 ** Correct a string while preparing to SQL-query
 ***************************************************/                
string Database::correctStringForSql( string inputString )
{
    string mOutString="";
    for(int loopStr = 0 ; loopStr < inputString.length() ; loopStr++)
    {
        if(inputString.at(loopStr) == '\'') mOutString += "''";
        else mOutString += inputString.at(loopStr);
    }
    mOutString = "'" + mOutString + "'";
    return mOutString;
}

