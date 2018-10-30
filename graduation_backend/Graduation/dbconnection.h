#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#include <qt4/QtSql/QSqlDatabase>
#include <qt4/QtSql/QSqlError>
#include <qt4/QtSql/QSqlQuery>
#include <QVariant>
#include <QDebug>
#include "common.h"
#include "exceptions.h"

namespace DB {
    typedef QSqlQuery QueryResult;
    class DBConnection
    {
    public:
        DBConnection(const char *dbName
                     , const char *hostName
                     , const char *userName = "test"
                     , const char *password = "123456"
                     , int port = 5432
                     , const char *dbtype = "QPSQL") throw(ConnectionFailed);
        ~DBConnection();

        bool query(const char *queryStr, QueryResult &ret);
    private:
        DBConnection();
        void echoErrorInfo(const char *tital);

        QSqlDatabase mdb;
    };
}

#endif // DBCONNECTION_H
