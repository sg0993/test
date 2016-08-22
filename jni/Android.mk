LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libaec-prebuilt
LOCAL_SRC_FILES := libaec.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libagc-prebuilt
LOCAL_SRC_FILES := libagc.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libnr2-prebuilt
LOCAL_SRC_FILES := libnr2.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE				:= hdaec
LOCAL_LDLIBS				:= -llog
LOCAL_CFLAGS               := -g -D__arm -DANDROID -DAECG4 -DAEC_PROFILE_DESKPHONE_ALL
LOCAL_SRC_FILES             := SimpleFileIOTest.cpp
LOCAL_STATIC_LIBRARIES		:= libaec-prebuilt libagc-prebuilt libnr2-prebuilt

include $(BUILD_SHARED_LIBRARY)