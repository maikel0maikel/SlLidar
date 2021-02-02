//
// Created by maikel on 21-1-30.
//

#ifndef LIDARTEST_JAVACALLBACKER_H
#define LIDARTEST_JAVACALLBACKER_H
#include "Log.h"
#include <jni.h>

#define MAIN_THREAD 1 //主线程
#define CHILD_THREAD 2//子线程

#define LIDAR_INIT_FAILURE -1//雷达创建失败
#define LIDAR_INIT_SUCCESS 0 //雷达创建成功
#define LIDAR_CONNECT_SUCCESS 1//雷达连接成功
#define LIDAR_CONNECT_FAILURE 2//雷达连接失败
#define LIDAR_NOT_HEALTHY 3//雷达有故障
#define LIDAR_GET_INFO_SUCCESS 4//获取雷达设备信息成功
#define LIDAR_GET_INFO_FAILURE 5//获取雷达设备信息失败
#define LIDAR_STOPED 6//已停止工作



class JavaCallbacker {
public:
    JavaCallbacker(JavaVM* vm,JNIEnv* env,jobject javaLidar);
    ~JavaCallbacker();
    void onState(int state,int threadId=MAIN_THREAD);
    void onDataResult(int flag,float degree,double distance,int quality,int threadId=MAIN_THREAD);
    void onDeviceInfo(int model,int firmwareVersion,int inthardwareVersion,const char *serialnum,int threadId=MAIN_THREAD);

private:
    JavaVM* mVm=0;
    JNIEnv* mEnv=0;
    jobject mJavaLidarObj=0;
    jmethodID mOnStateMethod=0;
    jmethodID mOnDataResultMethod=0;
    jmethodID mOnDeviceInfo=0;
};




#endif //LIDARTEST_JAVACALLBACKER_H
