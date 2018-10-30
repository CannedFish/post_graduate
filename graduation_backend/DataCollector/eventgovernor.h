#ifndef EVENTGOVERNOR_H
#define EVENTGOVERNOR_H

#include "common.h"
#include "eventhandle.h"

class EventGovernor
{
    struct IdxEvent {
        bool operator <(const IdxEvent &other) const {
            return ev->getFd() < other.ev->getFd();
        }
        EVENT::MyEvent *ev;
    };

    //typedef std::set<IdxEvent> RegistedHandle;
    //typedef RegistedHandle::iterator RegistedHandleItor;
    typedef std::map<IdxEvent, u_int> HandleContainer; //handle -> event types
    typedef HandleContainer::iterator HandleContainerItor;

public:
static const u_int LOWEST_SPEED = 10;
//When average speed change from upper to lower LOWEST_SPEED or vice versa
static const u_int TRAFFICJAM = 1;

public:
    ~EventGovernor() {}

    static EventGovernor * getInstance() {
        if(ins == NULL) {
            ins = new EventGovernor;
        }
        return ins;
    }

    int registEvent(u_int eventType, EVENT::MyEvent *ev) {
        if(!typeValidate(eventType)) return 1;
        printf("Regist event..\n");
        tmpEvent.ev = ev;
        /*
        if(eventType & TRAFFICJAM)
            handleContainer[TRAFFICJAM].insert(tmpEvent);
        //other types...
        */
        handleContainer[tmpEvent] |= eventType;

        return 0;
    }

    void notifyEvent(u_int eventType);

    bool unRegistEvent(u_int eventType, EVENT::MyEvent *ev) {
        if(!typeValidate(eventType)) return false;
        printf("Unregist event..\n");
        /*
        RegistedHandleItor rhItr;
        if(eventType & TRAFFICJAM) {
            tmpEvent.ev = ev;
            if((rhItr = handleContainer[TRAFFICJAM].find(tmpEvent)) != handleContainer[TRAFFICJAM].end()) {
                handleContainer[TRAFFICJAM].erase(rhItr);
            }
        }
        //other types...
        */
        tmpEvent.ev = ev;
        handleContainer[tmpEvent] &= (~eventType);

        return true;
    }

private:
    EventGovernor() {}

    bool typeValidate(u_int eventType)  {
        if(!(eventType & TRAFFICJAM)) {
            printf("Wrong type to be deal with!!\n");
            return false;
        }
        return true;
    }

private:
    static EventGovernor *ins;
    HandleContainer handleContainer;
    IdxEvent tmpEvent;
    timeval tp;
};

#endif // EVENTGOVERNOR_H
