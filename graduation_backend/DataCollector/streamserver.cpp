#include "streamserver.h"

inline void EngineServer::setPort(int port) {
    this->port = port;
}

bool EngineServer::initServer() {
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(fcntl(listenFd, F_SETFL, O_NONBLOCK) == -1) {
        printf("%s: set none block failed, error[%d]: %s.\n"
               , __func__, errno, strerror(errno));
        close(listenFd);
        return false;
    }

    sockaddr_in sin;
    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);
    if(bind(listenFd, (const sockaddr *)&sin, sizeof(sin)) == -1) {
        printf("%s: bind socket failed, error[%d]: %s.\n"
               , __func__, errno, strerror(errno));
        close(listenFd);
        return false;
    }
    if(listen(listenFd, SOMAXCONN) == -1) {
        printf("%s: listen socket failed, error[%d]: %s.\n"
               , __func__, errno, strerror(errno));
        close(listenFd);
        return false;
    }
    printf("server is running on [%s:%d].\n"
           , inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));

    EVENT::MyEvent *serverEvent = reIns->getServerEvent();
    serverEvent->initEvent(listenFd, EventHandler::AcceptEvent, serverEvent);
    reIns->add_event(EPOLLIN, serverEvent);
    return true;
}

void EngineServer::startServer() {
    if(!initServer())
        return ;
    while(1) {
        if(reIns->dispatch() < 0) break;
    }
}

EngineServer::EngineServer() : port(8931) {
    reIns = RS_Epoll::getInstance();
}

EngineServer * EngineServer::engineServer = NULL;
