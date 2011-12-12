project(dcptime)

include_directories(
    ${QT_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${libDcpClient_SOURCE_DIR}
)

## DcpTimeServer ##
set(dcptimeserver_SRCS dcptimeserver.cpp dcptimeserver_main.cpp)
qt4_automoc(${dcptimeserver_SRCS})
add_executable(dcptimeserver ${dcptimeserver_SRCS})
target_link_libraries(dcptimeserver DcpClient
    ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})
install(TARGETS dcptimeserver RUNTIME DESTINATION bin)

## DcpTimeClient ##
if(QT_QTGUI_FOUND)
    set(dcptimeclient_SRCS dcptimeclient.cpp dcptimeclient_main.cpp)
    qt4_automoc(${dcptimeclient_SRCS})
    add_executable(dcptimeclient ${dcptimeclient_SRCS})
    target_link_libraries(dcptimeclient DcpClient
        ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTGUI_LIBRARY})
    install(TARGETS dcptimeclient RUNTIME DESTINATION bin)
else()
    message("Skipping build of dcptimeclient because of missing QtGui library.")
endif()
