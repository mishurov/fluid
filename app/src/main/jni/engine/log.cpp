#include "log.h"

#include <android/log.h>

#define  LOG_TAG_CPP    "Fluid"
#define  LOGI_CPP(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG_CPP,__VA_ARGS__)
#define  LOGE_CPP(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG_CPP,__VA_ARGS__)

void LogInfo(std::string str) {
    LOGI_CPP("%s", str.c_str());
    return;
}

void LogError(std::string str) {
    LOGE_CPP("%s", str.c_str());
    return;
}
