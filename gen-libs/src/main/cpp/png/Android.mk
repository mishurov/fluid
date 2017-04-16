LOCAL_PATH:= $(call my-dir)

# We need to build this for both the device (as a shared library)
# and the host (as a static library for tools to use).

common_SRC_FILES := \
    platform_external_libpng/png.c \
    platform_external_libpng/pngerror.c \
    platform_external_libpng/pngget.c \
    platform_external_libpng/pngmem.c \
    platform_external_libpng/pngpread.c \
    platform_external_libpng/pngread.c \
    platform_external_libpng/pngrio.c \
    platform_external_libpng/pngrtran.c \
    platform_external_libpng/pngrutil.c \
    platform_external_libpng/pngset.c \
    platform_external_libpng/pngtrans.c \
    platform_external_libpng/pngwio.c \
    platform_external_libpng/pngwrite.c \
    platform_external_libpng/pngwtran.c \
    platform_external_libpng/pngwutil.c \

ifeq ($(ARCH_ARM_HAVE_NEON),true)
my_cflags_arm := -DPNG_ARM_NEON_OPT=2
endif

my_cflags_arm64 := -DPNG_ARM_NEON_OPT=2

my_src_files_arm := \
    platform_external_libpng/arm/arm_init.c \
    platform_external_libpng/arm/filter_neon.S \
    platform_external_libpng/arm/filter_neon_intrinsics.c

my_cflags_intel := -DPNG_INTEL_SSE_OPT=1

my_src_files_intel := \
    platform_external_libpng/contrib/intel/intel_init.c \
    platform_external_libpng/contrib/intel/filter_sse2_intrinsics.c

common_CFLAGS := -std=gnu89 -Wno-unused-parameter #-fvisibility=hidden ## -fomit-frame-pointer

# For the device (static) for NDK
# =====================================================

include $(CLEAR_VARS)
LOCAL_CLANG := true
LOCAL_SRC_FILES := $(common_SRC_FILES)
LOCAL_CFLAGS += $(common_CFLAGS) -ftrapv

ifeq ($(TARGET_ARCH_ABI),x86)
    LOCAL_SRC_FILES += $(my_src_files_intel)
    LOCAL_CFLAGS += $(my_cflags_intel)
endif
ifeq ($(TARGET_ARCH_ABI),x86_64)
    LOCAL_SRC_FILES += $(my_src_files_intel)
    LOCAL_CFLAGS += $(my_cflags_intel)
endif

ifeq ($(TARGET_ARCH_ABI),armeabi)
    LOCAL_SRC_FILES += $(my_src_files_arm)
    LOCAL_CFLAGS += $(my_cflags_arm)
endif
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_SRC_FILES += $(my_src_files_arm)
    LOCAL_CFLAGS += $(my_cflags_arm)
endif
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
    LOCAL_SRC_FILES += $(my_src_files_arm)
    LOCAL_CFLAGS += $(my_cflags_arm64)
endif

LOCAL_ASFLAGS += $(common_ASFLAGS)
LOCAL_SANITIZE := never
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_EXPORT_LDLIBS    := -lz
LOCAL_MODULE:= libpng_ndk
include $(BUILD_STATIC_LIBRARY)

