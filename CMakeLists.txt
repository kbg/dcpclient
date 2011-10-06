project(dcpclient)
cmake_minimum_required(VERSION 2.8)

find_package(Qt4 4.7.0 COMPONENTS QtCore QtNetwork REQUIRED)
include(${QT_USE_FILE})

##

set(dcpclient_MOC_HDRS
  src/dcpconnection.h)
qt4_wrap_cpp(dcpclient_MOC_SCRS ${dcpclient_MOC_HDRS})

set(dcpclient_SRCS
  ${dcpclient_MOC_SCRS}
  src/dcpmessage.cpp
  src/dcpconnection.cpp
  src/dcp_p.cpp
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

set(dcpSend_SRCS
  ${dcpclient_SRCS}
  src/dcpsend.cpp
)

add_executable(dcpSend ${dcpSend_SRCS})
target_link_libraries(dcpSend ${QT_LIBRARIES})
