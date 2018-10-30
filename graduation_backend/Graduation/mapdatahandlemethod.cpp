#include "mapdatahandlemethod.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MapDataHandleMethod::DBDataHandleOperation::rawLine2PointList(const std::string &_rawLine, std::vector<Point> &ret) {
    //std::cout << rawLine << std::endl;
    std::string rawLine(_rawLine.substr(11, _rawLine.length() - 12));
    //The rawLine is without "LINESTRING(" and ")"
    QString str(rawLine.c_str());
    QStringList strList = str.split(','), tmp;
    Point tmpPoint;
    for(QStringList::iterator qIt = strList.begin(); qIt != strList.end(); ++qIt) {
        tmp = qIt->split(' ');
        tmpPoint.point.first = tmp[0].toStdString();
        tmpPoint.point.second = tmp[1].toStdString();
        ret.push_back(tmpPoint);
        //std::cout << tmpPoint.point.first << " " << tmpPoint.point.second << std::endl;
    }
}

void MapDataHandleMethod::DBDataHandleOperation::pointList2RawLine(const PointList &pointList, std::string &rawLine) {
    rawLine = "LINESTRING(";
    typedef PointList::const_iterator PIter;
    for(PIter pIter = pointList.begin(); pIter != pointList.end(); ++pIter) {
        rawLine += (pIter->first + " " + pIter->second + ",");
    }
    rawLine[rawLine.length() - 1] = ')';
}

bool MapDataHandleMethod::DBDataHandleOperation::createJuncTable(DB::DBConnection *dbSave, DB::QueryResult &qRet, const char *mapName) {
    char cmd[128];
    sprintf(cmd, "CREATE TABLE %s_Junc ("
            "id bigint NOT NULL PRIMARY KEY, "
            "point geometry(POINT, 900913))", mapName);
    return dbSave->query(cmd, qRet);
}

