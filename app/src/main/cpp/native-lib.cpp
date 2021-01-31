#include <jni.h>
#include <string>
#include <rptypes.h>
#include "LiderHelper.h"
#include "Log.h"




JavaVM *mVm = 0;

jlong initLIdar(JNIEnv *env,jobject obj){
    LOGE("do initLIdar");
    LiderHelper*lidar = new LiderHelper(mVm,env,obj);
    int ret = lidar->init();
    if (ret){
		return reinterpret_cast<jlong>(lidar);
    }
	return 0;

}
void unInitLidar(JNIEnv *env,jobject obj,jlong nativePtr){
	LiderHelper* lidar = reinterpret_cast<LiderHelper *>(nativePtr);
	if (lidar){
		lidar->unInit();
	}
}


void start(JNIEnv *env,jobject obj,jlong nativePtr){
	LiderHelper*lidar = reinterpret_cast<LiderHelper *>(nativePtr);
	if (lidar){
		lidar->start();
	}
}

void stop(JNIEnv *env,jobject obj,jlong nativePtr){
	LiderHelper*lidar = reinterpret_cast<LiderHelper *>(nativePtr);
	if (lidar){
		lidar->stop();

	}
}

static JNINativeMethod methods[] = { {"init","()J",(void *)initLIdar},
                                     {"unInit","(J)V",(void *)unInitLidar},
                                     {"start","(J)V",(void *)start},
									 {"stop","(J)V",(void *)stop},
};

void registerNativeMethod(JNIEnv *env){
    jclass clz = env->FindClass("com/pudutech/lidartest/LidarHelper");
    jint ret = env->RegisterNatives(clz,methods,ARRAY_LEN(methods));
    LOGE("ret==%d",ret);
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved){
        mVm = vm;
        JNIEnv *env;
        LOGE("JNI_OnLoad");
        if ((mVm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6)) == JNI_OK){
            registerNativeMethod(env);
            return JNI_VERSION_1_6;
        } else if ((mVm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_4)) == JNI_OK){
            registerNativeMethod(env);
            return JNI_VERSION_1_4;
        }
    return -1;
}

void unRegisterNativeMethod(JNIEnv *env){
	jclass clz = env->FindClass("com/pudutech/lidartest/LidarHelper");
	jint ret = env->UnregisterNatives(clz);
	LOGE("unRegisterNativeMethod ret=%d",ret);
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved){
	JNIEnv *env;
	if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) == JNI_OK) {
		unRegisterNativeMethod(env);
	} else if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_4) == JNI_OK) {
		unRegisterNativeMethod(env);
	}
	mVm = 0;
}

