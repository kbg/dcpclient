project(libDcpClient)

include_directories(
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTNETWORK_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(DCPCLIENT_VERSION_MAJOR 0)
set(DCPCLIENT_VERSION_MINOR 1)
set(DCPCLIENT_VERSION_RELEASE 0)
set(DCPCLIENT_VERSION_STRING "${DCPCLIENT_VERSION_MAJOR}.${DCPCLIENT_VERSION_MINOR}.${DCPCLIENT_VERSION_RELEASE}")
set(DCPCLIENT_LIB_VERSION ${DCPCLIENT_VERSION_STRING})
set(DCPCLIENT_LIB_SOVERSION ${DCPCLIENT_VERSION_MAJOR})
configure_file(version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

set(libDcpClient_SRCS
    dcpclient.cpp
    dcpmessage.cpp
    dcpmessageparser.cpp
    dcpclient_p.cpp
    version.cpp
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

install(FILES dcpclient.h
              dcpmessage.h
              dcpmessageparser.h
              ${CMAKE_CURRENT_BINARY_DIR}/version.h
        DESTINATION include)

install(TARGETS DcpClient
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
