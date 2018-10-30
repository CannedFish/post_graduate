#include "task.h"
#include "routeplan.h"

Task::Task() {}

void MonitorRecvQueue::run(void *_arg) {
    MsgQueue::RecvQueue *recvQueue = MsgQueue::RecvQueue::getInstance();
    for( ; true; usleep(20)) {
        while(recvQueue->isEmpty()) {
            usleep(1000);
        }

        MsgQueue::RecvQueue::QueryInfo tmpMsg = recvQueue->getAQuery();
        printf("Monitor::%s: %d, %s\n", __func__, tmpMsg.type, tmpMsg.mapName);
        //set a GetRoutePlan task
        Task *getRoutePlan = new GetRoutePlan();
        defaultThreadPool.setTask(getRoutePlan, &tmpMsg, sizeof(tmpMsg));
    }
}

void MonitorSendQueue::run(void *_arg) {
    MsgQueue::SendQueue *sendQueue = MsgQueue::SendQueue::getInstance();
    MsgQueue::MsgType *tmpMsg;
    for( ; true; usleep(20)) {
        while(sendQueue->isEmpty()) {
            usleep(1000);
        }

        tmpMsg = sendQueue->getAResult();
        //register a send event
        EVENT::MyEvent *ev = (EVENT::MyEvent *)tmpMsg->ev;
        ev->initEvent(tmpMsg->fd, EVENT::SendEvent, ev);
        ev->setBufContent(tmpMsg->content.c_str(), tmpMsg->content.length());
        RS_Epoll::getInstance()->add_event(EPOLLOUT, ev);
        delete tmpMsg;
    }
}

void GetRoutePlan::run(void *_arg) {
    MsgQueue::RecvQueue::QueryInfo *arg = (MsgQueue::RecvQueue::QueryInfo *)_arg;
    //get A plan and input the result to send queue
    printf("%s--GetRoutePlan::%s: %d %s\n", __DATE__, __func__, arg->type, arg->mapName);
    /*std::cout << arg->mapName << " "
              << arg->from.first << " " << arg->from.second << " "
              << arg->to.first << " " << arg->to.second << std::endl;
    */
    BASICTYPE::Point start, end;
    start.first = std::string(arg->fromX);
    start.second = std::string(arg->fromY);
    end.first = std::string(arg->toX);
    end.second = std::string(arg->toY);
    //std::cout << "Result: " << routePlan.getPlan(arg->mapName, start, end) << std::endl;

    /*
    MsgQueue::MsgType tmp(arg->mapName);
    printf("send content: %d--%s--%s\n", arg->type, arg->mapName, tmp.content.c_str());
    tmp.content += ": Welcome";
    printf("send content: %d--%s\n", arg->type, tmp.content.c_str());
    */
    MsgQueue::MsgType *tmp = new MsgQueue::MsgType;
    //tmp.content = "routePlan.getPlan(arg->mapName, start, end)";
    if(arg->type == 0) {//distance shortest path
        RoutePlan<> routePlan;
        tmp->content = routePlan.getPlan(arg->mapName, start, end);
    } else if(arg->type == 1) {//time shortest path
        RoutePlan<RouteStrategy::HeuristicApproach<
                BASICTYPE::Junc,
                BASICTYPE::Road,
                EvFunc::TimeShortestG,
                EvFunc::TimeEv<BASICTYPE::Junc> > > routePlan;
        tmp->content = routePlan.getPlan(arg->mapName, start, end);
    }
    //printf("%d: %u\n", arg->fd, tmp.content.size());
    tmp->ev = arg->ev;
    tmp->fd = arg->fd;
    MsgQueue::SendQueue::getInstance()->insertAResult(tmp);
}

void UpdateRoadInfo::run(void *_arg) {
    try {
        DB::DBConnection dbUpdate("mapdata", "127.0.0.1");
        DB::QueryResult qRet;
        BASICTYPE::RoadInfo roadInfo;
        char cmd[128];
        char *mapName = (char *)_arg;
        sprintf(cmd, "SELECT id, avgspeed, traffictrend "
                "FROM %s_Road", mapName);
        MapType *mapIns = MapType::getMap(mapName);
        for( ; true; sleep(60)) {
            printf("Updating road information..\n");
            if(!dbUpdate.query(cmd, qRet)) continue;
            while(qRet.next()) {
                roadInfo.avgSpeed = qRet.value(1).toDouble();
                roadInfo.trafficTrend = qRet.value(2).toDouble();
                mapIns->updateRoadInfo(qRet.value(0).toString().toStdString()
                                       , roadInfo);
            }
            printf("Done!!\n");
            /*debug info
            mapIns->saveMap2JsonFile(mapName);
            mapIns->showAllRoads();
            */
        }
    } catch(ConnectionFailed &e) {
        printf("%s\n", e.what());
        return ;
    }
}
