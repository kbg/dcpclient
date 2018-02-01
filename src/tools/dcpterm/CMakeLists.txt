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
if (TARGET Qt5::Core)
    qt5_wrap_ui(dcpterm_UI_HDRS dcptermwin.ui configdialog.ui)
else()
    qt4_wrap_ui(dcpterm_UI_HDRS dcptermwin.ui configdialog.ui)
endif()

add_executable(dcpterm ${dcpterm_SRCS} ${dcpterm_UI_HDRS})

if (TARGET Qt5::Core)
    target_link_libraries(dcpterm DcpClient Qt5::Widgets)
else()
    target_link_libraries(dcpterm DcpClient Qt4::QtGui)
endif()

install(TARGETS dcpterm RUNTIME DESTINATION bin)
