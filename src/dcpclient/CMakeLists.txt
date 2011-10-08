project(libDcpClient)

include_directories(
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTNETWORK_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(libDcpClient_SRCS
    dcpclient.cpp
    dcpmessage.cpp
    dcp_p.cpp
)
qt4_automoc(${libDcpClient_SRCS})

if(BUILD_STATIC_LIBRARY)
    set(_libtype STATIC)
else()
    set(_libtype SHARED)
endif()

add_library(DcpClient ${_libtype} ${libDcpClient_SRCS})
target_link_libraries(DcpClient ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})
target_link_libraries(DcpClient LINK_INTERFACE_LIBRARIES
    ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})

set_target_properties(DcpClient PROPERTIES
    VERSION ${DCPCLIENT_LIB_VERSION}
    SOVERSION ${DCPCLIENT_LIB_SOVERSION}
)
