project(dcphub)

include_directories(
    ${QT_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(dcphub_SRCS dcphub.cpp dcphub_main.cpp dcppacket.cpp hexformatter.cpp)
qt4_automoc(${dcphub_SRCS})
qt4_wrap_cpp(dcphub_MOC_SRCS dcphub.h)
add_executable(dcphub ${dcphub_SRCS} ${dcphub_MOC_SRCS})
target_link_libraries(dcphub DcpClient
   ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})

install(TARGETS dcphub RUNTIME DESTINATION bin)
