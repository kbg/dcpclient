project(libDcpClient)

set(DCPCLIENT_LIB_VERSION ${DCPCLIENT_VERSION_STRING})
set(DCPCLIENT_LIB_SOVERSION ${DCPCLIENT_VERSION_MAJOR})
configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
file(COPY dcpclient_export.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(libDcpClient_public_HDRS
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
    dcpclient_export.h
    dcpclient.h
    client.h
    message.h
    messageparser.h
)

set(libDcpClient_SRCS
    client.cpp
    message.cpp
    messageparser.cpp
    dcpclient_p.cpp
    version.cpp
)

if(BUILD_STATIC_LIBRARY)
    # The -fPIC option is needed to link the resulting static library from
    # shared libraries, in this case from the Python module.
    set(CMAKE_CXX_FLAGS "-fPIC")
    set(_libtype STATIC)
    add_definitions(-DDCPCLIENT_STATIC_LIB)
else()
    set(_libtype SHARED)
endif()

add_library(DcpClient ${_libtype} ${libDcpClient_SRCS})

if (TARGET Qt5::Core)
    target_link_libraries(DcpClient PUBLIC Qt5::Core Qt5::Network)
else()
    target_link_libraries(DcpClient PUBLIC Qt4::QtCore Qt4::QtNetwork)
endif()

set_target_properties(DcpClient PROPERTIES
    DEFINE_SYMBOL MAKE_DCPCLIENT_LIBRARY
    VERSION ${DCPCLIENT_LIB_VERSION}
    SOVERSION ${DCPCLIENT_LIB_SOVERSION}
)

install(FILES ${libDcpClient_public_HDRS}
        DESTINATION include/dcpclient)

install(TARGETS DcpClient
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
