//
// Created by maikel on 21-1-30.
//

#include "LiderHelper.h"
#include "Log.h"

LiderHelper::LiderHelper(JavaVM *vm, JNIEnv *env, jobject javaLidar) {
    lidarCallbacker = new JavaCallbacker(vm,env,javaLidar);
}

LiderHelper::~LiderHelper() {
    delete lidarCallbacker;
    lidarCallbacker = nullptr;
}

int LiderHelper::init() {
    lidar =  rp::standalone::rplidar::RPlidarDriver::CreateDriver();
    if (lidar){
        LOGE("lidar create success\n");
        return 1;
    } else{
        LOGE("lidar create failure\n");
        return 0;
    }
}

void *connect_t(void *args) {
    LiderHelper* lidar = static_cast<LiderHelper *>(args);
    lidar->startConnect();
    return 0;
}


void LiderHelper::start() {
    if(isStart){
        LOGE("lidar already has started\n");
        return;
    }
    pthread_create(&mLidarTask, 0, connect_t, this);
    isStart = 1;
}

void LiderHelper::startConnect() {
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
            lidarCallbacker->onState(4,CHILD_THREAD);
        }else{
            delete lidar;
            lidar = NULL;
            connect = 0;
            LOGE("Failed to getDeviceInfo ");
            lidarCallbacker->onState(5,CHILD_THREAD);
        }
    }
    else
    {
        lidarCallbacker->onState(6,CHILD_THREAD);
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
            size_t count = ARRAY_LEN(nodes);
            op_result = lidar->grabScanDataHq(nodes, count);
            if (IS_OK(op_result)) {
                lidar->ascendScanData(nodes, count);
                for (int pos = 0; pos < (int)count ; ++pos) {
                    float angle = nodes[pos].angle_z_q14 * 90.f / (1 << 14);
                    double distance = nodes[pos].dist_mm_q2/4.0f;
                    LOGE("%s theta: %03.2f Dist: %08.2f Q: %d \n",
                         (nodes[pos].flag & RPLIDAR_RESP_MEASUREMENT_SYNCBIT) ?"S ":"  ",
                         angle,
                         distance,
                         nodes[pos].quality);
                    lidarCallbacker->onDataResult(nodes[pos].flag,angle
                            ,distance,nodes[pos].quality);
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
    connect = 0;
    if (mUnInitFlag){
        unInit();
        mUnInitFlag = 0;
    }
}


void LiderHelper::stop() {
        connect = 0;
}

void LiderHelper::unInit() {
    if (connect){
        mUnInitFlag = 1;
    } else  if(lidar){
        LOGE("lidar unInitLidarDriver");
        rp::standalone::rplidar::RPlidarDriver::DisposeDriver(lidar);
    } else{
        lidarCallbacker->onState(3);
    }
}

