#include "datafrominfodb.h"
#include "eventgovernor.h"

DataFromInfoDB::~DataFromInfoDB() {
    if(dbFCD != NULL) {
        delete dbFCD;
        dbFCD = NULL;
    }
    if(dbMapData != NULL) {
        delete dbMapData;
        dbMapData = NULL;
    }
}

void DataFromInfoDB::updateInfo() {//template method
    if(!getRoadInfo()) return ;
    updataMapData();
    return ;
}

bool DataFromInfoDB::initDBHandle() {
    try {
        dbFCD = new DB::DBConnection("gis"
                                     , "192.168.1.152"
                                     , "postgres"
                                     , "123456"
                                     , 5433);
        dbMapData = new DB::DBConnection("mapdata"
                                         , "127.0.0.1");
    } catch(ConnectionFailed &e) {
        printf("%s\n", e.what());
        return false;
    }

    return true;
}

bool DataFromInfoDB::initOSMIDStr() {
    try {
        DB::DBConnection dbConn("myrouting", "127.0.0.1");
        if(!dbConn.query("SELECT osm_id FROM planet_osm_line "
                     "WHERE highway IS NOT null AND "
                     "railway IS null AND highway NOT IN "
                     "('construction', 'steps', 'cycleway', 'path', "
                     "'birdleway', 'footway', 'pedestrian') "
                     "ORDER BY osm_id", qRet)) return false;
        while(qRet.next()) {
            osmIdStr += qRet.value(0).toString().toStdString() + ",";
        }
        qRet.clear();
        osmIdStr[osmIdStr.length() - 1] = ')';
        osmIdStr = "SELECT roadid, emptytaxispeed FROM its_ptroadstatusins_tbl WHERE roadid IN ("
                + osmIdStr + "ORDER BY roadid";
    } catch(ConnectionFailed &e) {
        std::cout << e.what() << std::endl;
        return false;
    }

    return true;
}

bool DataFromInfoDB::initOSM2DataMap() {
    if(!dbMapData->query("SELECT id FROM Wuhan_Road ORDER BY id", qRet)) return false;
    std::string id;
    //The roadid can be seen as a set of extension of osm_id
    while(qRet.next()) {
        id = qRet.value(0).toString().toStdString();
        o2dMap[id.substr(0, id.length() - 4)].insert(std::make_pair(id, 40));
    }
    qRet.clear();
    return true;
}

inline bool DataFromInfoDB::getRoadInfo() {
    return dbFCD->query(osmIdStr.c_str(), qRet);
}

void DataFromInfoDB::updataMapData() {
    u_int value, evType(0);
    int trend;
    char cmd[256];
    EventGovernor *egIns = EventGovernor::getInstance();
    dbMapData->query("BEGIN TRANSACTION", tmp);
    while(qRet.next()) {
        value = qRet.value(1).toUInt();
        if(value == 9999) continue;
        StrArray &strArray = o2dMap[qRet.value(0).toString().toStdString()];
        for(StrArrayItor itr = strArray.begin(); itr != strArray.end(); ++itr) {
            //check value changes, set the notify bit if it satifies one or more event
            if((value > EventGovernor::LOWEST_SPEED && itr->second <= EventGovernor::LOWEST_SPEED)
                    || (value <= EventGovernor::LOWEST_SPEED && itr->second > EventGovernor::LOWEST_SPEED)) {
                evType |= EventGovernor::TRAFFICJAM;
            }

            trend = value - itr->second;
            itr->second = value;
            //use new trend and value to calculate a new mark for this road

            sprintf(cmd, "UPDATE Wuhan_Road SET "
                    "avgspeed = %d, traffictrend = %d WHERE "
                    "id = '%s'", value, trend, (itr->first).c_str());
            dbMapData->query(cmd, tmp);
        }
    }
    dbMapData->query("COMMIT", tmp);
    qRet.clear();
    //If the notify bit is setted, send the corresponding event type
    if(evType) egIns->notifyEvent(evType);

    return ;
}
