cmake_minimum_required(VERSION 3.28)
project(imgui VERSION 1.90.4 LANGUAGES CXX)

add_library(
    ${PROJECT_NAME}
    include/imgui/imconfig.h
    include/imgui/imgui.h
    include/imgui/imgui_internal.h
    include/imgui/imstb_rectpack.h
    include/imgui/imstb_textedit.h
    include/imgui/imstb_truetype.h

    src/imgui.cpp
    src/imgui_draw.cpp
    src/imgui_tables.cpp
    src/imgui_widgets.cpp
    src/imgui_demo.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/imgui")

set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
)
