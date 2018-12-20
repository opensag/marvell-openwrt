# Copyright 2006 The Android Open Source Project

# XXX using libutils for simulator build only...
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	CrcTool.cpp \
	DiagCmd.cpp \
	DiagCommandGenerator.cpp \
	DiagLogFile.cpp \
	DiagPort.cpp \
	DmcFilePart.cpp \
	inifile.c \
	main.cpp 

LOCAL_SHARED_LIBRARIES := \
	libcutils libutils

# for asprinf
LOCAL_CFLAGS := -D_GNU_SOURCE

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../ctoandroid \
					$(LOCAL_PATH)/../androidtoc \
					$(LOCAL_PATH)/.


#build executable
LOCAL_LDLIBS += -lpthread      
LOCAL_MODULE:= bp_log
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE :=false
include $(BUILD_EXECUTABLE)

