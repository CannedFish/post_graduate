#include "basicmapdatatype.h"

BASICTYPE::Junc::Junc() : id(0) {}

BASICTYPE::Junc::Junc(u_int jId, std::string x, std::string y) : id(jId) {
    point = std::make_pair(x, y);
}

BASICTYPE::Junc::~Junc() {}

BASICTYPE::Junc * BASICTYPE::Junc::getNextAdjunc(u_int vIdx, BASICTYPE::RoadInfo &roadInfo) {//need change
    u_int len = adRoad.size();
    /*debug info
    printf("%u: %u, %u\n", getId(), vIdx, len);*/

    if(vIdx >= len) return NULL;
    BASICTYPE::Road *oneRoad = adRoad[vIdx].second;
    oneRoad->getRoadInfo(roadInfo);
    //printf("%lf\n", roadInfo.length);
    return oneRoad->getOtherJunc(*this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BASICTYPE::Road::Road() : juncNum(0), roadInfo(0.0, 0.0, 0.0) {}

BASICTYPE::Road::Road(std::string rId) : id(rId), juncNum(0), roadInfo(0.0, 0.0, 0.0) {}

BASICTYPE::Road::~Road() {}

void BASICTYPE::Road::initRoad(std::string rId
                               , const BASICTYPE::IdxPoint &idxPointA
                               , BASICTYPE::Junc *juncA
                               , const BASICTYPE::IdxPoint &idxPointB
                               , BASICTYPE::Junc *juncB) {
    id = rId;
    juncNum = 0;
    rawLine.clear();
    /*debug info
    if(juncA->getId() == 4115) {
        printf("%s[%s(), line %d]: %u %u\n", __FILE__, __func__, __LINE__
               , juncA->getId(), juncB->getId());
    }*/

    adJunc[juncNum++] = std::make_pair(idxPointA, juncA);
    adJunc[juncNum++] = std::make_pair(idxPointB, juncB);
}

bool BASICTYPE::Road::attachOneJunc(const BASICTYPE::IdxPoint &idxPoint, BASICTYPE::Junc *junc) {
    if(juncNum == 2) {
        printf("%s: Illigle junction: this road already has two end point!\n", __func__);
        return false;
    }
    adJunc[juncNum++] = std::make_pair(idxPoint, junc);
    return true;
}

BASICTYPE::Junc * BASICTYPE::Road::getOtherJunc(const Junc &oneJunc) const {
    /*debug info
    printf("%u: %u, %u\n", oneJunc.getId(), adJunc[0].second->getId(), adJunc[1].second->getId());
    printf("%s[%s(), line %d]: %s\n", __FILE__, __func__, __LINE__
           , this->getID().c_str());*/

    if(&oneJunc == adJunc[0].second) {
        return adJunc[1].second;
    } else if(&oneJunc == adJunc[1].second) {
        return adJunc[0].second;
    } else {
        printf("%s: Strange error: the junction offered is not belongs to this road. The juncID is %u.\n", __func__, oneJunc.getId());
    }
    return NULL;
}

std::string BASICTYPE::Road::toString() {
    std::string str = "";
    u_int len = rawLine.size();
    for(u_int i = 0; i < len - 1; ++i) {
        str += rawLine[i].first + " " + rawLine[i].second + ", ";
    }
    str += rawLine[len - 1].first + " " + rawLine[len - 1].second;
    return str;
}

std::string BASICTYPE::Road::toJsonString() {
    char str[128];
    sprintf(str, ")\", \"roadinfo\": {\"avespeed\": %lf, \"traffictrend\": "
            "%lf, \"length\": %lf}}", roadInfo.avgSpeed
            , roadInfo.trafficTrend, roadInfo.length);
    return "{\"roadid\": \"" + id + "\", \"coors\": \"LINESTRING("
            + toString() + std::string(str);
}
