#ifndef RS_EPOLL_H
#define RS_EPOLL_H

#include "common.h"
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

        int recvToBuf(int fd) {
            int tLen = recv(fd, buf + len, sizeof(buf) - 1 - len, 0);
            len += tLen;
            return tLen;
        }

        int sendFromBuf(int fd) {
            int tLen = send(fd, buf + s_offset, len - s_offset, 0);
            if(tLen > 0) {
                s_offset += tLen;
                return len - s_offset;
            } else {
                return tLen - 1;
            }
        }

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
        long getLastActive() {
            return this->last_active;
        }

        int getFd() {
            return fd;
        }

        int recvToBuf() {
            if(buf == NULL) {
                buf = new Buf();
            }
            return buf->recvToBuf(fd);
        }

        int sendFromBuf() {
            if(buf == NULL) {
                buf = new Buf();
            }
            return buf->sendFromBuf(fd);
        }

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
