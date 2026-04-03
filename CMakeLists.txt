cmake_minimum_required(VERSION 3.14)
project(ikuyo C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

set(IKUYO_LIBRARIES webp ikuyo_bridge lz4)

include_directories(include external/bcdec external/stb)
add_library(ikuyo SHARED
        # TODO: export a shared library interface
        src/common/bytestream.c
        src/common/texture.c
        src/common/image.c

        src/format/dds/dds.c
        src/format/unityfs/assetbundle.c
        src/format/ue4/uexp.c
)
target_link_libraries(ikuyo PUBLIC ${IKUYO_LIBRARIES})
set_target_properties(ikuyo PROPERTIES PDB_NAME "ikuyo-lib")

add_executable(ikuyo_cli
        $<TARGET_OBJECTS:ikuyo>
        src/cli.c
)
target_link_libraries(ikuyo_cli PUBLIC ${IKUYO_LIBRARIES})
set_target_properties(ikuyo_cli PROPERTIES
        OUTPUT_NAME "ikuyo"
        PDB_NAME "ikuyo-cli"
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
set(WEBP_BUILD_TESTS OFF)

add_subdirectory(external/libwebp)

# LZ4

add_subdirectory(external/lz4/build/cmake)

# Bridge

add_library(ikuyo_bridge STATIC
        src/bridge.cpp
        external/fpng/src/fpng.cpp
)
set_target_properties(ikuyo_bridge PROPERTIES LANGUAGE CXX)
target_include_directories(ikuyo_bridge PUBLIC include external/fpng/src)
target_compile_definitions(ikuyo_bridge PUBLIC FPNG_NO_SSE)