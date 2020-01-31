#include <argon2.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <list>
#include <time.h>
#include <ctime>
#include "LogMgr.h"
#include "FileDesc.h"

LogMgr::LogMgr(){

}
LogMgr::LogMgr(const char *log_file):_log_file(log_file) {

}


LogMgr::~LogMgr() {

}

int LogMgr::logMsg(const char *msg) {
    std::cout << "Logging\n";
    current_time = time(NULL);
    char buffer [80];
    struct tm * timeinfo = localtime(&current_time);
    FileFD logFD(_log_file.c_str());

    if(!logFD.openFile(FileFD::appendfd))
        throw(pwfile_error("Could not open log file"));
    strftime(buffer, 80, "%h %a %T: ", timeinfo);
    logFD.writeFD(buffer);
    logFD.writeFD(msg);
    logFD.writeFD("\n");
    logFD.closeFD();

}