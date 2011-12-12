project(dcplisten)

include_directories(
    ${QT_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(dcplisten_SRCS dcplisten.cpp)
add_executable(dcplisten ${dcplisten_SRCS})
target_link_libraries(dcplisten DcpClient
    ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})

install(TARGETS dcplisten RUNTIME DESTINATION bin)
