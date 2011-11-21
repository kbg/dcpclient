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

## DcpSend example ##
set(dcpsend_SRCS dcpsend.cpp)
add_executable(dcpsend ${dcpsend_SRCS})
target_link_libraries(dcpsend DcpClient
    ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})

## DcpTime example ##
set(dcptime_SRCS dcptime.cpp dcptime_main.cpp)
qt4_automoc(${dcptime_SRCS})
add_executable(dcptime ${dcptime_SRCS})
target_link_libraries(dcptime DcpClient
    ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})

## DcpTimeClient example ##
set(dcptimeclient_SRCS dcptimeclient.cpp dcptimeclient_main.cpp)
qt4_automoc(${dcptimeclient_SRCS})
add_executable(dcptimeclient ${dcptimeclient_SRCS})
target_link_libraries(dcptimeclient DcpClient
    ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTGUI_LIBRARY})
