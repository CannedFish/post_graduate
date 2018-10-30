#include "collectortasks.h"
#include "streamserver.h"

int main() {
    Task *dbCollectTask = new CollectorTasks::DBCollector();
    defaultThreadPool.setTask(dbCollectTask, NULL);

    EngineServer::getInstance()->startServer();

    return 0;
}
