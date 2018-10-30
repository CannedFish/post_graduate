#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "eventhandle.h"
#include "eventgovernor.h"

namespace EventHandler {
    void AcceptEvent(int fd, int eventType, void *arg);
    void RecvEvent(int fd, int eventType, void *arg);
    void SendEvent(int fd, int evnetType, void *arg);
}

#endif // EVENTHANDLER_H