bool MapDataHandleMethod::DBDataHandleOperation::createRoadTable(DB::DBConnection *dbSave, DB::QueryResult &qRet, const char *mapName) {
    char cmd[256];
    sprintf(cmd, "CREATE TABLE %s_Road ("
            "id text NOT NULL PRIMARY KEY,"
            "pointAid bigint,"
            "pointBid bigint,"
            "avgSpeed double precision,"
            "trafficTrend double precision,"
            "length double precision,"
            "oneway int,"
            "points geometry(LINESTRING, 900913),"
            "mark double precision,"
            "waytype text)", mapName);
    return dbSave->query(cmd, qRet);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define GETMAPDATA "SELECT osm_id, ST_AsText(ST_Transform(way, 900913)), oneway, highway, z_order FROM planet_osm_line WHERE highway IS NOT null AND railway IS null AND highway NOT IN ('construction', 'steps', 'cycleway', 'path', 'birdleway', 'footway', 'pedestrian')"
//#define ZORDER

MapDataHandleMethod::ReverseIndexLike::~ReverseIndexLike() {
    if(dbConn != NULL) {
        delete dbConn;
        dbConn = NULL;
    }
}

bool MapDataHandleMethod::ReverseIndexLike::operator ()(const char *mapName) {
    std::cout << "Initialize map from raw data.." << std::endl;
    try {
        dbConn = new DB::DBConnection("myrouting", "127.0.0.1");
        {
            DB::QueryResult qRet;
            dbConn->query(GETMAPDATA, qRet);

            std::string name;
            name.assign(mapName);
            if(!dataHandle(qRet, MapType::getMap(name))) {
                delete dbConn;
                dbConn = NULL;
                return false;
            } else {
                //store initialed data to database
                DB::DBConnection dbSave("mapdata", "127.0.0.1");
                saveMapDataToDB(&dbSave, mapName, MapType::getMap(name));
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
}

bool MapDataHandleMethod::ReverseIndexLike::dataHandle(DB::QueryResult &rawData, MapType *mapIns) {
    SLMAP rawMap;// osm_id -> lineinfo
    SIJMAP roadMap;// osm_id' -> points
    {
        ISMAP juncMap;// point -> osm_ids
        while(rawData.next()) {
            /*std::cout << rawData.value(0).toString().toStdString() << " "
                          << rawData.value(1).toString().toStdString() << " "
                      << rawData.value(2).toString().toStdString() << std::endl;*/
            //if has some problems, it has to make osmId unique. need change!!
            std::string osmId;
            generateAUniqueRoadID(rawMap, rawData.value(0).toString().toStdString(), osmId);
            std::string lineString = rawData.value(1).toString().toStdString();
            DBop.rawLine2PointList(lineString, rawMap[osmId].roadPoints);
            rawMap[osmId].oneway = rawData.value(2).toString().toStdString();
            rawMap[osmId].highway = rawData.value(3).toString().toStdString();
            rawMap[osmId].z_order = rawData.value(4).toInt();

            //std::cout << osmId << ": ";
            for(LIJIT itr = rawMap[osmId].roadPoints.begin()
                ; itr != rawMap[osmId].roadPoints.end()
                ; ++itr) {
                //std::cout << "<" << (itr->point).first << ", "
                 //         << (itr->point).second << ">, ";
                juncMap[*itr].push_back(osmId);
            }
            //std::cout << std::endl;

            roadMap[osmId].insert(rawMap[osmId].roadPoints[0]);
            insertJunc(mapIns, rawMap[osmId].roadPoints[0]);
            roadMap[osmId].insert(rawMap[osmId].roadPoints[rawMap[osmId].roadPoints.size() - 1]);
            insertJunc(mapIns, rawMap[osmId].roadPoints[rawMap[osmId].roadPoints.size() - 1]);
        }
        rawData.clear();
        //scan juncMap and insert junc to roadMap
        //std::cout << "junction map: " <<juncMap.size() << std::endl;
        for(ISMAPIT itr = juncMap.begin(); itr != juncMap.end(); itr++) {
            /* print key
            std::cout << "<" << (itr->first).point.first
                      << ", " << (itr->first).point.second
                      << ">: ";
                      */
            LS &tmpLS = itr->second;
            //std::cout << "[" << tmpLS.size() << "] ";
            if(tmpLS.size() > 1) {
                initJunctions(mapIns, itr, tmpLS, rawMap, roadMap);
            }
            //printf("\n");
        }
    }//release juncMap
    //init road by rawMap and roadMap
    //std::cout << "Road map: " << roadMap.size() << std::endl;
    initRoadAndTies(mapIns, rawMap, roadMap);

    //mapIns->removeIslends();
    std::cout << "Map initialized OK" << std::endl;
    //mapIns->removeIslends();

    return true;
}

void MapDataHandleMethod::ReverseIndexLike::initJunctions(MapType *mapIns, ISMAPIT &subJunc, LS &roadIDs, SLMAP &rawMap, SIJMAP &roadMap) {
#ifndef ZORDER
    for(LSIT its = roadIDs.begin(); its != roadIDs.end(); ++its) {
        //std::cout << *its << ", ";
        roadMap[*its].insert(subJunc->first);
        //std::cout << rawMap[*its].z_order << " ";
    }
    insertJunc(mapIns, subJunc->first);
    //std::cout << mapIns->findJunc(subJunc->first)->getId() << std::endl;
#else
    memset(juncVisited, 0, sizeof(juncVisited));
    bool isJunc;

    for(u_int i = 0; i < roadIDs.size(); ) {
        isJunc = false;
        juncVisited[i] = 1;

        for(u_int j = i + 1; j < roadIDs.size(); ++j) {
            if(juncVisited[j] == 0) {
                if(rawMap[roadIDs[i]].z_order == rawMap[roadIDs[j]].z_order) {
                    juncVisited[j] = 1;
                    roadMap[roadIDs[j]].insert(subJunc->first);
                    isJunc = true;
                }
            }
        }
        if(isJunc) {
            roadMap[roadIDs[i]].insert(subJunc->first);
            insertJunc(mapIns, subJunc->first);
        }

        for(u_int j = i + 1; j < sizeof(juncVisited); ++j) {
            if(juncVisited[j] == 0) {
                i = j;
                break;
            }
        }
    }
#endif
}

void MapDataHandleMethod::ReverseIndexLike::generateAUniqueRoadID(SLMAP &rawMap, const std::string &oldID, std::string &uniqueID) {
    uniqueID = oldID + "A";
    while(rawMap.find(uniqueID) != rawMap.end()) {
        uniqueID[uniqueID.length() - 1] += 1;
    }
}
/*
void MapDataHandleMethod::ReverseIndexLike::rawLine2PointList(const std::string &rawLine, LIJ &ret) {
    //std::cout << rawLine << std::endl;
    QString str(rawLine.c_str());
    QStringList strList = str.split(','), tmp;
    IdxJunc tmpPoint;
    for(QStringList::iterator qIt = strList.begin(); qIt != strList.end(); ++qIt) {
        tmp = qIt->split(' ');
        tmpPoint.point.first = tmp[0].toStdString();
        tmpPoint.point.second = tmp[1].toStdString();
        ret.push_back(tmpPoint);
        //std::cout << tmpPoint.point.first << " " << tmpPoint.point.second << std::endl;
    }
}
*/
inline void MapDataHandleMethod::ReverseIndexLike::insertJunc(MapType *mapIns, const IdxJunc &junc) {
    tmpJunc.initJunc(juncNum++, junc.point.first, junc.point.second);
    mapIns->insertJunc(junc, tmpJunc);
}

void MapDataHandleMethod::ReverseIndexLike::initRoadAndTies(MapType *mapIns, SLMAP &rawMap, SIJMAP &roadMap) {
    SIJIT sItor;
    LIJIT lItor, preItor;
    for(SIJMAPIT itr = roadMap.begin(); itr != roadMap.end(); ++itr) {
        SIJ &tmpJuncs = const_cast<SIJ &>(itr->second);
        LineInfo &tmpPoints = rawMap[itr->first];
        /*debug Info
        if(itr->first == "107278030A")
            printf("length: %lu, %lu\n", tmpJuncs.size(), tmpPoints.roadPoints.size());
        */
        preItor = tmpPoints.roadPoints.end();
        char subID[4];
        int subId = 0;
        for(lItor = tmpPoints.roadPoints.begin()
            ; lItor != tmpPoints.roadPoints.end()
            ; ++lItor) {
            if((sItor = tmpJuncs.find(*lItor)) != tmpJuncs.end()) {
                if(preItor == tmpPoints.roadPoints.end()) {
                    preItor = lItor;
                    continue;
                }
                //initialize a road
                BASICTYPE::Junc *juncA = mapIns->findJunc(*preItor);
                BASICTYPE::Junc *juncB = mapIns->findJunc(*lItor);
                if(juncA == NULL || juncB == NULL) {
                    std::cout << "junc not found" << std::endl;
                }
                /*debug Info
                if(juncA->getId() == 4115 || juncB->getId() == 4115) {
                    printf("%u, %u\n", juncA->getId(), juncB->getId());
                    printf("roadId: %s\n", (itr->first).c_str());
                }*/

                //the road id need change to unique!!!!
                sprintf(subID, "%03X", subId);
                std::string id(itr->first + subID);
                subId++;
                tmpRoad.initRoad(id, *preItor, juncA, *lItor, juncB);
                BASICTYPE::Road *road = mapIns->insertRoad(id, tmpRoad);
                for(LIJIT tmpItor = preItor; tmpItor != lItor; ++tmpItor) {
                    road->insertAPoint(tmpItor->point);
                }
                road->insertAPoint(lItor->point);
                preItor = lItor;
                //std::cout << "OK" << std::endl;
                road->setWaytype(tmpPoints.highway);
                //tie this road to cooresponding junction
                if(tmpPoints.oneway == "yes" || tmpPoints.oneway == "1") {
                    juncA->attachARoad(id, road);
                    road->setOneway(1);

                    /*debug Info
                    if(juncB->getId() == 4115) {
                        if(road->getOtherJunc(*juncB) == NULL) printf("FUCK\n");
                    }
                    */
                } else if(tmpPoints.oneway == "-1") {
                    juncB->attachARoad(id, road);
                    road->setOneway(-1);
                } else {
                    road->setOneway(0);
                    juncA->attachARoad(id, road);
                    /*
                    if(tmpPoints.highway == "unclassified"
                            || tmpPoints.highway == "residential"
                            || tmpPoints.highway == "service")
                            */
                        juncB->attachARoad(id, road);
                }
            }// end if((sItor = tmpJuncs.find(*lItor)) != tmpJuncs.end())
        }// end for
    }// end for
}

bool MapDataHandleMethod::ReverseIndexLike::saveMapDataToDB(DB::DBConnection *dbSave, const char *mapName, MapType *mapIns) {
    DB::QueryResult qRet;
    char values[4096];
    std::string rawData;
    u_int num, sum;
/**/
    //build a new junction data table based on the map name
    if(!DBop.createJuncTable(dbSave, qRet, mapName)) {
        printf("Fail to create a database table at file %s, line %d.\n", __FILE__, __LINE__);
        return false;
    }
    //save junction data
    printf("Saving junction data to database...\033[s  0%%");
    num = 0;
    sum = mapIns->getJuncCount();
    dbSave->query("BEGIN TRANSACTION", qRet);
    for(MapType::JIt itor = mapIns->juncListBegin(); itor != mapIns->juncListEnd(); ++itor) {
        DBop.point2rawPoint((itor->second).getCoor(), rawData);
        sprintf(values, "INSERT INTO %s_Junc Values (%u, GeomFromEWKT('SRID=900913; %s'))", mapName, (itor->second).getId(), rawData.c_str());
        dbSave->query(values, qRet);
        printf("\033[u\033[s%4d%%", ++num * 100 / sum);
    }
    printf("\ncommitting...");
    dbSave->query("COMMIT", qRet);
    printf(" OK\n");

    //build a new road data table based on the map name
    if(!DBop.createRoadTable(dbSave, qRet, mapName)) {
        printf("Fail to create a database table at file %s, line %d.\n", __FILE__, __LINE__);
        return false;
    }
    //save road data
    printf("Saving road data to database...\033[s  0%%");
    u_int pIDa, pIDb;
    num = 0;
    sum = mapIns->getRoadCount();
    dbSave->query("BEGIN TRANSACTION", qRet);
    for(MapType::RIt itor = mapIns->roadListBegin(); itor != mapIns->roadListEnd(); ++itor) {
        DBop.pointList2RawLine((itor->second).getRawLine(), rawData);
        (itor->second).getEndPointID(pIDa, pIDb);
        sprintf(values, "INSERT INTO %s_Road "
                //"(id, pointAid, pointBid, oneway, length, points) "
                "Values ('%s', %u, %u, 40.0, 0.0, %lf, %d, "
                "GeomFromEWKT('SRID=900913; %s'), 0.0, '%s')"
                , mapName, (itor->second).getID().c_str()
                , pIDa, pIDb, (itor->second).getLength()
                , (itor->second).getOneway(), rawData.c_str()
                , (itor->second).getWaytype().c_str());
        dbSave->query(values, qRet);
        printf("\033[u\033[s%4d%%", ++num * 100 / sum);
    }
    printf("\ncommitting...");
    dbSave->query("COMMIT", qRet);
    printf(" OK\n");

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define GETVERTICES "SELECT id, ST_AsText(the_geom) FROM edges_vertices_pgr"
#define GETTOPO "SELECT id, ST_AsText(the_geom), len_km, x1, y1, x2, y2 FROM edges ORDER BY id"

bool MapDataHandleMethod::GetMapTOPOFromDB::operator ()(const char *mapName) {
    printf("Initialize map topology from DB..\n");
    try {
        dbConn = new DB::DBConnection("myrouting", "127.0.0.1");
        {
            DB::QueryResult qRet;
            dbConn->query(GETVERTICES, qRet);

            std::string name(mapName);
            if(!initJuncs(qRet, MapType::getMap(name))) {
                delete dbConn;
                dbConn = NULL;
                return false;
            }

            dbConn->query(GETTOPO, qRet);
            if(!initTopo(qRet, MapType::getMap(name))) {
                delete dbConn;
                dbConn = NULL;
                return false;
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
}

bool MapDataHandleMethod::GetMapTOPOFromDB::initJuncs(DB::QueryResult &juncData, MapType *mapIns) {
    QStringList tmp;
    BASICTYPE::IdxPoint tmpPoint;
    while(juncData.next()) {
        std::string str1(juncData.value(1).toString().toStdString());
        QString str(str1.substr(6, str1.length() - 7).c_str());
        tmp = str.split(' ');
        tmpPoint.point.first = tmp[0].toStdString();
        tmpPoint.point.second = tmp[1].toStdString();
        tmpJunc.initJunc(juncData.value(0).toInt(), tmpPoint.point.first, tmpPoint.point.second);
        mapIns->insertJunc(tmpPoint, tmpJunc);
    }
    juncData.clear();

    return true;
}

bool MapDataHandleMethod::GetMapTOPOFromDB::initTopo(DB::QueryResult &topoData, MapType *mapIns) {
    QStringList tmp, tmp2;
    BASICTYPE::IdxPoint tmpPointA, tmpPointB;
    while(topoData.next()) {
        tmpPointA.point.first = topoData.value(3).toString().toStdString();
        tmpPointA.point.second = topoData.value(4).toString().toStdString();
        BASICTYPE::Junc *juncA = mapIns->findJunc(tmpPointA);
        tmpPointB.point.first = topoData.value(5).toString().toStdString();
        tmpPointB.point.second = topoData.value(6).toString().toStdString();
        BASICTYPE::Junc *juncB = mapIns->findJunc(tmpPointB);
        if(juncA == NULL || juncB == NULL) {
            printf("Junc not found\n");
        }

        std::string id(topoData.value(0).toString().toStdString());
        tmpRoad.initRoad(id, tmpPointA, juncA, tmpPointB, juncB);
        BASICTYPE::Road *road = mapIns->insertRoad(id, tmpRoad);
        juncA->attachARoad(id, road);

        std::string stdStr(topoData.value(1).toString().toStdString());
        QString str(stdStr.substr(11, stdStr.length() - 12).c_str());
        tmp = str.split(',');
        for(QStringList::iterator qIt = tmp.begin(); qIt != tmp.end(); ++qIt) {
            tmp2 = qIt->split(' ');
            tmpPointA.point.first = tmp2[0].toStdString();
            tmpPointA.point.second = tmp2[1].toStdString();
            road->insertAPoint(tmpPointA.point, false);
        }

        roadInfo.length = topoData.value(2).toDouble();
        road->updateRoadInfo(roadInfo);
    }

    return true;
}
