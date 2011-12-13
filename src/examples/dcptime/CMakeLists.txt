project(dcptime)

include_directories(
    ${QT_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

## DcpTimeServer ##
set(dcptimeserver_SRCS dcptimeserver.cpp dcptimeserver_main.cpp)
qt4_wrap_cpp(dcptimeserver_MOC_SRCS dcptimeserver.h)
add_executable(dcptimeserver ${dcptimeserver_SRCS} ${dcptimeserver_MOC_SRCS})
target_link_libraries(dcptimeserver DcpClient
    ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})
install(TARGETS dcptimeserver RUNTIME DESTINATION bin)

## DcpTimeClient ##
if(QT_QTGUI_FOUND)
    set(dcptimeclient_SRCS dcptimeclient.cpp dcptimeclient_main.cpp)
    qt4_wrap_cpp(dcptimeclient_MOC_SRCS dcptimeclient.h)
    add_executable(dcptimeclient ${dcptimeclient_SRCS} ${dcptimeclient_MOC_SRCS})
    target_link_libraries(dcptimeclient DcpClient
        ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTGUI_LIBRARY})
    install(TARGETS dcptimeclient RUNTIME DESTINATION bin)
else()
    message("Skipping build of dcptimeclient because of missing QtGui library.")
endif()
