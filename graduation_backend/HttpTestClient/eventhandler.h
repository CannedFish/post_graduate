#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "eventhandle.h"

namespace EventHandler {
    struct Arg {
        int threadID;
        EVENT::MyEvent *ev;
    };

    void ConnCompleteEvent(int fd, int eventType, void *arg);//for Async Connectio
    void AcceptEvent(int fd, int eventType, void *arg);
    void RecvEvent(int fd, int eventType, void *arg);
    void SendEvent(int fd, int evnetType, void *arg);
}

#endif // EVENTHANDLER_H
