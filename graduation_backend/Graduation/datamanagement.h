#ifndef DATAMANAGEMENT_H
#define DATAMANAGEMENT_H

#include "mapdatahandlemethod.h"
#include "task.h"

typedef Map<BASICTYPE::Junc, BASICTYPE::Road> MapType;
template<typename HandleMethod = MapDataHandleMethod::ReverseIndexLike>
class DataManagement
{
    typedef std::list<std::string> InitedMap;
    typedef typename InitedMap::iterator IMIT;
public:
    ~DataManagement();
    bool initialMap(const char *mapName);
    MapType *getInitedMap(const char *mapName);

    static DataManagement * getInstance() {
        if(dmIns == NULL) {
            dmIns = new DataManagement();
        }
        return dmIns;
    }

private:
    DataManagement();
    bool initialMapJunc(const char *mapName);
    bool initialMapRoad(const char *mapName);
    bool initialMapFromInitedData(const char *mapName);
    bool initialMapFromRawData(const char *mapName);

    static DataManagement *dmIns;
    HandleMethod dataHandle;
    MapType *mapIns;
    InitedMap initedMapList;
    MapDataHandleMethod::DBDataHandleOperation dbUtilOPs;
};

/////////////////////////////////// implement ///////////////////////////////////////////////////////////////////////
template<typename HandleMethod>
DataManagement<HandleMethod>::~DataManagement() {
    for(IMIT imIt = initedMapList.begin(); imIt != initedMapList.end(); ++imIt) {
        delete MapType::getMap(*imIt);
    }
    initedMapList.clear();
    dmIns = NULL;
}

template<typename HandleMethod>
bool DataManagement<HandleMethod>::initialMap(const char *mapName) {
    if(!initialMapJunc(mapName) || !initialMapRoad(mapName)) {
        if(!initialMapFromInitedData(mapName)) {
            std::cout << "Oops!! There aren't inited data in DB." << std::endl;
            if(!initialMapFromRawData(mapName)) return false;
        }
    }
//start a task for initialed map to update road information such as average speed.
    Task *updateRoadInfo = new UpdateRoadInfo;
    defaultThreadPool.setTask(updateRoadInfo, const_cast<char *>(mapName), sizeof(const char *));

    std::cout << "Map initialized OK" << std::endl;
    return true;
}

template<typename HandleMethod>
MapType * DataManagement<HandleMethod>::getInitedMap(const char *mapName) {
    std::string name;
    name.assign(mapName);
    IMIT imIt = std::find(initedMapList.begin(), initedMapList.end(), name);
    if(imIt == initedMapList.end()) {
        printf("%s: No match inited map\n", __func__);
        return NULL;
    } else {
        return MapType::getMap(mapName);
    }
}

template<typename HandleMethod>
DataManagement<HandleMethod>::DataManagement() {
}

template<typename HandleMethod>
bool DataManagement<HandleMethod>::initialMapJunc(const char *mapName) {
    RecordFile rFile;
    char path[32];
    sprintf(path, "saves/%s/junc.r", mapName);
    try {
        rFile.openFile(path, O_RDWR | O_TRUNC);
        //restore data from record file...
    } catch(FileNotFound &e) {
        std::cout << e.what() << std::endl;
        //get raw data from database and format it
        return false;
    } catch(OpenFileFailed &e) {
        std::cout << e.what() << std::endl;
        return false;
    }

    return true;
}

template<typename HandleMethod>
bool DataManagement<HandleMethod>::initialMapRoad(const char *mapName) {
    RecordFile rFile;
    char path[32];
    sprintf(path, "saves/%s/road.r", mapName);
    try {
        rFile.openFile(path, O_RDWR | O_TRUNC);
        //restore data from record file...
    } catch(FileNotFound &e) {
        std::cout << e.what() << std::endl;
        return false;
    }

    return true;
}

