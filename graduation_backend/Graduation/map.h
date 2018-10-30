#ifndef MAP_H
#define MAP_H

#include "basicmapdatatype.h"
#include "recordfile.h"

template<typename J, typename R>
class Map
{
    typedef std::map<std::string, Map *> RegisterList;
    typedef typename RegisterList::iterator RLIT;
public:
    typedef std::map<BASICTYPE::IdxPoint ,J> Juncs;
    typedef typename Juncs::iterator JIt;
    typedef std::map<BASICTYPE::IdxRoad ,R> Roads;
    typedef typename Roads::iterator RIt;

    ~Map();

    static Map *getMap(const std::string &name)  {
        RLIT rlIt = registerList.find(name);
        if(rlIt == registerList.end()) {
            mapIns = new Map<J, R>(name);
        } else {
            mapIns = rlIt->second;
        }
        return mapIns;
    }

    void saveMap2JsonFile(const char *name) {
        saveJunc2JsonFile(name);
        saveRoad2JsonFile(name);
    }

    void removeIslends() {
        for(JIt itr = junctions.begin(); itr != junctions.end(); ) {
            if((itr->second).isIsland()) {
                std::cout << "Island " << (itr->second).getId()
                          << " is removed." << std::endl;
                JIt tmp = itr;
                ++itr;
                junctions.erase(tmp);
            } else {
                ++itr;
            }
        }
    }

    J * insertJunc(const BASICTYPE::IdxPoint &idxPoint, J &junc) {
        junctions.insert(std::make_pair(idxPoint, junc));
        return &(junctions[idxPoint]);
    }

    bool removeJunc(const BASICTYPE::IdxPoint &idxPoint) {return false;}

    J * findJunc(const BASICTYPE::IdxPoint &idxPoint);

    u_int getJuncCount() {
        return junctions.size();
    }

    void showAllJuncs();

    R * insertRoad(const BASICTYPE::IdxRoad idxRoad, R &road) {
        roads.insert(std::make_pair(idxRoad, road));
        return &(roads[idxRoad]);
    }

    bool removeRoad(BASICTYPE::IdxRoad idxRoad) {return false;}

    R * findRoadByEndPoint(J &junca, J &juncb);

    R * findRoadByIdx(const BASICTYPE::IdxRoad &idxRoad) {
        if(roads.find(idxRoad) == roads.end()) return NULL;
        /*debug info
        if(idxRoad == "101967988A001") {
            printf("%s[%s(), line %d]: ", __FILE__, __func__, __LINE__);
            roads[idxRoad].showEndPoint();
        }
        */
        return &(roads[idxRoad]);
    }

    void updateRoadInfo(const BASICTYPE::IdxRoad &idxRoad, BASICTYPE::RoadInfo &roadInfo) {
        R * road = findRoadByIdx(idxRoad);
        if(road != NULL) {
            road->updateRoadInfo(roadInfo);
        }
    }

    u_int getRoadCount() {
        return roads.size();
    }

    void showAllRoads();

    JIt juncListBegin() {
        return junctions.begin();
    }

    JIt juncListEnd() {
        return junctions.end();
    }

    RIt roadListBegin() {
        return roads.begin();
    }

    RIt roadListEnd() {
        return roads.end();
    }

private:
    Map();
    Map(std::string name) : mapName(name) {
        registerList[name] = this;
    }
    void saveJunc2JsonFile(const char *name);
    void saveRoad2JsonFile(const char *name);

    std::string mapName;
    static RegisterList registerList;
    static Map *mapIns;
    Juncs junctions;
    Roads roads;
};

template<typename J, typename R>
Map<J, R>::Map() {}

template<typename J, typename R>
Map<J, R>::~Map() {
    junctions.clear();
    roads.clear();
    registerList.erase(mapName);
}

template<typename J, typename R>
J * Map<J, R>::findJunc(const BASICTYPE::IdxPoint &idxPoint) {
    if(junctions.find(idxPoint) == junctions.end()) {
        std::cout << idxPoint.point.first << " "
                     << idxPoint.point.second << std::endl;
        std::cout << getJuncCount() << std::endl;
        return NULL;
    }
    return &(junctions[idxPoint]);
}

template<typename J, typename R>
void Map<J, R>::showAllJuncs() {
    std::cout << "All junctions: " << std::endl;
    for(JIt jit = junctions.begin(); jit != junctions.end(); ++jit) {
        std::cout << (jit->second).toJsonString() << std::endl;
    }
}

template<typename J, typename R>
R * Map<J, R>::findRoadByEndPoint(J &junca, J &juncb) {
    for(typename J::AdRoadsItor rIt = junca.adRoadsBegin()
        ; rIt != junca.adRoadsEnd()
        ; ++rIt) {
        if(&juncb == (rIt->second)->getOtherJunc(junca))
            return rIt->second;
    }
    return NULL;
}

template<typename J, typename R>
void Map<J, R>::showAllRoads() {
    std::cout << "All roads: " << std::endl;
    for(RIt rit = roads.begin(); rit != roads.end(); ++rit) {
        std::cout <<(rit->second).toJsonString() << std::endl;
    }
}

template<typename J, typename R>
void Map<J, R>::saveJunc2JsonFile(const char *name) {
    std::cout << "junctions: " << junctions.size() << std::endl;
    RecordFile rFile;
    char path[32];
    sprintf(path, "saves/%s/junc.json", name);
    try {
        rFile.openFile(path, O_CREAT | O_WRONLY | O_TRUNC);
        rFile >> "{\"contentListLength\": " >> junctions.size()
              >> ", \"contentList\": [";
        JIt jit = junctions.begin();
        rFile >> (jit->second).toJsonString();
        for(++jit; jit != junctions.end(); ++jit) {
            rFile >> "," >> (jit->second).toJsonString();
        }
        rFile >> "]}";
    } catch(FileNotFound &e) {
        std::cout << e.what() << std::endl;
    } catch(OpenFileFailed &e) {
        std::cout << e.what() << std::endl;
        return ;
    }
}

template<typename J, typename R>
void Map<J, R>::saveRoad2JsonFile(const char *name) {
    std::cout << "roads: " << roads.size() << std::endl;
    RecordFile rFile;
    char path[32];
    sprintf(path, "saves/%s/road.json", name);
    try {
        rFile.openFile(path, O_CREAT | O_WRONLY | O_TRUNC);
        rFile >> "{\"contentListLength\": " >> roads.size()
              >> ", \"contentList\": [";
        RIt rit = roads.begin();
        rFile >> (rit->second).toJsonString();
        for(++rit; rit != roads.end(); ++rit) {
            rFile >> "," >> (rit->second).toJsonString();
        }
        rFile >> "]}";
    } catch(FileNotFound &e) {
        std::cout << e.what() << std::endl;
    } catch(OpenFileFailed &e) {
        std::cout << e.what() << std::endl;
        return ;
    }
}

template<typename J, typename R>
typename Map<J, R>::RegisterList Map<J, R>::registerList;

template<typename J, typename R>
Map<J, R> *Map<J, R>::mapIns = NULL;

#endif // MAP_H
