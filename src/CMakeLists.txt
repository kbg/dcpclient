add_subdirectory(dcpclient)
add_subdirectory(examples)
add_subdirectory(tools)

if(BUILD_PYTHON_BINDINGS)
    add_subdirectory(python)
endif()
