#include "dbconnection.h"

DB::DBConnection::DBConnection(const char *dbName
                               , const char *hostName
                               , const char *userName
                               , const char *password
                               , int port
                               , const char *dbtype) throw(ConnectionFailed) {
    mdb = QSqlDatabase::addDatabase(dbtype, QString(dbName));
    mdb.setHostName(hostName);
    mdb.setDatabaseName(dbName);
    mdb.setUserName(userName);
    mdb.setPassword(password);
    mdb.setPort(port);
    if(!mdb.open()) {
        echoErrorInfo("Database connect error: ");
        throw ConnectionFailed();
    }
}

DB::DBConnection::~DBConnection() {
    if(mdb.isOpen()) {
        mdb.close();
        mdb.removeDatabase(mdb.connectionName());
    }
}

bool DB::DBConnection::query(const char *queryStr, DB::QueryResult &ret) {
    if(queryStr == NULL) return false;
    ret = mdb.exec(queryStr);
    if(mdb.lastError().isValid()) {
        echoErrorInfo("Query Error: ");
        return false;
    }
    return true;
}

DB::DBConnection::DBConnection() {
}

inline void DB::DBConnection::echoErrorInfo(const char *tital = "Database error: ") {
    qDebug() << tital << mdb.lastError().text().toUtf8();
}
