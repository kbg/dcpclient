project(dcpsend)

include_directories(${CMAKE_SOURCE_DIR}/src)

set(dcpsend_SRCS dcpsend.cpp)
add_executable(dcpsend ${dcpsend_SRCS})
target_link_libraries(dcpsend DcpClient)

install(TARGETS dcpsend RUNTIME DESTINATION bin)
