#ifndef __LOG_CPP_H__
#define __LOG_CPP_H__

#include <string>
// Output the string to the appropriate log file on the platform
void LogInfo(std::string);
void LogError(std::string);

#endif // __LOG_CPP_H__
