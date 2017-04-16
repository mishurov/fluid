cmake_minimum_required(VERSION 3.4.1)

project(libpng C)

set(external_libng "${CMAKE_CURRENT_SOURCE_DIR}/platform_external_libpng")

set(SRC_FILES_COMMON
    "${external_libng}/png.c"
    "${external_libng}/png.c"
    "${external_libng}/pngerror.c"
    "${external_libng}/pngget.c"
    "${external_libng}/pngmem.c"
    "${external_libng}/pngpread.c"
    "${external_libng}/pngread.c"
    "${external_libng}/pngrio.c"
    "${external_libng}/pngrtran.c"
    "${external_libng}/pngrutil.c"
    "${external_libng}/pngset.c"
    "${external_libng}/pngtrans.c"
    "${external_libng}/pngwio.c"
    "${external_libng}/pngwrite.c"
    "${external_libng}/pngwtran.c"
    "${external_libng}/pngwutil.c"
)

set(SRC_FILES_ARM
    "${external_libng}/arm/arm_init.c"
    "${external_libng}/arm/filter_neon.S"
    "${external_libng}/arm/filter_neon_intrinsics.c"
)

set(SRC_FILES_INTEL
    "${external_libng}/contrib/intel/intel_init.c"
    "${external_libng}/contrib/intel/filter_sse2_intrinsics.c"
)

set(CFLAGS_COMMON
    "-std=gnu89 -Wno-unused-parameter"
)

set(CFLAGS_INTEL
    "-DPNG_INTEL_SSE_OPT=1"
)

set(sources ${SRC_FILES_COMMON})
set(cflags ${CFLAGS_COMMON})


if( ${ANDROID_ABI} STREQUAL "x86"
    OR ${ANDROID_ABI} STREQUAL "x86_64" )
    set(sources ${SRC_FILES_COMMON} ${SRC_FILES_INTEL})
    set(cflags "${CFLAGS_COMMON} ${CFLAGS_INTEL}")
endif()

if( ${ANDROID_ABI} STREQUAL "armeabi"
    OR ${ANDROID_ABI} STREQUAL "armeabi-v7a"
    OR ${ANDROID_ABI} STREQUAL "arm64-v8a" )
    set(sources ${SRC_FILES_COMMON} ${SRC_FILES_ARM})
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${cflags}")


# download the sources from the android git repo

find_package(Git REQUIRED)

add_custom_command(OUTPUT ${sources}
  COMMAND ${GIT_EXECUTABLE} init
  COMMAND ${GIT_EXECUTABLE} pull https://github.com/android/platform_external_libpng
  COMMAND ${GIT_EXECUTABLE} checkout ad4a6db68e88059a3e6b9a27065ba7795c68159d
  WORKING_DIRECTORY "${external_libng}"
  COMMENT "Downloading libng source..."
  VERBATIM
)

add_library( png STATIC ${sources} )
target_link_libraries( png z )


# put the binary and headers into an appropriate location

set(distribution_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../distribution)

set_target_properties(png
                      PROPERTIES
                      ARCHIVE_OUTPUT_DIRECTORY
                      "${distribution_DIR}/png/lib/${ANDROID_ABI}")

file(MAKE_DIRECTORY ${distribution_DIR}/png/include)

add_custom_command(TARGET png POST_BUILD
                   COMMAND "${CMAKE_COMMAND}" -E
                   copy "${external_libng}/*.h"
                   "${distribution_DIR}/png/include/")

