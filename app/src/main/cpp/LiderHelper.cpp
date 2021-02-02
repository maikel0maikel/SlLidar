//
// Created by maikel on 21-1-30.
//

#include <dirent.h>
#include <unistd.h>
#include <asm/termbits.h>
#include <asm/fcntl.h>
#include <fcntl.h>
#include <termios.h>

#include "LiderHelper.h"
#include "Log.h"

#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))

using namespace rp::standalone::rplidar;

static int getDevProperty() {
    std::string k_tty_path = "/sys/class/tty/";
    std::string dev_vid = "10c4ea60";
    std::string product_name = "CP2102 USB to UART Bridge Controller";

    struct dirent *entry;

    auto deleter = [](DIR *p) { if (p) closedir(p); };
    std::unique_ptr<DIR, decltype(deleter)> dir(opendir(k_tty_path.c_str()), deleter);
    if (dir == nullptr) {
        printf("open dir error,path:%s\n", k_tty_path.c_str());
        exit(0);
    }

    while ((entry = readdir(dir.get())) != nullptr) {
        printf("dir type %d, dir name %s\n", entry->d_type, entry->d_name);
        if (entry->d_type != DT_LNK)
            continue;
        std::string path = k_tty_path + entry->d_name;

        char buf[256] = {0};
        if (readlink(path.c_str(), buf, 256) == -1) {
            printf("readlink error,errno=%d, path=%s\n", errno, path.c_str());
            continue;
        }

        std::string pid_path = k_tty_path + buf + "/../../../../idProduct";
        std::string vid_path = k_tty_path + buf + "/../../../../idVendor";
        std::string pro_path = k_tty_path + buf + "/../../../../product";

        std::fstream pid_stream(pid_path, std::ios::in);
        std::fstream vid_stream(vid_path, std::ios::in);
        std::fstream pro_stream(pro_path, std::ios::in);
        if (!pid_stream || !vid_stream || !pro_stream) {
            printf("stream open fail\n");
            continue;
        }

        std::string pid((std::istreambuf_iterator<char>(pid_stream)),
                        std::istreambuf_iterator<char>());
        std::string vid((std::istreambuf_iterator<char>(vid_stream)),
                        std::istreambuf_iterator<char>());
        std::string pro((std::istreambuf_iterator<char>(pro_stream)),
                        std::istreambuf_iterator<char>());
        if (!pid.empty())
            pid.pop_back();
        if (!vid.empty())
            vid.pop_back();
        if (!pro.empty())
            pro.pop_back();

        std::string tmp_dev_vid = vid + pid;

        printf("tmp_dev_vid: %s\n", tmp_dev_vid.c_str());
        printf("dev_vid:%s\n", dev_vid.c_str());
        printf("pro_name:%s\n", pro.c_str());

        if (tmp_dev_vid == dev_vid && product_name == pro) {
            printf("get devid success::%s\n", dev_vid.c_str());
            std::string tty_path = buf;
            std::size_t found = tty_path.rfind("ttyUSB");
            if (found == std::string::npos)
                continue;
            std::string tty_dev = "/dev/" + tty_path.substr(found);

            printf("tty_path:%s\n", tty_path.c_str());
            int index = atoi(tty_path.substr(found + 6).c_str());
            return index;
        }
    }
    return -1;
}


LiderHelper::LiderHelper(JavaVM *vm, JNIEnv *env, jobject javaLidar) {
    lidarCallbacker = new JavaCallbacker(vm, env, javaLidar);
}

LiderHelper::~LiderHelper() {
    delete lidarCallbacker;
    lidarCallbacker = nullptr;
}


void *connect_t(void *args) {
    LiderHelper *lidar = static_cast<LiderHelper *>(args);
    lidar->startConnect();
    return 0;
}


void LiderHelper::start() {
    if (isStart) {
        LOGE("lidar already has started\n");
        return;
    }
    pthread_create(&mLidarTask, 0, connect_t, this);
    isStart = 1;
}

int checkRPLIDARHealth(RPlidarDriver *drv) {
    u_result op_result;
    rplidar_response_device_health_t healthinfo;

    op_result = drv->getHealth(healthinfo);
    if (IS_OK(
            op_result)) { // the macro IS_OK is the preperred way to judge whether the operation is succeed.
        LOGE("RPLidar health status : %d\n", healthinfo.status);
        if (healthinfo.status == RPLIDAR_STATUS_ERROR) {
            LOGE("Error, rplidar internal error detected. Please reboot the device to retry.\n");
            // enable the following code if you want rplidar to be reboot by software
            // drv->reset();
            return 0;
        } else {

            return 1;
        }

    } else {
        LOGE("Error, cannot retrieve the lidar health code: %x\n", op_result);
        return 0;
    }
}


