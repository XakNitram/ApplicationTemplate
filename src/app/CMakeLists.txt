target_sources(
    ${CMAKE_PROJECT_NAME} PRIVATE
        Application.cpp

        Platform/GLFW/imgui_impl_glfw.h
        Platform/GLFW/imgui_impl_glfw.cpp
        Platform/OpenGL/imgui_impl_opengl3.h
        Platform/OpenGL/imgui_impl_opengl3.cpp
    PRIVATE FILE_SET CXX_MODULES FILES
        # CORE
        Core/Window.cppm

        # WORLD
        World/Board.cppm

        # STRUCTURES
        Structures/Array.cppm
#        Structures/Structures.cppm
        Structures/Tether.cppm
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_precompile_headers(${CMAKE_PROJECT_NAME} PRIVATE pch.hpp pch.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE lwvl)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE imgui)
