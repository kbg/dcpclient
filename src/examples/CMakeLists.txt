include_directories(
    ${QT_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${libDcpClient_SOURCE_DIR}
)

## DcpDump example ##
set(dcpdump_SRCS dcpdump.cpp dcpdump_main.cpp)
qt4_automoc(${dcpdump_SRCS})
add_executable(dcpdump ${dcpdump_SRCS})
target_link_libraries(dcpdump DcpClient
    ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})

## DcpListen example ##
set(dcplisten_SRCS dcplisten.cpp)
add_executable(dcplisten ${dcplisten_SRCS})
target_link_libraries(dcplisten DcpClient
    ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})

## DcpTimeServer example ##
set(dcptimeserver_SRCS dcptimeserver.cpp dcptimeserver_main.cpp)
qt4_automoc(${dcptimeserver_SRCS})
add_executable(dcptimeserver ${dcptimeserver_SRCS})
target_link_libraries(dcptimeserver DcpClient
    ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})

## DcpTimeClient example ##
set(dcptimeclient_SRCS dcptimeclient.cpp dcptimeclient_main.cpp)
qt4_automoc(${dcptimeclient_SRCS})
add_executable(dcptimeclient ${dcptimeclient_SRCS})
target_link_libraries(dcptimeclient DcpClient
    ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTGUI_LIBRARY})
