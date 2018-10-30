#ifndef ROUTESTRATEGYS_H
#define ROUTESTRATEGYS_H

#include "common.h"
#include "basicmapdatatype.h"
#include "datamanagement.h"

namespace RouteStrategy {

    template<typename J
             , typename R
             , typename GFunc
             , typename EvFunc
             , typename Map = MapType>
    class HeuristicApproach {

        typedef std::pair<BASICTYPE::IdxPoint, BASICTYPE::IdxPoint> DirectedRoad;
        struct DRoad;
        typedef std::set<DRoad> CloseChart;
        typedef typename CloseChart::iterator CloseITOR;
        struct DRoad {
            DirectedRoad dRoad;
            CloseITOR prev;
            double costed;
            bool operator <(const DRoad &other) const {
                if(dRoad.first == other.dRoad.first)
                    return dRoad.second < other.dRoad.second;
                else
                    return dRoad.first < other.dRoad.first;
            }
        };

        struct node {
            node() : junc(NULL), prev(NULL), h(0.0), g(0.0), f(0.0), visit(0), reEnter(false) {}
            node(J *j, J *p) : junc(j), prev(p), h(0.0), g(0.0), f(0.0), visit(0), reEnter(false) {}
            J *junc, *prev;
            CloseITOR prevR;
            double h, g, f;
            u_int visit;//use for debug
            bool reEnter;//use for debug
            bool operator <(const node &other) const {
                return f < other.f;
            }
            bool operator >(const node &other) const {
                return f > other.f;
            }
        };

        class OpenChart : public std::priority_queue<node, std::vector<node>, std::greater<node> > {
        public:
            node * find(J *target) {
                for(u_int i = 0; i < this->c.size(); ++i) {
                    if(this->c[i].junc == target) return &(this->c[i]);
                }
                return NULL;
            }

            void sort() {
                std::make_heap(this->c.begin(), this->c.end(), this->comp);
            }

            void showNodes() {
                printf("\nOpenChart: \n");
                for(typename std::vector<node>::iterator itor = this->c.begin();
                    itor != this->c.end(); ++itor) {
                    if(!itor->reEnter) {
                        if(itor->visit == 0) {
                            printf("%s%u\t%lf\t%lf\t%lf%s\n"
                                   , LIGHT_RED
                                   , itor->junc->getId()
                                   , itor->g, itor->h, itor->f
                                   , NONE);
                        } else {
                            printf("%u\t%lf\t%lf\t%lf\n"
                                   , itor->junc->getId()
                                   , itor->g, itor->h, itor->f);
                        }
                    } else {
                        printf("%s%u\t%lf\t%lf\t%lf%s\n"
                               , YELLOW
                               , itor->junc->getId()
                               , itor->g, itor->h, itor->f
                               , NONE);
                        itor->reEnter = false;
                    }
                    itor->visit++;
                }
                printf("\n");
            }
        };

    public:
        HeuristicApproach(Map *map, J* start, J *target)
            : myMap(map), top(start, NULL)
            , end(target), closeItor(closeChart.end()) {}
        ~HeuristicApproach() {}
        J * operator ()();
        void getRoute(std::list<std::string> &ret);
    private:
        HeuristicApproach() {}
        HeuristicApproach & operator =(const HeuristicApproach &other) {}

        Map *myMap;
        OpenChart openChart;
        CloseChart closeChart;
        EvFunc evFunc;
        GFunc gFunc;
        node top, tmpNode;
        J *end;
        BASICTYPE::RoadInfo roadInfo;
        CloseITOR closeItor;
        DRoad tmpDRoad;
    };
}

