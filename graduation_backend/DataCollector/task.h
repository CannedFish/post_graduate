#ifndef TASK_H
#define TASK_H

#include "common.h"
#include "threadpool.h"

class Task {
public:
    Task();
    void destroy() {delete this;}

    virtual void run(void *_arg) = 0;
protected:
    virtual ~Task() {}
};

#endif // TASK_H
