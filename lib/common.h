#ifndef SRC_COMMON_H__
#define SRC_COMMON_H__

#include <string>
#include <vector>

// define our program name
#define PROGRAM_NAME "chips"

// Print msg, exit if error
enum MSGTYPE {
  M_ERROR = 0,
  M_WARNING = 1,
  M_DEBUG = 2,
  M_PROGRESS = 3
};
void PrintMessageDieOnError(const std::string& msg,
                            MSGTYPE msgtype);

void RegionParser(const std::string region, std::string& chromID, std::int32_t& start, std::int32_t& end);

#endif  // SRC_COMMON_H__
