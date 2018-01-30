project(dcplisten)

include_directories(${CMAKE_SOURCE_DIR}/src)

set(dcplisten_SRCS dcplisten.cpp)
add_executable(dcplisten ${dcplisten_SRCS})
target_link_libraries(dcplisten DcpClient Qt4::QtNetwork)

install(TARGETS dcplisten RUNTIME DESTINATION bin)
