function(export_resources)
    cmake_parse_arguments(PARSE_ARGV 0 "EXPORT" "" "TARGET;DIRECTORY" "DEPENDS")
    if(NOT EXPORT_TARGET)
        message(FATAL_ERROR "No target specified (with TARGET keyword) in call to function 'export_resources'")
        return()
    endif()

    if(NOT EXPORT_DIRECTORY)
        message(FATAL_ERROR "No directory specified (with DIRECTORY keyword) in call to function 'export_resources'")
        return()
    endif()

    if(NOT EXPORT_DEPENDS)
        message(FATAL_ERROR "No dependencies specified (with DEPENDS keyword) in call to function 'export_resources'")
        return()
    endif()

    string(TOLOWER "${EXPORT_DIRECTORY}" TARGET)

    set(OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Data/${EXPORT_DIRECTORY}/")
    list(TRANSFORM EXPORT_DEPENDS PREPEND ${OUTPUT_DIRECTORY} OUTPUT_VARIABLE RESOURCE_OUT)
    list(TRANSFORM EXPORT_DEPENDS PREPEND "${CMAKE_CURRENT_LIST_DIR}/${EXPORT_DIRECTORY}/" OUTPUT_VARIABLE RESOURCE_IN)

    add_custom_target(${TARGET} DEPENDS ${RESOURCE_OUT})
    add_dependencies(${EXPORT_TARGET} ${TARGET})

    add_custom_command(
        DEPENDS ${RESOURCE_IN}
        OUTPUT ${RESOURCE_OUT}

        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_COMMAND} -E copy ${RESOURCE_IN} ${OUTPUT_DIRECTORY}

        COMMENT "Copied ${TARGET} to output"
    )
endfunction()


# ****** Example ******
# export_resources(
#   TARGET resources
#   DIRECTORY Shaders
#   DEPENDS
#     default.vert
#     default.frag
# )
