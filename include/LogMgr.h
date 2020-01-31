#ifndef LOGMGR_H
#define LOGMGR_H

#include <string>
#include <stdexcept>
#include <time.h>
#include "FileDesc.h"

/****************************************************************************************
 * LogMgr - Manages server logging functions
 *
 ****************************************************************************************/

class LogMgr {
   public:
      LogMgr();
      LogMgr(const char *log_file);
      ~LogMgr();

      //bool checkPasswd(const char *name, const char *passwd);
      int logMsg(const char *msg);

      std::string _log_file;
   private:
      time_t current_time;
};

#endif
