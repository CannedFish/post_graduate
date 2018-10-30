#include "common.h"

int Connect_Nonblock(void *arg) {
    EventHandler::ConnArg *arg_ = static_cast<EventHandler::ConnArg *>(arg);
    int ret;
    if((ret = connect(fd, arg_->sin, arg_->len)) < 0) {
        if(ret == EINPROGRESS) {
            arg_->ev->initEvent(fd, EventHandler::ConnCompleteEvent, arg_->ev);
            return 0;
        }
    }
}
