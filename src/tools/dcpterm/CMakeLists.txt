project(dcpterm)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
)

set(dcpterm_SRCS
    dcpterm.cpp
    dcptermwin.cpp
    configdialog.cpp
    cmdlineoptions.cpp
)

# We cannot use CMAKE_AUTOUIC, because it is only available for CMake 3
qt4_wrap_ui(dcpterm_UI_HDRS dcptermwin.ui configdialog.ui)

add_executable(dcpterm ${dcpterm_SRCS} ${dcpterm_UI_HDRS})
target_link_libraries(dcpterm DcpClient Qt4::QtNetwork Qt4::QtGui)

install(TARGETS dcpterm RUNTIME DESTINATION bin)
