project(dcpdump)

include_directories(${CMAKE_SOURCE_DIR}/src)

set(dcpdump_SRCS
    dcpdump.cpp
    dcpdump_main.cpp
)

add_executable(dcpdump ${dcpdump_SRCS})
target_link_libraries(dcpdump DcpClient Qt4::QtNetwork)

install(TARGETS dcpdump RUNTIME DESTINATION bin)
