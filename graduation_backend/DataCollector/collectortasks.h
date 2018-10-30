#ifndef COLLECTORTASKS_H
#define COLLECTORTASKS_H

#include "task.h"
#include "datafrominfodb.h"

namespace CollectorTasks {
    class DBCollector : public Task {
        void run(void *_arg);
    };
}

#endif // COLLECTORTASKS_H
