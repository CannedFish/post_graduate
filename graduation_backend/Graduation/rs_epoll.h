#ifndef RS_EPOLL_H
#define RS_EPOLL_H

#include "common.h"
#include "msgqueue.h"
#define MAX_EVENTS 500

class RS_Epoll;

namespace EVENT {
    class Buf {
        #define BUFSIZE 1280
    public:
        Buf() {initBuf();}
        ~Buf() {}

        void initBuf() {
            len = s_offset = 0;
            memset(buf, 0, sizeof(buf));
        }
        int recvToBuf(int fd);
        int sendFromBuf(int fd);
        const char * getBufContent() {return buf;}
        void setBufContent(const char *buf, int len) {
            memcpy(this->buf, buf, len);
            this->len = len;
        }

    private:
        char buf[BUFSIZE];
        int len, s_offset;
    };

    class MyEvent
    {
    public:
        MyEvent();
        ~MyEvent();

        void initEvent(int fd, void (*callback)(int, int, void *), void *arg) {
            this->fd = fd;
            this->callback = callback;
            this->arg = arg;
            this->last_active = time(NULL);
            if(this->buf != NULL) {
                buf->initBuf();
            }
        }
        void setEventType(int eventType);
        int getEventType();
        void setStatus(int status);
        int getStatus();
        void setLastActive(long last_active);
        long getLastActive();
        int getFd();

        int recvToBuf();
        int sendFromBuf();
        const char * getBufContent() {return buf->getBufContent();}
        void setBufContent(const char *buf, int len) {this->buf->setBufContent(buf, len);}
        void callBack();
    private:
        int fd;
        void (*callback)(int fd, int eventType, void *arg);
        int eventType;
        void *arg;
        int status;
        long last_active;
        Buf *buf;
    };

    void AcceptEvent(int fd, int eventType, void *arg);
    void RecvEvent(int fd, int eventType, void *arg);
    void SendEvent(int fd, int evnetType, void *arg);
}

class RS_Epoll
{
public:
    ~RS_Epoll();
    bool add_event(int eventType, EVENT::MyEvent *ev);
    bool del_event(EVENT::MyEvent *ev);
    int dispatch();
    static RS_Epoll * getInstance() {
        if(epollIns == NULL) {
            epollIns = new RS_Epoll();
        }
        return epollIns;
    }
    EVENT::MyEvent * getIdleEvent();
    EVENT::MyEvent * getServerEvent() {
        return &evPool[MAX_EVENTS];
    }
private:
    RS_Epoll();
    void checkTimeout();

    static RS_Epoll *epollIns;
    int epollFd, checkPos;
    EVENT::MyEvent evPool[MAX_EVENTS + 1];//event pool
    struct epoll_event events[MAX_EVENTS];
};
#endif // RS_EPOLL_H
