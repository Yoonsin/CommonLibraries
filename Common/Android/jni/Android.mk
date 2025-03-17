LOCAL_PATH := $(call my-dir)/../..
include $(CLEAR_VARS)

LOCAL_MODULE := Common
COMMON_LIB_NAME := lib$(LOCAL_MODULE).a

LOCAL_CFLAGS := -Wall -pipe -fno-exceptions -fstrict-aliasing

ifndef NDEBUG
LOCAL_CFLAGS += -g -D_DEBUG
else
LOCAL_CFLAGS += -fexpensive-optimizations -O3
endif

# include ���丮 �߰�
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../Irrlicht/source/Irrlicht/zlib

LOCAL_SRC_FILES := \
    ConcurrentCommunicationEndpoint.cpp \
    CRC32.cpp \
    IniFile.cpp \
    IniIterator.cpp \
    IniParser.cpp \
    misc.cpp \
    Serial.cpp \
    cserial.c \
    SimpleSockets.cpp \
    StringHelpers.cpp \
    utf8.cpp \
    Threading.cpp \
    timing.cpp \
    ZSocket.cpp

# ���� ���̺귯�� ����
include $(BUILD_STATIC_LIBRARY)

# zlib ��� ����
#$(call import-module,zlib)