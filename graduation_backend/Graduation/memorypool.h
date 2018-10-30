#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include "common.h"
#include "mutex.h"

template<typename ClassName>
class MemoryPool
{
    static const size_t CAPACITY = 16;
    typedef std::list<void *> PoolList;
    typedef PoolList::iterator PoolListItor;
    typedef std::list<ClassName *> FreeList;

public:
    //Thread safe
    static MemoryPool *instance() {
        if(ins == NULL) {
            mutex.acquire();
            if(ins == NULL)
                ins = new MemoryPool;
            mutex.release();
        }
        return ins;
    }

    ~MemoryPool() {
        for(PoolListItor itor = poolList.begin(); itor != poolList.end(); ++itor) {
            ::operator delete(*itor);
        }
    }

    void * acquire(size_t size) {
        mutex.acquire();
        void *p = acquireImp(size);
        mutex.release();
        return p;
    }

    void release(void *block) {
        mutex.acquire();
        releaseImp(block);
        mutex.release();
    }

private:
    MemoryPool() {
        getMemory();
    }

    void getMemory() {
        void *pool = ::operator new(sizeof(ClassName) * CAPACITY);
        poolList.push_back(pool);

        ClassName *block = static_cast<ClassName *>(pool);
        for(size_t i = 0; i < CAPACITY; ++i) {
            freeList.push_back(block);
            block++;
        }
    }

    void * acquireImp(size_t size) {
        if(size > sizeof(ClassName)) {
            throw std::bad_alloc();
            return NULL;
        } else {
            if(freeList.empty()) {
                getMemory();
            }
            void *free_block = freeList.front();
            freeList.pop_front();
            return free_block;
        }
    }

    void releaseImp(void *block) {
        freeList.push_back(static_cast<ClassName *>(block));
    }

    class Garbo {
    public:
        ~Garbo() {
            if(MemoryPool::ins) {
                delete MemoryPool::ins;
            }
        }
    };

private:
    static MemoryPool *ins;
    static Garbo garbo;
    static Mutex mutex;
    PoolList poolList;
    FreeList freeList;
};

template<typename ClassName>
MemoryPool<ClassName> *MemoryPool<ClassName>::ins = NULL;

template<typename ClassName>
typename MemoryPool<ClassName>::Garbo MemoryPool<ClassName>::garbo;

template<typename ClassName>
Mutex MemoryPool<ClassName>::mutex;

#endif // MEMORYPOOL_H
