project(dcpclient)
cmake_minimum_required(VERSION 2.8)

find_package(Qt4 4.7.0 COMPONENTS QtCore QtNetwork REQUIRED)
include(${QT_USE_FILE})

##

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src)

##

set(dcpclient_SRCS
  src/dcpmessage.cpp
  src/dcpconnection.cpp
  src/dcpclient.cpp
  src/dcp_p.cpp
  src/dcp.cpp
)
qt4_automoc(${dcpclient_SRCS})

##

set(dcpdump_SRCS
  ${dcpclient_SRCS}
  src/examples/dcpdump.cpp
  src/examples/dcpdump_main.cpp
)
qt4_automoc(${dcpdump_SRCS})

add_executable(dcpdump ${dcpdump_SRCS})
target_link_libraries(dcpdump ${QT_LIBRARIES})

##

set(dcplisten_SRCS
  ${dcpclient_SRCS}
  src/examples/dcplisten.cpp
)

add_executable(dcplisten ${dcplisten_SRCS})
target_link_libraries(dcplisten ${QT_LIBRARIES})

##

set(dcpsend_SRCS
  ${dcpclient_SRCS}
  src/examples/dcpsend.cpp
)

add_executable(dcpsend ${dcpsend_SRCS})
target_link_libraries(dcpsend ${QT_LIBRARIES})
