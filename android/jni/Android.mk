# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

SAMPLE_PATH := $(call my-dir)/../../src
LIBPNG_PATH := ../../gameplay/external-deps/libpng/lib/android/arm
ZLIB_PATH := ../../gameplay/external-deps/zlib/lib/android/arm
LUA_PATH := ../../gameplay/external-deps/lua/lib/android/arm
BULLET_PATH := ../../gameplay/external-deps/bullet/lib/android/arm
VORBIS_PATH := ../../gameplay/external-deps/oggvorbis/lib/android/arm
OPENAL_PATH := ../../gameplay/external-deps/openal/lib/android/arm

# gameplay
LOCAL_PATH := ../../gameplay/gameplay/android/obj/local/armeabi
include $(CLEAR_VARS)
LOCAL_MODULE    := libgameplay
LOCAL_SRC_FILES := libgameplay.a
include $(PREBUILT_STATIC_LIBRARY)

# libpng
LOCAL_PATH := $(LIBPNG_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := libpng 
LOCAL_SRC_FILES := libpng.a
include $(PREBUILT_STATIC_LIBRARY)

# libzlib
LOCAL_PATH := $(ZLIB_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := libzlib
LOCAL_SRC_FILES := libzlib.a
include $(PREBUILT_STATIC_LIBRARY)

# liblua
LOCAL_PATH := $(LUA_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := liblua
LOCAL_SRC_FILES := liblua.a
include $(PREBUILT_STATIC_LIBRARY)

# libbullet
LOCAL_PATH := $(BULLET_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := libbullet
LOCAL_SRC_FILES := libbullet.a
include $(PREBUILT_STATIC_LIBRARY)

# libvorbis
LOCAL_PATH := $(VORBIS_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := libvorbis
LOCAL_SRC_FILES := libvorbis.a
include $(PREBUILT_STATIC_LIBRARY)

# libOpenAL
LOCAL_PATH := $(OPENAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := libOpenAL
LOCAL_SRC_FILES := libOpenAL.a
include $(PREBUILT_STATIC_LIBRARY)

# GameplayPolyVox
LOCAL_PATH := $(SAMPLE_PATH)
include $(CLEAR_VARS)

LOCAL_MODULE    := GameplayPolyVox
LOCAL_SRC_FILES := ../../gameplay/gameplay/src/gameplay-main-android.cpp ../../polyvox/library/polyvoxcore/source/ArraySizes.cpp Colour.cpp CubiquityUtility.cpp ExtraNodeData.cpp GameplayColouredCubesVolume.cpp GameplaySmoothTerrainVolume.cpp Main.cpp MultiMaterial.cpp ../../polyvox/library/polyvoxcore/source/Impl/RandomVectors.cpp SmoothTerrainVolume.cpp ../../polyvox/library/polyvoxcore/source/VertexTypes.cpp ../../polyvox/library/polyvoxcore/source/Log.cpp ../../polyvox/library/polyvoxcore/source/Impl/ErrorHandling.cpp ../../polyvox/library/polyvoxcore/source/Impl/MarchingCubesTables.cpp ../../polyvox/library/polyvoxcore/source/Impl/RandomUnitVectors.cpp ../../polyvox/library/polyvoxcore/source/Region.cpp ../../polyvox/library/polyvoxcore/source/Impl/Utility.cpp OctreeNode.cpp

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2 -lOpenSLES
LOCAL_CFLAGS    := -D__ANDROID__ -Wno-psabi -I"../../gameplay/external-deps/lua/include" -I"../../gameplay/external-deps/bullet/include" -I"../../gameplay/external-deps/libpng/include" -I"../../gameplay/external-deps/oggvorbis/include" -I"../../gameplay/external-deps/openal/include" -I"../../gameplay/gameplay/src" -I"../../polyvox/library/PolyVoxCore/include"

LOCAL_STATIC_LIBRARIES := android_native_app_glue libgameplay libpng libzlib liblua libbullet libvorbis libOpenAL

include $(BUILD_SHARED_LIBRARY)
$(call import-module,android/native_app_glue)
