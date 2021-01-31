//
// Created by maikel on 21-1-30.
//

#ifndef LIDARTEST_JAVACALLBACKER_H
#define LIDARTEST_JAVACALLBACKER_H
#include "Log.h"
#include <jni.h>

#define MAIN_THREAD 1 //主线程
#define CHILD_THREAD 2//子线程

class JavaCallbacker {
public:
    JavaCallbacker(JavaVM* vm,JNIEnv* env,jobject javaLidar);
    ~JavaCallbacker();
    void onState(int state,int threadId=MAIN_THREAD);

    void onDataResult(int flag,float degree,double distance,int quality,int threadId=MAIN_THREAD);

private:
    JavaVM* mVm;
    JNIEnv* mEnv;
    jobject mJavaLidarObj;
    jmethodID mOnStateMethod;
    jmethodID mOnDataResultMethod;
};




#endif //LIDARTEST_JAVACALLBACKER_H
