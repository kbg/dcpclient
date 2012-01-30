project(dcpclient)
cmake_minimum_required(VERSION 2.8)

set(DCPCLIENT_VERSION_MAJOR 0)
set(DCPCLIENT_VERSION_MINOR 1)
set(DCPCLIENT_VERSION_RELEASE 0)
set(DCPCLIENT_VERSION_STRING "${DCPCLIENT_VERSION_MAJOR}.${DCPCLIENT_VERSION_MINOR}.${DCPCLIENT_VERSION_RELEASE}")

find_package(Qt4 4.7.0 COMPONENTS QtCore QtNetwork REQUIRED)

option(BUILD_STATIC_LIBRARY "Build dcpclient as static library." FALSE)
option(BUILD_TOOLS "Build utility programs." FALSE)
option(BUILD_EXAMPLES "Build example programs." FALSE)
option(BUILD_DOCUMENTATION "Build doxygen documentation." FALSE)
option(INSTALL_EXAMPLE_SOURCES "Install source code of the utiliy and example programs." FALSE)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_subdirectory(src)
add_subdirectory(doc)
