LOCAL_PATH := $(call my-dir)    
include $(CLEAR_VARS)
  
LOCAL_CFLAGS := -D__STDC_CONSTANT_MACROS -fvisibility=hidden 

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../include

LOCAL_ARM_MODE := arm  
APP_ABI := armeabi-v7a

$(warning '.......................')
$(warning $(LOCAL_PATH))

LOCAL_CPPFLAGS := -fPIC -Wall -Wextra -DAPP_CFG_ANDROID -DLINUX -finline-functions -O3 -fno-strict-aliasing -fvisibility=hidden -lstdc++  -frtti -fexceptions
LOCAL_LDLIBS :=-L$(SYSROOT)/usr/lib -L$(NDK)/sources/cxx-stl/gnu-libstdc++/libs/armeabi -llog -lz -lm -ldl
LOCAL_LDLIBS += $(LOCAL_PATH)/../libs/lib/libavformat.a
LOCAL_LDLIBS += $(LOCAL_PATH)/../libs/lib/libavcodec.a
LOCAL_LDLIBS += $(LOCAL_PATH)/../libs/lib/libavutil.a
LOCAL_LDLIBS += $(LOCAL_PATH)/../libs/lib/libswscale.a
LOCAL_LDLIBS += $(LOCAL_PATH)/../libs/lib/libswresample.a
LOCAL_LDLIBS += $(LOCAL_PATH)/../libs/lib/libx264.a

LOCAL_SHARED_LIBRARIES ：= libdvm


LOCAL_C_INCLUDES += /usr/include/c++/4.5/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libs/include/
LOCAL_CPP_EXTENSION:=.cpp

LOCAL_MODULE := jni_ffmpeg
LOCAL_SRC_FILES := com_lingo_ffmpeg_lingoplayer_FFMpegLib.cpp player.cpp convert.cpp

include $(BUILD_SHARED_LIBRARY)

