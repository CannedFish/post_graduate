#include "msgqueue.h"

MsgQueue::RecvQueue *MsgQueue::RecvQueue::ins = NULL;

void MsgQueue::RecvQueue::queryParse(const std::string &ori, QueryInfo &tmpInfo) {
    /*
    std::size_t posA(0), posB = ori.find(',');
    tmpInfo.type = atoi(const_cast<const char *>(ori.substr(posA, posB - posA).c_str()));
    std::cout << tmpInfo.type << std::endl;

    posA = posB + 1;
    posB = ori.find(',', posA);
    strcpy(tmpInfo.mapName, ori.substr(posA, posB - posA).c_str());
    printf("%s\n", tmpInfo.mapName);

    posA = posB + 1;
    posB = ori.find(',', posA);
    strcpy(tmpInfo.fromX, ori.substr(posA, posB - posA));
    printf("%s\n", tmpInfo.fromX);

    posA = posB + 1;
    posB = ori.find(',', posA);
    strcpy(tmpInfo.fromY, ori.substr(posA, posB - posA));
    printf("%s\n", tmpInfo.fromY);

    posA = posB + 1;
    posB = ori.find(',', posA);
    strcpy(tmpInfo.toX, ori.substr(posA, posB - posA));
    std::cout << tmpInfo.toX << std::endl;

    posA = posB + 1;
    posB = ori.find(',', posA);
    tmpInfo.to.second = ori.substr(posA, posB - posA);
    std::cout << tmpInfo.to.second << std::endl;
    */
    sscanf(ori.c_str(), "%d%s%s%s%s%s"
           , &tmpInfo.type, tmpInfo.mapName, tmpInfo.fromX
           , tmpInfo.fromY, tmpInfo.toX, tmpInfo.toY);
    printf("%d: %s, %s, %s, %s, %s\n"
           , tmpInfo.type, tmpInfo.mapName, tmpInfo.fromX
           , tmpInfo.fromY, tmpInfo.toX, tmpInfo.toY);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

MsgQueue::SendQueue *MsgQueue::SendQueue::ins = NULL;
