#ifndef MSGQUEUE_H
#define MSGQUEUE_H

#include "common.h"
#include "basicmapdatatype.h"
#include "mutex.h"
#include "memorypool.h"

namespace MsgQueue
{
    struct MsgType {
        MsgType() {}
        MsgType(const char *str) : content(str) {
            printf("contruct: %s %s\n", str, content.c_str());
        }

        static void * operator new(size_t size) {
            Debug("%s\n", "MsgType::new");
            return MemoryPool<MsgType>::instance()->acquire(size);
        }
        static void operator delete(void *block) {
            Debug("%s\n", "MsgType::delete");
            MemoryPool<MsgType>::instance()->release(block);
        }

        std::string content;
        int fd;
        void *ev;
    };

    class RecvQueue : private std::queue<MsgType *> {
    public:
        struct QueryInfo {
            int type;
            char mapName[16];
            char fromX[16], fromY[16], toX[16], toY[16];
            int fd;
            void *ev;

            QueryInfo() : type(0), fd(0), ev(NULL) {
                bzero(mapName, 16);
                bzero(fromX, 16);
                bzero(fromY, 16);
                bzero(toX, 16);
                bzero(toY, 16);
            }

            QueryInfo(const QueryInfo &other) : type(other.type), fd(other.fd), ev(other.ev) {
                memcpy(mapName, other.mapName, 16);
                memcpy(fromX, other.fromX, 16);
                memcpy(fromY, other.fromY, 16);
                memcpy(toX, other.toX, 16);
                memcpy(toY, other.toY, 16);
            }

            QueryInfo & operator =(const QueryInfo &other) {
                if(&other == this) return *this;
                type = other.type;
                memcpy(mapName, other.mapName, 16);
                memcpy(fromX, other.fromX, 16);
                memcpy(fromY, other.fromY, 16);
                memcpy(toX, other.toX, 16);
                memcpy(toY, other.toY, 16);
                fd = other.fd;
                ev = other.ev;
                return *this;
            }
        };

    public:
        static RecvQueue *getInstance() {
            if(ins == NULL) {
                ins = new RecvQueue();
            }
            return ins;
        }

        bool isEmpty() {
            return this->empty();
        }

        void insertAMsg(const char *msg, int fd, void *ev) {
            //some check needed!!
            printf("%s: insert a message to recive queue\n", __func__);
            MsgType *tmpMsg = new MsgType;
            tmpMsg->content = std::string(msg);
            //printf("content: %s\n", tmpMsg.content.c_str());
            tmpMsg->fd = fd;
            tmpMsg->ev = ev;
            mutex.acquire();
            this->push(tmpMsg);
            mutex.release();
        }

        QueryInfo getAQuery() {
            mutex.acquire();
            printf("%s: get a message from recive queue\n", __func__);
            QueryInfo tmpInfo;
            MsgType *p = this->front();
            queryParse(p->content, tmpInfo);
            tmpInfo.fd = p->fd;
            tmpInfo.ev = p->ev;
            this->pop();
            mutex.release();
            delete p;
            return tmpInfo;
        }

    private:
        RecvQueue() : std::queue<MsgType *>() {}
        void queryParse(const std::string &ori, QueryInfo &tmpInfo);

        static RecvQueue *ins;
        Mutex mutex;
    };

    class SendQueue : private std::queue<MsgType *> {
    public:
        static SendQueue * getInstance() {
            if(ins == NULL) {
                ins = new SendQueue();
            }
            return ins;
        }

        bool isEmpty() {
            return this->empty();
        }

        void insertAResult(const MsgType *result) {
            mutex.acquire();
            printf("%s: insert a message to send queue\n", __func__);
            this->push(const_cast<MsgType *>(result));
            mutex.release();
        }

        MsgType * getAResult() {
            mutex.acquire();
            printf("%s: get a message from send queue\n", __func__);
            /*
            MsgType tmpMsg;
            tmpMsg.content = (this->front()).content;
            tmpMsg.fd = (this->front()).fd;
            tmpMsg.ev = (this->front()).ev;
            */
            MsgType *tmpMsg = this->front();
            this->pop();
            mutex.release();
            return tmpMsg;
        }

    private:
        SendQueue() : std::queue<MsgType *>() {}

        static SendQueue *ins;
        Mutex mutex;
    };
}

#endif // MSGQUEUE_H
