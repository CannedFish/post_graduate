#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include "common.h"

class FileNotFound : public std::exception {
public:
    const char * what() const throw() {
        return "Oops! The file is not exist for given path.";
    }
};

class OpenFileFailed : public std::exception {
public:
    const char * what() const throw() {
        return "Oops! File open failed.";
    }
};

class ConnectionFailed : public std::exception {
public:
    const char * what() const throw() {
        return "Oops! Connect to database failed.";
    }
};

#endif // EXCEPTIONS_H
