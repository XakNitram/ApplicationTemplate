target_sources(
    ${CMAKE_PROJECT_NAME} PRIVATE Application.cpp
    PRIVATE FILE_SET CXX_MODULES FILES
    # CORE
    Core/Window.cppm

    # STRUCTURES
    Structures/Structures.cppm
    Structures/UnmanagedArray.cppm
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_precompile_headers(${CMAKE_PROJECT_NAME} PRIVATE pch.hpp pch.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE lwvl)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glfw)
