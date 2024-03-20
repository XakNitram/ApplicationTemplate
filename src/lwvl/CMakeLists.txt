cmake_minimum_required(VERSION 3.17)
project(LWVL VERSION 3.0.0)

add_library(
    lwvl

    include/lwvl/lwvl.hpp
    include/lwvl/stb_image.h
    src/stb_image.cpp
    src/Buffer.cpp
    src/Common.cpp
    src/Shader.cpp
    src/Texture.cpp
    src/VertexArray.cpp
)

target_include_directories(lwvl PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(lwvl PUBLIC glad)
