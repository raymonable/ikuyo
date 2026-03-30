cmake_minimum_required(VERSION 4.1)
project(ikuyo C CXX)

set(CMAKE_C_STANDARD 17)

include_directories(include external/bcdec)
add_executable(ikuyo
        src/cli.c
        src/common/bytestream.c
        src/common/texture.c

        src/format/dds/dds.c
)

# Bridge

add_library(ikuyo_bridge STATIC
        src/bridge.cpp
        external/fpng/src/fpng.cpp
)
set_target_properties(ikuyo_bridge PROPERTIES LANGUAGE CXX)
target_include_directories(ikuyo_bridge PUBLIC include external/fpng/src)

target_link_libraries(ikuyo PUBLIC ikuyo_bridge)