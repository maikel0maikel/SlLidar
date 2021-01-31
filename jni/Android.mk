LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
PROJECT_PATH =$(LOCAL_PATH)/..

LOCAL_MODULE := sl_lidar
LOCAL_LDFLAGS +=-shared
LOCAL_CFLAGS +=-DMODULE_FLAG
LOCAL_C_INCLUDES += include \
					src \
					src/hal \
					src/arch/linux 
#MY_HEADER_PATH +=$(PROJECT_PATH)
#MY_FILES_PATH :=$(PROJECT_PATH)

#MY_FILES_SUFFIX := %.cpp %.c %.cc
#My_All_Files := $(foreach src_path,$(MY_FILES_PATH), $(shell find "$(src_path)" -type f) ) 
#My_All_Files := $(My_All_Files:$(MY_CPP_PATH)/./%=$(MY_CPP_PATH)%)
#MY_SRC_LIST  := $(filter $(MY_FILES_SUFFIX),$(My_All_Files)) 
#MY_SRC_LIST  := $(MY_SRC_LIST:$(LOCAL_PATH)/%=%)
#LOCAL_SRC_FILES += $(MY_SRC_LIST)
#$(warning "$(LOCAL_MODULE): LOCAL_SRC_FILES =$(LOCAL_SRC_FILES)")
LOCAL_SRC_FILES += $(LOCAL_PATH)/native-lib.cpp \
		  src/rplidar_driver.cpp \
          src/hal/thread.cpp \
		  src/arch/linux/net_serial.cpp \
          src/arch/linux/net_socket.cpp \
          src/arch/linux/timer.cpp 

LOCAL_LDLIBS += -llog -landroid -lc
		  
LOCAL_MODULE_TAGS := optional  

include $(BUILD_SHARED_LIBRARY)