project(dcpclient)
cmake_minimum_required(VERSION 2.8.12)

set(DCPCLIENT_VERSION_MAJOR 0)
set(DCPCLIENT_VERSION_MINOR 3)
set(DCPCLIENT_VERSION_RELEASE 2)
set(DCPCLIENT_VERSION_STRING "${DCPCLIENT_VERSION_MAJOR}.${DCPCLIENT_VERSION_MINOR}.${DCPCLIENT_VERSION_RELEASE}")

if(POLICY CMP0071)
    # CMake 3.10 changed the AUTOMOC behaviour on generated files (like UI
    # header files). To silence the warnings, we set it explicitly to the old
    # behaviour here.
    cmake_policy(SET CMP0071 OLD)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(BUILD_USE_QT4 "Use Qt4 even if Qt5 is installed" FALSE)

if (NOT BUILD_USE_QT4)
    find_package(Qt5 REQUIRED COMPONENTS Core Network)
    find_package(Qt5 QUIET COMPONENTS Widgets)
endif()

if (NOT TARGET Qt5::Core)
    find_package(Qt4 4.7.0 REQUIRED COMPONENTS QtCore QtNetwork)
    find_package(Qt4 4.7.0 QUIET COMPONENTS QtGui)
endif()

option(BUILD_STATIC_LIBRARY "Build dcpclient as static library." FALSE)
option(BUILD_TOOLS "Build utility programs." FALSE)
option(BUILD_EXAMPLES "Build example programs." FALSE)
option(BUILD_DOCUMENTATION "Build doxygen documentation." FALSE)
option(BUILD_PYTHON_BINDINGS "Build Python bindings." FALSE)
option(INSTALL_EXAMPLE_SOURCES "Install source code of the utility and example programs." FALSE)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# Use RPATH on UNIX operating systems
if(UNIX)
    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

add_subdirectory(src)
add_subdirectory(doc)
