cmake_minimum_required(VERSION 3.14)

# Specify multi-target for OSX

if (APPLE AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

project(ikuyo C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) # NOTE: required for Linux builds
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# Detect architecture

if(MSVC)
    string(TOLOWER "${MSVC_C_ARCHITECTURE_ID}" IKUYO_PROCESSOR)
else()
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" IKUYO_PROCESSOR)
endif()

if(APPLE)
    set(IKUYO_ARCHITECTURE "universal")
elseif(IKUYO_PROCESSOR MATCHES "^(x86_64|amd64|x64)$")
    set(IKUYO_ARCHITECTURE "x86_64")
elseif(IKUYO_PROCESSOR MATCHES "^(aarch64|arm64)$")
    set(IKUYO_ARCHITECTURE "arm64")
else()
    set(IKUYO_ARCHITECTURE "unknown")
    message(WARNING "Unknown architecture (${IKUYO_PROCESSOR}), only x86_64 and ARM64 are natively supported")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(WARNING "Building for 32-bit systems is not supported, issues may occur!")
endif()

message(STATUS "Detected architecture as ${IKUYO_ARCHITECTURE}")

set(IKUYO_LIBRARIES webp lz4 libdeflate_static 7zip ikuyo_bridge)

include_directories(include external/bcdec external/stb external/libdeflate)
add_library(ikuyo STATIC
    src/common/bytestream.c
    src/common/texture.c
    src/common/image.c

    src/format/dds/dds.c

    src/format/unityfs/assetbundle.c
    src/format/unityfs/cab.c

    src/format/ue4/uexp.c

    src/format/aft/farc.c
    src/format/aft/txp.c
)
target_link_libraries(ikuyo PUBLIC ${IKUYO_LIBRARIES})
set_target_properties(ikuyo PROPERTIES OUTPUT_NAME "ikuyo-lib")

# Command-line Interface

add_executable(ikuyo_cli src/interface/cli.c $<TARGET_OBJECTS:ikuyo>)
target_link_libraries(ikuyo_cli PUBLIC ${IKUYO_LIBRARIES})
set_target_properties(ikuyo_cli PROPERTIES
    OUTPUT_NAME "ikuyo"
    PDB_NAME "ikuyo-cli"
)

# Java Native Interface

option(IKUYO_JNI "Enable building of Ikuyo's JNI interface" ON)
option(IKUYO_JNI_COPY "Copy the JNI library to the Kotlin resources directory after build" ON)

if (IKUYO_JNI)
    set(JAVA_INCLUDE_PATH "$ENV{JAVA_HOME}/include")
    set(JAVA_AWT_INCLUDE_PATH "$ENV{JAVA_HOME}/include")
    find_package(JNI)
    if (JNI_FOUND)
        add_library(ikuyo_jni SHARED src/interface/jni.c $<TARGET_OBJECTS:ikuyo>)
        target_include_directories(ikuyo_jni PRIVATE ${JNI_INCLUDE_DIRS})
        target_link_libraries(ikuyo_jni PRIVATE ${IKUYO_LIBRARIES} JNI::JNI)
        set_target_properties(ikuyo_jni PROPERTIES
            OUTPUT_NAME "ikuyo-jni-${IKUYO_ARCHITECTURE}"
            PDB_NAME "ikuyo-jni"
        )
        if (IKUYO_JNI_COPY)
            add_custom_command(
                TARGET ikuyo_jni
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:ikuyo_jni>
                "${CMAKE_SOURCE_DIR}/jni/src/main/resources"
                COMMENT "Copying Ikuyo JNI library"
                VERBATIM
            )
        endif()
    else()
        message(WARNING "JDK not found, not building Ikuyo JNI")
    endif()
endif()

# libavif / libwebp

set(BUILD_SHARED_LIBS OFF)
set(WEBP_LINK_STATIC ON)
set(WEBP_BUILD_ANIM_UTILS OFF)
set(WEBP_BUILD_CWEBP OFF)
set(WEBP_BUILD_DWEBP OFF)
set(WEBP_BUILD_GIF2WEBP OFF)
set(WEBP_BUILD_IMG2WEBP OFF)
set(WEBP_BUILD_VWEBP OFF)
set(WEBP_BUILD_WEBPINFO OFF)
set(WEBP_BUILD_WEBPMUX OFF)
set(WEBP_BUILD_EXTRAS OFF)
set(WEBP_BUILD_TESTS OFF)

add_subdirectory(external/libwebp)

# LZ4 / libdeflate

set(LIBDEFLATE_BUILD_STATIC_LIB ON CACHE BOOL "" FORCE)
set(LIBDEFLATE_BUILD_SHARED_LIB OFF CACHE BOOL "" FORCE)
set(LIBDEFLATE_USE_SHARED_LIB OFF CACHE BOOL "" FORCE)
set(LIBDEFLATE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LIBDEFLATE_COMPRESSION_SUPPORT OFF CACHE BOOL "" FORCE)
set(LIBDEFLATE_BUILD_GZIP OFF CACHE BOOL "" FORCE) # NOTE: this disables the building of the `libdeflate-gzip`, it does NOT remove gzip support

set(LZ4_BUILD_CLI OFF CACHE BOOL "" FORCE)
set(LZ4_BUILD_LEGACY_LZ4C OFF CACHE BOOL "" FORCE)

add_subdirectory(external/lz4/build/cmake)
add_subdirectory(external/libdeflate)

# LZMA (7-zip)

file(GLOB 7ZIP_SOURCES external/7zip/C/*.c)
add_library(7zip ${7ZIP_SOURCES})
target_include_directories(7zip PUBLIC external/7zip/C)

# Bridge (for fpng)

add_library(ikuyo_bridge STATIC
    src/common/bridge/bridge.cpp
    external/fpng/src/fpng.cpp
)
set_target_properties(ikuyo_bridge PROPERTIES LANGUAGE CXX)
target_include_directories(ikuyo_bridge PUBLIC include external/fpng/src)
target_compile_definitions(ikuyo_bridge PUBLIC FPNG_NO_SSE)