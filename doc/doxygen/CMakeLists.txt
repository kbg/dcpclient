find_package(Doxygen QUIET)

if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/doc)

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY)

    add_custom_target(doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation..."
        VERBATIM)

    set_property(DIRECTORY APPEND PROPERTY
        ADDITIONAL_MAKE_CLEAN_FILES ${DOXYGEN_OUTPUT_DIR}/html)

    if(BUILD_DOCUMENTATION)
        add_custom_target(doc ALL)
        install(DIRECTORY ${DOXYGEN_OUTPUT_DIR}/html
                DESTINATION share/doc/dcpclient)
    else()
        add_custom_target(doc)
    endif()
    add_dependencies(doc doxygen)
else()
    if(BUILD_DOCUMENTATION)
        message("Doxygen not found, cannot generate documentation.")
    endif()
endif()
