#include "engineserver.h"
#include "datamanagement.h"
#include "task.h"

void cleanUp(int signum) {
    switch(signum) {
    case SIGHUP:
        printf("SIGHUP is catched, closing...\n");
        break;
    case SIGINT:
        printf("SIGINT is catched, closing...\n");
        break;
    case SIGTERM:
        printf("SIGTERM is catched, closing...\n");
        break;
    default:
        printf("Unkown signal is catched, closing...\n");
    }

    //delete DataManagement<>::getInstance();
    //delete EngineServer::getInstance();
    exit(signum);
}

bool registerSignalHandler() {
    if(signal(SIGINT, cleanUp) == SIG_ERR
            || signal(SIGHUP, cleanUp) == SIG_ERR
            || signal(SIGTERM, cleanUp) == SIG_ERR)
        return false;
    return true;
}

int main() {
    if(!registerSignalHandler()) {
        printf("Fail to register signal handlers.\n");
        exit(1);
    }

    /* data structure initialize
    */
    DataManagement<> *dm = DataManagement<>::getInstance();
    //DataManagement<MapDataHandleMethod::GetMapTOPOFromDB> *dm = DataManagement<MapDataHandleMethod::GetMapTOPOFromDB>::getInstance();
    if(!dm->initialMap("Wuhan")) {
        printf("Fail to initialize map data\n");
        exit(1);
    }

    Task *monitorRecvQueue = new MonitorRecvQueue();
    Task *monitorSendQueue = new MonitorSendQueue();

    defaultThreadPool.setTask(monitorRecvQueue, NULL);
    defaultThreadPool.setTask(monitorSendQueue, NULL);

    EngineServer *engineServer = EngineServer::getInstance();
    engineServer->startServer();

    return 0;
}
