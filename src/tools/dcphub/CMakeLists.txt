project(dcphub)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
)

set(dcphub_SRCS
    dcphub.cpp
    dcphub_main.cpp
    dcppacket.cpp
    hexformatter.cpp
    cmdlineoptions.cpp
)

add_executable(dcphub ${dcphub_SRCS})
target_link_libraries(dcphub DcpClient Qt4::QtNetwork)

install(TARGETS dcphub RUNTIME DESTINATION bin)
