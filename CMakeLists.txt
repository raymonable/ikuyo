cmake_minimum_required(VERSION 3.14)
project(ikuyo C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

include_directories(include external/bcdec external/stb)
add_executable(ikuyo
        src/cli.c
        src/common/bytestream.c
        src/common/texture.c
        src/common/image.c

        src/format/dds/dds.c
        src/format/unityfs/assetbundle.c
        src/format/ue4/uexp.c
)

# WebAssembly configuration

if (EMSCRIPTEN)
    set_target_properties(ikuyo PROPERTIES LINK_FLAGS
            "-Os -s WASM=1 -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=1MB -sMODULARIZE=1 -sENVIRONMENT=web -sEXPORTED_FUNCTIONS=_free,_malloc,_main"
    )
endif()

# Libwebp

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

# LZ4

add_subdirectory(external/lz4/build/cmake)
target_link_libraries(ikuyo PUBLIC lz4)

# Bridge

add_library(ikuyo_bridge STATIC
        src/bridge.cpp
        external/fpng/src/fpng.cpp
)
set_target_properties(ikuyo_bridge PROPERTIES LANGUAGE CXX)
target_include_directories(ikuyo_bridge PUBLIC include external/fpng/src)
target_compile_definitions(ikuyo_bridge PUBLIC FPNG_NO_SSE)

target_link_libraries(ikuyo PUBLIC ikuyo_bridge)