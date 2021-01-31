//
// Created by maikel on 21-1-30.
//

#ifndef LIDARTEST_LIDERHELPER_H
#define LIDARTEST_LIDERHELPER_H

#include <pthread.h>
#include <rplidar.h>
#include <rptypes.h>
#include <jni.h>
#include "JavaCallbacker.h"
class LiderHelper {

public:
    LiderHelper(JavaVM *vm,JNIEnv*env,jobject javaLidar);

    ~LiderHelper();

    int init();

    void unInit();

    void start();

    void stop();

    void startConnect();

private:
    int connect = 0;
    int isStart = 0;
    pthread_t mLidarTask;//准备线程id
    rp::standalone::rplidar::RPlidarDriver* lidar;
    JavaCallbacker* lidarCallbacker;
    int mUnInitFlag;
};


#endif //LIDARTEST_LIDERHELPER_H
