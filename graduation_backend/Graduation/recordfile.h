#ifndef RECORDFILE_H
#define RECORDFILE_H

#include "common.h"
#include "exceptions.h"

class RecordFile
{
    enum SIZE {MAXSIZE = 512 * 1024};//512KB

    int recordFile;
    u_char buf[MAXSIZE];
    size_t wLen, rLen, rOffset;
    bool end;
public:
    RecordFile();
    RecordFile(const char *path, int mode);
    ~RecordFile();

    void openFile(const char *path, int mode) throw(FileNotFound, OpenFileFailed);
    RecordFile & operator >>(const char *buf);
    RecordFile & operator >>(u_int content);
    RecordFile & operator >>(std::string str);
    RecordFile & operator <<(char *buf);
    bool isEnd() {
        return end;
    }
    void flush() {
        if(wLen > 0) writeToDisk();
    }

private:
    void writeToDisk();
    void readFromDisk();
};

#endif // RECORDFILE_H
