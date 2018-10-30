#include "eventhandler.h"

void EventHandler::ConnCompleteEvent(int fd, int eventType, void *arg) {}

void EventHandler::AcceptEvent(int fd, int eventType, void *arg) {
    struct sockaddr_in sin;
    socklen_t len = sizeof(struct sockaddr_in);
    int accFd;
    if((accFd = accept(fd, (struct sockaddr *)&sin, &len)) == -1) {
        if(errno != EAGAIN && errno != EINTR) {}
        printf("%s: accept, %d.\n", __func__, errno);
        return ;
    }

    RS_Epoll *epIns = RS_Epoll::getInstance();
    EVENT::MyEvent *idleEvent;
    if((idleEvent = epIns->getIdleEvent()) == NULL) {
        printf("%s: max connection limit[%d].\n", __func__, MAX_EVENTS);
        close(accFd);
        return ;
    }

    int ret = 0;
    if((ret = fcntl(accFd, F_SETFL, O_NONBLOCK)) < 0) {
        printf("%s: set nonblocking failed, %d\n", __func__, errno);
        return ;
    }

    idleEvent->initEvent(accFd, EventHandler::RecvEvent, idleEvent);
    epIns->add_event(EPOLLIN, idleEvent);
    printf("new connection[%s:%d][time:%ld][fd=%d]\n"
           , inet_ntoa(sin.sin_addr), ntohs(sin.sin_port)
           , idleEvent->getLastActive(), accFd);
}

void EventHandler::RecvEvent(int fd, int eventType, void *arg) {
    EVENT::MyEvent *ev = static_cast<EVENT::MyEvent *>(arg);
    int len = ev->recvToBuf();
    RS_Epoll *epIns = RS_Epoll::getInstance();
    if(len > 0) {
        printf("%s: [fd=%d]: %s\n", __func__, fd, ev->getBufContent());
        if(!strncmp(ev->getBufContent(), "REGIST", 6)) {
            //do some regist work based on regist type
            int evType, ret;
            sscanf(ev->getBufContent(), "REGIST%d", &evType);

            char content[32];
            if((ret = EventGovernor::getInstance()->registEvent(evType, ev)) == 0) {
                sprintf(content, "Regist OK!\n");
            } else {
                sprintf(content, "Regist Failed, Errno %d!\n", ret);
            }
            //ev->setBufContent("Regist OK!\n", 32);

            ev->initEvent(fd, EventHandler::SendEvent, ev);
            ev->setBufContent(content, strlen(content));
            epIns->add_event(EPOLLOUT, ev);
        } else if(!strncmp(ev->getBufContent(), "KEEPALIVE", 9)) {
            //just for keeping alive
            ev->initEvent(fd, EventHandler::RecvEvent, ev);
            epIns->add_event(EPOLLIN, ev);
        } else {
            ev->setBufContent("Wrong commod!!", 32);
            ev->initEvent(fd, EventHandler::SendEvent, ev);
            epIns->add_event(EPOLLOUT, ev);
        }
    } else if(len == 0) {
        epIns->del_event(ev);
        u_int e(0);
        EventGovernor::getInstance()->unRegistEvent(~e, ev);
        printf("%s: [fd=%d] has closed.\n", __func__, fd);
    } else {
        epIns->del_event(ev);
        u_int e(0);
        EventGovernor::getInstance()->unRegistEvent(~e, ev);
        printf("%s: [fd=%d] error[%d]: %s\n", __func__, fd, errno, strerror(errno));
    }
}

void EventHandler::SendEvent(int fd, int evnetType, void *arg) {
    EVENT::MyEvent *ev = static_cast<EVENT::MyEvent *>(arg);
    int len = ev->sendFromBuf();
    RS_Epoll *epIns = RS_Epoll::getInstance();
    if(len == 0) {
        ev->initEvent(fd, EventHandler::RecvEvent, ev);
        epIns->add_event(EPOLLIN, ev);
    } else if(len < 0) {
        printf("%s\n", ev->getBufContent());
        epIns->del_event(ev);
        u_int e(0);
        EventGovernor::getInstance()->unRegistEvent(~e, ev);
        printf("%s: [fd=%d] error[%d]: %s\n", __func__, fd, errno, strerror(errno));
    }
}
