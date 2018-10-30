#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "common.h"
#include "mutex.h"

class ThreadPool;
class Task;

class Thread {
public:
    Thread(ThreadPool *_pool);
    ~Thread();

    bool start();
    void run();
    void setTask(Task *_task, void *_taskArg, size_t argSize = 0);
    void stop() {
        _stop = true;
    }

private:
    static void *routine(void *arg) {
        Thread *thr = (Thread *)arg;
        thr->run();
        return NULL;
    }
    Thread(const Thread &other) {}
    void operator =(const Thread &other) {}

    pthread_t tId;
    ThreadPool *pool;
    Task *task;
    void *taskArg;
    bool _stop;
};

class ThreadPool {
#define DEFAULT_THREAD_NUM 16
    typedef std::set<Thread *> ThreadList;
    typedef ThreadList::iterator ThreadListItor;
public:
    ThreadPool();
    ThreadPool(int num);
    ~ThreadPool();

    void setTask(Task *_task, void *_taskArg, size_t argSize = 0);
private:
    Thread *getAnIdleThread();
    void moveThreadToBusy(Thread *th);
    void moveThreadToIdle(Thread *th);
    bool poolInit();

    ThreadList busyList;
    ThreadList idleList;
    int poolSize;
    Mutex idleMutex, busyMutex;

    friend class Thread;
};

extern ThreadPool defaultThreadPool;

#endif // THREADPOOL_H