void LiderHelper::startConnect(std::string dev_vid) {
    std::string opt_com_path;
    _u32 baudrateArray[2] = {115200, 256000};
    _u32 opt_com_baudrate = 0;
    u_result op_result;

    opt_com_path = "/dev/ttyUSB" + std::to_string(getDevProperty());
    LOGE("com path %s\n", opt_com_path.c_str());
    RPlidarDriver *lidar = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
    if (!lidar) {
        LOGE("insufficent memory, exit\n");
        lidarCallbacker->onState(LIDAR_INIT_FAILURE, CHILD_THREAD);
        return;
    }
    lidarCallbacker->onState(LIDAR_INIT_SUCCESS, CHILD_THREAD);
    rplidar_response_device_info_t devinfo;
    bool connectSuccess = false;

    size_t baudRateArraySize = (sizeof(baudrateArray)) / (sizeof(baudrateArray[0]));
    LOGE("baudRateArraySize = %d", baudRateArraySize);
    for (size_t i = 0; i < baudRateArraySize; ++i) {
        if (!lidar)
            lidar = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
        if (IS_OK(lidar->connect(opt_com_path.c_str(), baudrateArray[i]))) {
            op_result = lidar->getDeviceInfo(devinfo);

            if (IS_OK(op_result)) {
                connectSuccess = true;
                connect = 1;
                lidarCallbacker->onState(LIDAR_CONNECT_SUCCESS, CHILD_THREAD);
                break;
            } else {
                delete lidar;
                lidar = NULL;
            }
        }
    }

    if (!connectSuccess) {
        lidarCallbacker->onState(LIDAR_CONNECT_FAILURE, CHILD_THREAD);
        LOGE("Error, cannot bind to the specified serial port %s.\n", opt_com_path.c_str());
        return;
    }

    // print out the device serial number, firmware and hardware version number..
    LOGE("RPLIDAR S/N: ");
   // char des[50];

    for (int pos = 0; pos < 16; ++pos) {
        LOGE("RPLIDAR S/N:%02X", devinfo.serialnum[pos]);
       // snprintf(des, 2, "%02X", devinfo.serialnum[pos]);
    }


   // LOGE("serialnum:%s",des);
    LOGE("Firmware Ver: %d.%02d\n"
         "Hardware Rev: %d\n", devinfo.firmware_version >> 8, devinfo.firmware_version & 0xFF,
         (int) devinfo.hardware_version);
    //lidarCallbacker->onDeviceInfo(devinfo.model,devinfo.firmware_version >> 8,(int) devinfo.hardware_version,des);
    // check health...
    if (!checkRPLIDARHealth(lidar)) {
        LOGE("!checkRPLIDARHealth(drv)");
        lidarCallbacker->onState(LIDAR_NOT_HEALTHY,CHILD_THREAD);
        return;
    }
    lidar->startMotor();
    // start scan...
    lidar->startScan(0, 1);
    LOGE("start lidar");
    // fetech result and print it out...
    rplidar_response_measurement_node_hq_t nodes[8192];
    size_t count = _countof(nodes);
    while (1) {
        op_result = lidar->grabScanDataHq(nodes, count);
        if (IS_OK(op_result)) {
            lidar->ascendScanData(nodes, count);
            float min_angle = 1000000.0f;
            float max_angle = -1000000.0f;
            float max_dist = 0.0f;
            float min_dist = 100.0f;
            for (int pos = 0; pos < (int) count; ++pos) {
                float angle = nodes[pos].angle_z_q14 * 90.f / (1 << 14);
                if (angle <= min_angle) min_angle = angle;
                if (angle >= max_angle) max_angle = angle;

                float dist = nodes[pos].dist_mm_q2 / 4.0f;
                if (dist >= max_dist) max_dist = dist;
                if (dist >= 0.1f && dist <= min_dist) min_dist = dist;
//                printf("%s theta: %03.2f Dist: %08.2f Q: %d \n",
//                    (nodes[pos].flag & RPLIDAR_RESP_MEASUREMENT_SYNCBIT) ?"S ":"  ",
//                    (nodes[pos].angle_z_q14 * 90.f / (1 << 14)),
//                    nodes[pos].dist_mm_q2/4.0f,
//                    nodes[pos].quality);
                lidarCallbacker->onDataResult(nodes[pos].flag,angle
                        ,dist,nodes[pos].quality,CHILD_THREAD);
            }
        }

        if (!connect) {
            break;
        }
    }
    LOGE("end1----");
    lidar->stop();
    lidar->stopMotor();
    LOGE("end2----");
    connect = 0;
    lidarCallbacker->onState(LIDAR_STOPED,CHILD_THREAD);
    //RPlidarDriver::DisposeDriver(lidar);
    isStart = 0;
    //delete lidar;
    lidar = nullptr;
//    lidar = NULL;
//    LOGE("end----");
//    lidarCallbacker->onState(LIDAR_STOPED,CHILD_THREAD);
}


void LiderHelper::stop() {
    connect = 0;
}



