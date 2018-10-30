#include "eventhandle.h"
/*
inline int EVENT::Buf::recvToBuf(int fd) {
    int tLen = recv(fd, buf + len, sizeof(buf) - 1 - len, 0);
    len += tLen;
    return tLen;
}

inline int EVENT::Buf::sendFromBuf(int fd) {
    int tLen = send(fd, buf + s_offset, len - s_offset, 0);
    if(tLen > 0) {
        s_offset += tLen;
        return len - s_offset;
    } else {
        return tLen - 1;
    }
}
*/
EVENT::MyEvent::MyEvent()
    : fd(0), callback(NULL), eventType(0), arg(NULL), status(0), buf(NULL)
{
}

EVENT::MyEvent::~MyEvent() {
    if(buf != NULL) {
        delete buf;
        buf = NULL;
    }
}

inline void EVENT::MyEvent::setEventType(int eventType) {
    this->eventType = eventType;
}

inline int EVENT::MyEvent::getEventType() {
    return this->eventType;
}

inline void EVENT::MyEvent::setStatus(int status) {
    this->status = status;
}

inline int EVENT::MyEvent::getStatus() {
    return this->status;
}

inline void EVENT::MyEvent::setLastActive(long last_active) {
    this->last_active = last_active;
}
/*
inline long EVENT::MyEvent::getLastActive() {
    return this->last_active;
}

inline int EVENT::MyEvent::getFd() {
    return fd;
}

inline int EVENT::MyEvent::recvToBuf() {
    if(buf == NULL) {
        buf = new Buf();
    }
    return buf->recvToBuf(fd);
}

inline int EVENT::MyEvent::sendFromBuf() {
    if(buf == NULL) {
        buf = new Buf();
    }
    return buf->sendFromBuf(fd);
}
*/
inline void EVENT::MyEvent::callBack() {
    this->callback(fd, eventType, arg);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RS_Epoll::~RS_Epoll() {
    epollIns = NULL;
}

bool RS_Epoll::add_event(int eventType, EVENT::MyEvent *ev) {
    struct epoll_event epv = {0, {0}};
    int op;
    epv.data.ptr = ev;
    epv.events = eventType;
    ev->setEventType(eventType);
    if(ev->getStatus() == 1) {
        op = EPOLL_CTL_MOD;
    } else {
        op = EPOLL_CTL_ADD;
        ev->setStatus(1);
    }
    if(epoll_ctl(epollFd, op, ev->getFd(), &epv) < 0) {
        printf("%s: Event add failed[fd=%d], error[%d]: %s\n"
               , __func__, ev->getFd(), errno, strerror(errno));
        return false;
    } else {
        printf("%s: Event add OK[fd=%d], op[%d].\n"
               , __func__, ev->getFd(), op);
        return true;
    }
}

bool RS_Epoll::del_event(EVENT::MyEvent *ev) {
    struct epoll_event epv = {0, {0}};
    if(ev->getStatus() != 1) return true;
    epv.data.ptr = ev;
    ev->setStatus(0);
    if(epoll_ctl(epollFd, EPOLL_CTL_DEL, ev->getFd(), &epv) < 0) {
        printf("%s: Event delete failed[fd=%d], error[%d]: %s\n", __func__, ev->getFd(), errno, strerror(errno));
        close(ev->getFd());
        return false;
    } else {
        printf("%s: Event delete OK[fd=%d].\n"
               , __func__, ev->getFd());
        close(ev->getFd());
        return true;
    }
}

int RS_Epoll::dispatch() {
    checkTimeout();
    int fds = epoll_wait(epollFd, events, MAX_EVENTS, -1);
    if(fds == -1) {
        if(errno == EINTR) return 0;
        printf("%s: error[%d]: %s", __func__, errno, strerror(errno));
        return fds;
    }
    long activeTime = time(NULL);
    for(int i = 0; i < fds; ++i) {
        EVENT::MyEvent * ev = static_cast<EVENT::MyEvent *>(events[i].data.ptr);
        if((events[i].events & EPOLLIN) && (ev->getEventType() & EPOLLIN)) {
            ev->callBack();
            ev->setLastActive(activeTime);
        }
        if((events[i].events & EPOLLOUT) && (ev->getEventType() & EPOLLOUT)) {
            ev->callBack();
        }
    }
    return 0;
}

EVENT::MyEvent * RS_Epoll::getIdleEvent() {
    for(int i = 0; i < MAX_EVENTS; ++i) {
        if(evPool[i].getStatus() == 0)
            return &evPool[i];
    }
    return NULL;
}

RS_Epoll * RS_Epoll::epollIns = NULL;

RS_Epoll::RS_Epoll() : epollFd(0), checkPos(0)
{
    epollFd = epoll_create(MAX_EVENTS);
    if(epollFd <= 0) {
        printf("%s: create epoll failed, %d.\n", __func__, errno);
        delete this;
    }
}

void RS_Epoll::checkTimeout() {//check 100 events every time
    long now = time(NULL);
    for(int i = 0; i < 100; ++i, ++checkPos) {
        if(checkPos == MAX_EVENTS) checkPos = 0;
        if(evPool[i].getStatus() != 1) continue;
        if(now - evPool[i].getLastActive() >= 60) {
            //close(evPool[i].getFd());
            printf("[fd=%d] timeout[%ld--%ld].\n", evPool[i].getFd()
                   , evPool[i].getLastActive(), now);
            del_event(&evPool[i]);
        }
    }
}
