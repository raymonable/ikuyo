cmake_minimum_required(VERSION 3.14)
project(ikuyo C CXX)

set(CMAKE_C_STANDARD 17)

include_directories(include external/bcdec)
add_executable(ikuyo
        src/cli.c
        src/common/bytestream.c
        src/common/texture.c

        src/format/dds/dds.c
        src/common/image.c
)

# Libraries

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

add_subdirectory(external/libwebp)
target_link_libraries(ikuyo PUBLIC webp)

# Bridge

add_library(ikuyo_bridge STATIC
        src/bridge.cpp
        external/fpng/src/fpng.cpp
)
set_target_properties(ikuyo_bridge PROPERTIES LANGUAGE CXX)
target_include_directories(ikuyo_bridge PUBLIC include external/fpng/src)

target_link_libraries(ikuyo PUBLIC ikuyo_bridge)