template<typename HandleMethod>
bool DataManagement<HandleMethod>::initialMapFromInitedData(const char *mapName) {
    std::cout << "Initialize map from inited data stored in database.." << std::endl;
    try {
        DB::DBConnection dbConn("mapdata", "127.0.0.1");
        DB::QueryResult qRet;
        char cmd[256];

        //init juncs
        sprintf(cmd, "SELECT id, ST_AsText(point) FROM %s_Junc", mapName);
        printf("Initialing junctions...");
        if(!dbConn.query(cmd, qRet)) return false;
        MapType *mapIns = MapType::getMap(std::string(mapName));
        BASICTYPE::Junc tmpJunc;
        BASICTYPE::IdxPoint tmpPoint, tmpPoint2;
        while(qRet.next()) {
            dbUtilOPs.rawPoint2Point(qRet.value(1).toString().toStdString(), tmpPoint.point);
            tmpJunc.initJunc(qRet.value(0).toUInt(), tmpPoint.point.first, tmpPoint.point.second);
            mapIns->insertJunc(tmpPoint, tmpJunc);
        }
        printf(" OK!!\n");

        //init roads and relations
        sprintf(cmd, "SELECT R.id, ST_AsText(J1.point), "
                "ST_AsText(J2.point), R.oneway, ST_AsText(points), "
                "length, waytype from %s_Junc J1, %s_Junc J2, "
                "%s_Road R where J1.id = R.pointaid "
                "and J2.id = R.pointbid order by R.id"
                , mapName, mapName, mapName);
        if(!dbConn.query(cmd, qRet)) return false;
        BASICTYPE::Road tmpRoad, *road;
        BASICTYPE::Junc *juncA, *juncB;
        std::vector<BASICTYPE::IdxPoint> pointList;
        std::vector<BASICTYPE::IdxPoint>::iterator pItr;
        int oneway;
        printf("Initialing roads and relations...");
        while(qRet.next()) {
            dbUtilOPs.rawPoint2Point(qRet.value(1).toString().toStdString()
                                     , tmpPoint.point);
            dbUtilOPs.rawPoint2Point(qRet.value(2).toString().toStdString()
                                     , tmpPoint2.point);
            juncA = mapIns->findJunc(tmpPoint);
            juncB = mapIns->findJunc(tmpPoint2);
            if(juncA == NULL || juncB == NULL) {
                std::cout << "junc not found" << std::endl;
            }
            tmpRoad.initRoad(qRet.value(0).toString().toStdString()
                             , tmpPoint, juncA
                             , tmpPoint2, juncB);
            road = mapIns->insertRoad(tmpRoad.getID(), tmpRoad);
            road->setLength(qRet.value(5).toDouble());
            road->setWaytype(qRet.value(6).toString().toStdString());
            //initial point list of a road
            dbUtilOPs.rawLine2PointList(qRet.value(4).toString().toStdString()
                                        , pointList);
            for(pItr = pointList.begin(); pItr != pointList.end(); ++pItr) {
                road->insertAPoint(pItr->point, false);
            }
            pointList.clear();
            //initial relations
            oneway = qRet.value(3).toInt();
            if(oneway == 1) {
                juncA->attachARoad(tmpRoad.getID(), road);
                road->setOneway(1);
            } else if(oneway == -1) {
                juncB->attachARoad(tmpRoad.getID(), road);
                road->setOneway(-1);
            } else {
                juncA->attachARoad(tmpRoad.getID(), road);
                juncB->attachARoad(tmpRoad.getID(), road);
                road->setOneway(0);
            }
        }
        printf(" OK!!\n");
        /*Debug Info
        mapIns->showAllRoads();
        */
    } catch(ConnectionFailed &e) {
        std::cout << e.what() << std::endl;
        return false;
    }

    initedMapList.push_back(std::string(mapName));
    //MapType::getMap(std::string(mapName))->saveMap2JsonFile(mapName);
    return true;
}

template<typename HandleMethod>
bool DataManagement<HandleMethod>::initialMapFromRawData(const char *mapName) {
    /*
    std::cout << "Initialize map from raw data.." << std::endl;
    try {
        dbConn = new DB::DBConnection("postgis", "192.168.1.191");
        {
            DB::QueryResult qRet;
            dbConn->query(GETMAPDATA, qRet);
    */
            std::string name;
            name.assign(mapName);

            if(dataHandle(mapName)) {
                initedMapList.push_back(name);

                MapType *myMap = getInitedMap(mapName);
                myMap->saveMap2JsonFile(mapName);

                return true;
            } else {
                return false;
            }
/*
                myMap->showAllJuncs();
                getchar();
                myMap->showAllRoads();

            }
        }
    } catch(ConnectionFailed &e) {
        std::cout << e.what() << std::endl;
        delete dbConn;
        dbConn = NULL;
        return false;
    }

    delete dbConn;
    dbConn = NULL;

    return true;
*/
}

template<typename HandleMethod>
DataManagement<HandleMethod> * DataManagement<HandleMethod>::dmIns = NULL;

#endif // DATAMANAGEMENT_H
