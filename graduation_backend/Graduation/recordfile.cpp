#include "recordfile.h"

RecordFile::RecordFile() : recordFile(0), wLen(0), rLen(0), rOffset(0), end(false) {
}

RecordFile::RecordFile(const char *path, int mode) : wLen(0), rLen(0), rOffset(0), end(false) {
    try {
        openFile(path, mode);
    } catch(FileNotFound &e) {
        std::cout << e.what() << std::endl;
    }
}

RecordFile::~RecordFile() {
    if(wLen != 0) writeToDisk();
    close(recordFile);
    recordFile = 0;
}

void RecordFile::openFile(const char *path, int mode) throw(FileNotFound, OpenFileFailed) {
    if((recordFile = open(path, mode, 0644)) == -1) {
        //printf("%s: file open failed, error[%d]: %s\n", __func__, errno, strerror(errno));
        if(errno == ENOENT) throw FileNotFound();
        else {
            printf("Error [%d]: %s\n", errno, strerror(errno));
            throw OpenFileFailed();
        }
    }
}

RecordFile & RecordFile::operator >>(const char *buf) {
    size_t l = strlen(buf);
    //std::cout << "length: " << l << std::endl;
    //printf("%s\n", buf);
    if(wLen + l >= MAXSIZE) {
        writeToDisk();
    }
    memcpy(this->buf + wLen, buf, l + 1);
    wLen += l;
    return *this;
}

RecordFile & RecordFile::operator >>(u_int content) {
    char tmp[32];
    sprintf(tmp, "%u", content);
    return operator >>(tmp);
}

RecordFile & RecordFile::operator >>(std::string str) {
    return operator >>(str.c_str());
}

RecordFile & RecordFile::operator <<(char *buf) {
    if(rLen == 0) {
        readFromDisk();
    }
    size_t i;
    for(i = rOffset; i < rLen; ++i) {
        if(this->buf[i] == '\n') {
            buf[i - rOffset] = '\0';
            rOffset = i + 1;
            break;
        } else {
            buf[i - rOffset] = this->buf[i];
        }
    }
    if(i == rLen) {
        if(rLen < MAXSIZE) {
            end = true;
            buf[i - rOffset] = '\0';
        } else {
            memcpy(this->buf, this->buf + rOffset, rLen - rOffset);
            rLen -= rOffset;
            readFromDisk();
        }
    }
    return *this;
}

void RecordFile::writeToDisk() {
    if(write(recordFile, buf, wLen) == -1) {
        printf("%s: fail to write to disk, error[%d]: %s", __func__, errno, strerror(errno));
    }
    wLen = 0;
}

void RecordFile::readFromDisk() {
    int l;
    if((l = read(recordFile, buf, MAXSIZE - rLen)) == -1) {
        printf("%s: fail to read from disk, error[%d]: %s", __func__, errno, strerror(errno));
    }
    rLen += l;
    rOffset = 0;
}
