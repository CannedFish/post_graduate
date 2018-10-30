#ifndef MUTEX_H
#define MUTEX_H

#include "common.h"

class Mutex
{
public:
    Mutex() {
        pthread_mutexattr_init(&mutexAttr);
        pthread_mutex_init(&mutex, &mutexAttr);
    }

    ~Mutex() {
        pthread_mutex_destroy(&mutex);
        pthread_mutexattr_destroy(&mutexAttr);
    }

    void acquire() {
        pthread_mutex_lock(&mutex);
    }

    void release() {
        pthread_mutex_unlock(&mutex);
    }

private:
    Mutex(const Mutex &other) {}
    void operator =(const Mutex &other) {}

    pthread_mutex_t mutex;
    pthread_mutexattr_t mutexAttr;
};

#endif // MUTEX_H
