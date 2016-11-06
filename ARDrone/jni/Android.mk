MY_LOCAL_PATH := $(call my-dir)
LOCAL_PATH := $(MY_LOCAL_PATH)

## ARTOOLKIT ---------------------------------------------------

// We added as much refernces as possible, to get as much functionality in the shortest time
include $(CLEAR_VARS)
ARTOOLKIT_DIR := C:/android-ndk-r10e/sources/artoolkit-5.3.2/android
ARTOOLKIT_LIBDIR := $(call host-path, $(ARTOOLKIT_DIR)/obj/local/$(TARGET_ARCH_ABI))
define add_artoolkit_module
	include $(CLEAR_VARS)
	LOCAL_MODULE:=$1
	LOCAL_SRC_FILES:=lib$1.a
	include $(PREBUILT_STATIC_LIBRARY)
endef
// Referenced as many libraries as I could (just to have the time saved)
#ARTOOLKIT_LIBS := eden argsub_es armulti arosg ar aricp arvideo util
#ARTOOLKIT_LIBS += OpenThreads osg osgAnimation osgDB osgFX osgGA osgParticle osgPresentation osgShadow osgSim osgTerrain osgText osgViewer osgUtil osgVolume osgWidget osgdb_osg osgdb_ive osgdb_jpeg jpeg osgdb_gif gif osgdb_tiff tiff osgdb_bmp osgdb_png png osgdb_tga osgdb_freetype ft2 osgdb_deprecated_osg osgdb_deprecated_osganimation osgdb_deprecated_osgfx  osgdb_deprecated_osgparticle osgdb_deprecated_osgshadow osgdb_deprecated_osgsim osgdb_deprecated_osgterrain osgdb_deprecated_osgtext osgdb_deprecated_osgviewer osgdb_deprecated_osgvolume osgdb_deprecated_osgwidget
ARTOOLKIT_LIBS := eden argsub_es2 armulti ar aricp arvideo util
LOCAL_PATH := $(ARTOOLKIT_LIBDIR)
$(foreach module,$(ARTOOLKIT_LIBS),$(eval $(call add_artoolkit_module,$(module))))
LOCAL_PATH := $(MY_LOCAL_PATH)

# Android arvideo depends on CURL.
CURL_DIR := $(ARTOOLKIT_DIR)/jni/curl
CURL_LIBDIR := $(call host-path, $(CURL_DIR)/libs/$(TARGET_ARCH_ABI))
define add_curl_module
	include $(CLEAR_VARS)
	LOCAL_MODULE:=$1
	#LOCAL_SRC_FILES:=lib$1.so
	#include $(PREBUILT_SHARED_LIBRARY)
	LOCAL_SRC_FILES:=lib$1.a
	include $(PREBUILT_STATIC_LIBRARY)
endef
#CURL_LIBS := curl ssl crypto
CURL_LIBS := curl
LOCAL_PATH := $(CURL_LIBDIR)
$(foreach module,$(CURL_LIBS),$(eval $(call add_curl_module,$(module))))
LOCAL_PATH := $(MY_LOCAL_PATH)

## FFMPEG + ARTOOLKIT---------------------------------------------------
include $(CLEAR_VARS)

LOCAL_CPPFLAGS += -Wno-extern-c-compat
ifeq ($(APP_OPTIM),debug)
    LOCAL_CPPFLAGS += -DDEBUG
endif

LOCAL_ARM_MODE := arm
LOCAL_PATH := $(MY_LOCAL_PATH)
LOCAL_MODULE    := FFMPEG_ARTOOLKIT
#LOCAL_SRC_FILES := FFMPEGstream.c ARNative.cpp ARMarkerSquare.c VirtualEnvironment.c
LOCAL_SRC_FILES := FFMPEGstream.c ARNative.cpp ARMarkerSquare.c
#LOCAL_CFLAGS := -DANDROID_BUILD
#LOCAL_LDLIBS := -llog -ljnigraphics -lz -lGLESv1_CM -landroid
LOCAL_LDLIBS := -llog -ljnigraphics -lz -lEGL -lGLESv2 -landroid
LOCAL_C_INCLUDES += $(ARTOOLKIT_DIR)/../include/android $(ARTOOLKIT_DIR)/../include
LOCAL_WHOLE_STATIC_LIBRARIES += ar
LOCAL_STATIC_LIBRARIES := eden argsub_es2 armulti aricp cpufeatures arvideo util
LOCAL_STATIC_LIBRARIES += eden argsub_es armulti arosg aricp cpufeatures arvideo util
LOCAL_STATIC_LIBRARIES += osgdb_osg osgdb_ive osgdb_jpeg jpeg osgdb_gif gif osgdb_tiff tiff osgdb_bmp osgdb_png png osgdb_tga osgdb_freetype ft2 osgAnimation osgFX osgParticle osgPresentation osgShadow osgSim osgTerrain osgText osgVolume osgWidget osgViewer osgGA osgDB osgUtil osgdb_deprecated_osg osgdb_deprecated_osganimation osgdb_deprecated_osgfx  osgdb_deprecated_osgparticle osgdb_deprecated_osgshadow osgdb_deprecated_osgsim osgdb_deprecated_osgterrain osgdb_deprecated_osgtext osgdb_deprecated_osgviewer osgdb_deprecated_osgvolume osgdb_deprecated_osgwidget osg OpenThreads
LOCAL_SHARED_LIBRARIES += $(CURL_LIBS)
LOCAL_STATIC_LIBRARIES += $(CURL_LIBS)
LOCAL_STATIC_LIBRARIES += libavformat_static libavcodec_static libswscale_static libavutil_static

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/cpufeatures)

$(call import-module,ffmpeg-1.0.1/android/arm)