#ifndef ENGINESERVER_H
#define ENGINESERVER_H

#include "rs_epoll.h"

class EngineServer
{
public:
    ~EngineServer() {
        delete reIns;
        engineServer = NULL;
    }

    static EngineServer * getInstance() {
        if(engineServer == NULL) {
            engineServer = new EngineServer();
        }
        return engineServer;
    }
    void setPort(int port);
    bool initServer();
    void startServer();
private:
    EngineServer();

    static EngineServer *engineServer;
    unsigned short port;
    RS_Epoll *reIns;
};

#endif // ENGINESERVER_H
