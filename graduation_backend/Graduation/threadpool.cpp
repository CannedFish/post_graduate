#include "threadpool.h"
#include "task.h"

Thread::Thread(ThreadPool *_pool) : pool(_pool), task(NULL), _stop(false) {}

Thread::~Thread() {
    pthread_join(tId, NULL);
    pool = NULL;
    task = NULL;
    taskArg = NULL;
}

bool Thread::start() {
    int ret = pthread_create(&tId, NULL, routine, this);
    assert(ret == 0);
    return true;
}

void Thread::run() {
    for( ; !_stop; usleep(2000)) {
        while(task == NULL) {
            usleep(5000);
        }
        /*debug info
        if(taskArg != NULL) {
            MsgQueue::RecvQueue::QueryInfo *p = (MsgQueue::RecvQueue::QueryInfo *)taskArg;
            printf("before Thread::%s: thread%02d--%s\n", __func__, p->type, p->mapName);
        }
        */
        task->run(taskArg);
        /*debug info
        if(taskArg != NULL) {
            MsgQueue::RecvQueue::QueryInfo *p = (MsgQueue::RecvQueue::QueryInfo *)taskArg;
            printf("after Thread::%s: thread%02d--%s\n", __func__, p->type, p->mapName);
        }
        */

        if(taskArg != NULL) {
            //::operator delete(taskArg);
            free(taskArg);
            taskArg = NULL;
        }
        task->destroy();
        task = NULL;
        pool->moveThreadToIdle(this);
    }
}

void Thread::setTask(Task *_task, void *_taskArg, size_t argSize) {
    //printf("argsize: %lu\n", argSize);
    assert(_task != NULL);
    if(_taskArg != NULL) {
        //taskArg = ::operator new(argSize);
        taskArg = malloc(argSize);
        memcpy(taskArg, _taskArg, argSize);
        //MsgQueue::RecvQueue::QueryInfo *p = (MsgQueue::RecvQueue::QueryInfo *)taskArg;
        //printf("initialized Thread::%s: %d--%s\n", __func__, p->type, p->mapName);
    }
    task = _task;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
ThreadPool::ThreadPool() : poolSize(DEFAULT_THREAD_NUM) {
    if(!poolInit()) {
        delete this;
    }
}

ThreadPool::ThreadPool(int num) : poolSize(num) {
    assert(num > 1 && num <= DEFAULT_THREAD_NUM);
    if(!poolInit()) {
        delete this;
    }
}

ThreadPool::~ThreadPool() {
    ThreadListItor itor;
    for(itor = busyList.begin(); itor != busyList.end(); ++itor) {
        delete *itor;
    }
    busyList.clear();

    for(itor = idleList.begin(); itor != idleList.end(); ++itor) {
        delete *itor;
    }
    idleList.clear();
}

void ThreadPool::setTask(Task *_task, void *_taskArg, size_t argSize) {
    Thread *idleThread = getAnIdleThread();
    idleThread->setTask(_task, _taskArg, argSize);
}

Thread * ThreadPool::getAnIdleThread() {
    idleMutex.acquire();
    printf("%s\n", __func__);
    while(idleList.size() <= 0) {
        idleMutex.release();
        usleep(1000);
        idleMutex.acquire();
    }

    Thread *idleThread = *(idleList.begin());
    idleList.erase(idleThread);

    busyMutex.acquire();
    busyList.insert(idleThread);
    busyMutex.release();

    idleMutex.release();
    return idleThread;
}

void ThreadPool::moveThreadToBusy(Thread *th) {
    idleMutex.acquire();
    printf("%s\n", __func__);
    ThreadListItor itor = idleList.find(th);
    if(itor != idleList.end()) {
        idleList.erase(itor);
    } else {
        printf("can't find!\n");
    }
    idleMutex.release();

    busyMutex.acquire();
    busyList.insert(th);
    busyMutex.release();
}

void ThreadPool::moveThreadToIdle(Thread *th) {
    busyMutex.acquire();
    printf("%s\n", __func__);
    ThreadListItor itor = busyList.find(th);
    if(itor != busyList.end()) {
        busyList.erase(itor);
    }
    busyMutex.release();

    idleMutex.acquire();
    idleList.insert(th);
    idleMutex.release();
}

inline bool ThreadPool::poolInit() {
    busyList.clear();
    idleList.clear();

    for(int i = 0; i < poolSize; ++i) {
        Thread *th = new Thread(this);
        idleList.insert(th);
        th->start();
    }

    return true;
}

ThreadPool defaultThreadPool;
