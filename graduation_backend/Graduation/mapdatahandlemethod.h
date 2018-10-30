#ifndef MAPDATAHANDLEMETHOD_H
#define MAPDATAHANDLEMETHOD_H

#include "common.h"
#include "map.h"
#include "dbconnection.h"

namespace MapDataHandleMethod
{
    typedef Map<BASICTYPE::Junc, BASICTYPE::Road> MapType;
    typedef BASICTYPE::IdxPoint IdxJunc;

    struct DBDataHandleOperation {
        typedef IdxJunc Point;
        typedef BASICTYPE::RawLine PointList;

        void rawLine2PointList(const std::string &rawLine, std::vector<IdxJunc> &ret);
        void pointList2RawLine(const PointList &pointList, std::string &rawLine);

        void rawPoint2Point(const std::string &_rawPoint, BASICTYPE::Point &point) {
            std::string rawPoint(_rawPoint.substr(6, _rawPoint.length() - 7));
            //The rawPoint is without "POINT(" and ")"
            size_t splitor = rawPoint.find_first_of(' ');
            point.first = rawPoint.substr(0, splitor);
            point.second = rawPoint.substr(splitor + 1);
        }

        void point2rawPoint(const BASICTYPE::Point &point, std::string &rawPoint) {
            rawPoint = "POINT(" + point.first + " " + point.second + ")";
        }

        bool createJuncTable(DB::DBConnection *dbSave, DB::QueryResult &qRet, const char *mapName);
        bool createRoadTable(DB::DBConnection *dbSave, DB::QueryResult &qRet, const char *mapName);
    };

    class ReverseIndexLike {
        typedef std::vector<IdxJunc> LIJ;
        typedef LIJ::iterator LIJIT;
        struct LineInfo {
            LIJ roadPoints;
            std::string oneway;
            std::string highway;
            int z_order;
        };

        typedef std::map<std::string, LineInfo> SLMAP;//osm_id -> lineinfo
        typedef SLMAP::const_iterator SLMAPIT;

        typedef std::vector<std::string> LS;
        typedef LS::iterator LSIT;

        typedef std::map<IdxJunc, LS> ISMAP;//point -> osm_ids
        typedef ISMAP::iterator ISMAPIT;

        typedef std::set<IdxJunc> SIJ;
        typedef SIJ::iterator SIJIT;

        typedef std::map<std::string, SIJ> SIJMAP;//osm_id' -> points
        typedef SIJMAP::iterator SIJMAPIT;

    public:
        ReverseIndexLike() : juncNum(0), dbConn(NULL) {}
        ~ReverseIndexLike();
        bool operator ()(const char *mapName);

    private:
        bool dataHandle(DB::QueryResult &rawData, MapType *mapIns);
        void initJunctions(MapType *mapIns, ISMAPIT &subJunc, LS &roadIDs, SLMAP &rawMap, SIJMAP &roadMap);
        void generateAUniqueRoadID(SLMAP &rawMap, const std::string &oldID, std::string &uniqueID);
        //void rawLine2PointList(const std::string &rawLine, LIJ &ret);
        void insertJunc(MapType *mapIns, const IdxJunc &junc);
        void initRoadAndTies(MapType *mapIns, SLMAP &rawMap, SIJMAP &roadMap);
        bool saveMapDataToDB(DB::DBConnection *dbSave, const char *mapName, MapType *mapIns);

        u_long juncNum;
        BASICTYPE::Junc tmpJunc;
        BASICTYPE::Road tmpRoad;
        DB::DBConnection *dbConn;
        u_char juncVisited[16];
        DBDataHandleOperation DBop;
    };

    class GetMapTOPOFromDB {//using pgRouting
    public:
        GetMapTOPOFromDB() : dbConn(NULL) {}
        ~GetMapTOPOFromDB() {
            if(dbConn != NULL) {
                delete dbConn;
                dbConn = NULL;
            }
        }

        bool operator ()(const char *mapName);
    private:
        bool initJuncs(DB::QueryResult &juncData, MapType *mapIns);
        bool initTopo(DB::QueryResult &topoData, MapType *mapIns);

        DB::DBConnection *dbConn;
        BASICTYPE::Junc tmpJunc;
        BASICTYPE::Road tmpRoad;
        BASICTYPE::RoadInfo roadInfo;
    };
}

#endif // MAPDATAHANDLEMETHOD_H
