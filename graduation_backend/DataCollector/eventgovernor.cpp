#include "eventgovernor.h"
#include "eventhandler.h"

void EventGovernor::notifyEvent(u_int eventType) {
    RS_Epoll *evHandle = RS_Epoll::getInstance();
    HandleContainerItor hcItr;
    for(hcItr = handleContainer.begin(); hcItr != handleContainer.end(); ++hcItr) {
        if(eventType & hcItr->second) {
            char notification[64];
            gettimeofday(&tp, NULL);
            sprintf(notification, "Notify: %d,tp: %ld\n", eventType, tp.tv_sec * 1000 + tp.tv_usec / 1000);
            (hcItr->first).ev->initEvent((hcItr->first).ev->getFd()
                                        , EventHandler::SendEvent
                                        , (hcItr->first).ev);
            (hcItr->first).ev->setBufContent(notification, strlen(notification));
            evHandle->add_event(EPOLLOUT, (hcItr->first).ev);
        }
    }
    /*
    if(eventType & TRAFFICJAM) {
        hcItr = handleContainer.find(TRAFFICJAM);
        if(hcItr != handleContainer.end()) {
            for(RegistedHandleItor rhItr = (hcItr->second).begin()
                ; rhItr != (hcItr->second).end(); ++rhItr) {
                rhItr->ev->setBufContent("")
                rhItr->ev->initEvent(rhItr->ev->getFd()
                                     , EventHandler::SendEvent
                                     , rhItr->ev);
                evHandle->add_event(EPOLLOUT, rhItr->ev);
            }
        }
    }*/
}

EventGovernor * EventGovernor::ins = NULL;
