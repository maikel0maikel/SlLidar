//
// Created by maikel on 21-1-30.
//

#include "JavaCallbacker.h"

JavaCallbacker::JavaCallbacker(JavaVM *vm, JNIEnv *env,
        jobject javaLidar):mVm(vm),mEnv(env) {
    mJavaLidarObj = mEnv->NewGlobalRef(javaLidar);
    jclass clz = mEnv->GetObjectClass(mJavaLidarObj);
    mOnStateMethod = mEnv->GetMethodID(clz,"onLidarState","(I)V");
    mOnDataResultMethod = mEnv->GetMethodID(clz,"onDataResult","(IFDI)V");
    mOnDeviceInfo = mEnv->GetMethodID(clz,"onDeviceInfo","(IIILjava/lang/String;)V");
}
JavaCallbacker::~JavaCallbacker() {
    mEnv->DeleteGlobalRef(mJavaLidarObj);
    mJavaLidarObj = nullptr;
    mOnDataResultMethod = nullptr;
    mOnStateMethod = nullptr;
}

void JavaCallbacker::onState(int state, int threadId) {
    LOGE("call java method onState=%d\n",state);
    if (threadId == MAIN_THREAD){
        mEnv->CallVoidMethod(mJavaLidarObj,mOnStateMethod,state);
    } else{
        JNIEnv *env;
        if (mVm->AttachCurrentThread(&env,0)!=JNI_OK){
            LOGE("call java method onState occur an error\n");
            return;
        }
        env->CallVoidMethod(mJavaLidarObj,mOnStateMethod,state);
        mVm->DetachCurrentThread();
    }
}

void JavaCallbacker::onDataResult(int flag, float degree, double distance, int quality, int threadId) {
//    LOGE("call java onDataResult flag=%d,degree=%03.2f,distance=%08.2f,quality=%d\n",
//            flag,degree,distance,quality);
    if (threadId == MAIN_THREAD){
        mEnv->CallVoidMethod(mJavaLidarObj,mOnDataResultMethod,flag,degree,distance,quality);
    } else{
        JNIEnv *env;
        if (mVm->AttachCurrentThread(&env,0)!=JNI_OK){
            LOGE("call java method onDataResult occur an error\n");
            return;
        }
        env->CallVoidMethod(mJavaLidarObj,mOnDataResultMethod,flag,degree,distance,quality);
        mVm->DetachCurrentThread();
    }
}

void JavaCallbacker::onDeviceInfo(int model, int firmwareVersion, int inthardwareVersion, const char *serialnum, int threadId) {
    LOGE("call java onDeviceInfo model=%d,firmwareVersion=%d,inthardwareVersion=%d,serialnum=%s\n",
         model,firmwareVersion,inthardwareVersion,serialnum);
    if (threadId == MAIN_THREAD){
        jstring rtstr = mEnv->NewStringUTF(serialnum);
        mEnv->CallVoidMethod(mJavaLidarObj,mOnDeviceInfo,model,firmwareVersion,inthardwareVersion,rtstr);
    } else{
        JNIEnv *env;
        if (mVm->AttachCurrentThread(&env,0)!=JNI_OK){
            LOGE("call java method onDataResult occur an error\n");
            return;
        }
        jstring rtstr = mEnv->NewStringUTF(serialnum);
        env->CallVoidMethod(mJavaLidarObj,mOnDeviceInfo,model,firmwareVersion,inthardwareVersion,rtstr);
        mVm->DetachCurrentThread();
    }
}