template<typename J, typename R, typename GFunc, typename EvFunc, typename Map>
J * RouteStrategy::HeuristicApproach<J, R, GFunc, EvFunc, Map>::operator ()() {
    /*debug info
    myMap->findRoadByIdx("101967988A001");
    */
    int next = 0;
    J *adJunc, *junc = top.junc;
    BASICTYPE::IdxPoint coor, otherCoor;
    node *tmp;
    coor.point = junc->getCoor();
    /*debug Info
    printf("point: %s, %s\n", coor.point.first.c_str(), coor.point.second.c_str());
    */
    while((adJunc = junc->getNextAdjunc(next++, roadInfo)) != NULL) {
        otherCoor.point = adJunc->getCoor();
        /*debug Info
        printf("point: %s, %s\n", otherCoor.point.first.c_str(), otherCoor.point.second.c_str());
        printf("%s: %lf, %lf, %lf\n", __func__, roadInfo.length, roadInfo.avgSpeed, roadInfo.trafficTrend);
        */

        tmpDRoad.dRoad = std::make_pair(otherCoor, coor);
        if(closeChart.find(tmpDRoad) != closeChart.end())
            continue; // this road has already in close chart

        tmpDRoad.dRoad = std::make_pair(coor, otherCoor);
        if(closeChart.find(tmpDRoad) != closeChart.end())
            continue; // this road has already in close chart

        if((tmp = openChart.find(adJunc)) != NULL) { // this junc has already been in open chart
            double f, g;
            g = gFunc(closeItor->costed, roadInfo);
            f = g + tmp->h;
            /*Debug Info
            Debug("%lf, %lf, %lf\n", g, tmp->h, f);
            */
            if(f < tmp->f) {
                tmp->g = g;
                tmp->f = f;
                tmp->prev = junc;
                tmp->prevR = closeItor;
                tmp->reEnter = true;
            }
        } else { // this junc is reached at the first time.
            tmpNode.junc = adJunc;
            tmpNode.prev = junc;
            //printf("here\n");
            if(closeItor == closeChart.end()) {
                tmpNode.g = gFunc(0.0, roadInfo);
            } else {
                tmpNode.g = gFunc(closeItor->costed, roadInfo);
            }
            tmpNode.h = evFunc(adJunc, end, roadInfo);
            tmpNode.f = tmpNode.g + tmpNode.h;
            tmpNode.prevR = closeItor;
            openChart.push(tmpNode);

            //find the target (has problems!! maybe is not the best!!)
            /*
            if(adJunc == end) {
                coor.point = junc->getCoor();
                otherCoor.point = adJunc->getCoor();
                tmpDRoad.dRoad = std::make_pair(coor, otherCoor);
                tmpDRoad.prev = closeItor;
                tmpDRoad.costed = tmpNode.g;
                closeChart.insert(tmpDRoad);
                closeItor = closeChart.find(tmpDRoad);

                return adJunc;
            }
            */
        }
    }// end while

    if(!openChart.empty()) {
        openChart.sort();// rebuild the heap
        /*debug Info
        openChart.showNodes();
        */
        top = openChart.top();// get the candidate
        openChart.pop();
        //insert candidate to close chart
        coor.point = top.prev->getCoor();
        otherCoor.point = top.junc->getCoor();
        tmpDRoad.dRoad = std::make_pair(coor, otherCoor);
        tmpDRoad.prev = top.prevR;
        tmpDRoad.costed = top.g;
        closeChart.insert(tmpDRoad);
        closeItor = closeChart.find(tmpDRoad);

        return top.junc;
    } else {
        return NULL;
    }
}

template<typename J, typename R, typename GFunc, typename EvFunc, typename Map>
void RouteStrategy::HeuristicApproach<J, R, GFunc, EvFunc, Map>::getRoute(std::list<std::string> &ret) {
    J *tmpA, *tmpB;
    R *tmpR;
    for( ; closeItor != closeChart.end(); closeItor = closeItor->prev) {
        tmpA = myMap->findJunc((closeItor->dRoad).first);
        tmpB = myMap->findJunc((closeItor->dRoad).second);
        printf("juncA: %u, juncB: %u\n", tmpA->getId(), tmpB->getId());
        tmpR = myMap->findRoadByEndPoint(*tmpA, *tmpB);
        if(tmpR == NULL) {
            printf("%s: can't find this road!\n", __func__);
            continue;
        }
        ret.push_front(tmpR->getID());
    }
}

#endif // ROUTESTRATEGYS_H
