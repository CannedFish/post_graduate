#ifndef DATAFROMINFODB_H
#define DATAFROMINFODB_H

#include "common.h"
#include "dbconnection.h"

class DataFromInfoDB
{
    typedef std::map<std::string, u_int> StrArray; //roadid -> avgspeed
    typedef StrArray::iterator StrArrayItor;
    typedef std::map<std::string, StrArray> OSM2DataMap; //osm_id -> [<roadid, avgspeed>]
public:
    DataFromInfoDB()
        : dbFCD(NULL), dbMapData(NULL), osmIdStr("") {
        if(!initDBHandle() || !initOSMIDStr() || !initOSM2DataMap()) {
            delete this;
            return ;
        }
    }
    ~DataFromInfoDB();
    void updateInfo();

private:
    bool initDBHandle();
    bool initOSMIDStr();
    bool initOSM2DataMap();
    bool getRoadInfo();
    void updataMapData();

private:
    DB::DBConnection *dbFCD, *dbMapData;
    DB::QueryResult qRet, tmp;
    std::string osmIdStr;
    OSM2DataMap o2dMap;
};

#endif // DATAFROMINFODB_H
