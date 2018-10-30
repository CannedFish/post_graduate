#ifndef TASK_H
#define TASK_H

#include "common.h"
#include "rs_epoll.h"
#include "threadpool.h"
#include "memorypool.h"

class Task {
public:
    Task();
    void destroy() {delete this;}

    virtual void run(void *_arg) = 0;
protected:
    virtual ~Task() {}
};

class MonitorRecvQueue : public Task {
public:
    void run(void *_arg);
};

class MonitorSendQueue : public Task {
public:
    void run(void *_arg);
};

class GetRoutePlan : public Task {
public:
    static void * operator new(size_t size) {
        Debug("%s\n", "GetRoutePlan::new");
        return MemoryPool<GetRoutePlan>::instance()->acquire(size);
    }
    static void operator delete(void *block) {
        Debug("%s\n", "GetRoutePlan::delete");
        MemoryPool<GetRoutePlan>::instance()->release(block);
    }

    void run(void *_arg);
};

class UpdateRoadInfo : public Task {
public:
    void run(void *_arg);
};

#endif // TASK_H
