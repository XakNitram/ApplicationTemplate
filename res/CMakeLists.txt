include(${CMAKE_SOURCE_DIR}/cmake/export_resources.cmake)

# We define this target over the individual resource targets so they can all be built together,
# rather than shaders then GLFW, then fonts, then Glad
add_custom_target(resources)
add_dependencies(${CMAKE_PROJECT_NAME} resources)

export_resources(
    TARGET resources
    DIRECTORY Shaders
    DEPENDS
        default.vert
        default.frag
)
