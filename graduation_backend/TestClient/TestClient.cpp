#include <iostream>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <cassert>

using namespace std;
#define THREADNUM 200
#define JOBREQNUM 2
#define PORT 8930
#define IP "127.0.0.1"

//#define CONNTEST
#define SERVICETEST

#define REQNUM 4
const char *request[] = {"%d Wuhan 12730409.02 3576475.82 12731280.65 3575972.92"
                         , "%d Wuhan 12720920.14 3578018.28 12721323.12 3577921.3"
                         , "%d Wuhan 12720920.14 3578018.28 12721540.19 3577886.39"
                         , "%d Wuhan 12721323.12 3577921.3 12721540.19 3577886.39"};
//const char *request[] = {"%d Wuhan 12682189.87 3596098.39 12694807.93 3594666.35"
//                 , "%d Wuhan 12682597.3 3562795.99 12694807.93 3594666.35"};

const char *httpPostHead = "";

struct ARG {
    struct sockaddr *sin;
    socklen_t len;
};

struct ThreadArg {
    ARG *arg;
    int threadID;
};

#ifdef CONNTEST
long during[THREADNUM];
#endif

#ifdef SERVICETEST
double serviceCost[THREADNUM];
#endif

void *client(void *arg) {
    ThreadArg *thArg = (ThreadArg *)arg;

    int cliSock;
    if((cliSock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        //perror("socket");
        printf("Client %02d: error[%d]: %s\n", thArg->threadID, errno, strerror(errno));
        return NULL;
    }
    timeval start, stop;
#ifdef CONNTEST
    gettimeofday(&start, NULL);
#endif
    if(connect(cliSock, thArg->arg->sin, thArg->arg->len) == -1) {
        //perror("connect");
        printf("Client %02d: error[%d]: %s\n", thArg->threadID, errno, strerror(errno));
        close(cliSock);
        return NULL;
    }
#ifdef CONNTEST
    gettimeofday(&stop, NULL);
    during[thArg->threadID] = (stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000;
    printf("Client %02d: %ld ms.\n", thArg->threadID, during[thArg->threadID]);
#endif

    char buf[1280];

    //time_t begin, end;
#ifdef SERVICETEST
    long reqSum = 0;
#endif

    for(int t = 0; t < JOBREQNUM; ++t) {//, sleep(1)
        bzero(buf, sizeof(buf));
        //sprintf(buf, "Clinet %d: AA to BB", thArg->threadID);
        sprintf(buf, request[thArg->threadID % REQNUM]
                , thArg->threadID);
        //printf("request: %s\n", buf);
        if(send(cliSock, buf, strlen(buf), 0) == -1) {
            //perror("send");
            printf("Client %02d: error[%d]: %s\n", thArg->threadID, errno, strerror(errno));
            close(cliSock);
            return NULL;
        }
        //time(&begin);
#ifdef SERVICETEST
        gettimeofday(&start, NULL);
#endif
        //printf ("The current local time is: %s", ctime(&begin));

        int ret;
        if((ret = recv(cliSock, buf, 1280 - 1, 0)) == -1) {
            //perror("recv");
            printf("Client %02d: error[%d]: %s\n", thArg->threadID, errno, strerror(errno));
            close(cliSock);
            return NULL;
        }
        //time(&end);
#ifdef SERVICETEST
        gettimeofday(&stop, NULL);
#endif
        buf[ret] = '\0';
        //printf("Client %02d: %.03f seconds.\n", thArg->threadID, difftime(end, begin));

#ifdef SERVICETEST
        reqSum += ((stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000);
#endif
        //printf("Client %02d: %ld ms.\n", thArg->threadID, during[thArg->threadID]);
        //printf("Client %02d: %s\n", thArg->threadID, buf);
    }

#ifdef SERVICETEST
    serviceCost[thArg->threadID] = (double)reqSum / JOBREQNUM;
#endif

    close(cliSock);
    return NULL;
}

void test(void *xx) {
    assert(xx != NULL);
}

int main() {
    //test(NULL);

    sockaddr_in sin;
    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &sin.sin_addr);

    ARG serverAddr;
    serverAddr.sin = (struct sockaddr *)&sin;
    serverAddr.len = sizeof(sin);

    ThreadArg ta[THREADNUM];
    pthread_t pt[THREADNUM];

#ifdef CONNTEST
    memset(during, 0, sizeof(during));
#endif

    for(int i = 0; i < THREADNUM; ++i) {
        ta[i].arg = &serverAddr;
        ta[i].threadID = i;
        pthread_create(&pt[i], NULL, client, &ta[i]);
    }

#ifdef CONNTEST
    long sum = 0;
#endif

#ifdef SERVICETEST
    double sums = 0.0;
#endif

    for(int i = 0; i < THREADNUM; ++i) {
        pthread_join(pt[i], NULL);
#ifdef CONNTEST
        sum += during[i];
#endif

#ifdef SERVICETEST
        sums += serviceCost[i];
#endif
    }

#ifdef CONNTEST
    printf("Test complete: \033[1;31m%d\033[0m threads totally cost \033[1;31m%.03f\033[0m ms per thread for connection.\n", THREADNUM, (double)sum / THREADNUM);
#endif
#ifdef SERVICETEST
    printf("Test complete: \033[1;31m%d\033[0m requests on one of \033[1;31m%d\033[0m connections cost \033[1;31m%.03f\033[0m ms for one request.\n"
           , JOBREQNUM, THREADNUM, sums / THREADNUM);
#endif

    return 0;
}
