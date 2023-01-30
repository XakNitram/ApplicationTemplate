cmake_minimum_required(VERSION 3.24)
project(Template VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(${CMAKE_PROJECT_NAME})
set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO

    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/
)

add_subdirectory(ext)
add_subdirectory(res)
add_subdirectory(src)
