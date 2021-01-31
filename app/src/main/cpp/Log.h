//
// Created by maikel on 21-1-30.
//

#ifndef LIDARTEST_LOG_H
#define LIDARTEST_LOG_H
#include "android/log.h"
#define ARRAY_LEN(a)  sizeof(a)/sizeof(a[0])
#define TAG "zbq"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
#endif //LIDARTEST_LOG_H
