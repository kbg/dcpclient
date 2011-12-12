project(dcpclient)
cmake_minimum_required(VERSION 2.8)

find_package(Qt4 4.7.0 COMPONENTS QtCore QtNetwork REQUIRED)

option(BUILD_STATIC_LIBRARY "Build dcpclient as static library." FALSE)
option(BUILD_TOOLS "Build utility programs." FALSE)
option(BUILD_EXAMPLES "Build example programs." FALSE)
option(INSTALL_EXAMPLE_SOURCES "Install source code of the utiliy and example programs." FALSE)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_subdirectory(src)
