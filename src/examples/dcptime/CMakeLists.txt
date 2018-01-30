project(dcptime)

include_directories(${CMAKE_SOURCE_DIR}/src)

## DcpTimeServer ##
set(dcptimeserver_SRCS
    dcptimeserver.cpp
    dcptimeserver_main.cpp
)
add_executable(dcptimeserver ${dcptimeserver_SRCS})
target_link_libraries(dcptimeserver DcpClient Qt4::QtNetwork)
install(TARGETS dcptimeserver RUNTIME DESTINATION bin)

## DcpTimeClient ##
if(TARGET Qt4::QtGui)
    set(dcptimeclient_SRCS
        dcptimeclient.cpp
        dcptimeclient_main.cpp
    )
    add_executable(dcptimeclient ${dcptimeclient_SRCS})
    target_link_libraries(dcptimeclient DcpClient Qt4::QtNetwork Qt4::QtGui)
    install(TARGETS dcptimeclient RUNTIME DESTINATION bin)
else()
    message("Skipping build of dcptimeclient because of missing QtGui library.")
endif()
