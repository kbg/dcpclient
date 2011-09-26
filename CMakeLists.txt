project(dcpclient)
cmake_minimum_required(VERSION 2.8)

find_package(Qt4 4.6.0 COMPONENTS QtCore QtNetwork REQUIRED)
include(${QT_USE_FILE})

set(dcpEcho_MOC_HDRS
  src/dcp.h
  src/echoserver.h
)
qt4_wrap_cpp(dcpEcho_MOC_SCRS ${dcpEcho_MOC_HDRS})

set(dcpEcho_SRCS ${dcpEcho_MOC_SCRS}
  src/dcp.cpp
  src/dcpecho.cpp
  src/echoserver.cpp
)

add_executable(dcpEcho ${dcpEcho_SRCS})
target_link_libraries(dcpEcho ${QT_LIBRARIES})
