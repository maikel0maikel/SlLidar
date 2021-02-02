//
// Created by maikel on 21-1-30.
//

#ifndef LIDARTEST_LIDERHELPER_H
#define LIDARTEST_LIDERHELPER_H

#include <sstream>
#include <fstream>
#include <string>
#include <pthread.h>
#include <rplidar.h>
#include <rptypes.h>
#include <jni.h>
#include "JavaCallbacker.h"

class LiderHelper {

public:
    LiderHelper(JavaVM *vm,JNIEnv*env,jobject javaLidar);

    ~LiderHelper();

    void start();

    void stop();

    void startConnect(std::string dev_vid = "10c4ea60");

private:

    int connect = 0;
    int isStart = 0;
    pthread_t mLidarTask;//准备线程id
    JavaCallbacker* lidarCallbacker;

};


#endif //LIDARTEST_LIDERHELPER_H
