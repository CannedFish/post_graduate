#ifndef ROUTEPLAN_H
#define ROUTEPLAN_H

#include "common.h"
#include "routestrategys.h"
#include "evaluationfunctions.h"
#include "mapiterator.h"

template<typename Strategy
         = RouteStrategy::HeuristicApproach<
             BASICTYPE::Junc,
             BASICTYPE::Road,
             EvFunc::SimpleDistanceG,
             EvFunc::SimpleDistanceEv<BASICTYPE::Junc> > >
class RoutePlan {
    typedef std::list<std::string> RouteList;
    typedef RouteList::iterator RouteListItor;
public:
    std::string getPlan(const char *mapName, BASICTYPE::Point &start, BASICTYPE::Point &end);
private:
    std::string retJsonString();

    RouteList routeList;
};

/////////////////////////////////////////// implementation /////////////////////////////////////////
template<typename Strategy>
std::string RoutePlan<Strategy>::getPlan(const char *mapName, BASICTYPE::Point &start, BASICTYPE::Point &end) {
    DataManagement<> *dm = DataManagement<>::getInstance();
    //DataManagement<MapDataHandleMethod::GetMapTOPOFromDB> *dm = DataManagement<MapDataHandleMethod::GetMapTOPOFromDB>::getInstance();
    MapType *map = dm->getInitedMap(mapName);
    if(map == NULL) {
        printf("%s: this has not initialized.\n", __func__);
        return "";
    }

    MapIterator<BASICTYPE::Junc, Strategy, MapType> mapItor(map, map->findJunc(start), map->findJunc(end));
    printf("Now begin to get route plan!!\n");
    u_int steps = 0;
    for( ; !mapItor.isDone(); ++mapItor, ++steps) {
        //printf("Junction ID: %d\n", (*mapItor).getId());
    }
    printf("Finished with finding route plan by searching \033[1;31m%u\033[0m juncs totally.\n", steps);
    mapItor.getRoute(routeList);
    printf("Finished with getting route plan. The route steps' length is \033[1;31m%lu\033[0m.\n", routeList.size());
    return retJsonString();
}

template<typename Strategy> inline
std::string RoutePlan<Strategy>::retJsonString() {
    if(routeList.size() == 0) {
        return "{\"status\": \"Error\", "
                "\"content\": \"no route between this two points.\"}";
    }
    std::string ret("");
    ret += "{\"status\": \"OK\", \"contentList\": [";
    RouteListItor itor = routeList.begin();
    ret += "\"" + *itor + "\"";
    for( ++itor; itor != routeList.end(); ++itor) {
        ret += ", \"" + *itor + "\"";
    }
    ret += "]}\n";
    /*
    RecordFile rFile;
    try {
        rFile.openFile("routes.json", O_CREAT | O_WRONLY | O_TRUNC | O_APPEND);
        rFile >> ret >> "\n";
    } catch(FileNotFound &e) {
        std::cout << e.what() << std::endl;
    } catch(OpenFileFailed &e) {
        std::cout << e.what() << std::endl;
    }
    */
    return ret;
}

#endif // ROUTEPLAN_H
