project(dcpclient)
cmake_minimum_required(VERSION 2.8)

find_package(Qt4 4.7.0 COMPONENTS QtCore QtNetwork REQUIRED)
include(${QT_USE_FILE})

##

set(dcpclient_MOC_HDRS
  src/dcp.h)
qt4_wrap_cpp(dcpclient_MOC_SCRS ${dcpclient_MOC_HDRS})

set(dcpclient_SRCS
  ${dcpclient_MOC_SCRS}
  src/dcp.cpp
)

##

set(dcpDump_MOC_HDRS
  src/dcpdump.h)
qt4_wrap_cpp(dcpDump_MOC_SCRS ${dcpDump_MOC_HDRS})

set(dcpDump_SRCS
  ${dcpDump_MOC_SCRS}
  ${dcpclient_SRCS}
  src/dcpdump.cpp
  src/dcpdumpmain.cpp
)

add_executable(dcpDump ${dcpDump_SRCS})
target_link_libraries(dcpDump ${QT_LIBRARIES})

##

set(dcpListen_SRCS
  ${dcpclient_SRCS}
  src/dcplisten.cpp
)

add_executable(dcpListen ${dcpListen_SRCS})
target_link_libraries(dcpListen ${QT_LIBRARIES})

##

#set(dcpEcho_MOC_HDRS
#  src/echoserver.h)
#qt4_wrap_cpp(dcpEcho_MOC_SCRS ${dcpEcho_MOC_HDRS})
#
#set(dcpEcho_SRCS
#  ${dcpEcho_MOC_SCRS}
#  ${dcpclient_SRCS}
#  src/dcpecho.cpp
#  src/echoserver.cpp
#)
#
#add_executable(dcpEcho ${dcpEcho_SRCS})
#target_link_libraries(dcpEcho ${QT_LIBRARIES})
