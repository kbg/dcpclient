project(dcpsend)

include_directories(
    ${QT_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${libDcpClient_SOURCE_DIR}
)

set(dcpsend_SRCS dcpsend.cpp)
add_executable(dcpsend ${dcpsend_SRCS})
target_link_libraries(dcpsend DcpClient
   ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})

install(TARGETS dcpsend RUNTIME DESTINATION bin)
