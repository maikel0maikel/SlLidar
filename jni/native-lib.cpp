#include <jni.h>
#include <string>
#include <rplidar.h>
#include <rptypes.h>
#include "android/log.h"
#include <pthread.h>


#define TAG "zbq"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
#define ARRAY_LEN(a)  sizeof(a)/sizeof(a[0])

rp::standalone::rplidar::RPlidarDriver* lidar = 0;

JavaVM *mVm = 0;
int connect = 0;
int isStart = 0;
pthread_t mLidarTask;//准备线程id
void initLIdar(JNIEnv *env,jobject obj){
    LOGE("do initLIdar");
    lidar =  rp::standalone::rplidar::RPlidarDriver::CreateDriver();
    if (lidar){
        LOGE("lidar create success\n");
    } else{
        LOGE("lidar create failure\n");
    }
}
void unInitLidarDriver(JNIEnv *env,jobject obj){
	if(lidar){
		 LOGE("lidar unInitLidarDriver");
		rp::standalone::rplidar::RPlidarDriver::DisposeDriver(lidar);
	}
}
 

 
void unInitLidar(JNIEnv *env,jobject obj){
	if(lidar){
		LOGE("lidar unInitLidar");
		rp::standalone::rplidar::RPlidarDriver::DisposeDriver(lidar);
	}
}

void *readLidaData_t(void *args) {
	
	u_result res = lidar->connect("/dev/ttyUSB0", 115200);

	if (IS_OK(res))
	{
		LOGE("lidar connect /dev/ttyUSB0 success\n");
		rplidar_response_device_info_t devinfo;
		res = lidar->getDeviceInfo(devinfo);

        if (IS_OK(res)) 
        {
          connect = 1;
		  LOGE("lidar getDeviceInfo success\n");
        }else{
           delete lidar;
           lidar = NULL;
		   connect = 0;
		   LOGE("Failed to getDeviceInfo ");
        }
	}
	else
	{
		connect = 0;
		LOGE("Failed to connect to LIDAR %08x\r\n", res);
	}	
	if(connect){
	 
		lidar->startMotor();
		// start scan...
		lidar->startScan(0,1);
		rplidar_response_measurement_node_hq_t nodes[8192];
		u_result     op_result;
		while (1) {
			size_t   count = ARRAY_LEN(nodes);

			op_result = lidar->grabScanDataHq(nodes, count);
			if (IS_OK(op_result)) {
				lidar->ascendScanData(nodes, count);
				for (int pos = 0; pos < (int)count ; ++pos) {
					LOGE("%s theta: %03.2f Dist: %08.2f Q: %d \n", 
						(nodes[pos].flag & RPLIDAR_RESP_MEASUREMENT_SYNCBIT) ?"S ":"  ", 
						(nodes[pos].angle_z_q14 * 90.f / (1 << 14)), 
						nodes[pos].dist_mm_q2/4.0f,
						nodes[pos].quality);
				}
			}

			if (!connect){ 
				break;
			}
		}
	}else{
		isStart = 0;
		connect = 0;
		LOGE("lidar is not connect");
	}

	if(lidar){
		lidar->stop();
		lidar->stopMotor();
		lidar->disconnect();
	}
	
    return 0;
}

void start(JNIEnv *env,jobject obj){
	if(isStart){
		LOGE("lidar already has started\n");
		return;
	}
	pthread_create(&mLidarTask, 0, readLidaData_t,NULL);
	isStart = 1;
}

void stop(JNIEnv *env,jobject obj){
	if(connect){
		connect = 0;
		
	}
}

static JNINativeMethod methods[] = { {"initLidar","()V",(void *)initLIdar},
                                     {"unInitLidar","()V",(void *)unInitLidarDriver},
                                     {"start","()V",(void *)start},
									 {"stop","()V",(void *)stop},
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

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved){

}