#include "collectortasks.h"

void CollectorTasks::DBCollector::run(void *_arg) {
    DataFromInfoDB dbCollector;
    for( ; true; sleep(60)) {//update 1 per minute
        printf("Start update through infomation of database..\n");
        dbCollector.updateInfo();
        printf("Done!!\n");
    }
}
