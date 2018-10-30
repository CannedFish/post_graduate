#ifndef BASICMAPDATATYPE_H
#define BASICMAPDATATYPE_H

#include "common.h"

namespace BASICTYPE {
    typedef std::pair<std::string, std::string> Point;
    struct IdxPoint {
        IdxPoint() {}
        IdxPoint(const Point &other) : point(other) {}
        Point point;
        bool operator <(const IdxPoint &rhs) const {
            if(point.first == rhs.point.first)
                return point.second < rhs.point.second;
            else
                return point.first < rhs.point.first;
        }
        bool operator ==(const IdxPoint &rhs) const {
            return (point.first == rhs.point.first && point.second == rhs.point.second);
        }
    };
    typedef std::vector<Point> RawLine;
    typedef std::string IdxRoad;
    class Junc;
    class Road;
    typedef std::pair<IdxRoad, Road *> AdRoad;
    typedef std::pair<IdxPoint, Junc *> AdJunc;
    typedef std::vector<AdRoad> AdRoads;
    typedef AdJunc AdJuncs[2];

    struct RoadInfo {
        RoadInfo() : avgSpeed(0.0), trafficTrend(0.0), length(0.0) {}
        RoadInfo(double a, double t, double l) : avgSpeed(a), trafficTrend(t), length(l) {}
        RoadInfo & operator =(const RoadInfo &other) {
            length = other.length;
            avgSpeed = other.avgSpeed;
            trafficTrend = other.trafficTrend;
            return *this;
        }

        double avgSpeed, trafficTrend, length;
    };

    class Junc {
    public:
        typedef AdRoads::iterator AdRoadsItor;

    public:
        Junc();
        Junc(u_int jId, std::string x, std::string y);
        ~Junc();

        void initJunc(u_int jId, std::string x, std::string y) {
            id = jId;
            point = std::make_pair(x, y);
            adRoad.clear();
        }
        bool operator ==(const Junc &other) {
            return (point.first == other.point.first && point.second == other.point.second);
        }

        bool operator ==(const Point &other) {
            return (point.first == other.first && point.second == other.second);
        }

        void attachARoad(const IdxRoad &idxRoad, Road *road) {
            /*debug Info
            if(this->getId() == 7829) printf("%s\n", __func__);
            */
            adRoad.push_back(std::make_pair(idxRoad, road));
        }

        std::string toString() {
            return point.first + " " + point.second;
        }

        std::string toJsonString() {
            char tmp[64];
            sprintf(tmp, "{\"juncid\": \"%u\", \"coor\": \"POINT(", id);
            return std::string(tmp) + toString() + ")\"}";
        }
        //returen NULL, if no more adjuncs
        Junc * getNextAdjunc(u_int vIdx, RoadInfo &roadInfo);

        Point getCoor() const {return point;}

        bool isIsland() const {
            return (adRoad.size() == 0);
        }

        u_int getId() const {
            return id;
        }

        AdRoadsItor adRoadsBegin() {
            return adRoad.begin();
        }

        AdRoadsItor adRoadsEnd() {
            return adRoad.end();
        }

    private:
        u_int id;
        Point point;
        AdRoads adRoad;
    };

    class Road {
    public:
        Road();
        Road(std::string rId);
        ~Road();

        void initRoad(std::string rId
                      , const IdxPoint &idxPointA
                      , Junc *juncA
                      , const IdxPoint &idxPointB
                      , Junc *juncB);

        void setRoadId(std::string rId) {
            id = rId;
        }

        bool attachOneJunc(const IdxPoint &idxPoint, Junc *junc);

        Junc * getOtherJunc(const Junc &oneJunc) const;

        void insertAPoint(const Point &point, bool calcLength = true) {
            rawLine.push_back(point);

            if(calcLength) {
                if(rawLine.size() > 1) {
                    pB.first = atof(const_cast<const char *>(point.first.c_str()));
                    pB.second = atof(const_cast<const char *>(point.second.c_str()));
                    updateLength();
                    pA = pB;
                } else {
                    pA.first = atof(const_cast<const char *>(point.first.c_str()));
                    pA.second = atof(const_cast<const char *>(point.second.c_str()));
                }
            }
        }

        void updateRoadInfo(RoadInfo &other) {
            roadInfo.avgSpeed = other.avgSpeed;
            roadInfo.trafficTrend = other.trafficTrend;
        }

        void getRoadInfo(RoadInfo &roadInfo) const {
            roadInfo = this->roadInfo;
            /*Debug Info
            printf("%s: %lf, %lf, %lf\n", __func__, roadInfo.length, roadInfo.avgSpeed, roadInfo.trafficTrend);
            */
            return ;
        }

        std::string toString();
        std::string toJsonString();
        std::string getID() const {return id;}
        void showEndPoint() const {
            printf("%s's end points: %u %u\n", id.c_str()
                   , adJunc[0].second->getId()
                   , adJunc[1].second->getId());
        }

        const RawLine & getRawLine() const {
            return rawLine;
        }

        void getEndPointID(u_int &pIDa, u_int &pIDb) const {
            pIDa = adJunc[0].second->getId();
            pIDb = adJunc[1].second->getId();
        }

        void setOneway(int _oneway) {
            oneway = _oneway;
        }

        int getOneway() const {
            return oneway;
        }

        void setLength(double _length) {
            roadInfo.length = _length;
        }

        double getLength() const {
            return roadInfo.length;
        }

        void setWaytype(const std::string &_waytype) {
            waytype = _waytype;
        }

        const std::string & getWaytype() const {
            return waytype;
        }

    private:
        void updateLength() {
            double x(pB.first - pA.first), y(pB.second - pA.second);
            double l = sqrt(x * x + y * y);
            roadInfo.length += l;
        }

        std::string id;
        int oneway;
        std::string waytype;
        AdJuncs adJunc;
        RawLine rawLine;
        u_int juncNum;
        RoadInfo roadInfo;
        std::pair<double, double> pA, pB;
    };
}

#endif // BASICMAPDATATYPE_H
