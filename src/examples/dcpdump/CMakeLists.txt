project(dcpdump)

include_directories(
    ${QT_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${libDcpClient_SOURCE_DIR}
)

set(dcpdump_SRCS dcpdump.cpp dcpdump_main.cpp)
qt4_automoc(${dcpdump_SRCS})
add_executable(dcpdump ${dcpdump_SRCS})
target_link_libraries(dcpdump DcpClient
    ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})

install(TARGETS dcpdump RUNTIME DESTINATION bin)
